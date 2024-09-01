#ifndef OLED_CONTROLLER_HPP
#define OLED_CONTROLLER_HPP

#include <Arduino.h>
#include <U8g2lib.h>


#define OLED_UPDATE_INTERVAL_MS 350

extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;

extern unsigned long last_update_time_millis;



namespace disp
{


  void setup_display();
  void add_text_to_display(String text, int x, int y);
  void clear_display();
  void update_stats_V1(bool connected, unsigned long time_since_last_packet, unsigned long system_time_ms, const char* last_msg_recvd);
  bool should_update_display();
  
}

#endif
