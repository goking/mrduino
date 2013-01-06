# put your *.o targets here, make should handle the rest!

RBSRCS = rbmain.rb
SRCS = main.c stm32f4xx_it.c system_stm32f4xx.c syscalls.c $(RBSRCS:.rb=.c)

# all the files will be generated with this name (main.elf, main.bin, main.hex, etc)

PROJ_NAME=main

# that's it, no need to change anything below this line!

###################################################

MRBC=$(MRB_HOME)/build/host/bin/mrbc
CC=arm-none-eabi-gcc
OBJCOPY=arm-none-eabi-objcopy

LIBRARY_PATH = `which $(CC)`

MRB_LIB_PATH = $(MRB_HOME)/build/arm-cortex-m4/lib
MRB_INC_PATH = $(MRB_HOME)/include

CFLAGS  = -g -O2 -Wall -Tstm32_flash.ld 
CFLAGS += -mlittle-endian -mthumb -mcpu=cortex-m4 -mthumb-interwork
CFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16

###################################################

vpath %.c src
vpath %.a lib
vpath %.rb src

ROOT=$(shell pwd)

CFLAGS += -Iinc -Ilib -Ilib/inc
CFLAGS += -Ilib/inc/core -Ilib/inc/peripherals -Ilib/inc/usb
CFLAGS += -I$(MRB_INC_PATH)
CFLAGS += -std=c99
CFLAGS += -D USE_USB_OTG_FS=1
#CFLAGS += -specs=rdpmon.specs 

LIBS = -L$(MRB_LIB_PATH) -lmruby
LIBS += -Llib -lstm32f4

SRCS += lib/startup_stm32f4xx.s # add startup file to build

OBJS = $(SRCS:.c=.o)

###################################################

.PHONY: lib proj

all: lib proj

lib:
	$(MAKE) -C lib

proj: 	$(PROJ_NAME).elf

%.c: %.rb
	$(MRBC) -Bmrbmain_irep $^ -o$@

$(PROJ_NAME).elf: $(SRCS)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)
	$(OBJCOPY) -O ihex $(PROJ_NAME).elf $(PROJ_NAME).hex
	$(OBJCOPY) -O binary $(PROJ_NAME).elf $(PROJ_NAME).bin

clean:
	rm -f *.o
	rm -f $(RBSRCS:.rb=.c)
	rm -f $(PROJ_NAME).elf
	rm -f $(PROJ_NAME).hex
	rm -f $(PROJ_NAME).bin
	$(MAKE) -C lib clean
