dtc -O dtb -o bone_eqep1_2-00A0.dtbo -b 0 -@ bone_eqep1_2.dts
cp bone_eqep1_2-00A0.dtbo /lib/firmware
echo bone_eqep1_2 > /sys/devices/bone_capemgr.*/slots

