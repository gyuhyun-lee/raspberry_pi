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

xu32_data .req w0
xu64_uart_lsr_reg_addr .req x1
xu64_AUX_MU_IO_REG_addr .req x2

xu32_bitshift_amount .req w8 // double-check the arm64 abi, how many input parameters there can be?

xu32_uart_lsr_reg .req w9
xu64_uart_lsr_reg .req x9
xu32_nibble .req w10

    mov xu32_bitshift_amount, #32

// check if the transmitter FIFO is empty, so it's safe to write another byte
spinloop_until_fifo_emtpy : 
    ldr xu32_uart_lsr_reg, [xu64_uart_lsr_reg_addr]
    tbz xu32_uart_lsr_reg, #5, spinloop_until_fifo_emtpy

// trasnsmit byte worth of info
transmit_byte :
    sub xu32_bitshift_amount, xu32_bitshift_amount, #4

    lsr xu32_nibble, xu32_data, xu32_bitshift_amount
    and xu32_nibble, xu32_nibble, 0xf

    add xu32_nibble, xu32_nibble, 0x30 // convert to ascii by adding 0x30
    str xu32_nibble, [xu64_AUX_MU_IO_REG_addr]

    cbnz xu32_bitshift_amount, spinloop_until_fifo_emtpy

    // print 'space' as a last character
last_spinloop_until_fifo_emtpy : 
    ldr xu32_uart_lsr_reg, [xu64_uart_lsr_reg_addr]
    tbz xu32_uart_lsr_reg, #5, last_spinloop_until_fifo_emtpy

    mov xu32_nibble, #0x20
    str xu32_nibble, [xu64_AUX_MU_IO_REG_addr]

    ret




    

