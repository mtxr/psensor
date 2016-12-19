#!/bin/bash

cd /root/psensor

git pull --all

autoreconf -i 
./configure
make clean all
