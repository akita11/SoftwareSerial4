#include "arduino.h"
#include <pins_arduino.h>
#include <avr/interrupt.h>
#include "SoftwareSerial4.h"

SoftwareSerial4::SoftwareSerial4(int TxPinN, int RxPinN, int TxPinS, int RxPinS, int TxPinE, int RxPinE, int TxPinW, int RxPinW)
{
  uint8_t i;
  _TxPin[DIR_N] = TxPinN; _RxPin[DIR_N] = RxPinN;
  _TxPin[DIR_S] = TxPinS; _RxPin[DIR_S] = RxPinS;
  _TxPin[DIR_E] = TxPinE; _RxPin[DIR_E] = RxPinE;
  _TxPin[DIR_W] = TxPinW; _RxPin[DIR_W] = RxPinW;
  for (i = 0; i < 4; i++){
    _pRxRead[i] = 0; _pRxWrite[i] = 0; _pTxRead[i] = 0; _pTxWrite[i] = 0;
    _TxStatus[i] = 0; _RxStatus[i] = 0;
    pinMode(_TxPin[i], OUTPUT); digitalWrite(_TxPin[i], HIGH);
    pinMode(_RxPin[i], INPUT); digitalWrite(_RxPin[i], HIGH);
  }
  TCCR1A = 0; // clear control register A 
  TCCR1B = _BV(WGM13); // set mode as phase and frequency correct pwm, stop the timer
  //  ICR1 = 2804;
  //  ICR1 = 3300; // 8MHz/2/3300=1.2kHz:825us / 825us*3*8=19.8ms/frame
  ICR1 = 3500;
  //  ICR1 = 3840; // 23ms/frame (43.4fps)
  TCCR1B &= ~(_BV(CS10) | _BV(CS11) | _BV(CS12));
  TCCR1B |= _BV(CS10); // no prescaler
  TIMSK1 = _BV(TOIE1); // sets the timer overflow interrupt enable bit

  SoftwareSerial4::active_object = this;
}

SoftwareSerial4 *SoftwareSerial4::active_object = 0;

ISR(TIMER1_OVF_vect)
{
  if(SoftwareSerial4::active_object){
    SoftwareSerial4::active_object->_process();
  }
}

void SoftwareSerial4::_process()
{
  uint8_t i;
  for (i = 0; i < 4; i++){
    // RX
    if (_RxStatus[i] == 0 && digitalRead(_RxPin[i]) == 0){
      _RxStatus[i] = 1;
      _RxBuf[i][_pRxWrite[i]] = 0;
    }
    else if (_RxStatus[i] != 0){
      _RxStatus[i]++;
      if ((_RxStatus[i] % COUNT_BIT) == (COUNT_BIT / 2)){
	if (_RxStatus[i] > COUNT_BIT){
	  if (_RxStatus[i] < COUNT_BIT * 9){
	    _RxBuf[i][_pRxWrite[i]] = _RxBuf[i][_pRxWrite[i]] >> 1;
	    if (digitalRead(_RxPin[i]) == 1) _RxBuf[i][_pRxWrite[i]] |= 0x80;
	    else  _RxBuf[i][_pRxWrite[i]] &= ~0x80;
	  }
	  else{
	    // check stop bit
	    if (digitalRead(_RxPin[i]) != 1){
	      _RxStatus[i] = 0;
	    }
	  }
	}
      }
      if (_RxStatus[i] == COUNT_BIT * 10){
	_RxStatus[i] = 0;
	_pRxWrite[i] = (_pRxWrite[i] + 1) % RXBUF_SIZE;
      }
    }

    // TX
    if (_pTxRead[i] != _pTxWrite[i] && _TxStatus[i] == 0)
      _TxStatus[i] = 1;
    if (_TxStatus[i] != 0){
      _TxStatus[i]++;
      if (_TxStatus[i] == 2){
	_TxSendingData = _TxBuf[i][_pTxRead[i]];
	_pTxRead[i] = (_pTxRead[i] + 1) % TXBUF_SIZE;
	digitalWrite(_TxPin[i], 0); // start bit
      }
      else if ((_TxStatus[i] % COUNT_BIT) == 2){
	if (_TxStatus[i] == (COUNT_BIT * 9 + 2)) digitalWrite(_TxPin[i], 1); // stop bit
	else{
	  if ((_TxSendingData & 0x01) != 0) digitalWrite(_TxPin[i], 1);
	  else digitalWrite(_TxPin[i], 0);
	  _TxSendingData = _TxSendingData >> 1;
	}
      }
      if (_TxStatus[i] == COUNT_BIT * 10 + 2){
	_TxStatus[i] = 0;
	digitalWrite(_TxPin[i], 1);
      }
    }
  }
}

int SoftwareSerial4::write(uint8_t dir, uint8_t dat)
{
  // _pTxWrite 0--->1->2->3->......->0->1->2->3(=overrun)
  // _pTxRead  0----(TX)---->1-(TX)->2
  // ToDo: check buffer full based on buffer size
  if (_pTxWrite[dir] >= _pTxRead[dir]){ // buffer available
    _TxBuf[dir][_pTxWrite[dir]] = dat;
    _pTxWrite[dir] = (_pTxWrite[dir] + 1) % TXBUF_SIZE;
    return(0); // OK
  }
  else return(-1); // NG (buffer overrun)
}

uint8_t SoftwareSerial4::available(uint8_t dir)
{
  if (_pRxRead[dir] != _pRxWrite[dir]) return(1); else return(0);
}

int SoftwareSerial4::read(uint8_t dir)
{
  uint8_t dat;
  if (available(dir) == 1){
    dat = _RxBuf[dir][_pRxRead[dir]];
    _pRxRead[dir] = (_pRxRead[dir] + 1) % RXBUF_SIZE;
    return(dat);
  }
  else return(-1);
}
