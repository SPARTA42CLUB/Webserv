#!/bin/bash

hexdump testfile.png > testfile.hex
hexdump testImage.png > testImage.hex

diff testfile.hex testImage.hex > diff.txt

rm testfile.hex testImage.hex
