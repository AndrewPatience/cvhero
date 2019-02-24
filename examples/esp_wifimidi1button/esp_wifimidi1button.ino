/*
 *******************************************************************************
   MIDI WiFi 1 button controller or keyboard

   Copyright (C) 2017 gdsport625@gmail.com

   Use with any ESP8266. Tested using NodeMCU devkit 1.0 because it has a button on GPIO0.
   Use with any ESP32. Tested using ESP Dev Module because it has a button on IO0.

   DIN, UART, and USB hardware are NOT used. All MIDI is sent over WiFi.

   Supports MIDI over multicast UDP (ipMidi, qmidinet, multimidinet, etc.)

   USB MIDI -> MIDI muliticast UDP

   Uses for a 1 button MIDI controller.
   1. Send ALL NOTES OFF on all channels. Sometimes called a MIDI panic button.
   2. Send Control Change messages for pedals such as Sustain/Damper. It might be possible
   to install the ESP and battery inside a foot pedal. No wires to trip over.
   3. A 1 key piano? Of course, adding more buttons is easy. Arcade buttons look very useful.

// Debounce buttons and switches, https://github.com/thomasfredericks/Bounce2/wiki
// Define the following here or in Bounce2.h. This make button change detection more responsive.
*/

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ESP8266WiFiMulti.h>
ESP8266WiFiMulti wifiMulti;
#elif defined(ESP32)
#include <WiFi.h>
#include <WiFiUdp.h>
#include <WiFiMulti.h>
WiFiMulti wifiMulti;
#else
#error This works only on ESP8266 or ESP32
#endif

#include <Wire.h>
// I2C Stuff
#define ADDRESS 0x52

const char SSID[] = "SHM";           //  your network SSID (name)
const char PASS[] = "hackmeplease";    // your network password

// UDP Multicast declarations for multicast MIDI Port 3. This is the same port used by TouchDAW.
IPAddress ipMulti(225, 0, 0, 37);
const uint16_t MIDI_MCAST_OUT_PORT = 21928 + 2; // Base port + 2

WiFiUDP MultiMIDIOut;

#define MIDI_CHANNEL  (0)

void MIDI_MULTICAST_write(const uint8_t *outbuf, size_t size)
{
#if defined(ESP8266)
  MultiMIDIOut.beginPacketMulticast(ipMulti, MIDI_MCAST_OUT_PORT, WiFi.localIP());
#elif defined(ESP32)
  MultiMIDIOut.beginMulticastPacket();
#endif

  MultiMIDIOut.write(outbuf, size);
  MultiMIDIOut.endPacket();
}

// Various MIDI messages
uint8_t NOTE_ON[] = {0x90 | MIDI_CHANNEL, 0x2b, 0x64};
uint8_t NOTE_OFF[] = {0x80 | MIDI_CHANNEL, 0x2b, 0x64};
uint8_t ALL_NOTES_OFF[] = {0xb0, 0x7b, 0x00};
uint8_t SUSTAIN_ON[] = {0xB0 | MIDI_CHANNEL, 0x40, 0x7F};
uint8_t SUSTAIN_OFF[] = {0xB0 | MIDI_CHANNEL, 0x40, 0x00};

void setup()
{
  Serial.begin(115200);
  Serial.println(F("\nmidi1button setup"));

  wifiMulti.addAP(SSID, PASS);
  // Add more SSID and passwords. WiFiMulti will connect to the available AP
  // with the strongest signal.
  //wifiMulti.addAP("ssid_from_AP_2", "your_password_for_AP_2");
  //wifiMulti.addAP("ssid_from_AP_3", "your_password_for_AP_3");

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
  static bool connected = false;

  if (wifiMulti.run() == WL_CONNECTED) {
    if (!connected) {
      Serial.println(F("WiFi connected!"));
#if defined(ESP32)
      MultiMIDIOut.stop();
      int rc = MultiMIDIOut.beginMulticast(ipMulti, MIDI_MCAST_OUT_PORT);
      Serial.print("beginMulticast returned "); Serial.println(rc);
#endif
      connected = true;
    }
  }
  else {
    if (connected) {
      Serial.println(F("WiFi not connected!"));
      connected = false;
    }
    delay(500);
  }

  // DJHero stuff
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
  
  Serial.println(redraw_button);
  // 1 Key piano
  if (redraw_button == 1) {
    // button pressed so send Note On
    MIDI_MULTICAST_write(NOTE_ON, sizeof(NOTE_ON));
    Serial.println("Note on!");
  }
  else if (redraw_button == 0) {
    // button released so semd Note Off
    MIDI_MULTICAST_write(NOTE_OFF, sizeof(NOTE_OFF));
    Serial.println("Note off :(");
  }
  Wire.beginTransmission(ADDRESS);
  Wire.write((uint8_t)0x00);
  Wire.endTransmission();
  delay(50);
  
}
