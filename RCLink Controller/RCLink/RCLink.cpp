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
    
    servoControllerVector.push_back(ServoController(1,  90, 180, 135, sf::Keyboard::Q, sf::Keyboard::E, 3, 2, "Left Aileron", false, true));
    servoControllerVector.push_back(ServoController(2,  90, 180, 135, sf::Keyboard::E, sf::Keyboard::Q, 3, 2, "Right Aileron", false, false));

	servoControllerVector.push_back(ServoController(5, 90, 130, 110, sf::Keyboard::A, sf::Keyboard::D, 3, 2, "Front Wheel", false, false));
    //95
    servoControllerVector.push_back(ServoController(8, 25, 165, 95, sf::Keyboard::W, sf::Keyboard::S, 5, 0, "Left Elevator", false, false));
    
    //servoControllerVector.push_back(ServoController(10,  25, 165, 95, sf::Keyboard::W, sf::Keyboard::S, 3, 0, "Right Elevator", false));
    servoControllerVector.push_back(ServoController(10, 25, 165, 95, sf::Keyboard::W, sf::Keyboard::S, 5, 0, "Right Elevator", false, true));
	servoControllerVector.push_back(ServoController(9, 45, 135, 90, sf::Keyboard::A, sf::Keyboard::D, 3, 2, "Rudder", false, false));


}



void load_models()
{
    //C:\Users\aspen\Desktop\RCLink\RCLink\Models\Cambered Wing 1.obj
	//q3d::SVF model;
    q3d::TM left_elev_model;
	q3d::LoadTriangleMeshOBJ(left_elev_model, "C:\\Users\\aspen\\Desktop\\RCLink\\RCLink Controller\\Models\\LeftElevator.obj");
	
    
   
    q3d::SetPositionTM(left_elev_model, -12, 20, 100.0f);
    q3d::SetScaleTM(left_elev_model, 4);
	q3d::VertexTransformUpdateTM(left_elev_model);

	models.push_back(left_elev_model);


    q3d::TM right_elev_model;
    q3d::LoadTriangleMeshOBJ(right_elev_model, "C:\\Users\\aspen\\Desktop\\RCLink\\RCLink Controller\\Models\\RightElevator.obj");
    
    
	q3d::SetPositionTM(right_elev_model, 12, 20, 100.0f);
	q3d::SetScaleTM(right_elev_model, 4);
	q3d::VertexTransformUpdateTM(right_elev_model);

    models.push_back(right_elev_model);


    
	//load LeftAileron and RightAileron models
	q3d::TM left_aileron_model;
	q3d::LoadTriangleMeshOBJ(left_aileron_model, "C:\\Users\\aspen\\Desktop\\RCLink\\RCLink Controller\\Models\\LeftAileron.obj");
	q3d::SetPositionTM(left_aileron_model, -20, -20, 130.0f);
	q3d::SetScaleTM(left_aileron_model, 5);
	q3d::VertexTransformUpdateTM(left_aileron_model);
    
	models.push_back(left_aileron_model);
    
	q3d::TM right_aileron_model;
	q3d::LoadTriangleMeshOBJ(right_aileron_model, "C:\\Users\\aspen\\Desktop\\RCLink\\RCLink Controller\\Models\\RightAileron.obj");
    
	q3d::SetPositionTM(right_aileron_model, 20, -20, 130.0f);
	q3d::SetScaleTM(right_aileron_model, 5);
	q3d::VertexTransformUpdateTM(right_aileron_model);
    
	models.push_back(right_aileron_model);
    
    //load rudder
	q3d::TM rudder_model;
	q3d::LoadTriangleMeshOBJ(rudder_model, "C:\\Users\\aspen\\Desktop\\RCLink\\RCLink Controller\\Models\\Rudder.obj");

    //for all the verts subtract 0.5 z
	q3d::OffsetOriginalCoords(rudder_model, 0, 0, 0.2);


	q3d::SetPositionTM(rudder_model, 0, 30, 100.f);
	q3d::SetScaleTM(rudder_model, 8);
	q3d::VertexTransformUpdateTM(rudder_model);

	models.push_back(rudder_model);
    
}

void update_model_rotations()
{


	ServoController* left_elev_servo = GetServoControllerByName("Left Elevator");

	if (left_elev_servo != nullptr)
	{
		q3d::SetRotationTM(models[0], (-left_elev_servo->curAngle + 300)* 3.14159f / 180.f , 0, 0);
	}
    q3d::VertexTransformUpdateTM(models[0]);
    


    
	ServoController* right_elev_servo = GetServoControllerByName("Right Elevator");
   
    if (right_elev_servo != nullptr)
    {
        //+330
        q3d::SetRotationTM(models[1], (-right_elev_servo->curAngle + 300) * 3.14159f / 180.f, 0, 0);
    }

    
    q3d::VertexTransformUpdateTM(models[1]);


	ServoController* left_aileron_servo = GetServoControllerByName("Left Aileron");
    
	if (left_aileron_servo != nullptr)
	{
		q3d::SetRotationTM(models[2], (left_aileron_servo->curAngle + 80) * 3.14159f / 180.f, 0, 0);
	}

	q3d::VertexTransformUpdateTM(models[2]);

	ServoController* right_aileron_servo = GetServoControllerByName("Right Aileron");

	if (right_aileron_servo != nullptr)
	{
		q3d::SetRotationTM(models[3], (right_aileron_servo->curAngle + 80) * 3.14159f / 180.f, 0, 0);
	}

	q3d::VertexTransformUpdateTM(models[3]);


	ServoController* rudder_servo = GetServoControllerByName("Rudder");
    
	if (rudder_servo != nullptr)
	{
		q3d::SetRotationTM(models[4], 0, (rudder_servo->curAngle + 90) * 3.14159f / 180.f, 0);
	}

	q3d::VertexTransformUpdateTM(models[4]);

    
    
    
    
    
    
}




#include "StateSync.hpp"



void UpdateControllerState()
{
	controllerState.FrontWheel = GetServoControllerByName("Front Wheel")->GetPhysicalAngle();
	controllerState.LeftElevator = GetServoControllerByName("Left Elevator")->GetPhysicalAngle();
	controllerState.RightElevator = GetServoControllerByName("Right Elevator")->GetPhysicalAngle();
	controllerState.LeftAileron = GetServoControllerByName("Left Aileron")->GetPhysicalAngle();
	controllerState.RightAileron = GetServoControllerByName("Right Aileron")->GetPhysicalAngle();
	controllerState.Rudder = GetServoControllerByName("Rudder")->GetPhysicalAngle();
    controllerState.Throttle = throttleController.GetMappedThrottle();
    
    
    uint64_t time_since_start = get_timestamp_ms() - controller_start_time;
    
    
    controllerState.ControllerTimestamp = time_since_start;
    
    //sin of time since start, normalized 0-255
	float sinVal = sin(time_since_start / 400.0f) * 127.5f + 127.5f;

    controllerState.jitter_test_byte = (uint8_t)sinVal;

    //std::cout << (int)controllerState.jitter_test_byte << std::endl;

}

int main() {
    
    //bool encoderDecoderStatus = EncodeDecodeTest(false);

	//std::cout << "Encoder Decoder Test: " << encoderDecoderStatus << std::endl;
    
    //exit(-1);


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

    SetupAttitudeDrawing(window);
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

        
        {
			//muttex for controller state
			std::lock_guard<std::mutex> lock(controller_mutex);


            UpdateControllerState();

        }

        

        //DrawBufferVisualization(window);


        UpdateMainThreadTelemetryVariables();


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
        


        /*
        00000000000000000000000000000000
        MS4NNXXX-MMXXX-LLXXX-KKXXX-PPXXX
        MS415095000950113502135
        
        multi servo number 3 and then num-angle
        
        */

		if (frameCounter % 5 == 0) {
			//push_msg("Hello from main loop");

            //SendAllServos();

			//std::string throttleControlStr = throttleController.GetCommandSTR();
			//push_msg(throttleControlStr);

            
		}
        


        {
            std::lock_guard<std::mutex> lock1(last_incoming_message_mutex);

            if (!is_error(last_incoming_message.msg))
            {
                {
                    std::lock_guard<std::mutex> lock(connection_status_mutex);

			        connection_status = ConnectionStatus::CONNECTED;
			        last_success_packet_millis = get_timestamp_ms();
                }
                

                //HandleIncomingMessage(msg);
            }

            else
            {
                failed_packet_count++;
            }


            
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
