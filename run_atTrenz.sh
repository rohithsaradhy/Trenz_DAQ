#!/bin/bash




HOST="root@10.0.0.10"
echo "Sending software to Trenz"
# sshpass -p "root" scp get_data.c $HOST:
sshpass -p "root" scp $1do_load.sh $HOST:
# sshpass -p "root" rsync -rva -r $1data_acq $HOST:
sshpass -p "root" scp -r -r $1data_acq $HOST:
sshpass -p "root" ssh -T $HOST << EOF
./do_load.sh main.bit.bin

cd data_acq

chmod +x compile.sh
./compile.sh
./get_data2 $2
EOF



