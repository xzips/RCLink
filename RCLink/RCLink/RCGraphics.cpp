
#include "RadioCommunication.hpp"
#include <SFML/Graphics.hpp>
#include <iostream>
#include "RCGraphics.hpp"

std::vector<ServoController> servoControllerVector;
sf::Font font;
unsigned long frameCounter = 0;


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






void ProcessControlInputs()
{

	
	//for each servo controller, check if the keys are pressed and update the servo angle
	for (auto& servo : servoControllerVector)
	{
		
		if (!(sf::Keyboard::isKeyPressed(servo.increase_key) && sf::Keyboard::isKeyPressed(servo.decrease_key)))
			
		{
			//std::cout << "Both keys are not pressed" << std::endl;
			
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
				servo.curAngle -= servo.angle_per_frame_pressed;;
				if (servo.curAngle < servo.min_angle_cal)
				{
					servo.curAngle = servo.min_angle_cal;
				}
				
			}

			else
			{
				//return to neutral position
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


void DrawBufferVisualization(sf::RenderWindow& window)
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
	sf::FloatRect textRect = text.getLocalBounds();
	text.setPosition(window.getSize().x - box_width - outline_thickness - edge_margin + send_text_offset_x, outline_thickness + edge_margin + 110 + 20 + send_text_offset_y);

	// Underline text
	sf::RectangleShape underline(sf::Vector2f(textRect.width, 2));
	underline.setPosition(window.getSize().x - box_width - outline_thickness - edge_margin + send_text_offset_x, outline_thickness + edge_margin + 110 + 26 + send_text_offset_y + textRect.height);
	underline.setFillColor(sf::Color::White);
	window.draw(underline);
	window.draw(text);

	text.setCharacterSize(10);

	// Draw send queue lines
	for (int i = 0; i < SEND_BUFFER_LINES; i++)
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
		textRect = text.getLocalBounds();
		text.setPosition(window.getSize().x - box_width - outline_thickness - edge_margin + send_text_offset_x, outline_thickness + edge_margin + 110 + 20 + send_text_offset_y + 30 + i * 12);
		window.draw(text);
	}

	// Do the same for receive queue, but offset to the right
	int receive_text_offset_x = 180;

	// Draw receive queue title
	text.setString("Receive Log");
	text.setCharacterSize(16);
	textRect = text.getLocalBounds();
	text.setPosition(window.getSize().x - box_width - outline_thickness - edge_margin + receive_text_offset_x, outline_thickness + edge_margin + 110 + 20 + send_text_offset_y);
	underline.setPosition(window.getSize().x - box_width - outline_thickness - edge_margin + receive_text_offset_x, outline_thickness + edge_margin + 110 + 26 + send_text_offset_y + textRect.height);
	underline.setSize(sf::Vector2f(textRect.width, 2));
	window.draw(underline);
	window.draw(text);

	text.setCharacterSize(10);

	// Draw receive queue lines
	for (int i = 0; i < SEND_BUFFER_LINES; i++)
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
		textRect = text.getLocalBounds();
		text.setPosition(window.getSize().x - box_width - outline_thickness - edge_margin + receive_text_offset_x, outline_thickness + edge_margin + 110 + 20 + send_text_offset_y + 30 + i * 12);
		window.draw(text);
	}
}
