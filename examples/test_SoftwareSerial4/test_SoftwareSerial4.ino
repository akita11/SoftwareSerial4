// Test for SoftwareSerial4 library
// (c)Junichi Akita (akita@ifdl.jp)

#include "SoftwareSerial4.h"

// declare SoftwareSerial4 object
// - specify TX&RX pins for N/S/E/W direction
// - order: TxN RxN TxS RxS TxE RxE TxW RxW
SoftwareSerial4 ss4(2, 3, 4, 5, 6, 7, 8, 9);

void setup() {
  Serial.begin(9600);
}

int d = 0;

void loop() {
  int i;
  // check data received for N/S/E/W
  for (i = 0; i < 4; i++){
    if (ss4.available(i) == 1){
      Serial.print('R');
      Serial.print(i); Serial.print(' ');
      Serial.println(ss4.read(i), HEX);
    }
  }

  // send data for N/S/E/W
  Serial.println(d, HEX);
  if (ss4.write(DIR_N, d) == 0) Serial.println("Write error for N");
  if (ss4.write(DIR_S, 0x20 | d) == 0) Serial.println("Write error for S");
  if (ss4.write(DIR_E, 0x40 | d) == 0) Serial.println("Write error for E");
  if (ss4.write(DIR_W, 0x80 | d) == 0) Serial.println("Write error for W");
  d = (d + 1) & 0x1f;
  delay(1000);
}
