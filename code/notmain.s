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

@; Questions


@; I learn!

@; 2023/11/29
@; some cpu's have a 'parity bit' because the circuit is potentially susceptible to a cosmic ray 
@; critical word first linefill on cache miss. this is possible because when a certain row is powered up in DRAM,
@; the memory controller can just go to 4B that was requested by the program and fetch that first.

@; physically indexed(used to find the set) - physically tagged(used to find the block within the set)
@; so it follows the order of the sentence - it's index first, and then the tag
@; because both the index and the tag is physical memory based, the memory should be translated first.
@; for a bare metal programming, this won't be an issue since 

@; branch target instruction cache. 
@; because of the instruction cache line is 64B in A53, some of the instructions will be pre-fetched after the branch even though the branch predictor predicts a different code path.
@; some of these instructions would be stored in the branch target instruction cache, which means that if the branch prediction was wrong, 
@; the front end can just use this cache to avoid stalling(stalling of which part?)

@; 


    

    
