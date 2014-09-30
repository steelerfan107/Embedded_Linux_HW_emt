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
#include "beaglebone_gpio.h"

#define BICOLOR 
#define POLL_TIMEOUT (3 * 1000)
#define MAX_BUF 64

#define QEP0_BASE 0x48300000
#define QEP1_BASE 0x48302000
#define QEP2_BASE 0x48304000
#define EQEP_OFFSET 0x180

void reset_eqep(){
	int fd;


	fd = open("/sys/devices/ocp.3/48302000.epwmss/48302180.eqep/enable",O_WRONLY);
	write(fd, "0", 2);
	write(fd, "1", 2);	
	close(fd);


        fd = open("/sys/devices/ocp.3/48302000.epwmss/48304180.eqep/enable",O_WRONLY);
        write(fd, "0", 2);
        write(fd, "1", 2); 
        close(fd);

}

int eqep_get_pos(int channel)
{
	int len;
	int fd;
	char buf[MAX_BUF];
	char ch[2];
	
	if(channel == 1){
		len = snprintf(buf, sizeof(buf), "/sys/devices/ocp.3/48302000.epwmss/48302180.eqep/position");
	}else{
		len = snprintf(buf, sizeof(buf), "/sys/devices/ocp.3/48304000.epwmss/48304180.eqep/position");
	}
	fd = open(buf, O_RDONLY);
	if (fd < 0) {
		return -1;
	}
	read(fd, ch, 1); 
	
	len = atoi(ch);
	
	close(fd);
	return len;
}


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
	size= 8;
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
	int x_pos;
	int y_pos;
	char* x_in;
	char* y_in;

	reset_eqep();
	// Running Loop
        while(keepgoing){
			
                memset((void*)fdset, 0, sizeof(fdset));
		
		write_block(file, board);
		x_pos = eqep_get_pos(1);
		y_pos = eqep_get_pos(2);

		printf("%d,%d\n",x_pos,y_pos);

		if(x_pos > 7)
			x_pos = 7;
		else if(x_pos <= 0)
			x_pos = 0;

		if(y_pos > 7)
			y_pos = 7;
		else if(y_pos <= 0)
			y_pos = 0;

		//y Corresponds to row in the bit map. Then x is how far in the bit has to be turned on, so I used a shift
		board[y_pos] = (1 << x_pos) | board[y_pos];

		
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

