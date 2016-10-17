#!/bin/bash

cd /sys/class/gpio

echo '17' > export

cd gpio17

echo 'out' > direction
