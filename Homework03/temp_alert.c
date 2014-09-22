#include "gpio-utils.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <poll.h>
#include <signal.h>
#include <ncurses.h>

#define POLL_TIMEOUT (3 * 1000)
#define MAX_BUF 64

int keepgoing = 1;


int main(int argc, char** argv, char**envp)
{
        struct pollfd fdset[2];
        int nfds = 2;
        int gpio_fd1, timeout, rc;
	int len,wait;
        char buf[MAX_BUF];
        unsigned int gpio_in1;
        unsigned int gpio_out1;
	wait = 0;
	// Set GPIO pin numbers
        gpio_in1 = 50;

	// Open each of the GPIO ports
        gpio_export(gpio_in1);

	// Make all the GPIO ports inputs
        gpio_set_dir(gpio_in1, "in");

        // Only trigger on the rising edge of the button
	gpio_set_edge(gpio_in1, "rising");

	// Open the button
        gpio_fd1 = gpio_fd_open(gpio_in1, O_RDONLY);

        timeout = POLL_TIMEOUT;
	
	// Set Registers to Alert low and high temps
	system("./setAlertTemp.sh");
	
	// Running Loop
        while(keepgoing){
                memset((void*)fdset, 0, sizeof(fdset));

		// Setup interrups for each button
                fdset[1].fd = gpio_fd1;
                fdset[1].events = POLLPRI;

		// Poll the buttons
                rc = poll(fdset, nfds, timeout);

		// If Alert Pin is Triggered
                if (fdset[1].revents){
                        lseek(fdset[1].fd, 0, SEEK_SET);
                        len = read(fdset[1].fd, buf, MAX_BUF);
			//system("./setAlertTemp.sh");	
			if(wait == 1){
			  printf("\nALERT TEMP!\n");
			  system("./takeTemp.sh");
			}else{
			  wait++;
			}
                      
                }

		if (rc == 0) {
		//	system("i2cget -y 1 0x48 0");

			printf(".");
		}
	
                fflush(stdout);

        }
	
	// Close GPIO ports
        gpio_fd_close(gpio_fd1);
        return 0;
}

