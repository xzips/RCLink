#include "RadioCommunication.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include "RCGraphics.hpp"
#include <SFML/Graphics.hpp>



using namespace std;



void define_hardware()
{
    servoControllerVector.push_back(ServoController(15, 0, 270, 135, sf::Keyboard::W, sf::Keyboard::S, 10, 6, "Real Test Servo"));
    servoControllerVector.push_back(ServoController(0,  0, 270, 135, sf::Keyboard::A, sf::Keyboard::D, 10, 6, "Virtual Placeholder A"));
    servoControllerVector.push_back(ServoController(4,  0, 270, 135, sf::Keyboard::A, sf::Keyboard::D, 10, 6, "Virtual Placeholder B"));

}



int main() {
    thread background(serial_thread);

    LoadFont();

    define_hardware();


    sf::RenderWindow window(sf::VideoMode(1600, 1000), "RCLink Controller");

    //set 60fps framerate limit
    window.setFramerateLimit(60);


    while (window.isOpen())
    {

        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

        }

        window.clear();

        ProcessControlInputs();

        DrawServoControllers(servoControllerVector, window);

        UpdateDrawConnectionStats(window);

        DrawBufferVisualization(window);

		if (frameCounter % 20 == 0) {
			//push_msg("Hello from main loop");

            std::string servoControlStr = servoControllerVector[0].GetCommandSTR();
			push_msg(servoControlStr);
            
		}
        


        while (true) {
            string msg = pop_msg();
            if (msg == "Queue is empty" or msg == "Serial not connected") {
                break;
            }

            packet_count++;

            if (!is_error(msg))
            {
                std::lock_guard<std::mutex> lock(connection_status_mutex);

				connection_status = ConnectionStatus::CONNECTED;
				last_success_packet_millis = get_timestamp_ms();
                
            }

            else
            {
                failed_packet_count++;
            }

            

            

            //cout << "Received message: " << msg << endl;
        }


        window.display();

        frameCounter++;
    }

    
    cleanupFlag.store(true);

	//wait 50ms for the serial thread to finish
	this_thread::sleep_for(std::chrono::milliseconds(50));
    
    background.join();
    

    
    return 0;
}
