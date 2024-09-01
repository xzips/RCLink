#include "RadioCommunication.hpp"
#include <iostream>
#include <boost/asio.hpp>
#include <chrono>
#include <thread>
#include <string>
#include "StateSync.hpp"


using namespace boost::asio;
using namespace std;

// Global queues and their mutexes
//vector<Message> send_queue;
//vector<Message> receive_queue;
//vector<Message> display_recv_queue;//since these are processed instantly, we need a separate vector to display them in gui

std::mutex controller_mutex;
std::mutex telemetry_mutex;
std::mutex last_incoming_message_mutex;
std::mutex last_outgoing_message_mutex;



//extern const int MAX_SEND_QUEUE_SIZE = 16;

std::atomic<bool> cleanupFlag(false);

// Serial port global
io_service io;
serial_port* serial_ = nullptr;


unsigned long last_success_packet_millis;
unsigned long packet_count;
unsigned long failed_packet_count;

unsigned long last_packet_count;
unsigned long last_failed_packet_count;


float yaw_orientation = 0.0f;
float pitch_orientation = 0.0f;
float roll_orientation = 0.0f;

std::mutex connection_status_mutex;
ConnectionStatus connection_status = ConnectionStatus::DISCONNECTED;

uint64_t controller_start_time = get_timestamp_ms();


std::string ThrottleController::GetCommandSTR()
{
    //if calibration state is 0, we need to instead send SET_PWM_XX_XXXX_XXXX for CH 04, 0000, 0000
	if (calibration_state == 0)
	{
		std::string channelStr = std::to_string(throttle_channel);
	
	    return "SET_PWM_" + channelStr + "_0000_0000";
	}
    
    
    //SET_THROTTLE_XXXX

    int mapped_throttle = (int)((cur_throttle) * (pwm_full_duty - pwm_neutral_duty) + pwm_neutral_duty);

    

	std::string throttleStr = std::to_string((int)(mapped_throttle));
	while (throttleStr.length() < 4)
	{
		throttleStr = "0" + throttleStr;
	}

	return "SET_THROTTLE_" + throttleStr;
    


}


uint16_t ThrottleController::GetMappedThrottle()
{
	return (uint16_t)((cur_throttle) * (pwm_full_duty - pwm_neutral_duty) + pwm_neutral_duty);
}

Message last_incoming_message;
Message last_outgoing_message;




std::string generate_6char_timestamp(unsigned long timestamp_millis)
{

    timestamp_millis = timestamp_millis % 1000000;
    
	std::string str = std::to_string(timestamp_millis);
	while (str.length() < 6)
	{
		str = "0" + str;
	}

	return str;
}

uint64_t get_timestamp_ms()
{
	return (uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
   
}

bool is_error(std::string msg_str)
{
    //if strncmp first 6 chars matches ERROR: then return true, else false
	return msg_str.substr(0, 6) == "ERROR:";
    
    
}


void reset_serial() {
    if (serial_) {
        serial_->close();
        delete serial_;
        serial_ = nullptr;
    }
}

bool initialize_serial() {
    try {

        //COM7 for desktop, COM5 for laptop

        serial_ = new serial_port(io, "COM6");  // Adjust the port name depending on your OS and port number
        serial_->set_option(serial_port_base::baud_rate(1000000));
        serial_->set_option(serial_port_base::character_size(8));
        serial_->set_option(serial_port_base::stop_bits(serial_port_base::stop_bits::one));
        serial_->set_option(serial_port_base::parity(serial_port_base::parity::none));
        serial_->set_option(serial_port_base::flow_control(serial_port_base::flow_control::none));

		//lock the connection status mutex
		lock_guard<mutex> lock(connection_status_mutex);
		connection_status = ConnectionStatus::SERIAL_OK_NO_REMOTE;


        return true;
    }
    catch (std::exception& e) {
        cerr << "Error initializing serial port: " << e.what() << endl;
        
        //lock the connection status mutex
		lock_guard<mutex> lock(connection_status_mutex);
        
		connection_status = ConnectionStatus::DISCONNECTED;

        reset_serial();
        return false;
    }
}



void serial_thread() {
    string accumulated_data;
    boost::system::error_code ec;

    char transcode_buf[32];
    
    while (!cleanupFlag.load()) {
        if (serial_ && serial_->is_open()) {
            try {
                char buf[512];
                size_t len = serial_->read_some(buffer(buf), ec);

                if (!ec && len > 0) {
                    accumulated_data.append(buf, len);

                    // Check if \r\n is present in the accumulated data
                    size_t pos;
                    while ((pos = accumulated_data.find("\r\n")) != string::npos) {
                        string message = accumulated_data.substr(0, pos + 2); // Include \r\n in the message
                        accumulated_data.erase(0, pos + 2); // Remove processed message from buffer

     
						//strip any \n from the message
						message.erase(std::remove(message.begin(), message.end(), '\n'), message.end());

						//strip and remove any \r from the message
						message.erase(std::remove(message.begin(), message.end(), '\r'), message.end());
                       
                        

						std::cout << "Received: " << message << std::endl;


                        
                        int strdiff1 = strncmp(message.c_str(), "ERROR: Initiate Con. Failed", 33);
                        int strdiff2 = strncmp(message.c_str(), "ERROR: Transmission Failed", 26);

                        if (strdiff1 == 0 or strdiff2 == 0) {
                            lock_guard<mutex> lock2(connection_status_mutex);
                            connection_status = ConnectionStatus::SERIAL_OK_NO_REMOTE;
                            continue;
                        }

						TelemetryState recvTelem = TelemetryState();
                             

						//bool telemetryDecodedSuccess = decode_TelemetryState(message.c_str(), &telemetryState);
						bool telemetryDecodedSuccess = decode(message, recvTelem);

                    
                        {
                            lock_guard<mutex> lock3(last_incoming_message_mutex);
                        
                            last_incoming_message = message;

                            if (!telemetryDecodedSuccess)
                            {
							    last_incoming_message.msg += std::string(" (Decoding Error)");
								std::cout << "Decoding Error: " << message << std::endl;
                                continue;
                            }

                            else
                            {
                                if (recvTelem.NetworkID == 0xa9)
                                {
								    telemetryState = recvTelem;

                                }

                                else
                                {
									std::cout << "Received telemetry from unknown network ID: " << telemetryState.NetworkID << std::endl;

                                }
                            }
                            

                        }



                        
                    }
                }
                else if (ec) {
                    cerr << "Read error: " << ec.message() << endl;
                    reset_serial();
                    
                    //io.stop();
                   // return;

                    this_thread::sleep_for(std::chrono::milliseconds(200));
                    continue;
                }

                string message;
                
                {
                    lock_guard<mutex> lock(controller_mutex);
               
					//encode_ControllerState(&controllerState, transcode_buf);
					message = encode(controllerState) + "\n";

                }

				//message = std::string(transcode_buf) + "\n";

                write(*serial_, buffer(message.data(), message.size()), ec);

				//std::cout << "Sent: " << message << std::endl;
                
                
                if (ec) {
                    cerr << "Write error: " << ec.message() << endl;
                    reset_serial();
                    //io.stop();
                    //return;
                   this_thread::sleep_for(std::chrono::milliseconds(200));
                    continue;
                }
            }
            catch (std::exception& e) {
                cerr << "Error in serial_thread: " << e.what() << endl;
                reset_serial();
                return;
            }
        }
        else {
            if (initialize_serial()) {
                cout << "Serial port reconnected" << endl;
                //wait 10ms
				this_thread::sleep_for(std::chrono::milliseconds(20));
            }
            else {
                cout << "Waiting to reconnect..." << endl;
                this_thread::sleep_for(std::chrono::milliseconds(200));
            }
        }
    }

    reset_serial();
    io.stop();
    return;
}









void UpdateMainThreadTelemetryVariables()
{

    

    {
		//lock last_incoming_message_mutex
		lock_guard<mutex> lock(last_incoming_message_mutex);
        
        if (last_incoming_message.msg.find("CONNECTION_ACK") == 0)
        {

            //set connection status to connected
            lock_guard<mutex> lock2(connection_status_mutex);
            connection_status = ConnectionStatus::CONNECTED;
            return;
        }
    }

    
	

    

    {
        //lock telemetry mutex
		lock_guard<mutex> lock(telemetry_mutex);

        //update the yaw pitch roll values
        yaw_orientation = telemetryState.Yaw / 100.f;
		pitch_orientation = telemetryState.Pitch /100.f;
		roll_orientation = telemetryState.Roll/ 100.f;

    }

        

    //swap pitch and roll!
	double temp = pitch_orientation;
	pitch_orientation = roll_orientation;
	roll_orientation = temp;

    //negate pitch
	//pitch_orientation = pitch_orientation;
        
    //negate roll
	roll_orientation = roll_orientation;

	return;
	

	
   
}
