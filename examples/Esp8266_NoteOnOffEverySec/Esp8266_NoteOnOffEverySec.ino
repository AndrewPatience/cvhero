// These need to be included when using standard Ethernet
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include <Wire.h>

#include "AppleMidi.h"

#define I2C_ADDRESS 0x52

char ssid[] = "SHM"; //  your network SSID (name)
char pass[] = "hackmeplease";    // your network password (use for WPA, or use as key for WEP)

unsigned long t0 = millis();
bool isConnected = false;

APPLEMIDI_CREATE_INSTANCE(WiFiUDP, AppleMIDI); // see definition in AppleMidi_Defs.h

// Forward declaration
void OnAppleMidiConnected(uint32_t ssrc, char* name);
void OnAppleMidiDisconnected(uint32_t ssrc);
void OnAppleMidiNoteOn(byte channel, byte note, byte velocity);
void OnAppleMidiNoteOff(byte channel, byte note, byte velocity);

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void setup()
{

  Wire.begin(); //Join i2c bus with SDA=D1 and SCL=D2
  Wire.beginTransmission(I2C_ADDRESS);
  Wire.write(0xF0);
  Wire.write(0x55);
  Wire.endTransmission();
  delay(1);
  Wire.beginTransmission(I2C_ADDRESS);
  Wire.write(0xFB);
  Wire.write((uint8_t)0x00);
  Wire.endTransmission();

  // Serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  Serial.print(F("Getting IP address..."));


  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
  }
  Serial.println(F(""));
  Serial.println(F("WiFi connected"));


  Serial.println();
  Serial.print(F("IP address is "));
  Serial.println(WiFi.localIP());

  Serial.println(F("OK, now make sure you an rtpMIDI session that is Enabled"));
  Serial.print(F("Add device named Arduino with Host/Port "));
  Serial.print(WiFi.localIP());
  Serial.println(F(":5004"));
  Serial.println(F("Then press the Connect button"));
  Serial.println(F("Then open a MIDI listener (eg MIDI-OX) and monitor incoming notes"));

  // Create a session and wait for a remote host to connect to us
  AppleMIDI.begin("test");

  AppleMIDI.OnConnected(OnAppleMidiConnected);
  AppleMIDI.OnDisconnected(OnAppleMidiDisconnected);

  AppleMIDI.OnReceiveNoteOn(OnAppleMidiNoteOn);
  AppleMIDI.OnReceiveNoteOff(OnAppleMidiNoteOff);

  Serial.println(F("Sending NoteOn/Off of note 45, every second"));
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void loop()
{
  // Listen to incoming notes
  AppleMIDI.run();

  Wire.requestFrom (I2C_ADDRESS, 6);
  int count = 0;
  int I2C_Values[6];

  int redraw_button = (~I2C_Values[5] & 31) >> 4;

  while (Wire.available()) {
    I2C_Values[count] = Wire.read();
    count++;
  }

  // send a note every second
  // (dont cáll delay(1000) as it will stall the pipeline)
  //Serial.println(isConnected);
  if (isConnected && (millis() - t0) > 1000)
  {
    t0 = millis();
    //   Serial.print(".");

    byte note = 45;
    byte velocity = 55;
    byte channel = 1;
    Serial.println(I2C_Values[5]);
    Serial.println(redraw_button);
    if (redraw_button == 0) {
      AppleMIDI.sendNoteOn(note, velocity, channel);
      AppleMIDI.sendNoteOff(note, velocity, channel);
    }
  }
}

// ====================================================================================
// Event handlers for incoming MIDI messages
// ====================================================================================

// -----------------------------------------------------------------------------
// rtpMIDI session. Device connected
// -----------------------------------------------------------------------------
void OnAppleMidiConnected(uint32_t ssrc, char* name) {
  isConnected  = true;
  Serial.print(F("Connected to session "));
  Serial.println(name);
}

// -----------------------------------------------------------------------------
// rtpMIDI session. Device disconnected
// -----------------------------------------------------------------------------
void OnAppleMidiDisconnected(uint32_t ssrc) {
  isConnected  = false;
  Serial.println(F("Disconnected"));
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
