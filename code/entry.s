.globl _start
_start:
    b skip

skip:
    mov sp, #0x080000 // is this fine? bootloader also has the same stack pointer?
    bl notmain
hang: b hang

// starting from armv8, PC is never accessible as a named register.
// instead, we get the link register(x30) which should be set by the CPU using the instruction bl 
// right before coming inside this subroutine
.globl get_pc
get_pc : 
    mov x0, x30 // copy the link register
    ret

.globl get_el
get_el :
    mrs x0, CurrentEL
    lsr x0, x0, 2
    ret

.globl get_system_control_register
get_system_control_register :
    mrs x0, SCTLR_EL1
    ret



    

