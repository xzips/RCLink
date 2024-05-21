#pragma once

#include <boost/asio.hpp>
#include <string>
#include <vector>
#include <mutex>
#include <string>
#include "SFML/Graphics.hpp"
#include <string>

extern const int MAX_SEND_QUEUE_SIZE;


// Function declarations
void serial_thread();
void push_msg(const std::string& msg);
std::string pop_msg();




bool is_error(std::string msg_str);



std::string generate_6char_timestamp(unsigned long timestamp_millis);
unsigned long get_timestamp_ms();

struct Message
{
	std::string msg;
	unsigned long timestamp;
	Message(std::string msg, unsigned long timestamp) : msg(msg), timestamp(timestamp) {}

};

// Extern declarations for global variables
extern std::vector<Message> send_queue;
extern std::vector<Message> receive_queue;
extern std::vector<Message> display_recv_queue;

extern std::atomic<bool> cleanupFlag;

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

extern unsigned long last_packet_count;
extern unsigned long last_failed_packet_count;

extern std::mutex connection_status_mutex;
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
	bool symmetric_response;

	ServoController(
		int servoNum,
		float min_angle_cal,
		float max_angle_cal,
		float neutral_angle,
		sf::Keyboard::Key increase_key,
		sf::Keyboard::Key decrease_key,
		float angle_per_frame_pressed,
		float angle_per_frame_released,
		std::string servo_name,
		bool symmetric_response)
		
		:
		
		servoNum(servoNum),
		min_angle_cal(min_angle_cal),
		max_angle_cal(max_angle_cal),
		neutral_angle(neutral_angle),
		increase_key(increase_key),
		decrease_key(decrease_key),
		angle_per_frame_pressed(angle_per_frame_pressed),
		angle_per_frame_released(angle_per_frame_released), 
		servo_name(servo_name),
		symmetric_response(symmetric_response)
			
	{
		curAngle = neutral_angle;
	}
	
	std::string GetCommandSTR();


};

struct ThrottleController
{
	float cur_throttle;
	int throttle_channel;
	sf::Keyboard::Key increase_key;
	sf::Keyboard::Key decrease_key;
	float throttle_per_frame_pressed;
	int pwm_neutral_duty;
	int pwm_full_duty;

	int calibration_state;
	
	ThrottleController(
		int throttle_channel,
		sf::Keyboard::Key increase_key,
		sf::Keyboard::Key decrease_key,
		float throttle_per_frame_pressed,
		int pwm_neutral_duty,
		int pwm_full_duty)
		:
		throttle_channel(throttle_channel),
		increase_key(increase_key),
		decrease_key(decrease_key),
		throttle_per_frame_pressed(throttle_per_frame_pressed),
		pwm_neutral_duty(pwm_neutral_duty),
		pwm_full_duty(pwm_full_duty)
	{
		cur_throttle = 0;

		//-1 indicates not calibrating right now, 0 means begin (should be setting pwm to 0), 1 means neutral, 2 means full, 3 means neutral again, then should be set back to -1
		calibration_state = -1;
	}

	std::string GetCommandSTR();
	

};