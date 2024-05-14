#include <Arduino.h>
#include <U8g2lib.h>
#include <SPI.h>
#include <Wire.h>
#include "OLEDController.hpp"

unsigned long last_update_time_millis;




U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0); 

namespace disp
{
    
  void setup_display()
  {
    u8g2.begin();
    last_update_time_millis = millis();
  }

  void add_text_to_display(String text, int x, int y)
  {
  
    u8g2.setFont(u8g2_font_logisoso28_tr);
    u8g2.drawStr(x, y, text.c_str());
    u8g2.sendBuffer();
  }

  void clear_display()
  {
    u8g2.clearBuffer();
  }

  void update_stats_V1(bool connected, unsigned long time_since_last_packet, unsigned long system_time_ms, const char* last_msg_recvd)
  {
    clear_display();

    int con_pos_x = 0;
    int con_pos_y = 8;

    int time_last_pos_x = 0;
    int time_last_pos_y = 24;

    int system_time_pos_x = 0;
    int system_time_pos_y = 16;

    int last_msg_pos_x = 0;
    int last_msg_pos_y = 32;

    const char* connected_str = connected ? "Connected" : "Disconnected";

    u8g2.setFont(u8g2_font_tinytim_tr  );  // set a smaller font for more text
    
    u8g2.drawStr(con_pos_x, con_pos_y, connected_str);

    // Display time since last packet
    char time_last_str[32];
    sprintf(time_last_str, "LT: %lums", time_since_last_packet);
    u8g2.drawStr(time_last_pos_x, time_last_pos_y, time_last_str);

    // Display system time
    char system_time_str[32];
    sprintf(system_time_str, "SYS_T: %lums", system_time_ms);
    u8g2.drawStr(system_time_pos_x, system_time_pos_y, system_time_str);

    // Display last message received
    char last_msg_str[64];
    sprintf(last_msg_str, "LM: %s", last_msg_recvd);
    u8g2.drawStr(last_msg_pos_x, last_msg_pos_y, last_msg_str);

    u8g2.sendBuffer();  // transfer internal memory to the display
  }


  bool should_update_display()
  {
    unsigned long current_time = millis();
    if (current_time - last_update_time_millis > OLED_UPDATE_INTERVAL_MS)
    {
      last_update_time_millis = current_time;
      return true;
    }
    return false;
  }


}

