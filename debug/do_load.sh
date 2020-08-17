echo 0 > /sys/class/fpga_manager/fpga0/flags
mkdir -p /lib/firmware
cp ${1}  /lib/firmware/
echo ${1} > /sys/class/fpga_manager/fpga0/firmware
