#include <Arduino.h>
#include "RF24.h"
#include "RadioController.hpp"
#include "OLEDController.hpp"
#include "PWMController.hpp"


unsigned long startTime;
unsigned long endTime;
unsigned long elapsedTime;

RF24 radio(CE_PIN, CSN_PIN);



void setup() {
  rcon::radio_setup(radio);
  disp::setup_display();
  pwm::setup_pwm();

  last_packet_timestamp_millis = millis();

}  




void loop() {

  startTime = micros();

  //currently blocking!!
  rcon::RadioLoopState state = rcon::radio_loop(radio);

  endTime = micros();
  elapsedTime = endTime - startTime;
  
  //Serial.println("RF24 radio_loop function: " + String(elapsedTime) + " microseconds");


  if (state != rcon::RadioLoopState::CONNECTED_IDLE) {
    //rcon::print_radio_loop_state(state);

  }



  /*
  if (state == rcon::RadioLoopState::RECIVED_SENT_DATA)
  {
    //if servo request, in format "SET_SERVO_07_090" //set serrvo 7 to 90 degrees
    static const char* set_servo_str = "SET_SERVO_";
    if (strncmp(rf_incoming_buffer, set_servo_str, strlen(set_servo_str)) == 0)
    {
      char* servo_num_str = rf_incoming_buffer + strlen(set_servo_str);
      char* angle_str = servo_num_str + 3;
      int servo_num = atoi(servo_num_str);
      int angle = atoi(angle_str);

      pwm::set_servo_angle(servo_num, angle);
    }
   


  }

  */



  //

  
  if (disp::should_update_display()) {
    disp::update_stats_V1(radioConnected, millis() - last_packet_timestamp_millis, millis(), rf_incoming_buffer);
  }
 
  
  //servonum, degrees
  //pwm::set_servo_angle(15, 100);
  
  //endTime = micros();
  //elapsedTime = endTime - startTime;
  //Serial.println("OLED Update Time: " + String(elapsedTime) + " microseconds");
  

  //delay(50);


} 
