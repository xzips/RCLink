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

extern const int MAX_SEND_QUEUE_SIZE = 128;

std::atomic<bool> cleanupFlag(false);

// Serial port global
io_service io;
serial_port* serial_ = nullptr;


unsigned long last_success_packet_millis;
unsigned long packet_count;
unsigned long failed_packet_count;


std::mutex connection_status_mutex;
ConnectionStatus connection_status = ConnectionStatus::DISCONNECTED;



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
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
   
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
        serial_ = new serial_port(io, "COM7");  // Adjust the port name depending on your OS and port number
        serial_->set_option(serial_port_base::baud_rate(100000));
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
    while (!cleanupFlag.load()) {
        if (serial_ && serial_->is_open()) {
            try {
                char buf[512];
                boost::system::error_code ec;
                size_t len = serial_->read_some(buffer(buf), ec);
                if (!ec && len > 0) {
                    lock_guard<mutex> lock(receive_mutex);
                    receive_queue.push_back(Message(string(buf, len), get_timestamp_ms()));
                    
					//if we recieved the message "Initiate Connection Failed\n" we should back off and NOT send any messages (use strcmp to check the first characters up to end of "Initiate Connection Failed"

                    //msg = "Initiate Connection Failed\r\n"
                    int strdiff = strncmp(receive_queue.back().msg.c_str(), "Initiate Connection Failed", 25);


					if (strdiff == 0) {
                        continue;

                    }

                                        
                }
                else if (ec) {
                    cerr << "Read error: " << ec.message() << endl;
                    reset_serial();
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
                        message = send_queue.back().msg + "\n";
                        send_queue.pop_back();
                    }
                }
                write(*serial_, buffer(message.data(), message.size()), ec);
                if (ec) {
                    cerr << "Write error: " << ec.message() << endl;
                    reset_serial();
                    this_thread::sleep_for(std::chrono::milliseconds(200));
                    continue;
                    

                    
                }
            }
            catch (std::exception& e) {
                cerr << "Error in serial_thread: " << e.what() << endl;
                reset_serial();
            }
        }
        else {
            // Serial not connected, try to reconnect
            if (initialize_serial()) {
                cout << "Serial port reconnected" << endl;
            }
            else {
                cout << "Waiting to reconnect..." << endl;
                this_thread::sleep_for(std::chrono::milliseconds(200));  // Wait before retrying to avoid busy-waiting
            }
        }
    }

    //wait 30ms for anything else to be sent
	this_thread::sleep_for(std::chrono::milliseconds(20));
    

    //perform cleanup of serial
    reset_serial();


    io.stop();



    
    
    return;



}

void push_msg(const string& msg) {
    lock_guard<mutex> lock(send_mutex);
    send_queue.push_back(Message(msg, get_timestamp_ms()));

	//if there are more than 100 messages in the queue we should remove the oldest message
	if (send_queue.size() > MAX_SEND_QUEUE_SIZE)
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
