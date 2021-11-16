# Check tte-node makefile
CC=gcc
# CC=clang
PATMOSHOME=~/t-crest/patmos
SERIAL?=/dev/ttyUSB0
LIBETH=$(PATMOSHOME)/c/ethlib

# Flags for compiling normal applications
# Assuming 2 MB of main memory
CFLAGS?=-target patmos-unknown-unknown-elf -O2 \
        -I $(PATMOSHOME)/c -I $(PATMOSHOME)/c/libelf/ \
        -I $(LIBETH) \
        -mpatmos-disable-vliw \
        $(DEFINES)

# library for ethernet
.PHONY: libeth
libeth: $(LIBETH)
$(LIBETH): $(patsubst ethlib/%.c,$(BUILDDIR)/ethlib/%.o,$(wildcard ethlib/*.c))
		patmos-ar r $@ $^

tpip_tte_demo:
		patmos-clang -O2 $(CFLAGS) $(LIBETH)/*.c tpip.c test_eth_patmos.c -o tpip_tte_demo.elf
		cd $(PATMOSHOME) && $(MAKE) config BOARD=altde2-all
		patserdow -v /dev/ttyUSB0 tpip_tte_demo.elf
tpip_tte_pc:
		$(CC) -O2 $(CFLAGS) $(LIBETH)/*.c tpip.c test_eth.c -o tpip_tte_pc.elf
		./a.out

# tte_demo_latency:
#         patmos-clang -O2 $(CFLAGS) $(LIBETH)/*.c tte_demo_latency.c -o tte_demo_latency.elf
#         cd $(PATMOSHOME) && $(MAKE) config BOARD=altde2-all
#         patserdow -v /dev/ttyUSB0 tte_demo_latency.elf

# tte_demo_interrupts:
#         patmos-clang -O2 $(CFLAGS) $(LIBETH)/*.c tte_demo_interrupts.c -o tte_demo_interrupts.elf
#         cd $(PATMOSHOME) && $(MAKE) config BOARD=altde2-all
#         patserdow -v /dev/ttyUSB0 tte_demo_interrupts.elf


#TODO: make a use-case build system that takes an argument (use-case number)
#      and runs the use-case. This should be done for the two targets tpiphost and tpippatmos

help:
	# Make file for tpip
	#   pcap: tcpdump stuff
    #   wcet: wcet of one function and some png call graphs
	#   macrocheck: see how macros are tested
	#   slip: (ms: will you write what it does?)
	#   tpiphost: host listener (UDP on serial 2)
	#   tpippatmos: patmos will send UDP over serial 2
	#   clean: removing fi
pcap:
	@echo 'tcpdump output:'
	#tcpdump -r tpip.pcap
	#tcpdump -nnvXSs 0 -c1 -i any

wcet:
	patmos-clang -O2 -mserialize=tpip.pml test_obb.c tpip.c slip.c
#patmos-clang -O0 -mserialize=tpip.pml tpip.c
#the function to be analyzed in listed just before --report
#functions: checksum waitfornextperiod wait
#platin wcet -i tpip.pml -b a.out -e getipdstip  --report
#platin wcet -i tpip.pml -b a.out -e wait --report
	platin wcet -i tpip.pml -b a.out -e packip
	platin wcet -i tpip.pml -b a.out -e tpip_slip_run
	#platin wcet -i tpip.pml -b a.out -e xmit
	#platin visualize -i tpip.pml -f waitfornextperiod
#platin wcet -i tpip.pml -b a.out -e currenttimemillis --report
#platin wcet -i tpip.pml -b a.out -e calculateipchecksum --report

#use this target to see how macros are expanded
macrocheck:
	$(CC) -E *.c

slip:
	$(CC) test_slip.c slip.c
	./a.out /dev/ttyUSB0

tpiphost:
    #start listening on host
	$(CC) slip.c tpip.c test_tpip.c
	./a.out

tpippatmos:
	#start on patmos (a second terminal)
	patmos-clang -O2 slip.c test_obb.c tpip.c -o test_obb.elf
	cd $(PATMOSHOME) && $(MAKE) config
	patserdow -v /dev/ttyUSB0 test_obb.elf

clean:
	rm -f *.out *.pcap *.pml *.png *.elf
