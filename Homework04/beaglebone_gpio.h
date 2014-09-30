// From : http://stackoverflow.com/questions/13124271/driving-beaglebone-gpio-through-dev-mem
#ifndef _BEAGLEBONE_GPIO_H_
#define _BEAGLEBONE_GPIO_H_

//#define GPIO1_START_ADDR 0x4804C000
#define GPIO0_START_ADDR 0x44e07000
#define GPIO0_END_ADDR 0x44e09000
#define GPIO0_SIZE (GPIO0_END_ADDR - GPIO0_START_ADDR)

#define GPIO1_START_ADDR 0x4804C000
#define GPIO1_END_ADDR 0x4804e000
#define GPIO1_SIZE (GPIO1_END_ADDR - GPIO1_START_ADDR)

#define GPIO_OE 0x134
#define GPIO_DATAIN 0x138
#define GPIO_SETDATAOUT 0x194
#define GPIO_CLEARDATAOUT 0x190

#define USR0 (1<<21)
#define USR1 (1<<22)
#define USR2 (1<<23)
#define USR3 (1<<24)
#define GPIO_03  (1<<3)
#define GPIO_07  (1<<7)
#define GPIO_60  (1<<28)
#define GPIO_30  (1<<30)
#define GPIO_31  (1<<31)
#define GPIO_5   (1<<5)
#define GPIO_14   (1<<14)

#define QPOSCN 0x0000
#define QPOSINIT 0x0004
#define QPOSMAX 0x0008
#define QPOSCMP 0x000C
#define QPOSILAT 0x0010
#define QPOSSLAT 0x0014
#define QPOSLAT 0x0018
#define QUTMR 0x001C
#define QUPRD 0x0020
#define QWDTMR 0x0024
#define QWDPRD 0x0026
#define QDECCTL 0x0028
#define QEPCTL 0x002A
#define QCAPCTL 0x002C
#define QPOSCTL 0x002E
#define QEINT 0x0030
#define QFLG 0x0032
#define QCLR 0x0034
#define QFRC 0x0036
#define QEPSTS 0x0038
#define QCTMR 0x003A
#define QCPRD 0x003C
#define QCTMRLAT 0x003E
#define QCPRDLAT 0x0040
#define QREVID 0x005C

#endif
