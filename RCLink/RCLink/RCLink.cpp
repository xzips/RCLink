#include "RadioCommunication.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <thread>

#include <SFML/Graphics.hpp>



using namespace std;

int main() {
    //thread background(serial_thread);




    sf::RenderWindow window(sf::VideoMode(1280, 720), "RCLink Controller");
    sf::CircleShape shape(100.f);
    shape.setFillColor(sf::Color::Green);

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear();
        window.draw(shape);
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
