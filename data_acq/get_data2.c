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
    
    usleep(1000);
    pins_vals[0] = 0;
    pins_vals[1] = 0;
    pins_vals[2] = 0;
    pins_vals[3] = 0;
    set_gpio(pins_vals);
    // usleep(10000);


    struct dma_data dd1;
    unsigned int total_data = 0;




    char* memory_LongStore = (char *)mmap(NULL,LONG_MEM , PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd,0x4d000000); // Memory map destination address
    if(memory_LongStore == (void *) -1) printf("FATAL");  
    memset(memory_LongStore, 0xff, LONG_MEM);



    clock_t t; 
    t = clock(); 




    transfer_Data(mem_fd,0x4a000000,&dd1);//  0x40000000 is where the CMA is 
    // printf("Destination memory block: "); print_16Words(dd1.virtual_destination_addr, dd1.data_collected);
    // write_toFile( fp1, dd1.virtual_destination_addr, dd1.data_collected);


    memcpy(memory_LongStore+total_data,dd1.virtual_destination_addr,dd1.data_collected);
    total_data +=dd1.data_collected;
    // printf("%u is the data_collected\n",dd1.data_collected);
    memset(dd1.virtual_destination_addr, 0xff, dd1.data_collected );





    for(int i=0;i<N-1;i=i+1)

    {


     
        dump_Data(&dd1);
        // printf("Destination memory block: "); print_16Words(dd1.virtual_destination_addr, dd1.data_collected);
        
        
        // write_toFile( fp1, dd1.virtual_destination_addr, dd1.data_collected);
        
        memcpy(memory_LongStore+total_data,dd1.virtual_destination_addr,dd1.data_collected);
        memset(dd1.virtual_destination_addr, 0xff, dd1.data_collected );
        total_data +=dd1.data_collected;
    }


    t = clock()-t; 
    double time_taken = ((double)t)/CLOCKS_PER_SEC; // in seconds 
  
    printf("transfered at %f MBps for %u cycles \n",total_data /( (N)*time_taken *1024 *1024),N); 


    write_toFile( fp1, memory_LongStore, total_data);




    munmap((void *)memory_LongStore,LONG_MEM);
    munmap((void *)dd1.virtual_destination_addr, (size_t)DATA_SIZE);
    munmap((void *)dd1.virtual_dma_addr, (size_t)DATA_SIZE);
    close(mem_fd);
    fclose(fp1);
    fclose(fp2);

    


    gpio_close();


    return 0;


}