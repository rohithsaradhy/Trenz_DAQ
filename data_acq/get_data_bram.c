#include "dma_acq.h"

#include <gpiod.h>


#define BRAM_MAP_SIZE 8192
#define BRAM_MAP_MASK (BRAM_MAP_SIZE-1)


struct gpiod_chip *chip;
struct gpiod_line_bulk bulk;
int pins[4]={0,1,2,3};
int pins_vals[4]={0,0,0,0};

int gpio_init(){
char *chipname = "gpiochip0";

	int i, ret,err;

    chip = gpiod_chip_open_by_name(chipname);
	if (!chip) {
		perror("Open chip failed\n");
	}



    ret = gpiod_chip_get_lines(chip,pins,4,&bulk);
    if (ret < 0) {
		perror("Request line as output failed 1\n");
	}
    
    ret = gpiod_line_request_bulk_output(&bulk,"CONSUMER",pins_vals);
    if (ret < 0) {
		perror("Request line as output failed 2\n");
	}

    
    ret = gpiod_line_set_value_bulk(&bulk,pins_vals);
    if (ret < 0) {
		perror("Unable to set the line\n");
	}


    return 0;
}

void set_gpio(int* vals){
    int size = 4;
    int ret;

    // for (ret=0;ret<size;ret=ret+1)    printf("%u",vals[ret]);
    ret = gpiod_line_set_value_bulk(&bulk,vals);
    if (ret < 0) {
		perror("Unable to set the line\n");
	}
}

int gpio_close()
{
    gpiod_line_release_bulk(&bulk);
    gpiod_chip_close(chip);
    return 0;
};



int reset_fpga()
{
    pins_vals[0] = 1; // Reset
    pins_vals[1] = 0;
    pins_vals[2] = 0;
    pins_vals[3] = 0;
    set_gpio(pins_vals);
    usleep(1000);
    pins_vals[0] = 0;
    pins_vals[1] = 0;
    pins_vals[2] = 0;
    pins_vals[3] = 0;
    set_gpio(pins_vals);
    // usleep(10000);
    return 0;
}





int start_data_transfer()
{
    pins_vals[0] = 0; // Reset
    pins_vals[1] = 1;
    pins_vals[2] = 0;
    pins_vals[3] = 0;
    set_gpio(pins_vals);
    // usleep(1000);
    return 0;
}

int stop_data_transfer()
{
    pins_vals[0] = 0; // Reset
    pins_vals[1] = 0;
    pins_vals[2] = 0;
    pins_vals[3] = 0;
    set_gpio(pins_vals);
    return 0;
}

int data_transfer()
{
    start_data_transfer();
    usleep(0);
    stop_data_transfer();

}



int main(int argc, char** argv) {

int N;
if(argc == 2)
N=atoi(argv[1]); 
else
N=1;
gpio_init();

clock_t tic, toc;
double cpu_time_used;



int mem_fd = open("/dev/mem", O_RDWR | O_SYNC); // Open /dev/mem which represents the whole physical memory


char file_name1[100]="data_files/ddmtd1.txt";
char file_name2[100]="data_files/ddmtd2.txt";
char file_name3[100]="data_files/ddmtd3.txt";



remove(file_name1);
remove(file_name2);
remove(file_name3);



FILE * fp1,* fp2,* fp3;
fp1 = fopen(file_name1,"a");
fp2 = fopen(file_name2,"a");
fp3 = fopen(file_name3,"a");





const uint ADDR_MAP_SIZE =4*8192UL ;
uint num_words = 1000; //Should not exceed write depth
uint read_width = 256;
uint bytes_transferred = read_width*num_words/8;



char* memory_LongStore_1 = (char *)mmap(NULL,LONG_MEM , PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd,0x4a000000); // Memory map destination address
if(memory_LongStore_1 == (void *) -1) {printf("FATAL LONG MEM"); return 0;};  
memset(memory_LongStore_1, 0xff, LONG_MEM);

char* memory_LongStore_2 = (char *)mmap(NULL,LONG_MEM , PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd,0x4b000000); // Memory map destination address
if(memory_LongStore_2 == (void *) -1) {printf("FATAL LONG MEM"); return 0;};  
memset(memory_LongStore_2, 0xff, LONG_MEM);

char* memory_LongStore_3 = (char *)mmap(NULL,LONG_MEM , PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd,0x4c000000); // Memory map destination address
if(memory_LongStore_3 == (void *) -1) {printf("FATAL LONG MEM"); return 0;};  
memset(memory_LongStore_3, 0xff, LONG_MEM);






unsigned int *bram_addr_1 = mmap(NULL,ADDR_MAP_SIZE , PROT_READ | PROT_WRITE, MAP_SHARED,mem_fd,0x80000000); // Memory map AXI Lite register block
if(bram_addr_1 == (void *) -1) {printf("FATAL BRAM ADDR"); return 0;};  
bram_addr_1[0] = num_words; // Number of words to transfer // First addr reserved for fpga settings
uint *bram_data_addr_1 = bram_addr_1 + read_width/32; //data will be transferred from the Next line



unsigned int *bram_addr_2 = mmap(NULL,ADDR_MAP_SIZE , PROT_READ | PROT_WRITE, MAP_SHARED,mem_fd,0x82000000 ); // Memory map AXI Lite register block
if(bram_addr_2 == (void *) -1) {printf("FATAL BRAM ADDR"); return 0;};  
bram_addr_2[0] = num_words; // Number of words to transfer // First addr reserved for fpga settings
uint *bram_data_addr_2 = bram_addr_2 + read_width/32; //data will be transferred from the Next line


unsigned int *bram_addr_3 = mmap(NULL,ADDR_MAP_SIZE , PROT_READ | PROT_WRITE, MAP_SHARED,mem_fd,0x84000000 ); // Memory map AXI Lite register block
if(bram_addr_3 == (void *) -1) {printf("FATAL BRAM ADDR"); return 0;};  
bram_addr_3[0] = num_words; // Number of words to transfer // First addr reserved for fpga settings
uint *bram_data_addr_3 = bram_addr_3 + read_width/32; //data will be transferred from the Next line




//Reset FPGA
reset_fpga();
usleep(10000); //Dont Touch


// memdump(dma_addr,256/8);
uint total_data_inBytes = 0;
uint iter = N;



tic = clock();
for (int i =0; i < iter; i=i+1 )
{

    start_data_transfer();
    memcpy(memory_LongStore_1+total_data_inBytes,bram_data_addr_1,bytes_transferred);
    // print_16Words(bram_data_addr_1,bytes_transferred);

    memcpy(memory_LongStore_2+total_data_inBytes,bram_data_addr_2,bytes_transferred);
    // print_16Words(bram_data_addr_2,bytes_transferred);

    memcpy(memory_LongStore_3+total_data_inBytes,bram_data_addr_3,bytes_transferred);


    total_data_inBytes = total_data_inBytes+bytes_transferred;

    stop_data_transfer();
    // printf("%u \n ",);
    usleep(1000); // Adjusting this will give time for the FIFO to fill up


    

}

toc = clock();
cpu_time_used = ((double) (toc - tic)) / CLOCKS_PER_SEC;

printf("Total CPU Time Taken %f \n",cpu_time_used);
printf("Total Data Collected %f KB\n",(double)(3*total_data_inBytes/(1024)));
printf("Transfer Speed is %f MBps \n",(double)(3*total_data_inBytes/(cpu_time_used*1024*1024)));



write_toFile( fp1, memory_LongStore_1,total_data_inBytes);
// print_16Words(memory_LongStore_1,total_data_inBytes);

write_toFile( fp2, memory_LongStore_2,total_data_inBytes);
// // print_16Words(memory_LongStore_2,total_data_inBytes);

write_toFile( fp3, memory_LongStore_3,total_data_inBytes);
// // print_16Words(memory_LongStore_2,total_data_inBytes);






munmap((void *)memory_LongStore_1,LONG_MEM);
munmap((void *)bram_addr_1, (size_t)ADDR_MAP_SIZE);
close(mem_fd);
fclose(fp1);
fclose(fp2);
fclose(fp3);




gpio_close();


}

