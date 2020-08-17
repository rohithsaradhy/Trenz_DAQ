#include "dma_acq.h"




int main() {

    //Opening the GPIO
    int f;
    int ret;
    struct gpiohandle_request req;
    struct gpiohandle_data data;
    f = open("/dev/gpiochip0", O_RDONLY);
    for(int i=0;i<32;i=i+1) req.lineoffsets[i] = i; //requesting all 32 pins
    // req.lineoffsets[0] = 0;
    

    req.flags = GPIOHANDLE_REQUEST_OUTPUT;
    for(int i=0;i<32;i=i+1) req.default_values[i] = 0; //setting the default values to be zero
    // req.default_values[0] = 0;



    strcpy(req.consumer_label,"gpio-0-output");
    req.lines = 32; //Pulling 32 lines
    ret = ioctl(f,GPIO_GET_LINEHANDLE_IOCTL,&req);

    



    int dh = open("/dev/mem", O_RDWR | O_SYNC); // Open /dev/mem which represents the whole physical memory
    unsigned int* virtual_address = mmap(NULL, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, dh, 0x80000000); // Memory map AXI Lite register block
    if(virtual_address == (void *) -1) FATAL;  

    unsigned int* virtual_destination_address = mmap(NULL, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, dh,TARGET_REG  ); // Memory map destination address
    // unsigned int* virtual_destination_address = mmap(NULL, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, dh,TARGET_REG & ~MAP_MASK ); // Memory map destination address
    if(virtual_destination_address == (void *) -1) FATAL;  


  

    memset(virtual_destination_address, 0xff, DATA_SIZE); // Clear destination block
    printf("Destination memory block: "); print_16Words(virtual_destination_address, DATA_SIZE);




    printf("Resetting DMA\n");
    dma_set(virtual_address, S2MM_CONTROL_REGISTER, 4);
    dma_s2mm_status(virtual_address);

    printf("Halting DMA\n");
    dma_set(virtual_address, S2MM_CONTROL_REGISTER, 0);
    dma_s2mm_status(virtual_address);

    printf("Writing destination address\n");
    dma_set(virtual_address, S2MM_DESTINATION_ADDRESS, TARGET_REG); // Write destination address
    dma_s2mm_status(virtual_address);

    // struct timespec tick, tock;

    
    // sleep(2);



    data.values[0] = 0; //Enable
    data.values[1] = 0; //S_Axis_ENABLE
    ret = ioctl(req.fd,GPIOHANDLE_SET_LINE_VALUES_IOCTL,&data);





    // assert( !clock_gettime(CLOCK_MONOTONIC, &tick) );
    dump_Data(virtual_address);
    printf("Destination memory block: "); print_16Words(virtual_destination_address, DATA_SIZE);

    data.values[0] = 0; //Enable
    data.values[1] = 1; //S_Axis_ENABLE
    ret = ioctl(req.fd,GPIOHANDLE_SET_LINE_VALUES_IOCTL,&data);

    dump_Data(virtual_address);
    printf("Destination memory block: "); print_16Words(virtual_destination_address, DATA_SIZE);


    // dump_Data(virtual_address);



    // sleep(1);
    // assert( !clock_gettime(CLOCK_MONOTONIC, &tock) );

    // printf("Destination memory block: "); memdump(virtual_destination_address, DATA_SIZE);




    // // char* p = (void*)virtual_destination_address;
    // // printf("%02x%02x%02x%02x \n",p[DATA_SIZE-1],p[DATA_SIZE-2], p[DATA_SIZE-3],p[DATA_SIZE-4]);

    // // char last_word[4];
    // // last_word[3] = (0xff) & (*p << (DATA_SIZE-4));
    // // last_word[2] = (0xff) & (*p << (DATA_SIZE-3));
    // // last_word[1] = (0xff) & (*p << (DATA_SIZE-2));
    // // last_word[0] = (0xff) & (*p << (DATA_SIZE-1));
    // // printf("%02x%02x%02x%02x \n",last_word[0],last_word[1], last_word[2],last_word[3]);




    // print_throughput(&tick, &tock);



    data.values[0] = 0;
    data.values[1] = 0;
    ret = ioctl(req.fd,GPIOHANDLE_SET_LINE_VALUES_IOCTL,&data);

}