#!/bin/sh

#
# Write EEPROM test script for evaluating the reliability of writings with
# hub-ctrl.
#

# get required bus and device ID
usbdesc=$(lsusb | grep "Cypress Semiconductor Corp")
BUSNR=$(echo "$usbdesc" | cut -d ' ' -f2)
DEVNR=$(echo "$usbdesc" | cut -d ' ' -f4 | tr -d ':')

if [ -z "$BUSNR" -o -z "$DEVNR" ]; then
	echo 'Cannot find Cypress hub on the bus.'
	exit 1;
fi

echo
echo Bus Nr:    "$BUSNR"
echo Device Nr: "$DEVNR"
echo

# eeprom content to be written
eeprom="\xd4\xb4\x04\x60\x65\x00\x92\x88\x28\x5F\x00\x00\x50\xbe\x50\x64\x32"
eeprom+="\x90\x41\x00\x01\x07\x03\x03\x09\x04\x26\x00\x32\x00\x52\x00\x00\x00"
eeprom+="\x00\x00\x00\x00\x0c\x03\x43\x00\x31\x00\x30\x00\x30\x00\x38\x00\x20"
eeprom+="\x03\x41\x00\x44\x00\x54\x00\x30\x00\x37\x00\x31\x00\x36\x00\x2d\x00"
eeprom+="\x30\x00\x30\x00\x31\x00\x2d\x00\x30\x00\x30\x00\x30\x00\x18\x03\x34"
eeprom+="\x00\x34\x00\x31\x00\x36\x00\x2d\x00\x30\x00\x30\x00\x30\x00\x30\x00"
eeprom+="\x30\x00\x31\x00"

i=0
fails=0
runs=180
# number of bytes
bytes=$((${#eeprom} / 4))

while [ "$i" -lt "$runs" ]; do

        hub-ctrl -b "$BUSNR" -d "$DEVNR" -e "$bytes" > /dev/null

        printf "%s" "$eeprom" | hub-ctrl -b "$BUSNR" -d "$DEVNR" -w "$bytes" -f - > /dev/null
        ret=$?

        fails=$((fails + ret))
        i+=1

done;

successes=$((runs - fails))

echo
echo "EEPROM write succeeded $successes out of $runs times."
