#include "dma_acq.h"

#include <gpiod.h>



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
    usleep(1000);
    return 0;
}



int main(int argc, char** argv) {


    int N;
    if(argc == 2)
        N=atoi(argv[1]); 
    else
        N=1;



    

    //Opening the GPIO
    // int f;
    // int ret;
    // struct gpiohandle_request req;
    // struct gpiohandle_data data;
    // f = open("/dev/gpiochip0", O_RDONLY);
    // for(int i=0;i<32;i=i+1) req.lineoffsets[i] = i; //requesting all 32 pins
    // // req.lineoffsets[0] = 0;

    // req.flags = GPIOHANDLE_REQUEST_OUTPUT;
    // for(int i=0;i<32;i=i+1) req.default_values[i] = 0; //setting the default values to be zero
    // // req.default_values[0] = 0;
    // strcpy(req.consumer_label,"gpio-0-output");
    // req.lines = 32; //Pulling 32 lines
    // ret = ioctl(f,GPIO_GET_LINEHANDLE_IOCTL,&req);



    gpio_init();
    pins_vals[0] = 1; // Reset
    pins_vals[1] = 0;
    pins_vals[2] = 0;
    pins_vals[3] = 0;
    set_gpio(pins_vals);

    int mem_fd = open("/dev/mem", O_RDWR | O_SYNC); // Open /dev/mem which represents the whole physical memory
   

    unsigned int* vda1;
    unsigned int* vda2;

    char file_name1[100]="data_files/ddmtd1.txt";
    char file_name2[100]="data_files/ddmtd2.txt";


    remove(file_name1);
    remove(file_name2);


    FILE * fp1,* fp2;
    fp1 = fopen(file_name1,"a");
    fp2 = fopen(file_name2,"a");
    
 




    unsigned int *dma_addr = mmap(NULL, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED,mem_fd,DMA_ADDR ); // Memory map AXI Lite register block
    if(dma_addr == (void *) -1) {printf("FATAL DMA ADDR"); return 0;};  
    
    struct dma_data dd1; //create a dma_data struct with all necessary information
    dd1.mem_fd = mem_fd;//File Descriptor for /dev/mem
    dd1.target_addr =  0x4a000000; //destination for DMA to copy the data
    dd1.virtual_dma_addr = dma_addr;
    init_dma(&dd1); // initialize DMA transfer...



    char* memory_LongStore = (char *)mmap(NULL,LONG_MEM , PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd,0x4b000000); // Memory map destination address
    if(memory_LongStore == (void *) -1) {printf("FATAL LONG MEM"); return 0;};  
    memset(memory_LongStore, 0xff, LONG_MEM);




 //Setting reset to be zero
    usleep(1000);
    pins_vals[0] = 0;
    pins_vals[1] = 0;
    pins_vals[2] = 0;
    pins_vals[3] = 0;
    set_gpio(pins_vals);
    usleep(10000);


  

    suseconds_t tic,toc;
    suseconds_t time_taken;
    struct timeval current_time;
    gettimeofday(&current_time,NULL); 
    tic = current_time.tv_usec;




    for(int i=0;i<N;i=i+1)
    {
        transfer_Data(&dd1); //transfer data
        // printf("Destination memory block: "); print_16Words(dd1.virtual_destination_addr, dd1.data_collected);
        // write_toFile( fp1, dd1.virtual_destination_addr, dd1.data_collected);        

        memcpy(memory_LongStore+(dd1.total_data-dd1.data_collected),dd1.virtual_destination_addr,dd1.data_collected);
        memset(dd1.virtual_destination_addr, 0xff, dd1.data_collected );


        usleep(1000);

        // t_clock = clock(); 
        // time_taken = ((double)t_clock - t)/CLOCKS_PER_SEC; // in seconds 
        // t = t_clock;
        // printf("iter %u transfered at %f MBps \n",i+1,dd1.data_collected /( time_taken *1024 *1024)); 
        // sleep(1);
    }

    


    gettimeofday(&current_time,NULL); 
    toc = current_time.tv_usec; 
    time_taken = ((toc-tic)); // in useconds 

    printf("Total data Transfered %f KB \n",(float)(dd1.total_data)/(1024));
    printf("Total time Transfered %u us \n",time_taken);
    printf("transfered at %u KBps \n",dd1.total_data * 1000000 /(time_taken * 1024 )); 
    
    write_toFile( fp1, memory_LongStore, dd1.total_data);






    munmap((void *)memory_LongStore,LONG_MEM);
    munmap((void *)dd1.virtual_destination_addr, (size_t)DATA_SIZE);
    munmap((void *)dd1.virtual_dma_addr, (size_t)DATA_SIZE);
    close(mem_fd);
    fclose(fp1);
    fclose(fp2);

    


    gpio_close();


    return 0;


}