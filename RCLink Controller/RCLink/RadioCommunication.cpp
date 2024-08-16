#include "RadioCommunication.hpp"
#include <iostream>
#include <boost/asio.hpp>
#include <chrono>
#include <thread>
#include <string>

using namespace boost::asio;
using namespace std;

// Global queues and their mutexes
vector<Message> send_queue;
vector<Message> receive_queue;
vector<Message> display_recv_queue;//since these are processed instantly, we need a separate vector to display them in gui
mutex send_mutex;
mutex receive_mutex;



extern const int MAX_SEND_QUEUE_SIZE = 16;

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


std::string ServoController::GetCommandSTR()
{
    //SET_SERVO_07_090 for example format

	std::string servoNumStr = std::to_string(servoNum);
    //pad
	while (servoNumStr.length() < 2)
	{
		servoNumStr = "0" + servoNumStr;
	}

	std::string servoPosStr = std::to_string((int)curAngle);
	//pad

	while (servoPosStr.length() < 3)
	{
		servoPosStr = "0" + servoPosStr;
	}

	return "SET_SERVO_" + servoNumStr + "_" + servoPosStr;
    
}


std::string ServoController::GetCompactCommandSTR()
{
    //should return NNXXX where NN is the servo number padded with
	//a leading zero if necessary, and XXX is the angle padded with leading zeros if necessary
	std::string servoNumStr = std::to_string(servoNum);
	//pad
	while (servoNumStr.length() < 2)
	{
		servoNumStr = "0" + servoNumStr;
	}
    
	std::string servoPosStr = std::to_string((int)curAngle);
    
	//pad
	while (servoPosStr.length() < 3)
	{
		servoPosStr = "0" + servoPosStr;
	}

	return servoNumStr + servoPosStr;

}


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

unsigned long get_timestamp_ms()
{
	return (unsigned long)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
   
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

        serial_ = new serial_port(io, "COM5");  // Adjust the port name depending on your OS and port number
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

                        // Process the message
                        int strdiffz = strncmp(message.c_str(), "RECV_BUF_EMPTY", 14);

                        if (strdiffz != 0) {
                            lock_guard<mutex> lock(receive_mutex);
                            receive_queue.push_back(Message(message, get_timestamp_ms()));
                        }

						//check if recieve queue is empty, dont comp just set to empty

						if (receive_queue.empty()) {
							lock_guard<mutex> lock(connection_status_mutex);
							connection_status = ConnectionStatus::SERIAL_OK_NO_REMOTE;
							continue;
						}

                        int strdiff1 = strncmp(receive_queue.back().msg.c_str(), "ERROR: Initiate Connection Failed", 33);
                        int strdiff2 = strncmp(receive_queue.back().msg.c_str(), "ERROR: Transmission Failed", 26);

                        if (strdiff1 == 0 or strdiff2 == 0) {
                            lock_guard<mutex> lock2(connection_status_mutex);
                            connection_status = ConnectionStatus::SERIAL_OK_NO_REMOTE;
                            continue;
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
                    lock_guard<mutex> lock(send_mutex);
                    if (send_queue.empty()) {
                        message = "QUEUE_EMPTY\n";
                    }
                    else {
                        message = send_queue.front().msg + "\n";
                        send_queue.erase(send_queue.begin());
                    }
                }

                write(*serial_, buffer(message.data(), message.size()), ec);
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
				this_thread::sleep_for(std::chrono::milliseconds(10));
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





void push_msg(const string& msg) {
    lock_guard<mutex> lock(send_mutex);
    send_queue.push_back(Message(msg, get_timestamp_ms()));

	//if there are more than 100 messages in the queue we should remove the oldest message
	while (send_queue.size() > MAX_SEND_QUEUE_SIZE)
	{
		send_queue.erase(send_queue.begin());
	}

}

string pop_msg() {
    lock_guard<mutex> lock(receive_mutex);
    if (!receive_queue.empty()) {
        string msg = receive_queue.front().msg;
		display_recv_queue.insert(display_recv_queue.begin(), receive_queue.front());
        receive_queue.erase(receive_queue.begin());

           

        return msg;
    }
    return serial_ && serial_->is_open() ? "Queue is empty" : "Serial not connected";
}

void cleanup_queues() {
    lock_guard<mutex> lock1(send_mutex);
    lock_guard<mutex> lock2(receive_mutex);
    send_queue.clear();
    receive_queue.clear();
}






void HandleIncomingMessage(std::string msg)
{
    //just in case, check if msg begins with ERROR:, if so return
	if (msg.find("ERROR:") == 0)
	{
		return;
	}
    

	if (msg.find("CONNECTION_ACK") == 0)
	{
		//clear send buffer, lock mutex
		lock_guard<mutex> lock(send_mutex);
		send_queue.clear();
		//set connection status to connected
		lock_guard<mutex> lock2(connection_status_mutex);
		connection_status = ConnectionStatus::CONNECTED;
		return;
	}
    
	//if message is a yaw pitch rotation update, it will match: YPR_XXXXX_XXXXX_XXXXX where each XXXXX is an int value possibly including a negative sign as the first character
	if (msg.find("YPR_") == 0)
	{
		//parse the message
		std::string ypr = msg.substr(4);
		std::string yaw = ypr.substr(0, ypr.find_first_of('_'));
		ypr = ypr.substr(ypr.find_first_of('_') + 1);
		std::string pitch = ypr.substr(0, ypr.find_first_of('_'));
		ypr = ypr.substr(ypr.find_first_of('_') + 1);
		std::string roll = ypr;

		//convert the strings to doubles
		double yaw_d = std::stod(yaw)/100.f;
		double pitch_d = std::stod(pitch)/100.f;
		double roll_d = std::stod(roll)/100.f;
        

		//update the yaw pitch roll values
		yaw_orientation = yaw_d;
		pitch_orientation = pitch_d;
		roll_orientation = roll_d;


        /*corrections for the sensor placement*/

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

	

}