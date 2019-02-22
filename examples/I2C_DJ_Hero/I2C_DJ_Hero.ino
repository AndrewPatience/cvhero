#include <Wire.h>

#define BAUDRATE 19200
#define ADDRESS 0x52

void setup()
{
   Serial.begin(BAUDRATE);
   
   Wire.begin();
   Wire.beginTransmission(ADDRESS);      
   Wire.write(0xF0);
   Wire.write(0x55);
   Wire.endTransmission();
   delay(1);
   Wire.beginTransmission(ADDRESS);
   Wire.write(0xFB);
   Wire.write((uint8_t)0x00);
   Wire.endTransmission();
}

void loop()
{
  int count = 0;      
  int values[6];
  int joy_x;
  int joy_y;
  int r_depth;
  int r_button;
  int g_button;
  int b_button;
  int turntable;
  int rotary;
  int minus_button;
  int plus_button;  
  int redraw_button;

  Wire.requestFrom (ADDRESS, 6); 
    
  while(Wire.available())
  {
    values[count] = Wire.read();
    count++;
    
  }
  joy_x = values[0] & 63;
  joy_y = values[1] & 63;
  r_depth = (values[2] & 31)>>1;
  r_button = (~values[4] & 2)>>1;
  g_button = (~values[5] & 32)>>5;
  b_button = (~values[5] & 4)>>2;
  turntable = ((values[2] & 1)<<5) + ((values[0] & 192)>>3);
  turntable += ((values[1] & 192)>>5) + ((values[2] & 128)>>7);
  rotary = (values[2] & 96)>>2;
  rotary += (values[3] & 224)>>5;
  minus_button = (~values[4] & 16)>>4;
  plus_button = (~values[4] & 4)>>2;
  redraw_button = (~values[5] & 31)>>4;
  /*
  Serial.println(values[0], BIN);
  Serial.println(values[1], BIN);
  Serial.println(values[2], BIN);
  Serial.println(values[3], BIN);
  Serial.println(values[4], BIN);
  Serial.println(values[5], BIN);
  Serial.println("-------------------");
  */

  Serial.print(joy_x);
  Serial.print(",");
  Serial.print(joy_y);
  Serial.print(":");
  Serial.print(r_depth);
  Serial.print(":R");
  Serial.print(r_button);
  Serial.print(":G");
  Serial.print(g_button);
  Serial.print(":B");
  Serial.print(b_button);
  Serial.print(":M");
  Serial.print(minus_button);
  Serial.print(":P");
  Serial.print(plus_button);
  Serial.print(":RD");
  Serial.print(redraw_button);
  Serial.print(":ROT");
  Serial.print(rotary);
  Serial.print("||");
  Serial.println(turntable);
  
  
  Wire.beginTransmission(ADDRESS);
  Wire.write((uint8_t)0x00);
  Wire.endTransmission();
  delay(50);
  
}
