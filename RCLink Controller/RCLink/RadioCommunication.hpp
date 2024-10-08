#pragma once

#include <boost/asio.hpp>
#include <string>
#include <vector>
#include <mutex>
#include <string>
#include "SFML/Graphics.hpp"
#include <string>

//extern const int MAX_SEND_QUEUE_SIZE;


// Function declarations
void serial_thread();

void UpdateMainThreadTelemetryVariables();

bool is_error(std::string msg_str);



std::string generate_6char_timestamp(unsigned long timestamp_millis);
uint64_t get_timestamp_ms();

struct Message
{
	std::string msg;
	unsigned long timestamp;
	Message(std::string msg = "") : msg(msg)
	{
		timestamp = get_timestamp_ms();
	}

};


extern Message last_incoming_message;
extern Message last_outgoing_message;




extern std::atomic<bool> cleanupFlag;

//extern std::mutex send_mutex;
//extern std::mutex receive_mutex;

extern std::mutex controller_mutex;
extern std::mutex telemetry_mutex;
extern std::mutex last_incoming_message_mutex;
extern std::mutex last_outgoing_message_mutex;

extern float yaw_orientation;
extern float pitch_orientation;
extern float roll_orientation;


extern boost::asio::io_service io;
extern boost::asio::serial_port* serial_;

enum class ConnectionStatus
{
	CONNECTED,
	DISCONNECTED,
	SERIAL_OK_NO_REMOTE
};

extern uint64_t controller_start_time;

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
	bool inverted;

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
		bool symmetric_response,
		bool inverted)
		
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
		symmetric_response(symmetric_response),
		inverted(inverted)
			
	{
		curAngle = neutral_angle;
	}
	

	float GetPhysicalAngle()
	{

		int flippedAngle = (int)curAngle;

		if (inverted)
		{
			flippedAngle = max_angle_cal + (min_angle_cal - flippedAngle);

		}

		return flippedAngle;

	

	}

	std::string GetCommandSTR();

	std::string GetCompactCommandSTR();

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
	
	uint16_t GetMappedThrottle();

};