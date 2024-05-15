#pragma once

#include <boost/asio.hpp>
#include <string>
#include <vector>
#include <mutex>


// Function declarations
void serial_thread();
void push_msg(const std::string& msg);
std::string pop_msg();

// Extern declarations for global variables
extern std::vector<std::string> send_queue;
extern std::vector<std::string> receive_queue;
extern std::mutex send_mutex;
extern std::mutex receive_mutex;
extern boost::asio::io_service io;
extern boost::asio::serial_port* serial_;
