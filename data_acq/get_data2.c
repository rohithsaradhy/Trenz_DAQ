#include "dma_acq.h"






int main(int argc, char** argv) {


    int N;
    if(argc == 2)
        N=atoi(argv[1]); 
    else
        N=1;


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

    



    int mem_fd = open("/dev/mem", O_RDWR | O_SYNC); // Open /dev/mem which represents the whole physical memory
   

    unsigned int* vda;

    char file_name1[100]="data_files/ddmtd1.txt";
    char file_name2[100]="data_files/ddmtd2.txt";


    remove(file_name1);
    remove(file_name2);


    FILE * fp1,* fp2;
    fp1 = fopen(file_name1,"a");
    fp2 = fopen(file_name2,"a");
    




    for(int i=0;i<N;i=i+1)
    {
        data.values[0] = 0; //Reset
        data.values[1] = 0; //S_Axis_ENABLE
        ret = ioctl(req.fd,GPIOHANDLE_SET_LINE_VALUES_IOCTL,&data);
        vda = transfer_Data(mem_fd,0x6e000000) ;
        // printf("Destination memory block: "); print_16Words(vda, DATA_SIZE);
        write_toFile( fp1, vda, DATA_SIZE);


        data.values[0] = 0; //Reset
        data.values[1] = 1; //S_Axis_ENABLE
        ret = ioctl(req.fd,GPIOHANDLE_SET_LINE_VALUES_IOCTL,&data);
        vda = transfer_Data(mem_fd,0x6e000000) ;
        // printf("Destination memory block: "); print_16Words(vda, DATA_SIZE);
        write_toFile( fp2, vda, DATA_SIZE);
    }


    





    fclose(fp1);
    fclose(fp2);


    // data.values[1] = 0; //Choose DDMTD_Sampler
    // data.values[0] = 1; //Reset
    // ret = ioctl(req.fd,GPIOHANDLE_SET_LINE_VALUES_IOCTL,&data);
    // sleep(1);

    // data.values[0] = 0; //Reset
    // data.values[1] = 1; //Choose DDMTD_Sampler
    // ret = ioctl(req.fd,GPIOHANDLE_SET_LINE_VALUES_IOCTL,&data);
    // vda = transfer_Data(mem_fd,0x6e000000) ;
    // printf("Destination memory block: "); memdump(vda, DATA_SIZE);

 



    data.values[0] = 0;
    data.values[1] = 0;
    ret = ioctl(req.fd,GPIOHANDLE_SET_LINE_VALUES_IOCTL,&data);

}