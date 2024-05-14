#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <mutex>
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
serial_port serial_(io, "COM7"); // Adjust the port name depending on your OS and port number

// Function declarations
void serial_thread();
void push_msg(const string& msg);
string pop_msg();

int main() {
    // Start the serial communication in a background thread
    thread background(serial_thread);

    int n = 0;
    while (true)
    {
        // Example usage of push_msg and pop_msg
		push_msg(std::string("n = ") + to_string(n));
        n++;
        this_thread::sleep_for(std::chrono::milliseconds(1000)); // Simulate work
        
        int n_msgs = 0;
		//pop from recv queue until empty
		while (true) {
			string msg = pop_msg();
			
            if (msg == "Queue is empty") {
				break;
			}
            
			n_msgs += 1;

			cout << "Received message: " << msg << endl;
		}

		cout << "Number of messages received (in 1s): " << n_msgs << endl;
        
    }
    
    

    background.join();
    return 0;
}

void serial_thread() {
    serial_.set_option(serial_port_base::baud_rate(500000));
    serial_.set_option(serial_port_base::character_size(8));
    serial_.set_option(serial_port_base::stop_bits(serial_port_base::stop_bits::one));
    serial_.set_option(serial_port_base::parity(serial_port_base::parity::none));
    serial_.set_option(serial_port_base::flow_control(serial_port_base::flow_control::none));

    try {
        while (true) {
            // Buffer to store incoming data
            char buf[512];
            boost::system::error_code ec;

            // Read data from serial port
            size_t len = serial_.read_some(buffer(buf), ec);
            if (!ec && len > 0) {
                lock_guard<mutex> lock(receive_mutex);
                receive_queue.push_back(string(buf, len));
                //cout << "Received data: " << string(buf, len) << endl;
            }
            else if (ec) {
                cerr << "Read error: " << ec.message() << endl;
                continue;
            }

            // Check send_queue and send message or "QUEUE_EMPTY"
            string message;
            {
                lock_guard<mutex> lock(send_mutex);
                if (send_queue.empty()) {
                    message = "QUEUE_EMPTY\n";
                }
                else {
                    message = send_queue.back() + "\n"; // Change from front to back if required
                    send_queue.pop_back(); // Adjust to pop_back for LIFO
                }
            }

            // Send the message
            write(serial_, buffer(message.data(), message.size()), ec);
            if (ec) {
                cerr << "Write error: " << ec.message() << endl;
            }
            else {
                //cout << "Sent message: " << message;
            }
        }
    }
    catch (std::exception& e) {
        cerr << "Error in serial_thread: " << e.what() << endl;
    }
}

void push_msg(const string& msg) {
    lock_guard<mutex> lock(send_mutex);
    send_queue.push_back(msg); // Push to the back of the queue
}

string pop_msg() {
    lock_guard<mutex> lock(receive_mutex);
    if (!receive_queue.empty()) {
        string msg = receive_queue.front();
        receive_queue.erase(receive_queue.begin());
        return msg;
    }
    return "Queue is empty";
}
