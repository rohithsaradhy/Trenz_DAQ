/**
 * Proof of concept offloaded memcopy using AXI Direct Memory Access v7.1
 */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/mman.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/gpio.h>
#include <time.h>
#include <assert.h>
#include <stdlib.h>
#include <errno.h>



#define MM2S_CONTROL_REGISTER 0x00
#define MM2S_STATUS_REGISTER 0x04
#define MM2S_START_ADDRESS 0x18
#define MM2S_LENGTH 0x28

#define S2MM_CONTROL_REGISTER 0x30
#define S2MM_STATUS_REGISTER 0x34
#define S2MM_DESTINATION_ADDRESS 0x48
#define S2MM_LENGTH 0x58


#define SOURCE_REG  0x0e000000
// #define TARGET_REG  0x0f000000
#define TARGET_REG  0x7f000000


// #define DATA_SIZE 32*10/8 
#define DATA_SIZE 512*10/8


// #define MAP_SIZE 65535
#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)




#define FATAL do { fprintf(stderr, "Error at line %d, file %s (%d) [%s]\n", \
  __LINE__, __FILE__, errno, strerror(errno)); exit(1); } while(0)


const int NUM_TRIALS = 1;

unsigned int dma_set(unsigned int* dma_virtual_address, int offset, unsigned int value) ;
unsigned int dma_get(unsigned int* dma_virtual_address, int offset) ;
int dma_mm2s_sync(unsigned int* dma_virtual_address);
int dma_s2mm_sync(unsigned int* dma_virtual_address);
void dma_s2mm_status(unsigned int* dma_virtual_address);
void dma_mm2s_status(unsigned int* dma_virtual_address);
void memdump(void* virtual_address, int byte_count);




unsigned int dma_set(unsigned int* dma_virtual_address, int offset, unsigned int value) {
    dma_virtual_address[offset>>2] = value;
}

unsigned int dma_get(unsigned int* dma_virtual_address, int offset) {
    return dma_virtual_address[offset>>2];
}

int dma_mm2s_sync(unsigned int* dma_virtual_address) {
    unsigned int mm2s_status =  dma_get(dma_virtual_address, MM2S_STATUS_REGISTER);
    while(!(mm2s_status & 1<<12) || !(mm2s_status & 1<<1) ){
        dma_s2mm_status(dma_virtual_address);
        dma_mm2s_status(dma_virtual_address);

        mm2s_status =  dma_get(dma_virtual_address, MM2S_STATUS_REGISTER);
    }
}

int dma_s2mm_sync(unsigned int* dma_virtual_address) {
    unsigned int s2mm_status = dma_get(dma_virtual_address, S2MM_STATUS_REGISTER);

    int i=0;
    while(!(s2mm_status & 1<<12) || !(s2mm_status & 1<<1)){
        dma_s2mm_status(dma_virtual_address);
        // dma_mm2s_status(dma_virtual_address);

        s2mm_status = dma_get(dma_virtual_address, S2MM_STATUS_REGISTER);
        break;
        // if (i==1) break;
        // else i=i+1;
    }
    // printf("%3d \n",i);
}

void dma_s2mm_status(unsigned int* dma_virtual_address) {
    unsigned int status = dma_get(dma_virtual_address, S2MM_STATUS_REGISTER);
    printf("Stream to memory-mapped status (0x%08x@0x%02x):", status, S2MM_STATUS_REGISTER);
    if (status & 0x00000001) printf(" halted"); else printf(" running");
    if (status & 0x00000002) printf(" idle");
    if (status & 0x00000008) printf(" SGIncld");
    if (status & 0x00000010) printf(" DMAIntErr");
    if (status & 0x00000020) printf(" DMASlvErr");
    if (status & 0x00000040) printf(" DMADecErr");
    if (status & 0x00000100) printf(" SGIntErr");
    if (status & 0x00000200) printf(" SGSlvErr");
    if (status & 0x00000400) printf(" SGDecErr");
    if (status & 0x00001000) printf(" IOC_Irq");
    if (status & 0x00002000) printf(" Dly_Irq");
    if (status & 0x00004000) printf(" Err_Irq");
    printf("\n");
}

void dma_mm2s_status(unsigned int* dma_virtual_address) {
    unsigned int status = dma_get(dma_virtual_address, MM2S_STATUS_REGISTER);
    printf("Memory-mapped to stream status (0x%08x@0x%02x):", status, MM2S_STATUS_REGISTER);
    if (status & 0x00000001) printf(" halted"); else printf(" running");
    if (status & 0x00000002) printf(" idle");
    if (status & 0x00000008) printf(" SGIncld");
    if (status & 0x00000010) printf(" DMAIntErr");
    if (status & 0x00000020) printf(" DMASlvErr");
    if (status & 0x00000040) printf(" DMADecErr");
    if (status & 0x00000100) printf(" SGIntErr");
    if (status & 0x00000200) printf(" SGSlvErr");
    if (status & 0x00000400) printf(" SGDecErr");
    if (status & 0x00001000) printf(" IOC_Irq");
    if (status & 0x00002000) printf(" Dly_Irq");
    if (status & 0x00004000) printf(" Err_Irq");
    printf("\n");
}

void memdump(void* virtual_address, int byte_count) {
    char *p = virtual_address;
    int offset;
    for (offset = 0; offset < byte_count; offset=offset+4) {
        printf("%02x%02x%02x%02x",p[offset+3],p[offset+2], p[offset+1],p[offset]);
        printf(" ");
    }
    printf("\n");
}




int dump_Data(unsigned int* virtual_address)
{

    printf("Waiting for S2MM sychronization...\n");
    dma_set(virtual_address, S2MM_CONTROL_REGISTER, 0x0001);
    dma_set(virtual_address, S2MM_LENGTH, DATA_SIZE);
    dma_s2mm_sync(virtual_address); // If this locks up make sure all memory ranges are assigned under Address Editor!

    return 0;
}


void print_throughput(struct timespec *tick, struct timespec *tock)
{
    double start, end, diff, bytes_per_sec;
    double numBytes = (double)NUM_TRIALS * DATA_SIZE;
    start = tick->tv_sec + tick->tv_nsec/1e9;
    end   = tock->tv_sec + tock->tv_nsec/1e9;
    diff  = end - start;

    bytes_per_sec = numBytes / (double)(1<<20) / diff;

    printf("sent %d %d-byte packets in %.9f sec: %.3f MB/s\n",
            NUM_TRIALS, DATA_SIZE, diff, bytes_per_sec);
}


int print_16Words(void*, int)  ;

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

    




    // void* p = sbrk(MAP_SIZE);
    // printf("%p \n",p);
    // char *p = malloc(MAP_SIZE);
    // printf("%p \n",p);
    // printf("%x \n",(int64_t*)p);



    int dh = open("/dev/mem", O_RDWR | O_SYNC); // Open /dev/mem which represents the whole physical memory
    unsigned int* virtual_address = mmap(NULL, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, dh, 0x80000000); // Memory map AXI Lite register block
    if(virtual_address == (void *) -1) FATAL;  

    unsigned int* virtual_destination_address = mmap(NULL, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, dh,TARGET_REG  ); // Memory map destination address
    // unsigned int* virtual_destination_address = mmap(NULL, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, dh,TARGET_REG & ~MAP_MASK ); // Memory map destination address
    if(virtual_destination_address == (void *) -1) FATAL;  


    // unsigned int *virtual_destination_address = (unsigned int*) malloc(DATA_SIZE);
    // printf("0x%x \n",virtual_destination_address);



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
    // data.values[1] = 0; //S_Axis_ENABLE
    ret = ioctl(req.fd,GPIOHANDLE_SET_LINE_VALUES_IOCTL,&data);
    // sleep(2);
    data.values[0] = 0; //Enable
    // // data.values[1] = 0; //S_Axis_ENABLE
    ret = ioctl(req.fd,GPIOHANDLE_SET_LINE_VALUES_IOCTL,&data);






    // assert( !clock_gettime(CLOCK_MONOTONIC, &tick) );
    dump_Data(virtual_address);
    // dump_Data(virtual_address);



    // sleep(1);
    // assert( !clock_gettime(CLOCK_MONOTONIC, &tock) );

    // printf("Destination memory block: "); memdump(virtual_destination_address, DATA_SIZE);
    printf("Destination memory block: "); print_16Words(virtual_destination_address, DATA_SIZE);




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





int print_16Words(void* virtual_address, int byte_count)  
{
    char *p = virtual_address;
    int offset;
    printf("\n");


    int mod_num = 16;
    int word_byte = 4;
    for (offset = 0; offset < byte_count; offset=offset+word_byte) {
        if(offset % (4*mod_num) == 0 & (offset != 0) ) {printf("\n",offset);}
        for (int j=word_byte-1; j>=0;j=j-1)
        {
        printf("%02x",p[offset+j]);

        }
        printf(" ");
       

    }
    printf("\n");
    return 0;
}

