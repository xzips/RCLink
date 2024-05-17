#include "RadioCommunication.hpp"
#include <iostream>
#include <boost/asio.hpp>
#include <chrono>
#include <thread>

using namespace boost::asio;
using namespace std;

// Global queues and their mutexes
vector<string> send_queue;
vector<string> receive_queue;
mutex send_mutex;
mutex receive_mutex;

// Serial port global
io_service io;
serial_port* serial_ = nullptr;


unsigned long last_success_packet_millis;
unsigned long packet_count;
unsigned long failed_packet_count;
ConnectionStatus connection_status = ConnectionStatus::DISCONNECTED;


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
        return true;
    }
    catch (std::exception& e) {
        cerr << "Error initializing serial port: " << e.what() << endl;
        reset_serial();
        return false;
    }
}

void serial_thread() {
    while (true) {
        if (serial_ && serial_->is_open()) {
            try {
                char buf[512];
                boost::system::error_code ec;
                size_t len = serial_->read_some(buffer(buf), ec);
                if (!ec && len > 0) {
                    lock_guard<mutex> lock(receive_mutex);
                    receive_queue.push_back(string(buf, len));
                }
                else if (ec) {
                    cerr << "Read error: " << ec.message() << endl;
                    reset_serial();
                }

                string message;
                {
                    lock_guard<mutex> lock(send_mutex);
                    if (send_queue.empty()) {
                        message = "QUEUE_EMPTY\n";
                    }
                    else {
                        message = send_queue.back() + "\n";
                        send_queue.pop_back();
                    }
                }
                write(*serial_, buffer(message.data(), message.size()), ec);
                if (ec) {
                    cerr << "Write error: " << ec.message() << endl;
                    reset_serial();
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
                this_thread::sleep_for(std::chrono::seconds(1));  // Wait before retrying to avoid busy-waiting
            }
        }
    }
}

void push_msg(const string& msg) {
    lock_guard<mutex> lock(send_mutex);
    send_queue.push_back(msg);
}

string pop_msg() {
    lock_guard<mutex> lock(receive_mutex);
    if (!receive_queue.empty()) {
        string msg = receive_queue.front();
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
