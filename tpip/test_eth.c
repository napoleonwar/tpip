#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tte.h"
#include "slip.h"

/* Test TTEthernet: should transfer data between PC and FPGA
 * The FPGA should have a patmos core, so the first step is to compile
 * the patmos core and download it to the FPGA. 
 * if the compilation fails and said something about patmos memory,
 * the solution is uninstall the Quartus2020, install 19 version.
 * TPIP <-------> TTEthernet
 * May need the TTEthernet switch later
*/

//Buffer for store the input/output data 
static unsigned char bufin[2000];
static unsigned char bufout[2000];
int main() {
    memset(bufin, 0, sizeof(bufin));
    memset(bufout, 0, sizeof(bufout));
    // initial serial port, open ttyUSB1 
    if(!initserial())
	    printf("ERROR: Serial port is not initialized");
    // clean the UART internal buffer
    int clearcount = serialclear();
    printf("PC Step 0: Clear serial port for old data: %d bytes\n", clearcount);
    
    // here I need a new receive function, I donot need the ESC
    // END treatment, the basic function that get char from the UART
    // is useful
    int count = serialreceive(bufin, sizeof(bufin));

    // print the thing we receive from PATMOS FPGA
    printf("How many received in BUFIN: ");
    bufprint(bufin, count);
    printf("\n"); 
    
    //store the data in to ipin
    ip_t *ipin = malloc(sizeof(ip_t));
    ipin->udp.data = (char[]){0,0,0,0};
    unpackip(ipin, bufin);

    printf("ipin:\n");
    printipdatagram(ipin);
    printf("\n");
    
    // build up an IP package, send to FPGA
    // first clean the obb flag
    obb_t obb_msg_ack;
    obb_t *obb_msg = (obb_t *) ipin->udp.data;
    if(obb_msg->flags)
	    obb_msg_ack.flags = 1;
    else
	    obb_msg_ack.flags = 0;
    // prepare sending
    //   patmos, 10.0.0.2, 10002
    //   server, 10.0.0.3, 10003
    ip_t ipoutack = {.verhdl = (0x04 << 4) | 0x05,
                     .tos = 0x00,
                     .length = 20 + 8 + 4, // 5 + 2 + 1 words
                     .id = 1,
                     .ff = 0x4000,
                     .ttl = 0x40,
                     .prot = 0x11, // UDP
                     .checksum = 0x0000,
                     .srcip = (10 << 24) | (0 << 16) | (0 << 8) | 3,
                     .dstip = (10 << 24) | (0 << 16) | (0 << 8) | 2,
                     .udp.srcport = 10003, // 0x2713
                     .udp.dstport = 10002, // 0x2712
                     .udp.length = 8 + 4,
                     .udp.data = (unsigned char[]){obb_msg_ack.flags, 0, 0, 0}};
    int len = packip(bufout, &ipoutack);
    bufprint(bufout, len);
    // need a new send function, support ETHERNET PROTOCOL
    int sent = serialsend(bufout, len);
    return 0;
}
