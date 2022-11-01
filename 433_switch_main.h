//code from https://github.com/flax2000/esphome_433_rfm69_rc_new_remote_sw
#include "esphome.h"
//remove if you dont use rfm69 >> // #define use_rfm69
//#define use_rfm69   

#define _rx_pin 3 //connect this to DIO2 on rfm69 or data pin on ordinary reciever

#if defined (use_rfm69) 
#define CSNq      0  //(nss)        //q are added just to make sure it dont mess with ordinary spi things
// CSN BIT:  Digital Input     SPI Chip Select
#define SCKq      2 
// SCK BIT:  Digital Input     SPI Clock
#define MOSIq     1 
// MOSI BIT: Digital Input     SPI Slave Data Input
#endif

char have_data = 0;
std::string my_data;


unsigned long send_NRS_adress[4] = {0};
char send_NRS_unit[4] = {0};
char send_NRS_state[4] = {0};
int send_NRS_dimmer[4] = {0};

unsigned long recieve_NRS_adress = 0;
char recieve_NRS_unit = 0;
char recieve_NRS_state = 0;
char recieve_NRS_dimmer = 0;
unsigned long recieve_rc_adress = 0;
char recieve_rc_protocol = 0;

unsigned long send_rc_adress[4] = {0};
char send_rc_lenght[4] = {0};
char send_rc_protocol[4] = {0};


//-----------------------------------------------------new remote switch------------------------------------------------------------------------------
//i have modefied the reciever code to ignore the long sync pulse and just listen for enought valid pulses in a row, this helped me pick up my wall switches better, appears to work with all the other transmitters i have too


struct NewRemoteCode {
  uint32_t address;    // Address of received code. [0..2^26-1]
  boolean groupBit;     // Group bit set or not
  byte switchType;    // off, on, dim, on_with_dim.
  byte unit;          // Unit code of received code [0..15]
  byte dimLevel;        // Dim level [0..15]. Will be available if switchType is dim, on_with_dim or off_with_dim.
};





  void ICACHE_RAM_ATTR  NRS_rx_(uint16_t duration)
{
	
static uint16_t duration_old;	
	

//here you can change the tolerance for valid signals IS_ONE_PULSE should be around 260 and IS_FIVE_PULSE 260*5= 1300

#define IS_ONE_PULSE(interval)   (interval >= 50 && interval <= 600)
#define IS_FIVE_PULSE(interval)  (interval >= 1000 && interval <= 1700)
#define RESET_STATE _state = 0
  static uint8_t receivedBit = 0;
  static int8_t _state = 0;
  static boolean wait_next_NRS;
  static NewRemoteCode receivedCode;    // Contains received code
  if (wait_next_NRS) wait_next_NRS = false;

  else
  {

    if ( (_state % 2) == 0)
    {

      // There are 3 valid options for receivedBit:
      // 0, indicated by short short short long == B0001.
      // 1, short long shot short == B0100.
      // dim, short shot short shot == B0000.
      // Everything else: inconsistent data, trash the whole sequence.


      if (_state < 53) {
        // States 2 - 105 are address bit states

        receivedCode.address <<= 1;

        // Decode bit. Only 4 LSB's of receivedBit are used; trim the rest.
        switch (receivedBit & B11) {
          case B10: // Bit "0" received.
            // receivedCode.address |= 0; But let's not do that, as it is wasteful.
            break;
          case B01: // Bit "1" received.
            receivedCode.address |= 1;
            break;
          default: // Bit was invalid. Abort.
            RESET_STATE;

        }
      } else if (_state < 55) {
        // States 106 - 109 are group bit states.
        switch (receivedBit & B11) {
          case B10: // Bit "0" received.
            receivedCode.groupBit = false;
            break;
          case B01: // Bit "1" received.
            receivedCode.groupBit = true;
            break;
          default: // Bit was invalid. Abort.
            RESET_STATE;

        }
      } else if (_state < 57) {
        // States 110 - 113 are switch bit states.
        switch (receivedBit & B11) {
          case B10: // Bit "0" received.
            receivedCode.switchType = 0;
            break;
          case B01: // Bit "1" received. Note: this might turn out to be a on_with_dim signal.
            receivedCode.switchType = 1;
            break;
          case B11: // Bit "dim" received.
            receivedCode.switchType = 2;
            break;
          default: // Bit was invalid. Abort.
            RESET_STATE;

        }
      } else if (_state < 65) {
        // States 114 - 129 are unit bit states.
        receivedCode.unit <<= 1;

        // Decode bit.
        switch (receivedBit & B11) {
          case B10: // Bit "0" received.
            // receivedCode.unit |= 0; But let's not do that, as it is wasteful.
            break;
          case B01: // Bit "1" received.
            receivedCode.unit |= 1;
            break;
          default: // Bit was invalid. Abort.
            RESET_STATE;

        }

      } else if (_state < 73) {
        // States 130 - 145 are dim bit states.
        // Depending on hardware, these bits can be present, even if switchType is NewRemoteCode::on or NewRemoteCode::off

        receivedCode.dimLevel <<= 1;

        // Decode bit.
        switch (receivedBit & B11) {
          case B10: // Bit "0" received.
            // receivedCode.dimLevel |= 0; But let's not do that, as it is wasteful.
            break;
          case B01: // Bit "1" received.
            receivedCode.dimLevel |= 1;
            break;
          default: // Bit was invalid. Abort.
             RESET_STATE;

        }
      }

    }

    if (IS_ONE_PULSE(duration_old) && IS_FIVE_PULSE(duration))
    {
      receivedBit <<= 1;
      _state++;
      wait_next_NRS = true;
    }
    else if (IS_ONE_PULSE(duration_old) && IS_ONE_PULSE(duration))
    {
      receivedBit <<= 1;
      _state++;
      receivedBit |= 1;
      wait_next_NRS = true;
    }


    else  {
		if (_state == 64 || _state == 72)
      
      {			
          extern std::string my_data;
          my_data = "recieved address ";
          my_data += to_string(receivedCode.address & 0x3FFFFFF);

          my_data += "unit ";
          my_data += to_string(receivedCode.unit & B1111);

          my_data += "switchType ";
          my_data += to_string(receivedCode.switchType);
          if (receivedCode.switchType == 2 || receivedCode.dimLevel != 0)
          {
            my_data += "dimm ";
            my_data += to_string(receivedCode.dimLevel & B1111);
          }
          extern char have_data;
          have_data = 1;

          extern unsigned long recieve_NRS_adress;
          recieve_NRS_adress = receivedCode.address & 0x3FFFFFF;
          extern char recieve_NRS_unit;
          recieve_NRS_unit = receivedCode.unit & B1111;
          extern char recieve_NRS_state;
          recieve_NRS_state = receivedCode.switchType;
          extern char recieve_NRS_dimmer;
          recieve_NRS_dimmer = receivedCode.dimLevel & B1111;	
		  receivedCode.dimLevel=0;	
		
      }
      _state = 0;
    }




  }

  duration_old = duration;	
	
	


}

//--------------------------------------------------------------RCSWITCH------------------------------------------------------
#define RCSWITCH_MAX_CHANGES 67
uint16_t timings[RCSWITCH_MAX_CHANGES];
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


static inline unsigned int diff(int A, int B) {
  return abs(A - B);
}

 bool ICACHE_RAM_ATTR  receiveProtocol(const int p, unsigned int changeCount) {

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

  const Protocol &pro = proto[p - 1];
  int16_t nReceiveTolerance = 90;  //adjust the tolerance here 60 original value, 120 might be too high decrease if you get strange signals...
  unsigned long code = 0;
  //Assuming the longer pulse length is the pulse captured in timings[0]
  const uint16_t syncLengthInPulses =  ((pro.syncFactor.low) > (pro.syncFactor.high)) ? (pro.syncFactor.low) : (pro.syncFactor.high);
  const uint16_t delay = timings[0] / syncLengthInPulses;
  const uint16_t delayTolerance = delay * nReceiveTolerance / 100;

  /* For protocols that start low, the sync period looks like
                   _________
     _____________|         |XXXXXXXXXXXX|

     |--1st dur--|-2nd dur-|-Start data-|

     The 3rd saved duration starts the data.

     For protocols that start high, the sync period looks like

      ______________
     |              |____________|XXXXXXXXXXXXX|

     |-filtered out-|--1st dur--|--Start data--|

     The 2nd saved duration starts the data
  */
  const uint16_t firstDataTiming = (pro.invertedSignal) ? (2) : (1);

  for (uint16_t i = firstDataTiming; i < changeCount - 1; i += 2) {
    code <<= 1;
    if (diff(timings[i], delay * pro.zero.high) < delayTolerance &&
        diff(timings[i + 1], delay * pro.zero.low) < delayTolerance) {
      // zero
    } else if (diff(timings[i], delay * pro.one.high) < delayTolerance &&
               diff(timings[i + 1], delay * pro.one.low) < delayTolerance) {
      // one
      code |= 1;
    } else {
      // Failed
      return false;
    }
  }


if(code>100)// this will ignore 0 or 10 or ... sinals....
{
  extern unsigned long recieve_rc_adress;
  extern  char recieve_rc_protocol;
  recieve_rc_adress = code;
  recieve_rc_protocol = p;
  extern std::string my_data;
  my_data = "recieved address ";
  my_data += to_string(code);
  my_data += " lenght ";
  my_data += to_string(((changeCount - 1) / 2));
  my_data += " protocol ";
  my_data += to_string(p);
  extern char have_data;
  have_data = 1;
}
  
  return true;
}


 void ICACHE_RAM_ATTR  rc_sw_rx_(uint16_t duration)
{
  const uint16_t nSeparationLimit = 4300; //change this depending on the protocol you need to recieve mayby 1500-12000
  static uint16_t changeCount = 0;
  static uint16_t repeatCount = 0;
  
  if (duration > nSeparationLimit) {
    // A long stretch without signal level change occurred. This could
    // be the gap between two transmission.
    if ((repeatCount == 0) || (diff(duration, timings[0]) < 200)) {
      // This long signal is close in length to the long signal which
      // started the previously recorded timings; this suggests that
      // it may indeed by a a gap between two transmissions (we assume
      // here that a sender will send the signal multiple times,
      // with roughly the same gap between them).
      repeatCount++;
      if (repeatCount == 2) {

        if (((changeCount - 1) / 2) > 22) //only check for codes with lenght over 22, decrease if you need to catch shorter codes.
        {

          // /*                                              check all protocols or just the one you need? <----------------------------------------------
          for (uint16_t i = 1; i <= 12; i++) {
            if (receiveProtocol(i, changeCount)) {
              // receive succeeded for protocol i
              break;
            }
          }
		  // */
          //receiveProtocol(4, changeCount);//only check 4 to save interupt time
        }
        repeatCount = 0;
      }
    }
    changeCount = 0;
  }

  // detect overflow
  if (changeCount >= RCSWITCH_MAX_CHANGES) {
    changeCount = 0;
    repeatCount = 0;
  }
  timings[changeCount++] = duration;
}


//-------------------------------------------------ext_int_1----------------------------------------------------------------------
     void ICACHE_RAM_ATTR  ext_int_1()
    {
		
      static unsigned long edgeTimeStamp[3] = {0, };  // Timestamp of edges
      static bool skip;
      // Filter out too short pulses. This method works as a low pass filter.
      edgeTimeStamp[1] = edgeTimeStamp[2];
      edgeTimeStamp[2] = micros();

      if (skip) {
        skip = false;
        return;
      }

      if (edgeTimeStamp[2] - edgeTimeStamp[1] < 50) { 
        // Last edge was too short.
        // Skip this edge, and the next too.
        skip = true;
        return;
      }

      uint16_t duration = edgeTimeStamp[1] - edgeTimeStamp[0];
      edgeTimeStamp[0] = edgeTimeStamp[1];

      
      NRS_rx_(duration);
      rc_sw_rx_(duration);
    }



//------------------------------------------------------------------Last_sent_received------------------------------------------------------



class Last_sent_received : public PollingComponent, public TextSensor {
  public:
#include "433_switch_tx.h"
#include "rfm69_control.h"

    uint16_t interupt_start_delay = 200; // wait before starting radio and interupts

     Last_sent_received() : PollingComponent(50) {}

    void setup() override {


#if defined(ESP8266) // kill serial if rx/tx pins are used, like on esp01
if(_tx_pin==1||_tx_pin==3||_rx_pin==1||_rx_pin==3)
{
Serial.end();		
}
#endif

  #if defined(use_rfm69) 
  pinMode(_tx_pin, INPUT); //just to make sure...
  #else 
  pinMode(_tx_pin, OUTPUT); 
  #endif
   	  
    }
    void update() override {
      extern std::string my_data;
      extern char have_data;
     

      if (interupt_start_delay > 0)//wait for esp to start before attaching interupt
      {
		        
        interupt_start_delay--;
        if (interupt_start_delay == 0)
        {
						
#if defined(use_rfm69)
	RFM69OOK_initialize_new();			  
	delay(1);
	RFM69OOK_setPowerLevel(31);// set output power: 0=min, 31=max
	RFM69OOK_setMode(RF69OOK_MODE_RX);
	
#endif 				
interupt_start_delay=0;												
					
			
          attachInterrupt(_rx_pin, ext_int_1, CHANGE);
        }
      }



      if (have_data == 1)//we have data from recievers
      {
        have_data = 0;
        publish_state(my_data);
      }

      extern unsigned long send_NRS_adress[4] ;
      extern char send_NRS_unit[4];
      extern char send_NRS_state[4];
      extern int send_NRS_dimmer[4];

for (int i = 0; i <= 3; i++)
{
      if (send_NRS_adress[i])//we have data from NRS
      {
        detachInterrupt(_rx_pin);
		#if defined(use_rfm69) 
		RFM69OOK_setMode(RF69OOK_MODE_TX);
		#endif
        if (send_NRS_state[i] == 2 ) {
          sendDim(send_NRS_unit[i], send_NRS_dimmer[i], send_NRS_adress[i]);
        }
        else {
			
			 sendUnit(send_NRS_unit[i], send_NRS_state[i], send_NRS_adress[i]);							

        }

        my_data = "send address ";
        my_data += to_string(send_NRS_adress[i]);

        my_data += "unit ";
        my_data += to_string(send_NRS_unit[i]);

        my_data += "switchType ";
        my_data += to_string(send_NRS_state[i]);
        if (send_NRS_state[i] == 2)
        {
          my_data += " dimmer ";
          my_data += to_string(send_NRS_dimmer[i]);
        }
        publish_state(my_data);
        send_NRS_adress[i] = 0;
		#if defined(use_rfm69) 
		RFM69OOK_setMode(RF69OOK_MODE_RX);
		#endif
        attachInterrupt(_rx_pin, ext_int_1, CHANGE);
		break; // only send 1 signal per loop
      }
	  
}
      extern unsigned long send_rc_adress[4];
      extern char send_rc_protocol[4];
      extern char send_rc_lenght[4];

  
   for (int i = 0; i <= 3; i++)
   {
      if (send_rc_adress[i]) //we have data from rc switches
      {
        detachInterrupt(_rx_pin);
		#if defined(use_rfm69) 
		RFM69OOK_setMode(RF69OOK_MODE_TX);
		#endif
        RCSwitch_send(send_rc_adress[i], send_rc_protocol[i], send_rc_lenght[i]);

        my_data = "send address ";
        my_data += to_string(send_rc_adress[i]);
        my_data += "lenght ";
        my_data += to_string(send_rc_lenght[i]);
        publish_state(my_data);
        send_rc_adress[i] = 0;
		#if defined(use_rfm69) 
		RFM69OOK_setMode(RF69OOK_MODE_RX);
		#endif
        attachInterrupt(_rx_pin, ext_int_1, CHANGE);
		break;// only send 1 signal per loop
      }
	  
   }
      // This will be called every "update_interval" milliseconds.
      // Publish state
	  
    }
};

//--------------------------------------------------------------------------------------------------------------------

class MyCustomComponent : public Component, public CustomAPIDevice {
 public:
  void setup() override {


	
	    register_service(&MyCustomComponent::on_NRS_tx, "NRS_tx_manual",
                     {"adress", "unit","state"});
					 
		register_service(&MyCustomComponent::on_RC_tx, "RC_tx_manual",
                     {"adress", "protocol","lenght"});			 
	
  }

  void on_NRS_tx(std::string adress, int unit,int state) {
    ESP_LOGD("custom", "NRS data recieved %s", adress.c_str());   
      extern unsigned long send_NRS_adress[4];
      extern char send_NRS_unit[4];
      extern char send_NRS_state[4];
	  extern int send_NRS_dimmer[4];
	  int dimmer_send=0;
	  if(state>1)
	  {
		  dimmer_send=state-2;
		  if(dimmer_send>15)
		  { 
			  dimmer_send=15;
		  }		  
		  state=2;  		  
	  }

	  	  for (int i = 0; i <= 3; i++)
	  {
		  if(send_NRS_adress[i]==0)
		  {
	  send_NRS_dimmer[i]=dimmer_send;
	  send_NRS_state[i] = state;	  	  
	  send_NRS_unit[i] = unit;
      send_NRS_adress[i] = atol(adress.c_str());
	  break;
		  }
	  }
	  

  }

  void on_RC_tx(std::string adress, int protocol,int lenght) {
    ESP_LOGD("custom", "rc data recieved %s", adress.c_str());   
      extern unsigned long send_rc_adress[4];
      extern char send_rc_protocol[4];
      extern char send_rc_lenght[4];

for (int i = 0; i <= 3; i++)
{
        if(send_rc_adress[i]==0)
		{
        send_rc_protocol[i] = protocol;
        send_rc_lenght[i] = lenght;
		send_rc_adress[i] = atol(adress.c_str());
		break;
		}
}

  }

};

















