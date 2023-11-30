@.section
@.init 

.globl _start @;tell the GNU toochain that this is the program's entry point
_start : 
    @; TODO(gh) we can just store 00200000 
    @; since we don't care about other pins at the moment

    base_addr .req r0
    gpreg_base_addr .req r4
    gpfsel4 .req r1

    ldr base_addr, =0x20000000
    add gpreg_base_addr, base_addr, #0x200<<12 @; make 20200000

    ldr gpfsel4, [gpreg_base_addr, #0x10] @; load the GPFSEL 4 register(GPIO 47)
    bic gpfsel4, gpfsel4, #0xbf @; clear bit [21:23] inside the GPFSEL 4 register
    orr gpfsel4, gpfsel4, #00200000 @; // set bit [21:23] to 001(output mode)
    str gpfsel4, [gpreg_base_addr, #0x10] @; store GPFSEL 4 reg back

    r_gpio47_togglebit .req r1
    r_gpio47_addr .req r3

    @; prepare the bit that will toggle the GPIO 47
    mov r_gpio47_togglebit, #1 @; TODO(gh) can I mov & lshl at the same time? probably not in arm64?
    lsl r_gpio47_togglebit, #15 @; // shift 1 by (47-32)

    orr r_gpio47_addr, gpreg_base_addr, #0x20

    counter .req r2
    sysclock_addr .req r4
    sysclock .req r5
    add sysclock_addr, base_addr, #0x3<<12 @; make 0x20003000
    add sysclock_addr, sysclock_addr, #0x4
loop : 
    mov counter, #0x3F0000 @; initialize the counter

spinloop :
    ldr sysclock, [sysclock_addr]
    tst sysclock, #0x800000
    bne spinloop

    str r_gpio47_togglebit, [r_gpio47_addr] @; set either GPSET1 or GPCLR1

    eor r_gpio47_addr, r_gpio47_addr, #0xC @; circle between GPSET1 & GPCLR1
    b loop @;loop back to the beginning of this loop

@; ================================================================================================================================================================================================
@; Questions
@; what's the best way to find out what is the cache line(probably same), cache size(L1D, L2D ...)? also, is it important to know that?
@; if the cache is 4-way set associative, does that mean there would be 2 bits worth of tag?
@; upside of set-associative is that finding a cache is faster(4x faster)?
@; SCU - does all processors have something like this so that when the core do read/write it will just write to the other L1 cache 
@; and avoid the write to the main memory? if so, how does it know that noone(i.e GPU) will be using the memory? maybe it updates the L1 cache & write to the memory?
@; conceptual register(x0...x30) vs physical register(r0...) what should I think about when programming in assembly? 

@; ================================================================================================================================================================================================
@; I learn!

@; some cpu's have a 'parity bit' because the bit can be corrupted due to i.e cosmic ray

@; critical word first linefill(cacheline fill) on cache miss. this is possible because when a certain row is powered up in DRAM,
@; the memory controller can just go to 4B that was requested by the program and fetch that first.

@; 

@; physically indexed(used to find the set) - physically tagged(used to find the block within the set)
@; so it follows the order of the sentence - it's index first, and then the tag
@; because both the index and the tag is physical memory based, the memory should be translated first.
@; for a bare metal programming(without the OS), this won't be an issue since there is no virtual memory

@; branch target instruction cache
@; because of the instruction cache line is 64B in A53, some of the instructions will be pre-fetched after the branch even though the branch predictor predicts a different code path.
@; some of these instructions would be stored in the branch target instruction cache, which means that if the branch prediction was wrong, 
@; the front end can just use this cache to avoid stalling(stalling of which part?)

@; branch target address cache
@; used for indirect branching to predict what would be the next PC, doesn't get used in direct branching(conditional/unconditional branch)

@; return stack
@; a single function can be called from anywhere inside the code, so the return address can differ a lot. 
@; pushes the return address onto the stack when the pre-fetcher sees the ret instruction. (how can this be wrong?)

@; Snoop Control Unit(SCU)
@; looking at the diagram, it seems like it's located right next to the L2 cache, although L2 cache is optional in A53.
@; takes care of the L1 data coherency between four L1 Cache(4 cores)
@; enable direct read/write between L1 cache without going to the DRAM.

@; L1 cache
@; separate instruction & data cache, size for each of them can be 8KB, 16KB, 32KB, or 64KB.
@; instruction cache line = 64B, instruction cache = 2-way set associative
@; L1 cache is a write-back cache. similiar to RDNA, BIU(Bus Interface Unit) can be see if the processor is writing fully-dirty cache line, and if so, it can write directly to L2 
@; instead of beginning a linefill. this is called a read-allocate mode. unlike the RDNA, it will always look for a existing cache line in L1.  

@; MOESI protocol
@; M(modified) the cache line is only in this cache and is dirty.
@; O(owned) the cache line is shared, and is dirty. when the cache line is written, it should be broadcasted to the other cache lines
@; E(exclusive) the cache line is only in this cache and is clean.
@; S(shared) the cache line is shared and is clean
@; I(Invalid) the cache line should be re-fetched

@; TLB - maps virtual memory page to physcial memory page
@; A53 has 10 entry micro TLB, 512 entry main TLB. if it fails to find the cached mapping, then it will start the page walking using the page walk cache


    

    
