#ifndef PERIPHERAL_H
#define PERIPHERAL_H

typedef char u8;
typedef unsigned u32;
typedef long unsigned u64;
typedef int i32;

typedef float f32;
typedef double f64;

// this is the base 'physical' address of the peripheral IOs. 
// the 'bus' address actually starts at 0x7e.... 
#define PERIPHERAL_BASE (0x3f000000) 

// defining all the peripheral addresses
#define GPFSEL0 ((volatile u32 *)(PERIPHERAL_BASE + 0x200000))
#define GPFSEL1 ((volatile u32 *)(PERIPHERAL_BASE + 0x200004))
#define GPFSEL2 ((volatile u32 *)(PERIPHERAL_BASE + 0x200008))
#define GPSET0  ((volatile u32 *)(PERIPHERAL_BASE + 0x20001C))
#define GPCLR0  ((volatile u32 *)(PERIPHERAL_BASE + 0x200028))

// TODO(gh) find out what's the timer granularity here
#define SYSTEM_TIMER_LO ((volatile u32 *)(PERIPHERAL_BASE + 0x00003004)) 
#define SYSTEM_TIMER_HI ((volatile u32 *)(PERIPHERAL_BASE + 0x00003008)) 

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

#define AUX_IRQ_OFFSET 0x0 
#define AUX_ENABLES_OFFSET 0x4  // enable mini uart on raspberry pi
#define AUX_MU_IO_REG_OFFSET 0x40 // used to read/write 8 bits of data to the FIFO, all the other bits other ignored or cleared to 0

#define AUX_MU_IER_REG_OFFSET 0x44
#define AUX_MU_IIR_REG_OFFSET 0x48 // clear T/C FIFO

#define AUX_MU_LCR_REG_OFFSET 0x4c // bottom 2 bits says whether there are 7 or 8 data bits after the start bit. note that there is no 'stop' bit in raspberry pi uart
#define AUX_MU_MCR_REG_OFFSET 0x50
#define AUX_MU_LSR_REG_OFFSET 0x54 // tells us whether there is any data inside the fifo(both for reciever/transmitter), and if there was any reciever overrun(overflow)
#define AUX_MU_MSR_REG_OFFSET 0x58
#define AUX_MU_SCRATCH_OFFSET 0x5c
#define AUX_MU_CNTL_REG_OFFSET 0x60 // bit 0 is 'receiver enable', bit 1 is 'transmitter enable', these bits are set on reset
#define AUX_MU_STAT_REG_OFFSET 0x64 // similiar to AUX_MU_LSR_REG, but gives us more information(i.e how many 'bits' are available in T/R FIFO?)
#define AUX_MU_BAUD_REG_OFFSET 0x68 // configure the baud rate using the equation : system_clock_freq(*10^6, because we need this in HZ not MHZ) / (8*(AUX_MU_BAUD_REG + 1))

#endif
