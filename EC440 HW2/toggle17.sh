#!/bin/bash

cd /sys/class/gpio/gpio17

if [ `cat value` == 1 ]
then
    echo '0' > value
else
    echo '1' > value
fi
