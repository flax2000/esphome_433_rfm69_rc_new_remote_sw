#!/bin/bash
cd `dirname $0`
SCRIPTDIR=`pwd`
cd && source venv/bin/activate 
cd -
esphome logs 433_switch_control.yaml 
deactivate
read -p "Press any key to continue" x
