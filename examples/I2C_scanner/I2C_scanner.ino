// Robo India Tutorial 
// Digital Input and Output on LED 
// Hardware: NodeMCU

// Ref- Nick Gammon  http://www.gammon.com.au/forum/?id=10896
// I2C Scanner

#include <Wire.h>

void setup() {
  Serial.begin (115200); 
  while (!Serial) // Waiting for serial connection
    {
    }

  Serial.println ();
  
  Wire.begin();
}
void loop() {
  Serial.println ("I2C scanner. Scanning ...");
  byte count = 0;
  for (byte i = 8; i < 120; i++)
  {
    Wire.beginTransmission (i);
    if (Wire.endTransmission () == 0)
      {
      Serial.print ("Found address: ");
      Serial.print (i, DEC);
      Serial.print (" (0x");
      Serial.print (i, HEX);
      Serial.println (")");
      count++;
      delay (1);  // maybe unneeded?
      } // end of good response
  } // end of for loop
  Serial.println ("Done.");
  Serial.print ("Found ");
  Serial.print (count, DEC);
  Serial.println (" device(s).");
  delay(200);
}

