#pragma once
//----------------------------------------------------------------------rfm69------------------------------------------------------------
// parts copied from: https://github.com/kobuki/RFM69OOK/blob/master/RFM69OOK.cpp, and random forum post here and there and finally it just worked as i wanted it too...
// simple soft spi are good enought for this, write only spi, not any real need to read anything...	
#include "RFM69OOKregisters.h"	

#if defined(use_rfm69)
	
void RFM69_RW(unsigned char uint8_t)
{
  unsigned char i;
  for (i = 0; i < 8; i++)               // output 8-bit
  {
    if (uint8_t & 0x80)
    {
      digitalWrite(MOSIq, 1);    // output 'unsigned char', MSB to MOSI
    }
    else
    {
      digitalWrite(MOSIq, 0);
    }
    digitalWrite(SCKq, 1);                      // Set SCK high..
    uint8_t <<= 1;                         // shift next bit into MSB..
	delayMicroseconds(50);
    //if (digitalRead(MISOq) == 1)
    // {
    //   uint8_t |= 1;                         // capture current MISO bit
    // }
    digitalWrite(SCKq, 0);          // ..then set SCK low again
  }
  //return (uint8_t);                    // return read unsigned char
}

void RFM69_WriteReg(unsigned char reg, unsigned char value)
{
  digitalWrite(CSNq, 0);                   // CSN low, init SPI transaction
  RFM69_RW(reg | 0x80);                   // select register
  RFM69_RW(value);                          // ..and write value to it..
  digitalWrite(CSNq, 1);                   // CSN high again
}



void RFM69OOK_setMode(byte newMode)
{
  switch (newMode) {
    case RF69OOK_MODE_TX:
      RFM69_WriteReg(REG_OPMODE, RF_OPMODE_SEQUENCER_ON | RF_OPMODE_LISTEN_OFF | RF_OPMODE_TRANSMITTER);
	  pinMode(_tx_pin,  OUTPUT);
      break;
    case RF69OOK_MODE_RX:
	  pinMode(_tx_pin,  INPUT);
      RFM69_WriteReg(REG_OPMODE, RF_OPMODE_SEQUENCER_ON | RF_OPMODE_LISTEN_OFF  | RF_OPMODE_RECEIVER);
      break;
    case RF69OOK_MODE_SYNTH:
      RFM69_WriteReg(REG_OPMODE, RF_OPMODE_SEQUENCER_ON | RF_OPMODE_LISTEN_OFF | RF_OPMODE_SYNTHESIZER);
      break;
    case RF69OOK_MODE_STANDBY:
      RFM69_WriteReg(REG_OPMODE, RF_OPMODE_SEQUENCER_ON | RF_OPMODE_LISTEN_OFF | RF_OPMODE_STANDBY);
      break;
    case RF69OOK_MODE_SLEEP:
      RFM69_WriteReg(REG_OPMODE, RF_OPMODE_SEQUENCER_ON | RF_OPMODE_LISTEN_OFF | RF_OPMODE_SLEEP);
      break;
    default: return;
  }
  delay(10);
}


void RFM69OOK_setSensitivityBoost(uint8_t value)
{
  RFM69_WriteReg(REG_TESTLNA, value);
}


void RFM69OOK_setPowerLevel(byte _powerLevel)
{
  RFM69_WriteReg(REG_PALEVEL, 0xA0 | (_powerLevel > 31 ? 31 : _powerLevel));
}	
	
void RFM69OOK_initialize_new()
{	
  pinMode(SCKq, OUTPUT);
  pinMode(CSNq, OUTPUT);
  pinMode(MOSIq,  OUTPUT);
//init io
  digitalWrite(CSNq, 1);                 // Spi disable 	
  delay(1);		
	
  RFM69OOK_setMode(RF69OOK_MODE_STANDBY);
  delay(100);

  uint32_t freqHz = 433.92 * 1000000;
  freqHz /= RF69OOK_FSTEP; // divide down by FSTEP to get FRF
  uint16_t bitrate =3300;
  bitrate = RFM69_XO / bitrate;
  
  const byte CONFIG[][2] =
  {
    /* 0x01 */ { REG_OPMODE, RF_OPMODE_SEQUENCER_ON | RF_OPMODE_LISTEN_OFF | RF_OPMODE_STANDBY },
    /* 0x02 */ { REG_DATAMODUL, RF_DATAMODUL_DATAMODE_CONTINUOUSNOBSYNC | RF_DATAMODUL_MODULATIONTYPE_OOK | RF_DATAMODUL_MODULATIONSHAPING_00 }, //no shaping

  // set new bitrate
			   {REG_BITRATEMSB, (uint8_t)(bitrate >> 8)},
			   {REG_BITRATELSB, (uint8_t)bitrate},

    /* 0x07 */ {REG_FRFMSB, (uint8_t)(freqHz >> 16)},
    /* 0x08 */ {REG_FRFMID, (uint8_t)(freqHz >> 8)},
    /* 0x09 */ {REG_FRFLSB, (uint8_t)freqHz},

        /* 0x0D */ //{ REG_LISTEN1, 0x92 },
        /* 0x0E */ //{ REG_LISTEN2, 0x0C },
        /* 0x0F */ //{ REG_LISTEN3, 0xC8 },

        /* 0x0D */ { REG_LISTEN1, RF_LISTEN1_RESOL_IDLE_4100 | RF_LISTEN1_RESOL_RX_4100 | RF_LISTEN1_END_01},
        /* 0x0E */ { REG_LISTEN2, 2 },//ListenCoefIdle
        /* 0x0F */ { REG_LISTEN3, 35 },//ListenCoefRx


    /* 0x18 */ { REG_LNA, RF_LNA_ZIN_200 | RF_LNA_GAINSELECT_MAX}, //LNA input impedance 200 Ohm, auto gain select
    /* 0x19 */ { REG_RXBW, RF_RXBW_DCCFREQ_010 | RF_RXBW_MANT_16| RF_RXBW_EXP_0 }, 
    /* 0x1B */ { REG_OOKPEAK, RF_OOKPEAK_THRESHTYPE_PEAK | RF_OOKPEAK_PEAKTHRESHSTEP_000 | RF_OOKPEAK_PEAKTHRESHDEC_010 }, //74
    /* 0x1D */ { REG_OOKFIX, 12 }, 
    /* 0x25 */ { REG_DIOMAPPING1, RF_DIOMAPPING1_DIO2_00}, //DIO2 is the only IRQ we're using
    /* 0x26 */ { REG_DIOMAPPING2, RF_DIOMAPPING2_CLKOUT_OFF}, // DIO2 for data transfer
    /* 0x28 */ { REG_IRQFLAGS2, RF_IRQFLAGS2_FIFOOVERRUN }, // Writing to this bit ensures the FIFO & status flags are reset
    /* 0x29 */ { REG_RSSITHRESH, 240}, //must be set to dBm = (-Sensitivity / 2)  Decrease this if you get failed ota uploads or many random reboots, 200 shoule be stable if you have rf noice
    {255, 0}
  };
  

  RFM69_WriteReg(REG_PACKETCONFIG2, 0x02); // kill just incase encryption are set

  for (byte i = 0; CONFIG[i][0] != 255; i++)
    RFM69_WriteReg(CONFIG[i][0], CONFIG[i][1]);

}


	
#endif	
//---------------------------------------------------rfm69---------------------------------------------------------------	