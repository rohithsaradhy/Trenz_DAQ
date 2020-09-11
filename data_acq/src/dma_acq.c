
#include "dma_acq.h"

const int NUM_TRIALS = 1;



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
        // dma_s2mm_status(dma_virtual_address);
        // dma_mm2s_status(dma_virtual_address);

        s2mm_status = dma_get(dma_virtual_address, S2MM_STATUS_REGISTER);
        // break;

        if (i==1000){ printf("Transfer failed, aborting after 1000 sync cycles\n"); break;}
        else i=i+1;
        usleep(1000);

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







// void print_throughput(struct timespec *tick, struct timespec *tock)
// {
//     double start, end, diff, bytes_per_sec;
//     double numBytes = (double)NUM_TRIALS * DATA_SIZE;
//     start = tick->tv_sec + tick->tv_nsec/1e9;
//     end   = tock->tv_sec + tock->tv_nsec/1e9;
//     diff  = end - start;

//     bytes_per_sec = numBytes / (double)(1<<20) / diff;

//     printf("sent %d %d-byte packets in %.9f sec: %.3f MB/s\n",
//             NUM_TRIALS, DATA_SIZE, diff, bytes_per_sec);
// }


int print_16Words(void* virtual_address, int byte_count)  
{
    char *p = virtual_address;
    int offset;
    printf("\n");
    uint64_t val1;

    int mod_num = 4;
    int word_byte = 4;
    for (offset = 0; offset < byte_count; offset=offset+word_byte) {
        val1 = (uint64_t)(0xffffffff&((p[3+offset]&0b01111111)<<24|p[2+offset]<<16|p[1+offset]<<8|p[0+offset]));

        if(offset % (4*mod_num) == 0 & (offset != 0) ) {/*if(val1!=0x7fffffff)*/  printf("\n",offset);} //

        // if(1)
        // if(val1!=0x7fffffff)
        {
        printf("%" PRIu64,val1);
        printf(" ");

        }
        // printf("%x" PRIu64,p[3+offset]<<24|p[2+offset]<<16|p[1+offset]<<8|p[0+offset]);

    }
    printf("\n");
    return 0;
}

int dump_Data(struct dma_data *dd)
{
 
    // printf("Waiting for S2MM sychronization...\n");
    dma_set(dd->virtual_dma_addr, S2MM_CONTROL_REGISTER, 0x0001);
    dma_set(dd->virtual_dma_addr, S2MM_LENGTH, DATA_SIZE);
    dma_s2mm_sync(dd->virtual_dma_addr); // If this locks up make sure all memory ranges are assigned under Address Editor!
    dd->data_collected = dma_get(dd->virtual_dma_addr, S2MM_LENGTH);


}



unsigned int* transfer_Data(int mem_fd, unsigned int target_address,struct dma_data *dd) 
{

 
    unsigned int* virtual_address = mmap(NULL, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd,DMA_ADDR ); // Memory map AXI Lite register block
    if(virtual_address == (void *) -1) printf("FATAL");   
   
   
    unsigned int* virtual_destination_address = mmap(NULL, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd,target_address); // Memory map destination address
    if(virtual_destination_address == (void *) -1) printf("FATAL");  


    memset(virtual_destination_address, 0xff, DATA_SIZE); // Clear destination block
    // printf("Destination memory block: "); print_16Words(virtual_destination_address, DATA_SIZE);


    // // printf("Resetting DMA\n");
    // dma_set(virtual_address, S2MM_CONTROL_REGISTER, 4);
    // // dma_s2mm_status(virtual_address);

    // // printf("Halting DMA\n");
    // dma_set(virtual_address, S2MM_CONTROL_REGISTER, 0);
    // // dma_s2mm_status(virtual_address);

    // printf("Writing destination address\n");
    dma_set(virtual_address, S2MM_DESTINATION_ADDRESS, target_address); // Write destination address
    // dma_s2mm_status(virtual_address);



    // printf("Waiting for S2MM sychronization...\n");
    dma_set(virtual_address, S2MM_CONTROL_REGISTER, 0x0001); //start
    dma_set(virtual_address, S2MM_LENGTH, DATA_SIZE); //set length
    dma_s2mm_sync(virtual_address); // If this locks up make sure all memory ranges are assigned under Address Editor!
    dma_set(virtual_address, S2MM_CONTROL_REGISTER, 0x0000); //stop
    // dma_set(virtual_address, S2MM_CONTROL_REGISTER, 4); //reset
    // printf("Value of S2MM_LENGTH is %u",dma_get(virtual_address, S2MM_LENGTH));


    dd->virtual_dma_addr = virtual_address;
    dd->virtual_destination_addr = virtual_destination_address;
    dd->data_collected = dma_get(virtual_address, S2MM_LENGTH);
    // printf("%u \t %u \n",virtual_address,virtual_destination_address);
    return 0;
}



int write_toFile(FILE* fp,  void* virtual_address, int byte_count) { 
int offset;
uint64_t val1,val2;
uint64_t val_prev;

char *p = virtual_address;
    int mod_num = 32;
    int word_byte = 4;
    for (offset = 0; offset < byte_count; offset=offset+word_byte){

  val1 = (uint64_t)(0xffffffff&((p[3+offset]&0b01111111)<<24|p[2+offset]<<16|p[1+offset]<<8|p[0+offset]));


    {
        
        fprintf(fp,"%" PRIu64",",((p[3+offset]>>7)&0b1));
        // fprintf(fp,"%" PRIu64  ,(p[3+offset]&0x7f)<<24|p[2+offset]<<16|p[1+offset]<<8|p[0+offset]);
        fprintf(fp,"%" PRIu64  ,val1);
        if(((offset + word_byte) % (word_byte*mod_num) == 0 )&& (offset != 0)) {/*if(val1!=0x7fffffff)*/  fprintf(fp,"\n");} //
        else
        fprintf(fp,",");
        // fprintf(fp,"%" PRIu64",",((p[3+4+offset]>>7)&0b1));
        // fprintf(fp,"%" PRIu64  ,((p[3+4+offset]&0x7f)<<24|p[2+4+offset]<<16|p[1+4+offset]<<8|p[0+4+offset]));
        // fprintf(fp,"%" PRIu64  ,val2);
        // fprintf(fp,"\n");
    }

}
    // fprintf(fp,"$$$$$$$$\n");
    fprintf(fp,"\n\n");
// fprintf(fp,"%" PRIu64 "\n",("val1[mem_no]+value[mem_no]"));

return 0;
}



