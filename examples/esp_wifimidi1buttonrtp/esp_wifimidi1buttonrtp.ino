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
#include <AppleMidi.h>

// I2C Stuff
#define ADDRESS 0x52

APPLEMIDI_CREATE_INSTANCE(WiFiUDP, AppleMIDI); // see definition in AppleMidi_Defs.h
bool AppleMIDIConnected = false;

const char SSID[] = "SHM";           //  your network SSID (name)
const char PASS[] = "hackmeplease";    // your network password

// UDP Multicast declarations for multicast MIDI Port 3. This is the same port used by TouchDAW.
IPAddress ipMulti(225, 0, 0, 37);
const uint16_t MIDI_MCAST_OUT_PORT = 21928 + 2; // Base port + 2

WiFiUDP MultiMIDIOut;

#define MIDI_CHANNEL  (0)

void AppleMIDI_setup()
{
  Serial.println(F("AppleMIDI setup"));

  Serial.println();
  Serial.print(F("IP address is "));
  Serial.println(WiFi.localIP());

  Serial.println(F("OK, now make sure you an rtpMIDI session that is Enabled"));
  Serial.print(F("Add device named Arduino with Host/Port "));
  Serial.print(WiFi.localIP());
  Serial.println(F(":5004"));
  Serial.println(F("Then press the Connect button"));
  Serial.println(F("Then open a MIDI listener (eg MIDI-OX) and monitor incoming notes"));
  delay(500);
  
  // Create a session and wait for a remote host to connect to us
  AppleMIDI.stop();
  AppleMIDI.begin("DJHero");

  AppleMIDI.OnConnected(OnAppleMidiConnected);
  AppleMIDI.OnDisconnected(OnAppleMidiDisconnected);

  AppleMIDI.OnReceiveNoteOn(OnAppleMidiNoteOn);
  AppleMIDI.OnReceiveNoteOff(OnAppleMidiNoteOff);
}

void setup()
{
  Serial.begin(115200);
  Serial.println(F("\DJHero setup"));

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
      AppleMIDI_setup();
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

  // Listen to incoming notes
  AppleMIDI.run();

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
    AppleMIDI.noteOn(0x2b, 0x64, MIDI_CHANNEL);
    Serial.println("Note on!");
  }
  else if (redraw_button == 0) {
    // button released so semd Note Off
    AppleMIDI.noteOff(0x2b, 0x64, MIDI_CHANNEL);
    Serial.println("Note off :(");
  }
  Wire.beginTransmission(ADDRESS);
  Wire.write((uint8_t)0x00);
  Wire.endTransmission();
  delay(50);
  
}

// ====================================================================================
// Event handlers for incoming MIDI messages
// ====================================================================================

// -----------------------------------------------------------------------------
// rtpMIDI session. Device connected
// -----------------------------------------------------------------------------
void OnAppleMidiConnected(uint32_t ssrc, char* name) {
  AppleMIDIConnected  = true;
  Serial.print(F("Apple MIDI connected to session "));
  Serial.println(name);
}

// -----------------------------------------------------------------------------
// rtpMIDI session. Device disconnected
// -----------------------------------------------------------------------------
void OnAppleMidiDisconnected(uint32_t ssrc) {
  AppleMIDIConnected  = false;
  Serial.println(F("Apple MIDI disconnected"));
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void OnAppleMidiNoteOn(byte channel, byte note, byte velocity) {
  Serial.print(F("Incoming NoteOn from channel:"));
  Serial.print(channel);
  Serial.print(F(" note:"));
  Serial.print(note);
  Serial.print(F(" velocity:"));
  Serial.print(velocity);
  Serial.println();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void OnAppleMidiNoteOff(byte channel, byte note, byte velocity) {
  Serial.print(F("Incoming NoteOff from channel:"));
  Serial.print(channel);
  Serial.print(F(" note:"));
  Serial.print(note);
  Serial.print(F(" velocity:"));
  Serial.print(velocity);
  Serial.println();
}

