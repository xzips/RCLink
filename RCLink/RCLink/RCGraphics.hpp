#pragma once


#include "RadioCommunication.hpp"
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>




const int SEND_BUFFER_LINES = 32;

extern std::vector<ServoController> servoControllerVector;

extern unsigned long frameCounter;

extern sf::Font font;

void DrawServoControllers(std::vector<ServoController>& servoControllers, sf::RenderWindow& window);
void LoadFont();

void ProcessControlInputs();

void UpdateDrawConnectionStats(sf::RenderWindow& window);

void DrawBufferVisualization(sf::RenderWindow& window);
