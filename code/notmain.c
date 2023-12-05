typedef unsigned u32;
typedef long unsigned u64;
typedef int i32;

typedef float f32;
typedef double f64;

// extern functions from the assembly file
extern u64 get_pc();
extern void muart_transmit_32(u32 data, u64 uart_lsr_reg_addr, u64 AUX_MU_IO_REG_addr);

// this is the base 'physical' address of the peripheral IOs. 
// the 'bus' address actually starts at 0x7e.... 
#define PERIPHERAL_BASE (0x3f000000) 

// defining all the peripheral addresses
#define GPFSEL0 ((volatile u32 *)(PERIPHERAL_BASE + 0x200000))
#define GPFSEL1 ((volatile u32 *)(PERIPHERAL_BASE + 0x200004))
#define GPFSEL2 ((volatile u32 *)(PERIPHERAL_BASE + 0x200008))
#define GPSET0  ((volatile u32 *)(PERIPHERAL_BASE + 0x20001C))
#define GPCLR0  ((volatile u32 *)(PERIPHERAL_BASE + 0x200028))

#define AUX_IRQ ((volatile u32 *)(PERIPHERAL_BASE + 0x215000))
#define AUX_ENABLES ((volatile u32 *)(PERIPHERAL_BASE + 0x215004)) // enable mini uart on raspberry pi
#define AUX_MU_IO_REG ((volatile u32 *)(PERIPHERAL_BASE + 0x215040)) // used to read/write 8 bits of data to the FIFO, all the other bits other ignored or cleared to 0

#define AUX_MU_IER_REG ((volatile u32 *)(PERIPHERAL_BASE + 0x215044))
#define AUX_MU_IIR_REG ((volatile u32 *)(PERIPHERAL_BASE + 0x215048)) // clear T/C FIFO

#define AUX_MU_LCR_REG ((volatile u32 *)(PERIPHERAL_BASE + 0x21504C)) // bottom 2 bits says whether there are 7 or 8 data bits after the start bit. note that there is no 'stop' bit in raspberry pi uart
#define AUX_MU_MCR_REG ((volatile u32 *)(PERIPHERAL_BASE + 0x215050))
#define AUX_MU_LSR_REG ((volatile u32 *)(PERIPHERAL_BASE + 0x215054)) // tells us whether there is any data inside the fifo(both for reciever/transmitter), and if there was any reciever overrun(overflow)
#define AUX_MU_MSR_REG ((volatile u32 *)(PERIPHERAL_BASE + 0x215058))
#define AUX_MU_SCRATCH ((volatile u32 *)(PERIPHERAL_BASE + 0x21505c))
#define AUX_MU_CNTL_REG ((volatile u32 *)(PERIPHERAL_BASE + 0x215060)) // bit 0 is 'receiver enable', bit 1 is 'transmitter enable', these bits are set on reset
#define AUX_MU_STAT_REG ((volatile u32 *)(PERIPHERAL_BASE + 0x215064)) // similiar to AUX_MU_LSR_REG, but gives us more information(i.e how many 'bits' are available in T/R FIFO?)
#define AUX_MU_BAUD_REG ((volatile u32 *)(PERIPHERAL_BASE + 0x215068)) // configure the baud rate using the equation : system_clock_freq(*10^6, because we need this in HZ not MHZ) / (8*(AUX_MU_BAUD_REG + 1))

int 
notmain(void)
{
    // initialize uart
    *((AUX_ENABLES)) = 1;
    *((AUX_MU_IER_REG)) = 0; // disable interrupt
    //*((volatile u32 *)(AUX_MU_CNTL_REG)) = 3; // enable transmitter 
    *((AUX_MU_LCR_REG)) = 3; // data = 8 bits
    *((AUX_MU_MCR_REG)) = 0; 
    *((AUX_MU_IIR_REG)) = 0x6; // clear T/C FIFOs
    // *(u32 *)(AUX_MU_IIR_REG) = 0xC6; // clear T/C FIFO
    // *((AUX_MU_BAUD_REG)) = 270; // TODO(gh) system clock freq is not 400mhz in raspberry pi 3?
    *((AUX_MU_BAUD_REG)) = 433; // TODO(gh) system clock freq is not 400mhz in raspberry pi 3?

    // enable gpio 14(PI's transmitter), 15(PI's receiver) to for UART 
    u32 xu32_GPFSEL1 = *((GPFSEL1));
    xu32_GPFSEL1 &= ~((0x77) << 12);
    xu32_GPFSEL1 |= (2<<12)|(2<<15);
    *((GPFSEL1)) = xu32_GPFSEL1;

#if 0
    // set GPIO 29 function to 'output'
    u32 xu32_GPFSEL = *(u32 *)(GPFSEL2);
    xu32_GPFSEL &= ~(7<<27); // clear 3 bits for GPIO 29
    xu32_GPFSEL |= (1<<27); // set GPIO 29 to 'output' mode
    *(u32 *)(GPFSEL2) = xu32_GPFSEL;
#endif

    u64 xu64_pc = get_pc();
    while(1)
    {
        muart_transmit_32((u32)xu64_pc, (u64)AUX_MU_LSR_REG, (u64)AUX_MU_IO_REG);
    }

    return(0);
}

// ================================================================================================================================================================================================
// Questions

// find out what is the cache sizes in raspberry pi 3 b+

// it seems like there are multiple registers that I can use to attrieve the stats for the UART(is T/C FIFO empty, how many bits are available)
// and also, AUX_MU_STAT_REG Register says that some of the stats that it is providing is not available in normal 16550 UART.

// what does it mean that the 'FIFO' is always enabled, but I need to enable 'transmitter' & 'receiver' on UART?

// when is uart interrupt useful?

// ret x0 vs just ret? 

// armv8 ABI(how many input registers?)

// 115200 baud rate is being used very often, it's because it's the fastest one? 
// uart only supports up to 115200 baud rate because it's very old?

// how would you construct a multi-platform bare metal graphics project?(raspberry pi + windows/mac?)

// parity bit - is it really needed? (i.e when I transfer my code, do I need a more stable uart(PL01 UART instead of mini UART) connection)

// 

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



