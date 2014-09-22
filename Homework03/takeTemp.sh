#!/bin/bash
# Read Temperature and Print in F

declare -i tempa
#declare -i tempb

read tempa<<<$(i2cget -y 1 0x48 00)
#read tempb<<<$(i2cget -y 1 0x49 00)
tempaf=$(echo "$tempa * 1.8"|bc)
#tempbf=$(echo "$tempb * 1.8"|bc)

echo "Temp in F from Address 0x48 is $(echo "$tempaf+32"|bc)"
#echo "Temp in F from Address 0x49 is $(echo "$tempbf+32"|bc)"
