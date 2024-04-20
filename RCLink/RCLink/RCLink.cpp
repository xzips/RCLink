#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <string>

using namespace boost::asio;

class SerialPort {
public:
    SerialPort(io_service& io, const std::string& port_name)
        : io_(io),
        serial_(io, port_name),
        delim_('\n') { // Delimiter
        serial_.set_option(serial_port_base::baud_rate(9600));

        serial_.set_option(boost::asio::serial_port_base::character_size(8));
        serial_.set_option(boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::one));

        //parity
        serial_.set_option(boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::none));

        //flow control
        serial_.set_option(boost::asio::serial_port_base::flow_control(boost::asio::serial_port_base::flow_control::none));
    }

    void start_read() {
        serial_.async_read_some(
            buffer(read_msg_, 512),
            boost::bind(&SerialPort::read_complete,
                this,
                placeholders::error,
                placeholders::bytes_transferred));
    }

    void read_complete(const boost::system::error_code& error, size_t bytes_transferred) {
        if (!error) {
            std::copy(read_msg_, read_msg_ + bytes_transferred, std::back_inserter(data_buffer_));
            // Check for newline
            std::size_t pos = data_buffer_.find(delim_);
            while (pos != std::string::npos) {
                std::string line = data_buffer_.substr(0, pos);
                std::cout << "Received: " << line << std::endl;
                data_buffer_ = data_buffer_.substr(pos + 1);
                pos = data_buffer_.find(delim_);
            }

            //send back "ACK-RX"
			std::string ack = "ACK-RX\n";
			serial_.write_some(buffer(ack.c_str(), ack.size()));

            start_read();  // Start listening again
        }
        else {
            std::cerr << "Error on receive: " << error.message() << std::endl;
        }
    }

private:
    io_service& io_;
    serial_port serial_;
    std::string data_buffer_;
    char read_msg_[512];
    char delim_;
};

int main() {
    try {
        io_service io;
        SerialPort port(io, "COM7");  // Adjust the COM port as needed

        port.start_read();
        io.run();  // This will block. You might want to run this in a separate thread if you need non-blocking behavior in the main program.
    }
    catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    return 0;
}
