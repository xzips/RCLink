
#include "RadioCommunication.hpp"
#include <SFML/Graphics.hpp>
#include <iostream>
#include "RCGraphics.hpp"
#include "Quick3D.hpp"
#include <cmath>
#include <vector>
#include <iomanip>
#include <sstream>
#include <string>

std::vector<ServoController> servoControllerVector;
ThrottleController throttleController(4, sf::Keyboard::LShift, sf::Keyboard::LControl, 0.05f, 1500, 2000);
sf::Font font;
unsigned long frameCounter = 0;
long int escCalTimerMS;

//std::vector<q3d::SVF> models;
std::vector<q3d::TM> models;

std::vector<sf::Texture*> textures;

bool escCalibrateButtonPressed = false;


//create rendertexture of the size of the window
sf::RenderTexture render_texture;




ServoController* GetServoControllerByName(std::string name)
{
	for (auto& servo : servoControllerVector)
	{
		if (servo.servo_name == name)
		{
			return &servo;
		}
	}
	return nullptr;

}


void LoadFont()
{
    // C:\Users\aspen\Desktop\SFMLFonts\Helvetica.ttf
	if (!font.loadFromFile("C:\\Users\\aspen\\Desktop\\SFMLFonts\\Helvetica.ttf"))
	{
		std::cout << "Error loading font" << std::endl;
        exit(1);
	}


}

void DrawServoControllers(std::vector<ServoController>& servoControllers, sf::RenderWindow& window)
{

    float box_width = 100;
    float box_height = 150;
    float box_spacing = 75;

	float box_outline_thickness = 3;

    float box_y_pos_base = window.getSize().y - box_height - box_spacing; // Adjusted to have a margin from the bottom of the window


    float first_box_x = (window.getSize().x - (box_width * servoControllers.size() + box_spacing * (servoControllers.size() - 1))) / 2;

    for (auto& servo : servoControllers)
    {
        sf::RectangleShape box(sf::Vector2f(box_width, box_height));
        box.setPosition(first_box_x, box_y_pos_base);
        box.setOutlineThickness(3);
        box.setOutlineColor(sf::Color::White);
        box.setFillColor(sf::Color::Transparent);

        window.draw(box);

        float normalizedAngle = (servo.curAngle - servo.min_angle_cal) / (servo.max_angle_cal - servo.min_angle_cal);
		float line_y_pos = box_y_pos_base + box_height - (normalizedAngle * (box_height - box_outline_thickness * 2)) - box_outline_thickness * 2;

        sf::RectangleShape line(sf::Vector2f(box_width, 5));
        line.setPosition(first_box_x, line_y_pos);
        line.setFillColor(sf::Color(225, 148, 148)); // Light red color

        window.draw(line);
        
		sf::Text text;
		text.setFont(font);
		text.setString(servo.servo_name);
		text.setCharacterSize(16);
		text.setFillColor(sf::Color::White);

		//get text bounds width to center text above box
		sf::FloatRect textRect = text.getLocalBounds();
		text.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
		text.setPosition(first_box_x + box_width / 2, box_y_pos_base - 20);
        
        

		window.draw(text);

        int txt_offset_x = -20;

        //reuse same text, now put at the top left side text for the max angle number, and bottom for minimum angle number, and then at the same height as the line put current angle number
		text.setString(std::to_string((int)servo.max_angle_cal));

        //get local bounds
		textRect = text.getLocalBounds();
		text.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
        
		text.setPosition(first_box_x + txt_offset_x, box_y_pos_base + 10);
		window.draw(text);

        
        
		text.setString(std::to_string((int)servo.min_angle_cal));
		textRect = text.getLocalBounds();
		text.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
       
		text.setPosition(first_box_x + txt_offset_x, box_y_pos_base + box_height - 20);
		window.draw(text);

        first_box_x += box_width + box_spacing;
    }
}

void UpdateDrawThrottleController(sf::RenderWindow& window)
{
	//centered vertically on left side, draw a single box with throttle value
	float box_width = 100;
	float box_height = 150;

	float box_outline_thickness = 3;
	
	float box_x_pos = 50;
	float box_y_pos = (window.getSize().y - box_height) / 2;
	
	sf::RectangleShape box(sf::Vector2f(box_width, box_height));
	box.setPosition(box_x_pos, box_y_pos);
	box.setOutlineThickness(3);
	box.setOutlineColor(sf::Color::White);
	box.setFillColor(sf::Color::Transparent);
	
	window.draw(box);







	
	//already normalized 0 to 1
	float throt = throttleController.cur_throttle;
	
	

	float line_y_pos = box_y_pos + box_height - (throt * (box_height - box_outline_thickness * 2)) - box_outline_thickness * 2;
	
	sf::RectangleShape line(sf::Vector2f(box_width, 5));
	line.setPosition(box_x_pos, line_y_pos);
	line.setFillColor(sf::Color(148, 225, 148)); // Light green color
	
	window.draw(line);
	
	sf::Text text;
	text.setFont(font);
	text.setString("Throttle");
	text.setCharacterSize(16);
	text.setFillColor(sf::Color::White);
	
	//get text bounds width to center text above box
	sf::FloatRect textRect = text.getLocalBounds();
	text.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
	text.setPosition(box_x_pos + box_width / 2, box_y_pos - 20);
	
	window.draw(text);


	//draw a box above the text for the "Calibrate ESC" button
	sf::RectangleShape calibrate_button(sf::Vector2f(box_width, 30));
	calibrate_button.setPosition(box_x_pos, box_y_pos - 100);
	calibrate_button.setOutlineThickness(3);
	calibrate_button.setOutlineColor(sf::Color::White);
	//dark grey fill
	calibrate_button.setFillColor(sf::Color(16, 16, 16));
	
	if (throttleController.calibration_state != -1)
	{
		//set to red if calibrating
		calibrate_button.setFillColor(sf::Color(75, 40, 40));
	}

	//if mouse hovering over, change color to slightly lighter
	if (calibrate_button.getGlobalBounds().contains(sf::Mouse::getPosition(window).x, sf::Mouse::getPosition(window).y) && throttleController.calibration_state == -1)
	{
		calibrate_button.setFillColor(sf::Color(30, 30, 30));

		//if mouse clicked, change color to even lighter
		if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
		{
			calibrate_button.setFillColor(sf::Color(40, 40, 75));
		}
	}

	window.draw(calibrate_button);

	//text for calibrate button
	text.setString("Calibrate ESC");

	//if calibration state not -1, change text to "Calibrating..."
	if (throttleController.calibration_state != -1)
	{
		text.setString("Calibrating...");
	}


	text.setCharacterSize(12);
	text.setFillColor(sf::Color::White);
	textRect = text.getLocalBounds();
	text.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);

	
	
	//draw text centered in the box
	text.setPosition(box_x_pos + box_width / 2, box_y_pos - 100 + 15);
	window.draw(text);
	

	
	if (throttleController.calibration_state == -1)
	{
		if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
		{
			//if in bounds
			if (calibrate_button.getGlobalBounds().contains(sf::Mouse::getPosition(window).x, sf::Mouse::getPosition(window).y))
			{
				if (!escCalibrateButtonPressed)
				{
					escCalibrateButtonPressed = true;
					std::cout << "Calibrate ESC button pressed" << std::endl;

					throttleController.calibration_state = 0;

					//current time ms
					escCalTimerMS = get_timestamp_ms();

				}
			}
			else
			{
				escCalibrateButtonPressed = false;
			}
		}

		else
		{
			escCalibrateButtonPressed = false;
		}
	}

	//now handle calibration control, basically just incrementing the cal state
	if (throttleController.calibration_state != -1)
	{
		//in state 0 it's up to the get cmd string to send the pwm zeroing command

		//if 1 second has passed, set state to 1 which is neutral pos
		if (get_timestamp_ms() - escCalTimerMS > 1000)
		{
			throttleController.calibration_state = 1;
			throttleController.cur_throttle = 0;
		}

		//if 2 seconds have passed, set state to 2 which is full throttle
		if (get_timestamp_ms() - escCalTimerMS > 2000)
		{
			throttleController.calibration_state = 2;
			throttleController.cur_throttle = 1;
		}
		
		//if 3 seconds have passed, set state to 3 which is neutral pos
		if (get_timestamp_ms() - escCalTimerMS > 3000)
		{
			throttleController.calibration_state = 3;
			throttleController.cur_throttle = 0;
		}

		//once 4 seconds have passed, set state to -1 which is done
		if (get_timestamp_ms() - escCalTimerMS > 4000)
		{
			throttleController.calibration_state = -1;
		}

		
	}


	




	



	
	
	
	
	
	
	
	
	


}






void ProcessControlInputs()
{
	for (auto& servo : servoControllerVector)
	{
		if (!(sf::Keyboard::isKeyPressed(servo.increase_key) && sf::Keyboard::isKeyPressed(servo.decrease_key)))
		{
			if (servo.symmetric_response)
			{
				float wider_distance = std::max(servo.max_angle_cal - servo.neutral_angle, servo.neutral_angle - servo.min_angle_cal);
				float slimmer_distance = std::min(servo.max_angle_cal - servo.neutral_angle, servo.neutral_angle - servo.min_angle_cal);
				float wider_speed = servo.angle_per_frame_pressed;
				float slimmer_speed = wider_speed * (slimmer_distance / wider_distance);

				if (sf::Keyboard::isKeyPressed(servo.increase_key))
				{
					if (servo.curAngle >= servo.neutral_angle)
					{
						servo.curAngle += wider_speed;
					}
					else
					{
						servo.curAngle += slimmer_speed;
					}
					if (servo.curAngle > servo.max_angle_cal)
					{
						servo.curAngle = servo.max_angle_cal;
					}
				}
				else if (sf::Keyboard::isKeyPressed(servo.decrease_key))
				{
					if (servo.curAngle <= servo.neutral_angle)
					{
						servo.curAngle -= wider_speed;
					}
					else
					{
						servo.curAngle -= slimmer_speed;
					}
					if (servo.curAngle < servo.min_angle_cal)
					{
						servo.curAngle = servo.min_angle_cal;
					}
				}
				else
				{
					if (servo.curAngle > servo.neutral_angle)
					{
						if (servo.curAngle - servo.angle_per_frame_released > servo.neutral_angle)
						{
							servo.curAngle -= wider_speed;
						}
						else
						{
							servo.curAngle = servo.neutral_angle;
						}
					}
					else if (servo.curAngle < servo.neutral_angle)
					{
						if (servo.curAngle + servo.angle_per_frame_released < servo.neutral_angle)
						{
							servo.curAngle += wider_speed;
						}
						else
						{
							servo.curAngle = servo.neutral_angle;
						}
					}
				}
			}
			else
			{
				if (sf::Keyboard::isKeyPressed(servo.increase_key))
				{
					servo.curAngle += servo.angle_per_frame_pressed;
					if (servo.curAngle > servo.max_angle_cal)
					{
						servo.curAngle = servo.max_angle_cal;
					}
				}
				else if (sf::Keyboard::isKeyPressed(servo.decrease_key))
				{
					servo.curAngle -= servo.angle_per_frame_pressed;
					if (servo.curAngle < servo.min_angle_cal)
					{
						servo.curAngle = servo.min_angle_cal;
					}
				}
				else
				{
					if (servo.curAngle > servo.neutral_angle)
					{
						servo.curAngle -= servo.angle_per_frame_released;
						if (servo.curAngle < servo.neutral_angle)
						{
							servo.curAngle = servo.neutral_angle;
						}
					}
					else if (servo.curAngle < servo.neutral_angle)
					{
						servo.curAngle += servo.angle_per_frame_released;
						if (servo.curAngle > servo.neutral_angle)
						{
							servo.curAngle = servo.neutral_angle;
						}
					}
				}
			}
		}
	}

	
	bool throttleControlAllowed = true;

	if (throttleController.calibration_state != -1)
	{
		throttleControlAllowed = false;
	}


	

	if (!(sf::Keyboard::isKeyPressed(throttleController.increase_key) && sf::Keyboard::isKeyPressed(throttleController.decrease_key)))
	{
		//if increase key pressed, increase throttle
		if (sf::Keyboard::isKeyPressed(throttleController.increase_key))
		{
			throttleController.cur_throttle += throttleController.throttle_per_frame_pressed;
			if (throttleController.cur_throttle > 1)
			{
				throttleController.cur_throttle = 1;
			}
		}

		//if decrease key pressed, decrease throttle
		if (sf::Keyboard::isKeyPressed(throttleController.decrease_key))
		{
			throttleController.cur_throttle -= throttleController.throttle_per_frame_pressed;
			if (throttleController.cur_throttle < 0)
			{
				throttleController.cur_throttle = 0;
			}
		}

		
		
	}

}



//unsigned long last_success_packet_millis;
//unsigned long packet_count;
//ConnectionStatus connection_status;

void UpdateDrawConnectionStats(sf::RenderWindow& window)
{
	if (frameCounter % 60 == 0)
	{
		last_packet_count = packet_count;
		packet_count = 0;

		last_failed_packet_count = failed_packet_count;
		failed_packet_count = 0;
	}

	

	float outline_thickness = 3;
	float edge_margin = 20;

	//draw rectangle to hold connection stats, put it in top right corner, 150x150
	sf::RectangleShape box(sf::Vector2f(150, 110));
	box.setPosition(window.getSize().x - 150 - outline_thickness - edge_margin, outline_thickness + edge_margin);
	box.setOutlineThickness(outline_thickness);
	box.setOutlineColor(sf::Color::White);
	box.setFillColor(sf::Color::Transparent);
	
	window.draw(box);

	//draw text for connection status based on connection_status enum value
	sf::Text text;
	text.setFont(font);
	text.setCharacterSize(20);


	//lock connection_status_mutex
	std::lock_guard<std::mutex> lock(connection_status_mutex);

	
	
	//if connection status is connected, draw green text
	if (connection_status == ConnectionStatus::CONNECTED)
	{
		//76, 175, 80
		text.setFillColor(sf::Color(76, 175, 80));
	}

	//if connection status is disconnected, draw red text
	else if (connection_status == ConnectionStatus::DISCONNECTED)
	{
		//244, 67, 54
		text.setFillColor(sf::Color(185, 67, 54));
	}
	
	//if SERIAL_OK_NO_REMOTE
	else if (connection_status == ConnectionStatus::SERIAL_OK_NO_REMOTE)
	{
		//255, 235, 59
		text.setFillColor(sf::Color(232, 152, 0));
	}


	//set text based on connection status
	if (connection_status == ConnectionStatus::CONNECTED)
	{
		text.setString("Connected");
	}
	else if (connection_status == ConnectionStatus::DISCONNECTED)
	{
		text.setString("Disconnected");
	}
	else if (connection_status == ConnectionStatus::SERIAL_OK_NO_REMOTE)
	{
		text.setString("No Remote");


	}

	else
	{
		text.setString("Uknown Connection Status");
	}



	//center the text near the top of the box
	sf::FloatRect textRect = text.getLocalBounds();
	text.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
	text.setPosition(window.getSize().x - 150 - outline_thickness - edge_margin + 75, outline_thickness + edge_margin + 20);

	window.draw(text);


	

	//draw packets/sec stat, for now placeholder of -1 p/s, goes directly below connection status text in green if > 0, otherwise red
	text.setString(std::to_string(last_packet_count) + " p/s");
	textRect = text.getLocalBounds();
	text.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);

	if (last_packet_count > 0)
	{
		text.setFillColor(sf::Color(76, 175, 80));
	}
	else
	{
		text.setFillColor(sf::Color(185, 67, 54));
	}

	text.setPosition(window.getSize().x - 150 - outline_thickness - edge_margin + 75, outline_thickness + edge_margin + 20 + 30);
	
	window.draw(text);

	//draw packet success rate, for now placeholder of -1%, goes directly below packets/sec text in green if > 0, otherwise red
	float success_rate = 0;
	if (last_packet_count > 0)
	{
		success_rate = 1 - (float) last_failed_packet_count / (float)last_packet_count;

	}


	std::string success_rate_str = std::to_string(success_rate * 100);

	//limit to first 4 characters
	if (success_rate_str.length() > 4)
	{
		success_rate_str = success_rate_str.substr(0, 4);
	}

	text.setString(success_rate_str + "%");
	textRect = text.getLocalBounds();
	text.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
	
	if (success_rate > 0)
	{
		text.setFillColor(sf::Color(76, 175, 80));
	}
	else
	{
		text.setFillColor(sf::Color(185, 67, 54));
	}

	text.setPosition(window.getSize().x - 150 - outline_thickness - edge_margin + 75, outline_thickness + edge_margin + 20 + 30 + 30);
	
	window.draw(text);
	


	
}





void DrawBufferVisualization_SLOW(sf::RenderWindow& window)
{
	// Draw box to hold buffer visualization, put it below the connection stats box, and make it go down 300px
	float outline_thickness = 3;
	float edge_margin = 20;
	float box_width = 380;

	sf::RectangleShape box(sf::Vector2f(box_width, 450));
	box.setPosition(window.getSize().x - box_width - outline_thickness - edge_margin, outline_thickness + edge_margin + 110 + 20);
	box.setOutlineThickness(outline_thickness);
	box.setOutlineColor(sf::Color::White);
	box.setFillColor(sf::Color::Transparent);

	window.draw(box);

	// Lock send_queue and receive_queue, copy them to local variables, then unlock
	std::vector<Message> send_queue_copy;
	{
		std::lock_guard<std::mutex> lock(send_mutex);
		send_queue_copy = send_queue;
	}

	std::vector<Message> display_recv_queue_copy;
	{
		display_recv_queue_copy = display_recv_queue;
	}

	// Draw send queue
	sf::Text text;
	text.setFont(font);
	text.setCharacterSize(16);
	text.setFillColor(sf::Color::White);

	// Draw send queue title
	text.setString("Send Queue");
	int send_text_offset_x = 10;
	int send_text_offset_y = 10;
	sf::FloatRect textRectSendQueue = text.getLocalBounds();
	text.setPosition(window.getSize().x - box_width - outline_thickness - edge_margin + send_text_offset_x, outline_thickness + edge_margin + 110 + 20 + send_text_offset_y);

	// Underline text
	sf::RectangleShape underline(sf::Vector2f(textRectSendQueue.width, 2));
	underline.setPosition(window.getSize().x - box_width - outline_thickness - edge_margin + send_text_offset_x, outline_thickness + edge_margin + 110 + 26 + send_text_offset_y + textRectSendQueue.height);
	underline.setFillColor(sf::Color::White);
	window.draw(underline);
	window.draw(text);

	text.setCharacterSize(10);

	sf::FloatRect recv_text_rect = text.getLocalBounds();

	// Draw send queue lines
	for (unsigned int i = 0; i < SEND_BUFFER_LINES; i++)
	{
		if (i < send_queue_copy.size())
		{
			std::string timestamp = generate_6char_timestamp(send_queue_copy[i].timestamp);
			text.setString(timestamp + ": " + send_queue_copy[i].msg);
		}
		else
		{
			text.setString("");
		}

		// Set position
		text.setPosition(window.getSize().x - box_width - outline_thickness - edge_margin + send_text_offset_x, outline_thickness + edge_margin + 110 + 20 + send_text_offset_y + 30 + i * 12);
		window.draw(text);
	}

	// Do the same for receive queue, but offset to the right
	int receive_text_offset_x = 180;

	// Draw receive queue title
	text.setString("Receive Log");
	text.setCharacterSize(16);
	//textRect = text.getLocalBounds();
	text.setPosition(window.getSize().x - box_width - outline_thickness - edge_margin + receive_text_offset_x, outline_thickness + edge_margin + 110 + 20 + send_text_offset_y);
	underline.setPosition(window.getSize().x - box_width - outline_thickness - edge_margin + receive_text_offset_x, outline_thickness + edge_margin + 110 + 26 + send_text_offset_y + recv_text_rect.height);
	underline.setSize(sf::Vector2f(recv_text_rect.width, 2));
	window.draw(underline);
	window.draw(text);

	text.setCharacterSize(10);

	// Draw receive queue lines
	for (unsigned int i = 0; i < SEND_BUFFER_LINES; i++)
	{
		if (i < display_recv_queue_copy.size())
		{
			std::string timestamp = generate_6char_timestamp(display_recv_queue_copy[i].timestamp);
			text.setString(timestamp + ": " + display_recv_queue_copy[i].msg);
		}
		else
		{
			text.setString("");
		}

		// Set position
		//textRect = text.getLocalBounds();
		text.setPosition(window.getSize().x - box_width - outline_thickness - edge_margin + receive_text_offset_x, outline_thickness + edge_margin + 110 + 20 + send_text_offset_y + 30 + i * 12);
		window.draw(text);
	}
}



void DrawBufferVisualization(sf::RenderWindow& window)
{



	//use chrono to measure runtime
	//auto start = std::chrono::high_resolution_clock::now();





	// Draw box to hold buffer visualization
	float outline_thickness = 3;
	float edge_margin = 20;
	float box_width = 380;

	sf::RectangleShape box(sf::Vector2f(box_width, 450));
	box.setPosition(window.getSize().x - box_width - outline_thickness - edge_margin, outline_thickness + edge_margin + 110 + 20);
	box.setOutlineThickness(outline_thickness);
	box.setOutlineColor(sf::Color::White);
	box.setFillColor(sf::Color::Transparent);
	window.draw(box);



	// Lock send_queue and receive_queue, copy them to local variables, then unlock
	std::vector<Message> send_queue_copy;
	{
		std::lock_guard<std::mutex> lock(send_mutex);
		send_queue_copy = send_queue;
	}





	// Preparing text objects
	sf::Text text;
	text.setFont(font);
	text.setCharacterSize(10);
	text.setFillColor(sf::Color::White);

	// Concatenate send queue messages
	std::string send_queue_text;
	for (const auto& message : send_queue_copy) {
		send_queue_text += generate_6char_timestamp(message.timestamp) + ": " + message.msg + "";
	}


	// Draw send queue title and concatenated text
	text.setCharacterSize(16);
	text.setString("Send Queue");
	text.setPosition(box.getPosition().x + 10, box.getPosition().y + 10);
	window.draw(text);

	//draw underline for send queue text
	sf::FloatRect textRectSendQueue = text.getLocalBounds();
	sf::RectangleShape underline(sf::Vector2f(textRectSendQueue.width, 3));
	underline.setPosition(box.getPosition().x + 10, box.getPosition().y + 16 + textRectSendQueue.height);
	underline.setFillColor(sf::Color::White);
	window.draw(underline);
	
	
	
	text.setCharacterSize(10);
	text.setString(send_queue_text);
	text.setPosition(box.getPosition().x + 10, box.getPosition().y + 40);
	window.draw(text);



	


	//pop elements from the front of the receive queue until its < SEND_BUFFER_LINES
	while (display_recv_queue.size() > SEND_BUFFER_LINES) {
		display_recv_queue.pop_back();
	}


	// Concatenate receive queue messages
	std::string receive_queue_text;
	for (const auto& message : display_recv_queue) {
		receive_queue_text += generate_6char_timestamp(message.timestamp) + ": " + message.msg + "";
	}


	//print number of messages
	//std::cout << "Number of messages in recv queue: " << display_recv_queue_copy.size() << std::endl;

	// Draw receive queue title and concatenated text
	text.setCharacterSize(16);
	text.setString("Receive Log");
	text.setPosition(box.getPosition().x + 180, box.getPosition().y + 10);
	window.draw(text);

	//draw recv log underline
	sf::FloatRect textRectRecvLog = text.getLocalBounds();
	sf::RectangleShape underline2(sf::Vector2f(textRectRecvLog.width, 3));
	underline2.setPosition(box.getPosition().x + 180, box.getPosition().y + 16 + textRectRecvLog.height);
	underline2.setFillColor(sf::Color::White);
	window.draw(underline2);
	


	text.setCharacterSize(10);
	text.setString(receive_queue_text);
	text.setPosition(box.getPosition().x + 180, box.getPosition().y + 40);
	window.draw(text);




	//auto end = std::chrono::high_resolution_clock::now();
	//std::chrono::duration<double> elapsed_seconds = end - start;
	//std::cout << "Elapsed time: " << elapsed_seconds.count() << "s\n";


}





void LoadTextures()
{
	//C:\Users\aspen\Desktop\RCLink\RCLink\Textures\Final
	//  HorizonGroundWide.png, AngleMarksLayer.png,, StaticScopeLayer.png

	std::string base_texture_path = "C:\\Users\\aspen\\Desktop\\RCLink\\RCLink\\Textures\\Final\\";

	std::string horiz_tex_path = base_texture_path + "HorizonGroundWide.png";
	std::string angle_tex_path = base_texture_path + "AngleMarksLayer.png";
	std::string static_tex_path = base_texture_path + "StaticScopeLayer.png";
	

	//push back to textures vetor of pointers to textures
	textures.push_back(new sf::Texture());
	textures.push_back(new sf::Texture());
	textures.push_back(new sf::Texture());

	//load the texture from file
	textures[0]->loadFromFile(horiz_tex_path);
	textures[1]->loadFromFile(angle_tex_path);
	textures[2]->loadFromFile(static_tex_path);

	textures[0]->setSmooth(true);
	textures[1]->setSmooth(true);
	textures[2]->setSmooth(true);
	
	
	
	

}


void SetupAttitudeDrawing(sf::RenderWindow& window)
{

	render_texture.create(window.getSize().x, window.getSize().y);
}

void DrawAttitudeIndicator(sf::RenderWindow& window)
{
	//height and width
	float indicator_size = 240;
	float top_margin = 30;

	float horizon_extra_scale = 1.5f;

	
	// Static scope layer
	sf::Sprite static_scope;
	static_scope.setTexture(*textures[2]);
	static_scope.setPosition(window.getSize().x / 2 - indicator_size / 2, top_margin);
	static_scope.setScale(indicator_size / static_scope.getTexture()->getSize().x, indicator_size / static_scope.getTexture()->getSize().y);


	
	// Horizon
	sf::Sprite horizon;
	horizon.setTexture(*textures[0]);
	horizon.setScale(horizon_extra_scale * indicator_size / static_scope.getTexture()->getSize().x, horizon_extra_scale * indicator_size / static_scope.getTexture()->getSize().y);

	float pixels_per_pitch_degree = 146.f / 20.f;
	float pitch_offset = pitch_orientation * pixels_per_pitch_degree * horizon_extra_scale;



	sf::Vector2f base_origin = sf::Vector2f(horizon.getTexture()->getSize().x / 2, horizon.getTexture()->getSize().y / 2);

	
	
	base_origin.y -= pitch_offset;


	horizon.setOrigin(base_origin);
	
	
	horizon.setRotation(roll_orientation);

	
	sf::Vector2f base_position = sf::Vector2f(window.getSize().x / 2, top_margin + indicator_size / 2);

	horizon.setPosition(base_position);
	






	//clear the rendertexture
	render_texture.clear(sf::Color::Transparent);
	
	//draw a white circle in the static scope position of size indicator size
	sf::CircleShape circle(indicator_size / 2);
	circle.setFillColor(sf::Color::White);
	circle.setOrigin(indicator_size / 2, indicator_size / 2);
	circle.setPosition(window.getSize().x / 2, top_margin + indicator_size / 2);
	render_texture.draw(circle);

	//draw on the horizon sprite with multiply mode
	render_texture.draw(horizon, sf::BlendMultiply);
	
	
	//now draw that rendertexutre onto the window
	render_texture.display();
	sf::Sprite sprite(render_texture.getTexture());
	window.draw(sprite);
	


	//draw the angle marks
	sf::Sprite angle_marks;
	angle_marks.setTexture(*textures[1]);

	
	angle_marks.setPosition(window.getSize().x / 2 - indicator_size / 2, top_margin);
	angle_marks.setScale(indicator_size / angle_marks.getTexture()->getSize().x,indicator_size / angle_marks.getTexture()->getSize().x);
	

	window.draw(angle_marks);
	// Draw static scope
	window.draw(static_scope);

	//at the top center of the indicator draw the roll value
	sf::Text roll_text;
	roll_text.setFont(font);
	roll_text.setCharacterSize(20);
	roll_text.setFillColor(sf::Color::White);



	std::stringstream stream;

	// Set fixed floating-point notation and precision
	stream << std::fixed << std::setprecision(1) << roll_orientation;
	
	std::string roll_string = stream.str();


	roll_text.setString(roll_string);
	roll_text.setPosition(window.getSize().x / 2 - roll_text.getLocalBounds().width / 2, top_margin - 25);
	window.draw(roll_text);


	//on the right side of the scope draw the pitch value
	sf::Text pitch_text;
	pitch_text.setFont(font);
	pitch_text.setCharacterSize(20);
	pitch_text.setFillColor(sf::Color::White);
	
	stream.str(std::string());
	stream.clear();
	stream << std::fixed << std::setprecision(1) << pitch_orientation;
	std::string pitch_string = stream.str();
	pitch_text.setString(pitch_string);
	pitch_text.setPosition(window.getSize().x / 2 + indicator_size / 2 + 10, top_margin + indicator_size / 2 - pitch_text.getLocalBounds().height / 2);	
	window.draw(pitch_text);
	

	
	
}

























