#include <inttypes.h>

#ifndef SOFTWARESERIAL4_H
#define SOFTWARESERIAL4_H

#define DIR_N 0
#define DIR_S 1
#define DIR_E 2
#define DIR_W 3

#define RXBUF_SIZE 8
#define TXBUF_SIZE 8

#define COUNT_BIT 4

class SoftwareSerial4
{
 private:
  int _TxPin[4], _RxPin[4];
  uint8_t _pRxRead[4];
  uint8_t _pRxWrite[4];
  uint8_t _pTxRead[4];
  uint8_t _pTxWrite[4];
  uint8_t _TxBuf[4][TXBUF_SIZE];
  uint8_t _RxBuf[4][RXBUF_SIZE];
  volatile uint8_t _TxStatus[4]; // tx status (0=idle, 1-4=start bit, 5-36=data, 37-40=stop bit)
  volatile uint8_t _RxStatus[4]; // rx status (0=idle, 1-4=start bit, 5-36=data, 37-40=stop bit)
  volatile uint8_t _TxRxStatus;
 
 public:
  static SoftwareSerial4 *active_object;
  void _process();
  SoftwareSerial4(int TxPinN, int RxPinN,
		  int TxPinS, int RxPinS,
		  int TxPinE, int RxPinE,
		  int TxPinW, int RxPinW);
  void write(uint8_t dir, uint8_t dat);
  uint8_t available(uint8_t dir);
  int read(uint8_t dir); // return -1 if empty

};

#endif

