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
    //thread background(serial_thread);

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



        window.display();
    }


    /*

    int n = 0;
    while (true) {
        push_msg(std::string("n = ") + to_string(n));
        n++;
        this_thread::sleep_for(std::chrono::milliseconds(1000));

        int n_msgs = 0;
        while (true) {
            string msg = pop_msg();
            if (msg == "Queue is empty") {
                break;
            }
            n_msgs += 1;
            cout << "Received message: " << msg << endl;
        }

        cout << "Number of messages received (in 1s): " << n_msgs << endl;
    }

    */

    //background.join();
    return 0;
}
