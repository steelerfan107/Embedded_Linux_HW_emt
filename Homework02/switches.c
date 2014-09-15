// switches
// ECE 597
// Eric Taylor
//
// run ./a.out
//
// Simple program that takes 4 push buttons as inputs and turns on 4 LEDS



#include "gpio-utils.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <poll.h>
#include <signal.h>

#define POLL_TIMEOUT (3 * 1000)
#define MAX_BUF 64

int keepgoing = 1;

int main(int argc, char** argv, char**envp)
{
	struct pollfd fdset[5];
	int nfds = 5;
	int gpio_fd1,gpio_fd2,gpio_fd3,gpio_fd4,gpio_fd5, gpio_fd6, gpio_fd7, gpio_fd8, timeout, rc;
	char buf[MAX_BUF];
	unsigned int gpio_in1,gpio_in2,gpio_in3,gpio_in4;
	unsigned int gpio_out1,gpio_out2,gpio_out3,gpio_out4;

	int len;
	
	// Input which ports the push buttons are connected to
	gpio_in1 = 30;
	gpio_in2 = 31;
	gpio_in3 = 48;
	gpio_in4 = 60;
	
	// Input which ports have LEDS attached
	gpio_out1 = 3;
        gpio_out2 = 49;
        gpio_out3 = 2;
        gpio_out4 = 115;

	// Open up the GPIO ports
	gpio_export(gpio_in1);
	gpio_export(gpio_in2);
	gpio_export(gpio_in3);
	gpio_export(gpio_in4);

	gpio_export(gpio_out1);
        gpio_export(gpio_out2);
        gpio_export(gpio_out3);
        gpio_export(gpio_out4);

	// Set the button to inputs
	gpio_set_dir(gpio_in1, "in");
	gpio_set_dir(gpio_in2, "in");
	gpio_set_dir(gpio_in3, "in");
	gpio_set_dir(gpio_in4, "in");

	// Set the LEDs to Outputs
	gpio_set_dir(gpio_out1, "out");
        gpio_set_dir(gpio_out2, "out");
        gpio_set_dir(gpio_out3, "out");
        gpio_set_dir(gpio_out4, "out");

	// Trigger on bothe edges
	gpio_set_edge(gpio_in1, "both");
	gpio_set_edge(gpio_in2, "both");
	gpio_set_edge(gpio_in3, "both");
	gpio_set_edge(gpio_in4, "both");

	// Open all of the GPIO ports
	gpio_fd1 = gpio_fd_open(gpio_in1, O_RDONLY);
	gpio_fd2 = gpio_fd_open(gpio_in2, O_RDONLY);
	gpio_fd3 = gpio_fd_open(gpio_in3, O_RDONLY);
	gpio_fd4 = gpio_fd_open(gpio_in4, O_RDONLY);

	gpio_fd5 = gpio_fd_open(gpio_out1, O_WRONLY);
        gpio_fd6 = gpio_fd_open(gpio_out2, O_WRONLY);
        gpio_fd7 = gpio_fd_open(gpio_out3, O_WRONLY);
        gpio_fd8 = gpio_fd_open(gpio_out4, O_WRONLY);




	timeout = POLL_TIMEOUT;

	while(keepgoing){
		memset((void*)fdset, 0, sizeof(fdset));
		
		// Setup interrupts for each button

		fdset[0].fd = STDIN_FILENO;
		fdset[0].events = POLLIN;

		fdset[1].fd = gpio_fd1;
		fdset[1].events = POLLPRI;

		fdset[2].fd = gpio_fd2;
                fdset[2].events = POLLPRI;

		fdset[3].fd = gpio_fd3;
                fdset[3].events = POLLPRI;
	
		fdset[4].fd = gpio_fd4;
                fdset[4].events = POLLPRI;


		rc = poll(fdset, nfds, timeout);

		if( rc < 0){
			printf("\n POLL FAIL \n");
			return -1;
		}
		
		// Output to show program is still running
		if ( rc == 0){
			printf(".");
		}

		
		// If button is pressed
		if (fdset[1].revents){
                        lseek(fdset[1].fd, 0, SEEK_SET);
                        len = read(fdset[1].fd, buf, MAX_BUF);
                        printf("\npoll() GPIO %d int occured, value = %c, len %d\n",gpio_in1,buf[0],len);
			
			// Toggle LED
			gpio_set_value(gpio_out1,buf[0]-48);
                }
		// If button is pressed  
		if (fdset[2].revents){
                        lseek(fdset[2].fd, 0, SEEK_SET);
                        len = read(fdset[2].fd, buf, MAX_BUF);
                        printf("\npoll() GPIO %d int occured, value = %c, len %d\n",gpio_in2,buf[0],len);
	
			//Toggle LED
			gpio_set_value(gpio_out4,buf[0]-48);

                }  
		// If button is pressed
		if (fdset[3].revents){
                        lseek(fdset[3].fd, 0, SEEK_SET);
                        len = read(fdset[3].fd, buf, MAX_BUF);
                        printf("\npoll() GPIO %d int occured, value = %c, len %d\n",gpio_in3,buf[0],len);
		
			// Toggle LED
			gpio_set_value(gpio_out2,buf[0]-48);

                }  
		// If button is pressed
		if (fdset[4].revents){
			lseek(fdset[4].fd, 0, SEEK_SET);
			len = read(fdset[4].fd, buf, MAX_BUF);
			printf("\npoll() GPIO %d int occured, value = %c, len %d\n",gpio_in4,buf[0],len);

			// Toggle LED
			gpio_set_value(gpio_out3,buf[0]-48);
		}
		fflush(stdout);

	}

	gpio_fd_close(gpio_fd1);
	gpio_fd_close(gpio_fd2);
	gpio_fd_close(gpio_fd3);
	gpio_fd_close(gpio_fd4);
	gpio_fd_close(gpio_fd5);
        gpio_fd_close(gpio_fd6);
        gpio_fd_close(gpio_fd7);
        gpio_fd_close(gpio_fd8);

	return 0;}
