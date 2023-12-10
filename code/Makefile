
MAKEFLAGS += --silent

AS_FLAGS = --warn --fatal-warnings
COMPILER_FLAGS = -Wall -O2 -nostdlib -nostartfiles -ffreestanding  -ggdb -g0
DISASM_FLAGS = -d --source-comment=// -r 
BUILD_PATH = ../build
SDCARD_PATH = /Volumes/RPIBOOT

gcc : bootloader_kernel entry cleanup copy_to_sdcard 

entry :
	aarch64-none-elf-as $(AS_FLAGS) entry.s -o $(BUILD_PATH)/entry_as.o

	aarch64-none-elf-gcc $(COMPILER_FLAGS) -c entry.c -o $(BUILD_PATH)/entry.o

	aarch64-none-elf-ld $(BUILD_PATH)/entry_as.o $(BUILD_PATH)/entry.o -T memmap_entry -o $(BUILD_PATH)/entry.elf
	aarch64-none-elf-objcopy $(BUILD_PATH)/entry.elf -O srec --srec-len=255 entry.srec

	# output disassembly
	aarch64-none-elf-objdump $(DISASM_FLAGS) $(BUILD_PATH)/entry.elf > entry.disasm

# make a bootloader kernel(kernel8.img)
bootloader_kernel :  
	aarch64-none-elf-as $(AS_FLAGS) bootloader.s -o $(BUILD_PATH)/bootloader_as.o

	aarch64-none-elf-gcc $(COMPILER_FLAGS) -c bootloader.c -o $(BUILD_PATH)/bootloader.o

	aarch64-none-elf-ld $(BUILD_PATH)/bootloader_as.o $(BUILD_PATH)/bootloader.o -T memmap_bootloader -o $(BUILD_PATH)/bootloader.elf
	# aarch64-none-elf-objcopy --srec-forceS3 $(BUILD_PATH)/kernel8.elf -O srec $(BUILD_PATH)/kernel8.srec
	aarch64-none-elf-objcopy $(BUILD_PATH)/bootloader.elf -O binary $(BUILD_PATH)/kernel8.img

	# output disassembly
	aarch64-none-elf-objdump $(DISASM_FLAGS) $(BUILD_PATH)/bootloader.elf > bootloader.disasm

cleanup :
	rm -f $(BUILD_PATH)/*.o
	rm -f $(BUILD_PATH)/*.elf

# happens only if the sdcard(you can name however you want, just change the PRIBOOT part)
copy_to_sdcard : $(SDCARD_PATH)
	rm -f $(SDCARD_PATH)/kernel8.img 
	rm -f $(SDCARD_PATH)/config.txt 

	cp config.txt $(SDCARD_PATH)
	cp $(BUILD_PATH)/kernel8.img $(SDCARD_PATH)


