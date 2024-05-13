#ifndef OLED_CONTROLLER_HPP
#define OLED_CONTROLLER_HPP

#include <Arduino.h>
#include <U8g2lib.h>

extern U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2; 

namespace disp
{


  void setup_display();
  void add_text_to_display(String text, int x, int y);
  void clear_display();
  void update_stats_V1(bool connected, unsigned long time_since_last_packet, unsigned long system_time_ms, const char* last_msg_recvd);

  
}

#endif
