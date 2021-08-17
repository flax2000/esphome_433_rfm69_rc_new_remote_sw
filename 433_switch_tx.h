



#if defined(use_rfm69)
	#define _tx_pin _rx_pin //rfm69 use same rx as tx pin (DIO3)
#else 
	#define _tx_pin 1		//set tx pin here if you use ordinary transmitter
#endif
//-----------------------NewRemoteSwitch---------------https://github.com/1technophile/NewRemoteSwitch
// adjust the high low ratio to mimic the signal i read form my remote just set all 3 to 260 to get original ratio
//code changed to ignore the sync pulse and just read enought valid signals in a row

unsigned int pulselength;
bool invertedSignal;
uint8_t _repeats=6;
unsigned int _periodusec=260;
unsigned int _periodusec_high=220;
unsigned int _periodusec_low=320;
unsigned long _address;

void _sendBit(boolean isBitOne) {
  if (isBitOne) {
    // Send '1'
    digitalWrite(_tx_pin, HIGH);
    delayMicroseconds(_periodusec_high);
    digitalWrite(_tx_pin, LOW);
    delayMicroseconds(_periodusec * 5);
    digitalWrite(_tx_pin, HIGH);
    delayMicroseconds(_periodusec_high);
    digitalWrite(_tx_pin, LOW);
    delayMicroseconds(_periodusec_low);
  } else {
    // Send '0'
    digitalWrite(_tx_pin, HIGH);
    delayMicroseconds(_periodusec_high);
    digitalWrite(_tx_pin, LOW);
    delayMicroseconds(_periodusec_low);
    digitalWrite(_tx_pin, HIGH);
    delayMicroseconds(_periodusec_high);
    digitalWrite(_tx_pin, LOW);
    delayMicroseconds(_periodusec * 5);
  }
}

void _sendAddress() {
  for (int8_t i=25; i>=0; i--) {
     _sendBit((_address >> i) & 1);
  }
}

void _sendUnit(uint8_t unit) {
  for (int8_t i=3; i>=0; i--) {
     _sendBit(unit & 1<<i);
  }
}

void _sendStopPulse() {
  digitalWrite(_tx_pin, HIGH);
  delayMicroseconds(_periodusec);
  digitalWrite(_tx_pin, LOW);
  delayMicroseconds(_periodusec * 40);
}

  void _sendStartPulse(){
  digitalWrite(_tx_pin, HIGH);
  delayMicroseconds(_periodusec);
  digitalWrite(_tx_pin, LOW);
  delayMicroseconds(_periodusec * 10 + (_periodusec >> 1)); // Actually 10.5T insteat of 10.44T. Close enough.
}
void sendUnit(uint8_t unit, boolean switchOn,unsigned long adress) {
  for (int8_t i = _repeats; i >= 0; i--) {
    _address=adress;
    _sendStartPulse();
    _sendAddress();
    // No group bit
    _sendBit(false);
    // Switch on | off
    _sendBit(switchOn);
    _sendUnit(unit);
    _sendStopPulse();
  }}
 
  void sendDim(uint8_t unit, uint8_t dimLevel,unsigned long adress) {
	for (int8_t i = _repeats; i >= 0; i--) {
		_address=adress;
		
		_sendStartPulse();

		_sendAddress();

		// No group bit
		_sendBit(false);
//if we have dimmlevel send state 2 =dimmer..		
if(dimLevel)
{
		// Switch type 'dim'
		digitalWrite(_tx_pin, HIGH);
		delayMicroseconds(_periodusec);
		digitalWrite(_tx_pin, LOW);
		delayMicroseconds(_periodusec);
		digitalWrite(_tx_pin, HIGH);
		delayMicroseconds(_periodusec);
		digitalWrite(_tx_pin, LOW);
		delayMicroseconds(_periodusec);
}
else// if no dimmlevel send state 0 = off, sending dimmer 0 and state 2 leaves my dimmer turned on, on the lowest level
{
_sendBit(0);	
	
}
		_sendUnit(unit);
//0-15 dimm
		for (int8_t j=3; j>=0; j--) {
		   _sendBit(dimLevel & 1<<j);
		}

		_sendStopPulse();
	}
}

 
 //-------------------------------------------RCSwitch https://github.com/sui77/rc-switch/blob/master/RCSwitch.cpp------------------------------------------------------


void RCSwitch_transmit(uint8_t _high,uint8_t _low) {
	
  uint8_t firstLogicLevel = (invertedSignal) ? LOW : HIGH;
  uint8_t secondLogicLevel = (invertedSignal) ? HIGH : LOW;
 
  digitalWrite(_tx_pin, firstLogicLevel);
  delayMicroseconds( pulselength * _high);
  digitalWrite(_tx_pin, secondLogicLevel);
  delayMicroseconds( pulselength * _low);
}

void RCSwitch_send(uint32_t code,unsigned int protocol_, int length) {
	
	    struct HighLow {
        uint8_t high;
        uint8_t low;
        };

    struct Protocol {
        /** base pulse length in microseconds, e.g. 350 */
        uint16_t pulseLength;
        HighLow syncFactor;
        HighLow zero;
        HighLow one;
        bool invertedSignal;
    };
	
static const Protocol proto[] = {
  { 350, {  1, 31 }, {  1,  3 }, {  3,  1 }, false },    // protocol 1
  { 650, {  1, 10 }, {  1,  2 }, {  2,  1 }, false },    // protocol 2
  { 100, { 30, 71 }, {  4, 11 }, {  9,  6 }, false },    // protocol 3
  { 380, {  1,  6 }, {  1,  3 }, {  3,  1 }, false },    // protocol 4
  { 500, {  6, 14 }, {  1,  2 }, {  2,  1 }, false },    // protocol 5
  { 450, { 23,  1 }, {  1,  2 }, {  2,  1 }, true },     // protocol 6 (HT6P20B)
  { 150, {  2, 62 }, {  1,  6 }, {  6,  1 }, false },    // protocol 7 (HS2303-PT, i. e. used in AUKEY Remote)
  { 200, {  3, 130}, {  7, 16 }, {  3,  16}, false},     // protocol 8 Conrad RS-200 RX
  { 200, { 130, 7 }, {  16, 7 }, { 16,  3 }, true},      // protocol 9 Conrad RS-200 TX
  { 365, { 18,  1 }, {  3,  1 }, {  1,  3 }, true },     // protocol 10 (1ByOne Doorbell)
  { 270, { 36,  1 }, {  1,  2 }, {  2,  1 }, true },     // protocol 11 (HT12E)
  { 320, { 36,  1 }, {  1,  2 }, {  2,  1 }, true }      // protocol 12 (SM5212)
};	
		
const Protocol &pro = proto[protocol_-1];
pulselength=pro.pulseLength;
invertedSignal=pro.invertedSignal;
  for (int nRepeat = 0; nRepeat < _repeats; nRepeat++) {
    for (int i = length-1; i >= 0; i--) {
      if (code & (1L << i))
        RCSwitch_transmit(pro.one.high,pro.one.low);
      else
        RCSwitch_transmit(pro.zero.high,pro.zero.low);
    }
	RCSwitch_transmit(pro.syncFactor.high,pro.syncFactor.low);
  }

  // Disable transmit after sending (i.e., for inverted protocols)
  digitalWrite(_tx_pin, LOW);

}

