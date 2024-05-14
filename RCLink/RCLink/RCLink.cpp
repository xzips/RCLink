#include <boost/asio.hpp>
#include <iostream>
#include <string>

using namespace boost::asio;

int main() {
    io_service io;
    serial_port serial_(io, "COM7");  // Adjust the port name depending on your OS and port number

    // Setting serial port options
    serial_.set_option(serial_port_base::baud_rate(9600));
    serial_.set_option(serial_port_base::character_size(8));
    serial_.set_option(serial_port_base::stop_bits(serial_port_base::stop_bits::one));
    serial_.set_option(serial_port_base::parity(serial_port_base::parity::none));
    serial_.set_option(serial_port_base::flow_control(serial_port_base::flow_control::none));


    int n = 0;

    try {
        while (true) {
            // Buffer to store incoming data
            char buf[512];
            boost::system::error_code ec;

            // Read data from serial port
            size_t len = serial_.read_some(buffer(buf), ec);

            if (!ec) {
                // Print received data to console
                std::cout.write(buf, len);
                std::cout << std::endl;

                // Send response message
                std::string msg = std::string("TEST_MSG ") + std::to_string(n) + std::string("\n");
                n++;
                write(serial_, buffer(msg.data(), msg.size()), ec);

                // Check for any error in writing
                if (ec) {
                    throw boost::system::system_error(ec);
                }
            }
            else {
                throw boost::system::system_error(ec);
            }
        }
    }
    catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
