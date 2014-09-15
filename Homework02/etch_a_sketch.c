// Etch - A - Sketch
// Eric Taylor
// ECE 597
//
// run ./a.out <size> 
// size is the size of the etch-a-sketch board
//
// A simple program that uses interruts and 4 push buttons to create an etch-a-sketch game

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
        struct pollfd fdset[5];
        int nfds = 5;
        int gpio_fd1,gpio_fd2,gpio_fd3,gpio_fd4, timeout, rc;
        char buf[MAX_BUF];
        unsigned int gpio_in1,gpio_in2,gpio_in3,gpio_in4;
        unsigned int gpio_out1,gpio_out2,gpio_out3,gpio_out4;
	int x,y,wait,size;
	
	//Check to see proper number of inputs
	if(argc < 2)
	{
		printf("Input Size of the Etch-a-Sketch\n");
		return 0;
	}

	// Convert Size
	size = atoi(argv[1]);

	wait = 0;
	x = 0;
	y = -1;

        int len;
	
	// Set GPIO pin numbers
        gpio_in1 = 30;
        gpio_in2 = 31;
        gpio_in3 = 48;
        gpio_in4 = 60;

	// Open each of the GPIO ports
        gpio_export(gpio_in1);
        gpio_export(gpio_in2);
        gpio_export(gpio_in3);
        gpio_export(gpio_in4);

	// Make all the GPIO ports inputs
        gpio_set_dir(gpio_in1, "in");
        gpio_set_dir(gpio_in2, "in");
        gpio_set_dir(gpio_in3, "in");
        gpio_set_dir(gpio_in4, "in");

        // Only trigger on the rising edge of the button
	gpio_set_edge(gpio_in1, "rising");
        gpio_set_edge(gpio_in2, "rising");
        gpio_set_edge(gpio_in3, "rising");
        gpio_set_edge(gpio_in4, "rising");

	// Open the button
        gpio_fd1 = gpio_fd_open(gpio_in1, O_RDONLY);
        gpio_fd2 = gpio_fd_open(gpio_in2, O_RDONLY);
        gpio_fd3 = gpio_fd_open(gpio_in3, O_RDONLY);
        gpio_fd4 = gpio_fd_open(gpio_in4, O_RDONLY);

        timeout = POLL_TIMEOUT;
	
	// Initiate ncursers
	initscr();
	refresh();

	// Print the Numbers on the Boarder
	int i;
	for(i = 0; i < size; i++){
		mvprintw(0, i*3, "%d",i);
		refresh();
	}
	
	for(i = 0; i < size; i++){
		mvprintw(i, 0, "%d",i);
		refresh();
	}
	

	// Running Loop
        while(keepgoing){
                memset((void*)fdset, 0, sizeof(fdset));

		// Setup interrups for each button
                fdset[1].fd = gpio_fd1;
                fdset[1].events = POLLPRI;

                fdset[2].fd = gpio_fd2;
                fdset[2].events = POLLPRI;

                fdset[3].fd = gpio_fd3;
                fdset[3].events = POLLPRI;

                fdset[4].fd = gpio_fd4;
                fdset[4].events = POLLPRI;

		// Poll the buttons
                rc = poll(fdset, nfds, timeout);

		// If top button is pressed
                if (fdset[1].revents){
                        lseek(fdset[1].fd, 0, SEEK_SET);
                        len = read(fdset[1].fd, buf, MAX_BUF);

			// Check to see if boundry is hit
			if(y > 0 && wait > 2){	
				y = y - 1;
				mvprintw(y+1, (x+1)*3, "X");
				refresh();
	            	}
				wait++;
                      
                }
		// If left button is pressed
     	        if (fdset[2].revents){
		        lseek(fdset[2].fd, 0, SEEK_SET);
                        len = read(fdset[2].fd, buf, MAX_BUF);

			// Check to see if boundry is hit
			if(x > 0 && wait > 2){
				x = x - 1;
				mvprintw(y+1, (x+1)*3, "X");
				refresh();
			}else
				wait++;
			

                }
		// If right button pressed
                if (fdset[3].revents){
                        lseek(fdset[3].fd, 0, SEEK_SET);
                        len = read(fdset[3].fd, buf, MAX_BUF);

			// Check to see if boundry is hit
			if(x < size && wait > 2){
				x = x + 1;
				mvprintw(y+1, (x+1)*3, "X");
				refresh();
			}else
				wait++;
			

                }
		// If bottom button is pressed
                if (fdset[4].revents){
                        lseek(fdset[4].fd, 0, SEEK_SET);
                        len = read(fdset[4].fd, buf, MAX_BUF);
	
			// Check to see if boundry is hit
			if(y < size && wait > 2){
				y = y + 1;
				mvprintw(y+1, (x+1)*3, "X");
				refresh();
			}else
				wait++;
                }

		
                fflush(stdout);

        }
	
	// Close GPIO ports
        gpio_fd_close(gpio_fd1);
        gpio_fd_close(gpio_fd2);
        gpio_fd_close(gpio_fd3);
        gpio_fd_close(gpio_fd4);

	// Close ncursers
     	endwin();

        return 0;
}

