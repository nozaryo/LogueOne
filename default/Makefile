###############################################################################
# Makefile for the project SD_TEST
###############################################################################

## General Flags
PROJECT = SD_TEST
MCU = atmega328p
TARGET = SD_TEST.elf
PORT = /dev/ttyUSB0
LFUSE = 0b11101110
HFUSE = 0b11011001

CC = avr-gcc

CPP = avr-g++

## Options common to compile, link and assembly rules
COMMON = -mmcu=$(MCU)

## Compile options common for all C compilation units.
CFLAGS = $(COMMON)
CFLAGS += -Wall -gdwarf-2 -std=gnu99   -DF_CPU=16000000UL -Os -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
CFLAGS += -MD -MP -MT $(*F).o -MF dep/$(@F).d

## Assembly specific flags
ASMFLAGS = $(COMMON)
ASMFLAGS += $(CFLAGS)
ASMFLAGS += -x assembler-with-cpp -Wa,-gdwarf2

## Linker flags
LDFLAGS = $(COMMON)
LDFLAGS +=


## Intel Hex file production flags
HEX_FLASH_FLAGS = -R .eeprom -R .fuse -R .lock -R .signature

HEX_EEPROM_FLAGS = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0 --no-change-warnings


## Objects that must be built in order to link
OBJECTS = main.o ff.o mmc.o i2c.o bmp280.o

## Objects explicitly added by the user
LINKONLYOBJECTS =

## Build
all: $(TARGET) SD_TEST.hex SD_TEST.eep size

## Compile
main.o: ../main.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

ff.o: ../FatFs/ff.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

mmc.o: ../MMC/mmc.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

i2c.o: ../senser/i2c.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

bmp280.o: ../senser/bmp280.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

##Link
$(TARGET): $(OBJECTS)
	 $(CC) $(LDFLAGS) $(OBJECTS) $(LINKONLYOBJECTS) $(LIBDIRS) $(LIBS) -o $(TARGET)

%.hex: $(TARGET)
	avr-objcopy -O ihex $(HEX_FLASH_FLAGS)  $< $@

%.eep: $(TARGET)
	-avr-objcopy $(HEX_EEPROM_FLAGS) -O ihex $< $@ || exit 0

%.lss: $(TARGET)
	avr-objdump -h -S $< > $@

size: ${TARGET}
	@echo
	@avr-size -C --mcu=${MCU} ${TARGET}

write: $(OBJFILES)
	make clean
	make all
	sudo avrdude -c avrisp -P $(PORT) -b 19200 -p $(MCU) -U flash:w:$(TARGET):e -F

test:
	avrdude -c avrisp -P $(PORT) -b 19200 -p $(MCU)

fuse:
	avrdude -c avrisp -P $(PORT) -b 19200 -p $(MCU) -U lfuse:w:$(LFUSE):m
	avrdude -c avrisp -P $(PORT) -b 19200 -p $(MCU) -U hfuse:w:$(HFUSE):m

## Clean target
.PHONY: clean
clean:
	-rm -rf $(OBJECTS) SD_TEST.elf dep/* SD_TEST.hex SD_TEST.eep



## Other dependencies
-include $(shell mkdir dep 2>NUL) $(wildcard dep/*)
