.globl _start
_start:
    b skip

.space 0x400000-0x80004,0 // this is where the actual code through UART would be located?

skip:
    mov sp, #0x08000000
    bl notmain
hang: b hang

.globl dummy
dummy:
    ret

// starting from armv8, PC is never accessible as a named register.
// instead, we get the link register(x30) which should be set by the CPU using the instruction bl 
// right before coming inside this subroutine
.globl get_pc
get_pc : 
    mov x0, x30 // copy the link register
    ret

// transmit 4 byte value with mini uart(8 bit data)
.globl muart_transmit_32
muart_transmit_32:
   
    

