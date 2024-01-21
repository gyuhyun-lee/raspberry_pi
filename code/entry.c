#include "peripherals.h"

// extern functions from the assembly file
extern u64 get_pc();
extern u64 get_el();
extern u32 get_system_control_register_el1();
extern u32 get_system_control_register_el2();

extern void muart_transmit_32(u32 data, u64 uart_lsr_reg_addr, u64 AUX_MU_IO_REG_addr);
extern u32 muart_receive_8(u64 uart_lsr_reg_addr, u64 AUX_MU_IO_REG_addr);

#include "muart.c"

int 
notmain(void)
{
    // we would assume that the bootloader already initialized the UART
#if 0
    // initialize uart
    *(AUX_ENABLES) = 1; // enable uart
    *(AUX_MU_CNTL_REG) = 3; // enable t/r
    *(AUX_MU_LCR_REG) = 3; // data = 8 bits
    *(AUX_MU_IER_REG) = 0; // disable t/r  interrupts
    *(AUX_MU_IIR_REG) = 0x6; // clear t/r FIFOs
    // *((AUX_MU_BAUD_REG)) = 270; // force_tubo = 0 in config.txt
    // *(AUX_MU_BAUD_REG) = 1301; // force_tubo = 1, baud rate = 38400
    *(AUX_MU_BAUD_REG) = 433; // force_tubo = 1, baud rate = 115200
    // *((AUX_MU_BAUD_REG)) = 249999; // force_tubo = 1, baud rate = 200
    // enable gpio 14(PI's transmitter), 15(PI's receiver) for mini UART 
    u32 xu32_GPFSEL1 = *((GPFSEL1));
    xu32_GPFSEL1 &= ~((0x77) << 12);
    xu32_GPFSEL1 |= (2<<12)|(2<<15);
    *(GPFSEL1) = xu32_GPFSEL1;
#endif

#if 0
    // set GPIO 29 function to 'output'
    u32 xu32_GPFSEL = *(u32 *)(GPFSEL2);
    xu32_GPFSEL &= ~(7<<27); // clear 3 bits for GPIO 29
    xu32_GPFSEL |= (1<<27); // set GPIO 29 to 'output' mode
    *(u32 *)(GPFSEL2) = xu32_GPFSEL;
#endif

    // TODO(gh) this is not precise, since the timer starts with some arbitrary number 
    // is there any way to reset the timer?
    // wait until the uart is set & fifos are flushed. 
    // TODO(gh) is this really required?
#define TARGET_TIMER 0x00400000
#if 0
    while(1)
    {
        u32 xu32_system_timer = *(SYSTEM_TIMER);
        if((xu32_system_timer & TARGET_TIMER) == 0x00400000)
        {
            break;
        }
    }
#endif

    // we starts at EL2
    u32 xu32_EL = get_el();
    muart_transmit_byte_as_hex(xu32_EL);

    u32 xu32_sctl_register_el2 = get_system_control_register_el2();
    u32 xu32_instruction_cache_enabled = (xu32_sctl_register_el2 >> 12) & 1;
    muart_transmit_byte_as_hex(xu32_instruction_cache_enabled); 

    u32 xu32_data_cache_enabled = (xu32_sctl_register_el2 >> 2) & 1;
    muart_transmit_byte_as_hex(xu32_data_cache_enabled); 

    u32 xu32_mmu_enabled = (xu32_sctl_register_el2 >> 0) & 1;
    muart_transmit_byte_as_hex(xu32_mmu_enabled);

    muart_transmit_byte('h');
    muart_transmit_byte('e');
    muart_transmit_byte('l');
    muart_transmit_byte('l');
    muart_transmit_byte('o');
    muart_transmit_byte(' ');
    muart_transmit_byte('w');
    muart_transmit_byte('o');
    muart_transmit_byte('r');
    muart_transmit_byte('l');
    muart_transmit_byte('d');
    muart_transmit_byte('!');

    // game loop
    while(1)
    {
    }

    return(0);
}

// ================================================================================================================================================================================================
// Questions

// predication
// sve, sve2(128b - 2048b, based on the cpu vendors), shares the same vector registers + predication registers(execution mask)
// neon cache bypass?

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
// because the instruction cache line is 64B in A53, some of the instructions will be pre-fetched after the branch even though the branch predictor predicts a different code path.
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



