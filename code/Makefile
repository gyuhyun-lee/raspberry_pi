
MAKEFLAGS += --silent

AS_FLAGS = --warn --fatal-warnings
COMPILER_FLAGS = -Wall -O2 -nostdlib -nostartfiles -ffreestanding 
BUILD_PATH = ../build

gcc : clean kernel8.img

clean :
	rm -f $(BUILD_PATH)/*.o
	rm -f $(BUILD_PATH)/*.bin
	rm -f $(BUILD_PATH)/*.srec
	rm -f $(BUILD_PATH)/*.elf
	rm -f $(BUILD_PATH)/*.img
	rm -f $(BUILD_PATH)/*.disasm

kernel8.img :  
	aarch64-none-elf-as $(AS_FLAGS) entry.s -o $(BUILD_PATH)/entry.o

	aarch64-none-elf-gcc $(COMPILER_FLAGS) -c notmain.c -o $(BUILD_PATH)/notmain.o

	aarch64-none-elf-ld $(BUILD_PATH)/entry.o $(BUILD_PATH)/notmain.o -T memmap -o $(BUILD_PATH)/kernel8.elf
	# aarch64-none-elf-objcopy --srec-forceS3 $(BUILD_PATH)/kernel8.elf -O srec $(BUILD_PATH)/kernel8.srec
	aarch64-none-elf-objcopy $(BUILD_PATH)/kernel8.elf -O binary $(BUILD_PATH)/kernel8.img

	# output disassembly
	aarch64-none-elf-objdump -D $(BUILD_PATH)/kernel8.elf > kernel8.disasm


