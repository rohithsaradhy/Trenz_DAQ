# gcc -Iheader -lgpiod -O src/dma_acq.c get_data.c  -o get_data
# echo "get_data1 compiled"

# gcc -Iheader -lgpiod -O src/dma_acq.c get_data2.c  -o get_data2
# echo "get_data2 compiled"


gcc -Iheader -lgpiod -O src/dma_acq.c get_data_bram.c  -o get_data_bram
echo "get_data_bram compiled"


# gcc -Iheader -lgpiod -O src/dma_acq.c src/stream_shared.c get_data_driver.c  -o get_data_driver
# echo "get_data_driver compiled"

# gcc -Iheader -pthread -lgpiod -O  dma-proxy-test.c -o dma_test
# echo "dma_test compiled"
