#include "RadioCommunication.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include "RCGraphics.hpp"
#include <SFML/Graphics.hpp>
#include <cmath>


using namespace std;



void define_hardware()
{
    //95
    servoControllerVector.push_back(ServoController(15, 0, 270, 95, sf::Keyboard::W, sf::Keyboard::S, 6, 4, "Real Test Servo", true));
    servoControllerVector.push_back(ServoController(0,  0, 270, 135, sf::Keyboard::A, sf::Keyboard::D, 6, 4, "Virtual Placeholder A", false));
    servoControllerVector.push_back(ServoController(4,  0, 270, 135, sf::Keyboard::A, sf::Keyboard::D, 10, 6, "Virtual Placeholder B", false));

}

void load_models()
{
    //C:\Users\aspen\Desktop\RCLink\RCLink\Models\Cambered Wing 1.obj
	//q3d::SVF model;
    q3d::TM left_elev_model;
	q3d::LoadTriangleMeshOBJ(left_elev_model, "C:\\Users\\aspen\\Desktop\\RCLink\\RCLink\\Models\\LeftElevator.obj");
	
    
   
    q3d::SetPositionTM(left_elev_model, -20, 20, 100.0f);
    q3d::SetScaleTM(left_elev_model, 6);
	q3d::VertexTransformUpdateTM(left_elev_model);

	models.push_back(left_elev_model);


    q3d::TM right_elev_model;
    q3d::LoadTriangleMeshOBJ(right_elev_model, "C:\\Users\\aspen\\Desktop\\RCLink\\RCLink\\Models\\RightElevator.obj");
    
    
	q3d::SetPositionTM(right_elev_model, 20, 20, 100.0f);
	q3d::SetScaleTM(right_elev_model, 6);
	q3d::VertexTransformUpdateTM(right_elev_model);

    models.push_back(right_elev_model);
    

}

void update_model_rotations()
{


	ServoController* left_elev_servo = GetServoControllerByName("Real Test Servo");

	if (left_elev_servo != nullptr)
	{
		q3d::SetRotationTM(models[0], (-left_elev_servo->curAngle -80 +370)* 3.14159f / 180.f , 0, 0);
	}
    q3d::VertexTransformUpdateTM(models[0]);
    


    
	ServoController* right_elev_servo = GetServoControllerByName("Virtual Placeholder A");
   
    if (right_elev_servo != nullptr)
    {
        q3d::SetRotationTM(models[1], (-right_elev_servo->curAngle +330) * 3.14159f / 180.f, 0, 0);
    }

    
    q3d::VertexTransformUpdateTM(models[1]);
}


int main() {
    thread background(serial_thread);

    LoadFont();
    LoadTextures();

    define_hardware();


    load_models();

    roll_orientation = 20;
    pitch_orientation = 20;
    
    sf::ContextSettings settings;
    settings.antialiasingLevel = 8;

    //sf::RenderWindow window(sf::VideoMode(800, 600), "SFML shapes", sf::Style::Default, settings);
    sf::RenderWindow window(sf::VideoMode(1600, 1000), "RCLink Controller", sf::Style::Default, settings);

    //set 60fps framerate limit
    window.setFramerateLimit(60);


    float angle = 0;

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

        UpdateDrawThrottleController(window);

        DrawBufferVisualization(window);

        DrawAttitudeIndicator(window);


        //q3d::SetRotationTM(models[0], angle, 0, 0);
        //q3d::SetRotationTM(models[1], angle, 0, 0);
        
        
		//q3d::VertexTransformUpdateTM(models[0]);
		//q3d::VertexTransformUpdateTM(models[1]);

        update_model_rotations();

        



        //draw 3d viz
		for (int i = 0; i < models.size(); i++)
		{
			//q3d::DrawSVF(window, models[i]);
            
			q3d::DrawTM(window, models[i]);

		}
        

		if (frameCounter % 5 == 0) {
			//push_msg("Hello from main loop");

            std::string servoControlStr = servoControllerVector[0].GetCommandSTR();
			push_msg(servoControlStr);

			std::string throttleControlStr = throttleController.GetCommandSTR();
			push_msg(throttleControlStr);

            
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
