# esphome_433_rfm69_rc_new_remote_sw
uppdated version that compiles 2025-02-13
Warning messy code ahead!!
I made this for myself and I added it here in case someone else has any use of it

I never had any luck getting the original 433 control code in esphome to work the way I wanted it too so I made this
This are running rc switch and NewRemoteSwitch at the same time
You can either use separate transmit and receive modules or a rfm69

If you use separate modules remove this line ” #define use_rfm69  ” in 433_switch_main.h

And set rx pin in 433_switch_main.h  “#define _rx_pin 3”

And tx pin in 433_switch_tx.h “#define _tx_pin 1 ” 

If you want to use rfm69 just leave this line as it is” #define use_rfm69  ” in 433_switch_main.h and just set the pins you like for the rfm69 (_rx_pin, CSNq, SCKq, MOSIq), 
the current pins are setup to work with a esp01

If you are getting ota upload problems or random reboots try reducing the sensitivity in 
rfm69_control.h   “{ REG_RSSITHRESH, 240},” 
-always read and follow the laws regulating 433 radios where you live-


If neccecary change the  nSeparationLimit to receive the right rc switch code, 2000-12000 something
Find this line in 433_switch_main.h “const uint16_t nSeparationLimit = 4300;
And maybe change this in the same file “int16_t nReceiveTolerance = 80;”
I have changed NewRemoteSwitch to ignore the sync pulse and just read enough valid signals in a row, this helped me with a few wall switches that I had problems picking up signals from

After this just set a wifi network in the .yaml file and change the names/codes for the switches just follow the same structure to add or remove switches 

The bat file are for uploading from a windows computer if you have installed using commandline and the .sh for linux