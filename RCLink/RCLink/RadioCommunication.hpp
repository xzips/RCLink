#pragma once

#include <boost/asio.hpp>
#include <string>
#include <vector>
#include <mutex>
#include <string>
#include "SFML/Graphics.hpp"

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

enum class ConnectionStatus
{
	CONNECTED,
	DISCONNECTED,
	SERIAL_OK_NO_REMOTE
};

//timestamp of last successful packet in milliseconds
extern unsigned long last_success_packet_millis;
extern unsigned long packet_count;
extern unsigned long failed_packet_count;
extern ConnectionStatus connection_status;

struct ServoController
{
	float min_angle_cal;
	float neutral_angle;
	float max_angle_cal;
	int servoNum;
	float curAngle;
	sf::Keyboard::Key increase_key;
	sf::Keyboard::Key decrease_key;
	std::string servo_name;
	float angle_per_frame_pressed;
	float angle_per_frame_released;

	ServoController(
		int servoNum,
		float min_angle_cal,
		float max_angle_cal,
		float neutral_angle,
		sf::Keyboard::Key increase_key,
		sf::Keyboard::Key decrease_key,
		float angle_per_frame_pressed,
		float angle_per_frame_released,
		std::string servo_name)
		
		:
		
		servoNum(servoNum),
		min_angle_cal(min_angle_cal),
		max_angle_cal(max_angle_cal),
		neutral_angle(neutral_angle),
		increase_key(increase_key),
		decrease_key(decrease_key),
		angle_per_frame_pressed(angle_per_frame_pressed),
		angle_per_frame_released(angle_per_frame_released), 
		servo_name(servo_name)
	{
		curAngle = neutral_angle;
	}


};