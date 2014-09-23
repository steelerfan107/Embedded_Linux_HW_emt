// Etch_a_Sketch
// September 22 2014
// Eric Taylor
//
// This etch-a-sketch program uses four push buttons to control an 8x8 led array over an i2c

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
#include "i2c-dev.h"
#include "i2cbusses.h"

#define BICOLOR 
#define POLL_TIMEOUT (3 * 1000)
#define MAX_BUF 64

//	I2C Functions used from Example Program
//
//
//---------------------------------------------------------
//

static void help(void) __attribute__ ((noreturn));

static void help(void) {
        fprintf(stderr, "Usage: matrixLEDi2c (hardwired to bus 3, address 0x70)\n");
        exit(1);
}

static int check_funcs(int file) {
        unsigned long funcs;

        /* check adapter functionality */
        if (ioctl(file, I2C_FUNCS, &funcs) < 0) {
                fprintf(stderr, "Error: Could not get the adapter "
                        "functionality matrix: %s\n", strerror(errno));
                return -1;
        }

        if (!(funcs & I2C_FUNC_SMBUS_WRITE_BYTE)) {
                fprintf(stderr, MISSING_FUNC_FMT, "SMBus send byte");
                return -1;
        }
        return 0;
}

// Writes block of data to the display
static int write_block(int file, __u16 *data) {
        int res;
#ifdef BICOLOR
        res = i2c_smbus_write_i2c_block_data(file, 0x00, 16,
                (__u8 *)data);
        return res;
#else
/*
 * For some reason the single color display is rotated one column, 
 * so pre-unrotate the data.
 */
        int i;
        __u16 block[I2C_SMBUS_BLOCK_MAX];
//      printf("rotating\n");
        for(i=0; i<8; i++) {
                block[i] = (data[i]&0xfe) >> 1 |
                           (data[i]&0x01) << 7;
        }
        res = i2c_smbus_write_i2c_block_data(file, 0x00, 16,
                (__u8 *)block);
        return res;
#endif
}


int keepgoing = 1;


int main(int argc, char** argv, char**envp)
{
	// Initialize bitmap to be displayed
	__u16 board[]=
		{0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000};

        struct pollfd fdset[5];
        int nfds = 5;
        int gpio_fd1,gpio_fd2,gpio_fd3,gpio_fd4, timeout, rc;
        char buf[MAX_BUF];
        unsigned int gpio_in1,gpio_in2,gpio_in3,gpio_in4;
        unsigned int gpio_out1,gpio_out2,gpio_out3,gpio_out4;
	int x,y,wait,size;

	int res, i2cbus, address, file;
        char filename[20];
        int force = 0;
        int demo = 0;
	size = 8;

	// Find the i2c Bus
	i2cbus = lookup_i2c_bus("1");
        printf("i2cbus = %d\n", i2cbus);
        if (i2cbus < 0)
                help();
	
	// Getthe address of the led array
        address = parse_i2c_address("0x70");
        printf("address = 0x%2x\n", address);
        if (address < 0)
                help();

	// Get the file to operate the i2c bus
        file = open_i2c_dev(i2cbus, filename, sizeof(filename), 0);
        if (file < 0
         || check_funcs(file)
         || set_slave_addr(file, address, force))
                exit(1);

        // Check the return value on these if there is trouble
        i2c_smbus_write_byte(file, 0x21); // Start oscillator (p10)
        i2c_smbus_write_byte(file, 0x81); // Disp on, blink off (p11)
        i2c_smbus_write_byte(file, 0xe7); // Full brightness (page 15)


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
		write_block(file, board);
		// Poll the buttons
                rc = poll(fdset, nfds, timeout);

		// If top button is pressed
                if (fdset[1].revents){
                        lseek(fdset[1].fd, 0, SEEK_SET);
                        len = read(fdset[1].fd, buf, MAX_BUF);

			// Check to see if boundry is hit
			if(y > 0 && wait > 2){	
				y = y - 1;
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
			}else
				wait++;
                }

		//y Corresponds to row in the bit map. Then x is how far in the bit has to be turned on, so I used a shift
		board[y] = (1 << x) | board[y];
                fflush(stdout);

        }
	
	// Close GPIO ports
        gpio_fd_close(gpio_fd1);
        gpio_fd_close(gpio_fd2);
        gpio_fd_close(gpio_fd3);
        gpio_fd_close(gpio_fd4);

	// Close ncursers

        return 0;
}

