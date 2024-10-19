#pragma once


#include "RadioCommunication.hpp"
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include "Quick3D.hpp"



const int SEND_BUFFER_LINES = 28;

extern std::vector<ServoController> servoControllerVector;
extern ThrottleController throttleController;

extern std::vector<sf::Texture*> textures;

extern sf::RenderWindow* P_window;

extern unsigned long frameCounter;

extern std::vector<q3d::TM> models;


extern bool escCalibrateButtonPressed;
extern long int escCalTimerMS;

extern sf::Font font;

void DrawServoControllers(std::vector<ServoController>& servoControllers, sf::RenderWindow& window);
void LoadFont();
void LoadTextures();

ServoController* GetServoControllerByName(std::string name);

void ProcessControlInputs();

void UpdateDrawConnectionStats(sf::RenderWindow& window);

//void DrawBufferVisualization(sf::RenderWindow& window);

void SetupAttitudeDrawing(sf::RenderWindow& window);


void UpdateDrawThrottleController(sf::RenderWindow& window);

void DrawAttitudeIndicator(sf::RenderWindow& window);

