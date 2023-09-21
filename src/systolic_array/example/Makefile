.PHONY: all gem5-accel clean

GEM5_SRCS = ../systolic_array_connection.cc \
	        ../systolic_sys_constants.cc \
            ../systolic_sys_connection.cc

# For the MachSuite harness and support system.
CFLAGS = -O3 -Wall -static \
# -Wl,-rpath,/opt/riscv/sysroot/lib64/lp64/

ALL_SRCS = test.c

INCLUDES += -I/opt/riscv/sysroot/usr/include/  -I../../../ext/FP16/include -I../
BMARK = test_gem5_accel

CC = riscv64-unknown-linux-gnu-g++

all: gem5-accel

gem5-accel: $(ALL_SRCS) $(GEM5_SRCS)
	@echo Building benchmark $(BMARK) for gem5 with HW acceleration.
	$(CC) $(CFLAGS) $^ $(INCLUDES) -o $(BMARK)

clean:
	rm -rf $(BMARK) outputs stdout stderr sram_*
