// From : http://stackoverflow.com/questions/13124271/driving-beaglebone-gpio-through-dev-mem
//
// Be sure to set -O3 when compiling.
// Modified by Eric Taylor  29-Sept-2014

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h> 
#include <signal.h>    // Defines signal-handling functions (i.e. trap Ctrl-C)
#include "beaglebone_gpio.h"

/****************************************************************
 * Global variables
 ****************************************************************/
int keepgoing = 1;    // Set to 0 when ctrl-c is pressed

/****************************************************************
 * signal_handler
 ****************************************************************/
void signal_handler(int sig);
// Callback called when SIGINT is sent to the process (Ctrl-C)
void signal_handler(int sig)
{
	printf( "\nCtrl-C pressed, cleaning up and exiting...\n" );
	keepgoing = 0;
}

int main(int argc, char *argv[]) {
    volatile void *gpio_addr0;
    volatile unsigned int *gpio_oe_addr0;
    volatile unsigned int *gpio_setdataout_addr0;
    volatile unsigned int *gpio_cleardataout_addr0;

    volatile unsigned int *gpio_datain_addr0;

    volatile void *gpio_addr1;
    volatile unsigned int *gpio_oe_addr1;
    volatile unsigned int *gpio_setdataout_addr1;
    volatile unsigned int *gpio_cleardataout_addr1;
    
    unsigned int reg;
    unsigned int reg1; 
    // Set the signal callback for Ctrl-C
	signal(SIGINT, signal_handler);

    int fd = open("/dev/mem", O_RDWR);
	
    printf("Mapping %X - %X (size: %X)\n", GPIO0_START_ADDR, GPIO0_END_ADDR, GPIO0_SIZE);
    printf("Mapping %X - %X (size: %X)\n", GPIO1_START_ADDR, GPIO1_END_ADDR, GPIO1_SIZE);

  
    gpio_addr0 = mmap(0, GPIO0_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, GPIO0_START_ADDR);
    gpio_addr1 = mmap(0, GPIO1_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, GPIO1_START_ADDR);

    gpio_oe_addr1           = gpio_addr1 + GPIO_OE;
    gpio_setdataout_addr1   = gpio_addr1 + GPIO_SETDATAOUT;
    gpio_cleardataout_addr1 = gpio_addr1 + GPIO_CLEARDATAOUT;

    gpio_datain_addr0       = gpio_addr0 + GPIO_DATAIN;
    gpio_oe_addr0           = gpio_addr0 + GPIO_OE;
    gpio_setdataout_addr0   = gpio_addr0 + GPIO_SETDATAOUT;
    gpio_cleardataout_addr0 = gpio_addr0 + GPIO_CLEARDATAOUT;


    if(gpio_addr1 == MAP_FAILED) {
        printf("Unable to map GPIO\n");
        exit(1);
    }
    printf("GPIO mapped to %p\n", gpio_addr1);
    printf("GPIO OE mapped to %p\n", gpio_oe_addr1);
    printf("GPIO SETDATAOUTADDR mapped to %p\n", gpio_setdataout_addr1);
    printf("GPIO CLEARDATAOUT mapped to %p\n", gpio_cleardataout_addr1);

    // Set USR3 to be an output pin
    reg = *gpio_oe_addr1;
    printf("GPIO1 configuration: %X\n", reg);
    reg &= ~USR3 | USR2 | ~USR1 | USR0;       // Set USR3 bit to 0
    *gpio_oe_addr1 = reg;
    printf("GPIO1 configuration: %X\n", reg);

    printf("Start blinking LED USR3\n");
    while(keepgoing) {
        // printf("ON\n");
	reg = GPIO_30 & ~*gpio_datain_addr0;
	reg1 = GPIO_31 & ~*gpio_datain_addr0;
        
	if(reg)
		*gpio_setdataout_addr1 = USR3;
	else
		*gpio_cleardataout_addr1 = USR3;

	if(reg1)
                *gpio_setdataout_addr1 = USR1;
        else
                *gpio_cleardataout_addr1 = USR1;

    }

    munmap((void *)gpio_addr1, GPIO1_SIZE);
    close(fd);
    return 0;
}
