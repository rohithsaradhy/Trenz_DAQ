#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/gpio.h>




int main(void)
{
    int f;
    int ret;
    struct gpiohandle_request req;
    struct gpiohandle_data data;

    f = open("/dev/gpiochip0", O_RDONLY);
    req.lineoffsets[0] = 0;
    req.flags = GPIOHANDLE_REQUEST_OUTPUT;
    req.default_values[0] = 1;
    strcpy(req.consumer_label,"gpio-0-output");
    req.lines = 1;
    ret = ioctl(f,GPIO_GET_LINEHANDLE_IOCTL,&req);

    
    
    // The Values
    data.values[0] = 0;
    ret = ioctl(req.fd,GPIOHANDLE_SET_LINE_VALUES_IOCTL,&data);
    sleep(1);
    data.values[0] = 1;
    ret = ioctl(req.fd,GPIOHANDLE_SET_LINE_VALUES_IOCTL,&data);
    sleep(1);
    data.values[0] = 0;
    ret = ioctl(req.fd,GPIOHANDLE_SET_LINE_VALUES_IOCTL,&data);
    sleep(1);

    close(f);
    return 0 ;

}