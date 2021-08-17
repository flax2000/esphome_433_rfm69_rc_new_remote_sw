
//-----------------------------------------------------------------------------------------------------------
class RC_remote_switch : public Component, public Switch {
  public:
    int counter = 0;
    uint32_t  adress_on;
    uint32_t  adress_off;
    uint8_t  protocol;
    uint8_t  lenght;

    RC_remote_switch(uint32_t  adress_on_, uint32_t  adress_off_, uint8_t protocol_, uint8_t lenght_) {
      adress_on = adress_on_;
      adress_off = adress_off_;
      protocol = protocol_;
      lenght = lenght_;
    }

    void loop() override
    {
      counter++;
      if (counter > 5)  //no point in checking every loop, counter>5 checks around 10 times /sec
      {
        extern unsigned long recieve_rc_adress;
        extern char recieve_rc_protocol;
        if (recieve_rc_adress == adress_on && recieve_rc_protocol == protocol )
        {
          publish_state(1);
          recieve_rc_adress = 0;
        }
        if (recieve_rc_adress == adress_off && recieve_rc_protocol == protocol )
        {
          publish_state(0);
          recieve_rc_adress = 0;
        }
        counter = 0;
      }

    }

    void write_state(bool state) override {
      extern unsigned long send_rc_adress[4];
      extern char send_rc_protocol[4];
      extern char send_rc_lenght[4];

for (int i = 0; i <= 3; i++)
{
if(send_rc_adress[i]==0)
{	
      if (state)
      {
        send_rc_adress[i] = adress_on;
        send_rc_protocol[i] = protocol;
        send_rc_lenght[i] = lenght;
      }

      else
      {
        send_rc_adress[i] = adress_off;
        send_rc_protocol[i] = protocol;
        send_rc_lenght[i] = lenght;
      }
	break;  
}	  
}	  
	  
	  
      publish_state(state);
    }
};

//-------------------------------------------------------------------------------------------------------------
class New_remote_switch : public Component, public Switch {
  public:
    int counter = 0;
    uint32_t  adress;
    uint8_t  unit;

    New_remote_switch(uint32_t  adress_, uint8_t unit_) {
      adress = adress_;
      unit = unit_;
    }

    void loop() override
    {
      counter++;
      if (counter > 5)  //no point in checking every loop, counter>5 checks around 10 times /sec
      {
        extern unsigned long recieve_NRS_adress;
        extern char recieve_NRS_unit;
        extern char recieve_NRS_state;
        if (recieve_NRS_adress == adress && recieve_NRS_unit == unit )
        {
          recieve_NRS_adress = 0;
          publish_state(recieve_NRS_state);
        }
        counter = 0;
      }

    }
    void write_state(bool state) override {
								
      extern unsigned long send_NRS_adress[4];
      extern char send_NRS_unit[4];
      extern char send_NRS_state[4];
	  for (int i = 0; i <= 3; i++)
	  {
	if(send_NRS_adress[i]==0)
	{
      send_NRS_adress[i] = adress;
      send_NRS_unit[i] = unit;
      send_NRS_state[i] = state;
break;	  
	}		

	  }
	  
	  
	  
      publish_state(state);
	  
    }
};

//-------------------------------------------------------------------------------------------------------


class MyCustomLightOutput : public Component, public LightOutput {
  public:
    uint32_t  adress;
    uint8_t  unit;
    MyCustomLightOutput(uint32_t  adress_, uint8_t unit_) {
      adress = adress_;
      unit = unit_;
    }
    LightTraits get_traits() override {
      // return the traits this light supports
      auto traits = LightTraits();
      traits.set_supports_brightness(true);
      traits.set_supports_rgb(false);
      traits.set_supports_rgb_white_value(false);
      traits.set_supports_color_temperature(false);
      return traits;
    }

    void write_state(LightState *state) override {
      // This will be called by the light to get a new state to be written.
      float brightness;
      // use any of the provided current_values methods
      state->current_values_as_brightness(&brightness);
      if (millis() > 30000) //skip startup transmission
      {
        extern unsigned long send_NRS_adress[4];
        extern char send_NRS_unit[4];
        extern char send_NRS_state[4];
        extern int send_NRS_dimmer[4];
		uint8_t dimmer_level = brightness * 15;
		for (int i = 0; i <= 3; i++)
		{
		if(send_NRS_adress[i]==0)
		{
        send_NRS_adress[i] = adress;
        send_NRS_unit[i] = unit;
		//if(dimmer_level)
		//{
		send_NRS_dimmer[i] = dimmer_level;
        send_NRS_state[i] = 2;	
		//}
		//else
		//{
		//send_NRS_state[i] = 0;	
		//}

		break;
		}
		}
      }
    }
};
//-----------------------------------------------------------------------------------------------------------
//using cover component as a dimmer because it can handle 2 way communication with a float (position)
class MyCustomCover_Dimmer : public Component, public Cover {
  public:
    int counter = 0;
    uint32_t  adress;
    uint8_t  unit;

    MyCustomCover_Dimmer(uint32_t  adress_, uint8_t unit_) {
      adress = adress_;
      unit = unit_;
    }
	  void setup() override {
    // This will be called by App.setup()
          this->position = 0;
          this->publish_state();
  }
    CoverTraits get_traits() override {
      auto traits = CoverTraits();
      traits.set_is_assumed_state(false);
      traits.set_supports_position(true);
      traits.set_supports_tilt(false);
      return traits;
    }
    void control(const CoverCall &call) override {
      if (call.get_position().has_value()) {
        float pos = *call.get_position();
        // Write pos (range 0-1) to cover
        extern unsigned long send_NRS_adress[4];
        extern char send_NRS_unit[4];
        extern char send_NRS_state[4];
        extern int send_NRS_dimmer[4];
		
		for (int i = 0; i <= 3; i++)
		{
		if(send_NRS_adress[i]==0)
		{
        send_NRS_adress[i] = adress;
        send_NRS_unit[i] = unit;
        send_NRS_dimmer[i] = pos * 15;
        send_NRS_state[i] = 2;
		break;
		}
		}
		
		
		
        this->position = pos;
        this->publish_state();
      }
    }

    void loop() override
    {
      counter++;
      if (counter > 5) //no point in checking every loop, counter>5 checks around 10 times /sec
      {
        extern unsigned long recieve_NRS_adress;
        extern char recieve_NRS_unit;
        extern char recieve_NRS_state;
        extern char recieve_NRS_dimmer;
        if (recieve_NRS_adress == adress && recieve_NRS_unit == unit )
        {
          float brightness = (recieve_NRS_dimmer / 15.0);
          recieve_NRS_adress = 0;
          this->position = brightness;
          this->publish_state();
        }
        counter = 0;
      }
    }
};

