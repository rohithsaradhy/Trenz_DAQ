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
    pins_vals[0] = 0;
    pins_vals[1] = 0;
    pins_vals[2] = 0;
    pins_vals[3] = 0;
    set_gpio(pins_vals);

    int mem_fd = open("/dev/mem", O_RDWR | O_SYNC); // Open /dev/mem which represents the whole physical memory
   

    unsigned int* vda;

    char file_name1[100]="data_files/ddmtd1.txt";
    char file_name2[100]="data_files/ddmtd2.txt";


    remove(file_name1);
    remove(file_name2);


    FILE * fp1,* fp2;
    fp1 = fopen(file_name1,"a");
    fp2 = fopen(file_name2,"a");
    




    // // data.values[0] = 1; //Reset
    // // data.values[1] = 1; //S_Axis_ENABLE
    // // data.values[2] = 1; //S_Axis_ENABLE
    // // ret = ioctl(req.fd,GPIOHANDLE_SET_LINE_VALUES_IOCTL,&data);

    // // mydelay(1000);



    for(int i=0;i<N;i=i+1)
    {
        
        pins_vals[0] = 0;
        pins_vals[1] = 0;
        pins_vals[2] = 1;
        pins_vals[3] = 0;
        set_gpio(pins_vals);


        vda = transfer_Data(mem_fd,0x6e000000) ;
        write_toFile( fp1, vda, DATA_SIZE);
        // printf("Destination memory block: "); print_16Words(vda, DATA_SIZE);
        // printf("Destination memory block: "); memdump(vda, DATA_SIZE);


        usleep(100);
        pins_vals[0] = 0;
        pins_vals[1] = 1;
        pins_vals[2] = 0;
        pins_vals[3] = 0;
        set_gpio(pins_vals);

        usleep(100);
        vda = transfer_Data(mem_fd,0x7e000000) ;
        write_toFile( fp2, vda, DATA_SIZE);
        // printf("Destination memory block: "); print_16Words(vda, DATA_SIZE);
        // printf("Destination memory block: "); memdump(vda, DATA_SIZE);
    }
    fclose(fp1);
    fclose(fp2);

    


    gpio_close();


    return 0;




    // data.values[1] = 0; //Choose DDMTD_Sampler
    // data.values[0] = 1; //Reset
    // ret = ioctl(req.fd,GPIOHANDLE_SET_LINE_VALUES_IOCTL,&data);
    // sleep(1);

    // data.values[0] = 0; //Reset
    // data.values[1] = 1; //Choose DDMTD_Sampler
    // ret = ioctl(req.fd,GPIOHANDLE_SET_LINE_VALUES_IOCTL,&data);
    // vda = transfer_Data(mem_fd,0x6e000000) ;
    // printf("Destination memory block: "); memdump(vda, DATA_SIZE);

 



    // data.values[0] = 0;
    // data.values[1] = 0;
    // ret = ioctl(req.fd,GPIOHANDLE_SET_LINE_VALUES_IOCTL,&data);

}