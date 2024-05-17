#pragma once


#include "RadioCommunication.hpp"
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>


extern std::vector<ServoController> servoControllerVector;

extern sf::Font font;

void DrawServoControllers(std::vector<ServoController>& servoControllers, sf::RenderWindow& window);
void LoadFont();

void ProcessControlInputs();

void UpdateDrawConnectionStats(sf::RenderWindow& window);