#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/mman.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/gpio.h>
#include <time.h>
#include <sys/time.h>

#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>
#include <sys/types.h>


#include <sys/stat.h>
#include <inttypes.h>


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
#define DMA_ADDR  0x80000000


// #define DATA_SIZE 32*10/8 
// #define DATA_SIZE 512*1/8
// #define DATA_SIZE (8000/8+3)*32
#define DATA_SIZE 8*1048576 //8MB

#define DMA_SIZE 65535UL
#define MAP_SIZE 8*1048576  //8MB

#define LONG_MEM MAP_SIZE*32 //256MB

// #define MAP_SIZE 32768UL
// #define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)



struct dma_data {

  // virtual addresses
  int target_addr;

  // /dev/mem file descriptor
  int mem_fd;

  // address data 
  unsigned int*  virtual_dma_addr;
  unsigned int*  virtual_destination_addr;

  //data collected stuff
  unsigned int   data_collected;
  unsigned int   total_data;
};




#define FATAL do { fprintf(stderr, "Error at line %d, file %s (%d) [%s]\n", \
  __LINE__, __FILE__, errno, strerror(errno)); exit(1); } while(0)




unsigned int dma_set(unsigned int* dma_virtual_address, int offset, unsigned int value) ;
unsigned int dma_get(unsigned int* dma_virtual_address, int offset) ;
int dma_mm2s_sync(unsigned int* dma_virtual_address);
int dma_s2mm_sync(unsigned int* dma_virtual_address);
void dma_s2mm_status(unsigned int* dma_virtual_address);
void dma_mm2s_status(unsigned int* dma_virtual_address);
void memdump(void* virtual_address, int byte_count);

int print_16Words(void* virtual_address, int byte_count);
int dump_Data(struct dma_data *dd);

int init_dma(struct dma_data *dd);

void transfer_Data(struct dma_data *dd) ;


int write_toFile(FILE* fp,  void* virtual_address, int byte_count);



