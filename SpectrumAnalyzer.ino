#include <StandardCplusplus.h>

#include <algorithm>

const int analogInPin = A0;

uint16_t sample = 0, maxSampleValue = 0;

void setup() {
  // initialize serial communications at 9600 bps:
  Serial.begin(9600);
}

void loop() {
  // read the analog in value:
  sample = analogRead(analogInPin);
  maxSampleValue = std::max(maxSampleValue, sample);
  
  // print the results to the serial monitor:
  Serial.print("sensor = ");
  Serial.println(maxSampleValue);
  
  delay(1);
}
