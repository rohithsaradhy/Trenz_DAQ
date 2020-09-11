#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <stdint.h>
#include <unistd.h>
#include <assert.h>
#include "dma_acq.h"
#include "stream_shared.h"
#include "dma-proxy.h"

static struct dma_proxy_channel_interface *tx_proxy_interface_p;
static int tx_proxy_fd;
static int test_size; 


/* The following function is the transmit thread to allow the transmit and the
 * receive channels to be operating simultaneously. The ioctl calls are blocking
 * such that a thread is needed.
 */
void *tx_thread(int dma_count)
{
	int dummy, i, counter;

	/* Set up the length for the DMA transfer and initialize the transmit
 	 * buffer to a known pattern.
 	 */
	tx_proxy_interface_p->length = test_size;

	for (counter = 0; counter < dma_count; counter++) {
    		for (i = 0; i < test_size; i++)
       			tx_proxy_interface_p->buffer[i] = counter + i;

		/* Perform the DMA transfer and the check the status after it completes
	 	 * as the call blocks til the transfer is done.
 		 */
		ioctl(tx_proxy_fd, 0, &dummy);

		if (tx_proxy_interface_p->status != PROXY_NO_ERROR)
			printf("Proxy tx transfer error\n");
	}
}


int main(int argc, char *argv[])
{

    int rx_proxy_fd, i;
    struct dma_proxy_channel_interface *rx_proxy_interface_p;
	int dummy;
    int counter;
    
	rx_proxy_fd = open("/dev/dma_proxy_rx", O_RDWR);
	if (rx_proxy_fd < 1) {
		printf("Unable to open DMA proxy device file");
		exit(EXIT_FAILURE);
	}



	rx_proxy_interface_p = (struct dma_proxy_channel_interface *)mmap(NULL, sizeof(struct dma_proxy_channel_interface),
									PROT_READ | PROT_WRITE, MAP_SHARED, rx_proxy_fd, 0);

    if ((rx_proxy_interface_p == MAP_FAILED)) {
		printf("Failed to mmap\n");
		exit(EXIT_FAILURE);
	}



    int test_size = 100;
    for (i = 0; i < test_size; i++)
        rx_proxy_interface_p->buffer[i] = 0xff;

    ioctl(rx_proxy_fd, 0, &dummy);

    memdump(rx_proxy_interface_p->buffer,test_size);



    munmap(rx_proxy_interface_p, sizeof(struct dma_proxy_channel_interface));


    close(rx_proxy_fd);

    // printf("It comes till here");
    // usleep(10000);

}
