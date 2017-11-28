#include "SoftwareSerial4.h"

SoftwareSerial4 ss4(2, 3, 4, 5, 6, 7, 8, 9);

void setup() {
  ss4.write(DIR_N, 0x55);
  Serial.begin(9600);
}

void loop() {
  int i;
  for (i = 0; i < 4; i++){
    if (ss4.available(i) == 1){
      Serial.print(i); Serial.print(' ');
      Serial.println(ss4.read(i), HEX);
    }
  }
}
