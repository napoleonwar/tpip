#include <stdio.h>
#include <stdlib.h>
#include <machine/patmos.h>
#include <string.h>

#include "slip.h"
#include "tpip.h"
#include "ethlib/tte.h"
#include "ethlib/eth_mac_driver.h"

static unsigned char bufin[2000];
static unsigned char bufout[2000];
volatile _IODEV int *led_ptr = (volatile _IODEV int *) PATMOS_IO_LED;
#define N 10000 / 10
int i = 0;
signed long long error[N]; // for logging clock precision
unsigned long long r_pit[N];
static unsigned long long receive_point; //clock cycles at 80MHz (12.5 ns)

int sched_errors = 0;
int tte = 0;
int eth = 0;

int main(){
  int n = 2010;
  unsigned char CT[] = {0xAB,0xAD,0xBA,0xBE};
	unsigned char VL0[] = {0x0F,0xA1};
  unsigned char reply;

  //int_period = 10ms, cluster cycle=20ms, CT, 1 VL sending, max_delay, 
 //comp_delay, precision
	tte_initialize(100,200,CT,1,0x2A60,0xFA0,0x33E);
  tte_init_VL(1, 10, 20); // VL 4002 starts at 1ms and has a period of 2ms
  tte_start_ticking(1,0,0);

	unsigned int send_addr[] = {0x800,0xE00,0x1400,0x1A00,0x2000};
  unsigned int cur_RX_BD = 0x600;
  unsigned int ext_RX_BD = 0x608;
  unsigned int cur_RX = 0x000;
	unsigned int ext_RX = 0x800;
  unsigned long long rec_point[n];
	unsigned long long sched_point[n];

	static unsigned long long receive_point;
	static unsigned long long send_point;
 
    memset(bufin, 0, sizeof(bufin));
    memset(bufout, 0, sizeof(bufout));

    printf("sizeof(unsigned char) =%lu\n", sizeof(unsigned char));
    printf("sizeof(unsigned short)=%lu\n", sizeof(unsigned short));
    printf("sizeof(unsigned int)  =%lu\n", sizeof(unsigned int));

    set_mac_address(0x1D000400, 0x00000289);
 
  eth_iowr(0x04, 0x00000004); // clear receive frame bit in int_source
  eth_iowr(cur_RX_BD + 4,
           cur_RX); // set first receive buffer to store frame in 0x000
  eth_iowr(cur_RX_BD, 0x0000C000); // set empty and IRQ and not wrap
  eth_iowr(ext_RX_BD + 4,
           ext_RX); // set second receive buffer to store frame in 0x800
  eth_iowr(ext_RX_BD, 0x00006000); // set NOT empty, IRQ and wrap

  for(int i = 0; i < n; i++){
    // prepare sending
    //   patmos, 10.0.0.2, 10002
    //   server, 10.0.0.3, 10003
    printf("Get into loop.\n");
    ip_t ipout = {.verhdl = (0x4 << 4) | 0x5,
                  .tos = 0x03, //0x00,
                  .length = 20 + 8 + 4, // 5 + 2 + 1 words
                  .id = 1,
                  .ff = 0x4000,
                  .ttl = 0x30,
                  .prot = 0x11, // UDP
                  .checksum = 0x0000,
                  .srcip = (10 << 24) | (0 << 16) | (0 << 8) | 2,
                  .dstip = (192 << 24) | (168 << 16) | (96 << 8) | 129,
                  .udp.srcport = 10002, // 0x2712
                  .udp.dstport = 10003, // 0x2713
                  .udp.length = 8 + 4,
                  .udp.data = (unsigned char[]){0, 0, 0, 1}};//(unsigned char)obb_msg.flags}};
    int len = packip(bufout, &ipout);
    bufprint(bufout, len);
    tte_prepare_data(0x2000, VL0, bufout, 1514);
    tte_schedule_send(0x2000, 1514, 0); 
    *led_ptr = 0x1;
    printf("LED1 turns on, waiting for ethernet package.\n");
    tte_wait_for_message(&receive_point,1000);
    *led_ptr = 0x2;
    tte_clear_free_rx_buffer(ext_RX_BD); // start receiving in other buffer
    *led_ptr = 0x4;
    reply=tte_receive_log(cur_RX, receive_point, error, i);
    printf("Receive %d:", reply);
    switch(reply){
      case 0: // failed pcf, stop test
        *led_ptr = 0x70;
        printf("pcf out of schedule \n");
        // n = i + 1;
        break;
      case 1: // successfull pcf
        *led_ptr = 0x10;
        r_pit[i] = receive_point;
        // printf("#%d\t0.000\t%.3f\n", i, (int) error[i]);
        //printSegmentInt((unsigned) abs(error[i]));
        i++;
        break;
      case 2: // incoming tte
        *led_ptr = 0x20;
        tte++;
        reply = mem_iord_byte(cur_RX + 14);
        tte_prepare_data(0x2000, VL0, (unsigned char[]) {reply}, 1514);
        if (!tte_schedule_send(0x2000, 1514, 0))
          sched_errors++;  
      default:  // incoming ethernet msg
        *led_ptr = 0x40;
        eth++;
    
    }
    printf("patmos step 1: an UDP packet will now be sent to the PC with 4th byte set in the obb flag struct\n");
    obb_t obb_msg = (obb_t){.flags = 1};
  }
    return 0;
}

// printf("ipout:\n");
//     printipdatagram(&ipout);

//     int len = packip(bufout, &ipout);
//     printf("bufout before checksum:\n");
//     bufprint(bufout, len); 

//     ipchecksum(bufout);

//     printf("bufout after checksum:\n");
//     bufprint(bufout, len); 
//     printf("\n");
    
//     printf("patmos sending: \n");
  
//     xmitslip(bufout, len);

//     printf("\n");
 
//     printf("patmos step 2: an UDP packet will now be received from the PC with 1st byte set in the obb flag struct\n"); 
//     unsigned char recbuf[2000];
//     memset(recbuf, 0, sizeof(recbuf));
//     int reccnt = receive(recbuf);
//     tte_wait_for_message(&receive_point); //eth0
//     tte_clear_free_rx_buffer(ext_RX_BD); //start receiving in other buffer
//     reply=tte_receive(cur_RX,receive_point);
//     if(reply==0){ //failed pcf
//             printf("pcf out of schedule \n");
// 	    return 0;
//           }
//     printf("ipin mem \"raw\" (%d bytes):\n", reccnt);
//     bufprint(recbuf, reccnt); 
//     printf("\n");

//     ip_t *ipin = (ip_t*) recbuf;
//     unpackip(ipin, recbuf);
//     printf("ip datagram from pc:\n");
//     printipdatagram(ipin);
//     printf("obb flag test completed on patmos...\n");