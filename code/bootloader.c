#include "peripherals.h"

// extern functions from the assembly file
extern u64 get_pc();
extern u32 get_el();
extern void muart_transmit_32(u32 data, u64 uart_lsr_reg_addr, u64 AUX_MU_IO_REG_addr);

extern u32 muart_receive_8(u64 uart_lsr_reg_addr, u64 AUX_MU_IO_REG_addr);

#include "muart.c"

int 
notmain(void)
{
    u64 xu64_peripheral_base = (0x3f000000);
    // initialize uart
    u64 xu64_uart_base = (0x3f215000);
    *((volatile u32 *)(xu64_uart_base + AUX_ENABLES_OFFSET)) = 1; // AUX_ENABLES
    *((volatile u32 *)(xu64_uart_base + AUX_MU_CNTL_REG_OFFSET)) = 3; // enable t/r
    *((volatile u32 *)(xu64_uart_base + AUX_MU_LCR_REG_OFFSET)) = 3; // data = 8 bits
    *((volatile u32 *)(xu64_uart_base + AUX_MU_IER_REG_OFFSET)) = 3; // disable t/r  interrupts
    *((volatile u32 *)(xu64_uart_base + AUX_MU_IIR_REG_OFFSET)) = 0x6; // clear t/r FIFOs
    *((volatile u32 *)(xu64_uart_base + AUX_MU_BAUD_REG_OFFSET)) = 433; // force_tubo = 1, baud rate = 115200

#if 0
    // *(AUX_ENABLES) = 1; // enable uart
    *(AUX_MU_CNTL_REG) = 3; 
    *(AUX_MU_LCR_REG) = 3; // data = 8 bits
    *(AUX_MU_IER_REG) = 0; // disable t/r  interrupts
    *(AUX_MU_IIR_REG) = 0x6; // clear t/r FIFOs
    // *((AUX_MU_BAUD_REG)) = 270; // force_tubo = 0 in config.txt
    // *(AUX_MU_BAUD_REG) = 1301; // force_tubo = 1, baud rate = 38400
    *(AUX_MU_BAUD_REG) = 433; // force_tubo = 1, baud rate = 115200
    // *((AUX_MU_BAUD_REG)) = 249999; // force_tubo = 1, baud rate = 200
#endif
    // enable gpio 14(PI's transmitter), 15(PI's receiver) for mini UART 
    u32 xu32_GPFSEL1 = *((GPFSEL1));
    xu32_GPFSEL1 &= ~((0x77) << 12);
    xu32_GPFSEL1 |= (2<<12)|(2<<15);
    *(GPFSEL1) = xu32_GPFSEL1;

#if 0
    // enable gpio 24(TDO), 25(TCK), 26(TDI), 27(TMS)
    u32 xu32_GPFSEL2 = *((GPFSEL2));
    xu32_GPFSEL2 &= ~(0x7 << 12); // 24
    xu32_GPFSEL2 &= ~(0x7 << 15); // 25
    xu32_GPFSEL2 &= ~(0x7 << 18); // 26
    xu32_GPFSEL2 &= ~(0x7 << 21); // 27
    xu32_GPFSEL2 |= (0x3 << 12); // 24
    xu32_GPFSEL2 |= (0x3 << 15); // 25
    xu32_GPFSEL2 |= (0x3 << 18); // 26
    xu32_GPFSEL2 |= (0x3 << 21); // 27
    *(GPFSEL2) = xu32_GPFSEL2;
#endif

#if 0
    // set GPIO 29 function to 'output'
    u32 xu32_GPFSEL = *(u32 *)(GPFSEL2);
    xu32_GPFSEL &= ~(7<<27); // clear 3 bits for GPIO 29
    xu32_GPFSEL |= (1<<27); // set GPIO 29 to 'output' mode
    *(u32 *)(GPFSEL2) = xu32_GPFSEL;
#endif

    muart_transmit_byte('b');
    muart_transmit_byte('o');
    muart_transmit_byte('o');
    muart_transmit_byte('t');
    muart_transmit_byte('l');
    muart_transmit_byte('o');
    muart_transmit_byte('a');
    muart_transmit_byte('d');
    muart_transmit_byte('e');
    muart_transmit_byte('r');
    muart_transmit_byte(' ');
    muart_transmit_byte('s');
    muart_transmit_byte('t');
    muart_transmit_byte('a');
    muart_transmit_byte('r');
    muart_transmit_byte('t');
    muart_transmit_byte(0xa);
    muart_transmit_byte(0xd);

    muart_transmit_64_as_hex(0x0123456789abcdef);

    // loading srec file.
    // the format we're mainly using is S19-style 16-bit address records, which has
    // 1. S0 // header
    // 2. more than one S1 // actual data
    // 3. S9 // termination
    // each line of the srec has 
    // S Type ByteCount Address Data Checksum(0xff - (sum & 0xff))
    // i.e S00F00006E6F746D61696E2E737265631F

#define CODE_BASE_ADDRESS 0x80004 // IDEA(gh) use kernel_old=1 to start at 4
#define CARRIAGE_RETURN 0x0A
    u64 xu64_system_timer_start;
    while(1)
    {
        while(1)
        {
            u32 xu32_s_record = muart_receive_byte();
            if(xu32_s_record == 'S')
            {
                muart_transmit_byte(xu32_s_record);
                break;
            }
        }

        u32 xu32_record_type = muart_receive_byte();
        muart_transmit_byte(xu32_record_type);
        switch(xu32_record_type)
        {
            case '0' : // srec header line
            {
                u32 xu32_system_timer_lo = *(SYSTEM_TIMER_LO);
                u32 xu32_system_timer_hi = *(SYSTEM_TIMER_HI);
                xu64_system_timer_start = ((u64)xu32_system_timer_hi << 32) | ((u64)xu32_system_timer_lo);

                // S0 record has nothing interesting inside, so we'll skip this one 
                // LF(next line) + CR(start of the next line)
                muart_transmit_byte(0xa);
                muart_transmit_byte(0xd);
            }break;

            // IDEA(gh) write this entire case in assembly?
            case '1' : // 16 bit data line
            {
                // used for the checksum, this adds each 2 hex pairs
                // byte count, address, data
                u32 xu32_total_checksum = 0;
                
                // byte count = (byte count of address, data, and checksum data bytes)
                // 1 byte = 2 hex ascii
                u32 xu32_byte_count0 = muart_receive_byte();
                u32 xu32_byte_count1 = muart_receive_byte();
                u32 xu32_byte_count = convert_hex01_into_byte(xu32_byte_count0, xu32_byte_count1);
                xu32_total_checksum  += xu32_byte_count;

                // address
                u32 xu32_address0 = muart_receive_byte();
                u32 xu32_address1 = muart_receive_byte();
                u32 xu32_address01 = convert_hex01_into_byte(xu32_address0, xu32_address1);
                u32 xu32_address2 = muart_receive_byte();
                u32 xu32_address3 = muart_receive_byte();
                u32 xu32_address23 = convert_hex01_into_byte(xu32_address2, xu32_address3);
                xu32_total_checksum += (xu32_address01 + xu32_address23);

                u32 xu32_address = (xu32_address01 << 8) + xu32_address23;
                u64 xu64_data_address = CODE_BASE_ADDRESS + xu32_address;

                u32 xu32_data_byte_count = (xu32_byte_count - 3);
                for(u32 i = 0;
                        i < xu32_data_byte_count;
                        i++)
                {
                    u32 xu32_h0 = muart_receive_byte();
                    u32 xu32_h1 = muart_receive_byte();
                    u32 xu32_byte = convert_hex01_into_byte(xu32_h0, xu32_h1);

                    *((u8 *)xu64_data_address) = xu32_byte;
                    xu32_total_checksum += xu32_byte;
                    xu64_data_address += 1;
                }
                xu32_total_checksum = 0xff - (xu32_total_checksum & 0xff); // prepare for the checksum check

                // muart_transmit_byte_as_number(xu32_total_checksum);
                // muart_transmit_byte(' ');

                u32 xu32_checksum0 = muart_receive_byte();
                u32 xu32_checksum1 = muart_receive_byte();
                u32 xu32_checksum = convert_hex01_into_byte(xu32_checksum0, xu32_checksum1);
                // muart_transmit_byte(xu32_checksum0);
                // muart_transmit_byte(xu32_checksum1);

                if(xu32_checksum != xu32_total_checksum)
                {
                    // TODO(gh) assert
                    muart_transmit_byte('C');
                    muart_transmit_byte('h');
                    muart_transmit_byte('e');
                    muart_transmit_byte('c');
                    muart_transmit_byte('k');
                    muart_transmit_byte('s');
                    muart_transmit_byte('u');
                    muart_transmit_byte('m');
                    muart_transmit_byte('!');
                    muart_transmit_byte(' ');
                }

                // LF(next line) + CR(start of the next line)
                muart_transmit_byte(0xa);
                muart_transmit_byte(0xd);
            }break;

            case '9' : // termination line
            {
                // normally S9 record has a starting program counter,
                // but we already know that the program counter should be wherever 
                // we specified(i.e 0x80004)

                // LF(next line) + CR(start of the next line)
                muart_transmit_byte(0xa);
                muart_transmit_byte(0xd);

                // TODO(gh) flush the fifo until we find the carriage return
                goto SREC_RECEIVE_END;
            }break;

            default : // we do not support records other than s0(header), s1(data + 16bit-address), s9(termination)
            {
                // TODO(gh) assert here
                muart_transmit_byte('N');
                muart_transmit_byte('o');
                muart_transmit_byte('r');
                muart_transmit_byte('e');
                muart_transmit_byte('c');
                muart_transmit_byte('!');
                muart_transmit_byte(' ');
            }break;
        }
    }

SREC_RECEIVE_END :

    u32 xu32_system_timer_lo = *(SYSTEM_TIMER_LO);
    u32 xu32_system_timer_hi = *(SYSTEM_TIMER_HI);
    u64 xu64_system_timer_end = ((u64)xu32_system_timer_hi << 32) | ((u64)xu32_system_timer_lo);
    u64 xu64_system_timer_diff = xu64_system_timer_end - xu64_system_timer_start;

    char *a = "adflkjasdf;lkjll";
    muart_transmit_byte(a[0]);

    muart_transmit_byte('d');
    muart_transmit_byte('o');
    muart_transmit_byte('n');
    muart_transmit_byte('e');
    muart_transmit_byte(',');
    muart_transmit_byte(' ');
    muart_transmit_64_as_hex(xu64_system_timer_diff);
    muart_transmit_byte('t');
    muart_transmit_byte('i');
    muart_transmit_byte('c');
    muart_transmit_byte('k');
    muart_transmit_byte('s');
    muart_transmit_byte('.');

    muart_transmit_byte(0xa);
    muart_transmit_byte(0xd);

    return(0);
}

// ================================================================================================================================================================================================
// Questions

// armv8 ABI(how many input registers?)

/*
   JTAG
   Based on this document, TDI/TMS/TCK are all output-only, and TDI is input-only from the debugger's POV, At startup, the debugger sends TMS bits to the ARM's tap controller. Because the width of the instruction register(IR) varies based on the target, the debugger first has to know how many bits that the IR has. 
   Based on the warnings that I'm getting, this is where I'm failing. 
*/

// ================================================================================================================================================================================================
// I learn!

// some cpu's have a 'parity bit' because the bit can be corrupted due to i.e cosmic ray

// critical word first linefill(cacheline fill) on cache miss. this is possible because when a certain row is powered up in DRAM,
// the memory controller can just go to 4B that was requested by the program and fetch that first.

// physically indexed(used to find the set) - physically tagged(used to find the block within the set)
// so it follows the order of the sentence - it's index first, and then the tag
// because both the index and the tag is physical memory based, the memory should be translated first.
// for a bare metal programming(without the OS), this won't be an issue since there is no virtual memory

// branch target instruction cache
// because of the instruction cache line is 64B in A53, some of the instructions will be pre-fetched after the branch even though the branch predictor predicts a different code path.
// some of these instructions would be stored in the branch target instruction cache, which means that if the branch prediction was wrong, 
// the front end can just use this cache to avoid stalling(stalling of which part?)

// branch target address cache
// used for indirect branching to predict what would be the next PC, doesn't get used in direct branching(conditional/unconditional branch)

// return stack
// a single function can be called from anywhere inside the code, so the return address can differ a lot. 
// pushes the return address onto the stack when the pre-fetcher sees the ret instruction. (how can this be wrong?)

// Snoop Control Unit(SCU)
// looking at the diagram, it seems like it's located right next to the L2 cache, although L2 cache is optional in A53.
// takes care of the L1 data coherency between four L1 Cache(4 cores)
// enable direct read/write between L1 cache without going to the DRAM.

// L1 cache
// separate instruction & data cache, size for each of them can be 8KB, 16KB, 32KB, or 64KB.
// instruction cache line = 64B, instruction cache = 2-way set associative
// L1 cache is a write-back cache. similiar to RDNA, BIU(Bus Interface Unit) can be see if the processor is writing fully-dirty cache line, and if so, it can write directly to L2 
// instead of beginning a linefill. this is called a read-allocate mode. unlike the RDNA, it will always look for a existing cache line in L1.  

// Cache
// even if the cache is n-way associative, it will first use the 'index' to find within the buckets, 
// and then compare to whole remaining bits(except the bits that were inside the page boundary and the index bits)
// to identify the set within one bucket. Hardware normally does this using multiple comparators / set

// MOESI protocol
// M(modified) the cache line is only in this cache and is dirty.
// O(owned) the cache line is shared, and is dirty. when the cache line is written, it should be broadcasted to the other cache lines
// E(exclusive) the cache line is only in this cache and is clean.
// S(shared) the cache line is shared and is clean
// I(Invalid) the cache line should be re-fetched

// TLB - maps virtual memory page to physcial memory page
// A53 has 10 entry micro TLB, 512 entry main TLB. if it fails to find the cached mapping, then it will start the page walking using the page walk cache

// Harvard?
// separate path(and possibly the caches) for the instruction & data

// How to enable mini UART in raspberry pi 3 b+
// there are two versions of UART on BCM2835, one is mini UART(7/8 data bits, no parity, 1 start & stop bit)
// and the other one is PL01 UART, which is way more configurable
// 1. connect RXD 'cable' to pin 8, TXD 'cable' to pin 10. note that the documentation is saying it in backwards
// 2. type ls /dev | grep usb inside the terminal
// 3. check the last words that follow cu. & tty.
// 4. type minicom -b 115200 -D /dev/tty.usbserial-xxxx, where x is the last words in step 3



