#!/bin/bash


HOST="root@10.0.0.10"
echo "Sending software to Trenz"
# sshpass -p "root" scp get_data.c $HOST:
sshpass -p "root" scp do_load.sh $HOST:
sshpass -p "root" scp -r data_acq $HOST:
sshpass -p "root" ssh -T $HOST << EOF
./do_load.sh main.bit.bin

cd data_acq

chmod +x compile.sh
./compile.sh
./get_data2 $1
EOF
