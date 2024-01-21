
// NOTE(gh) because the imm in ldr operation is 12 bits, we cannot have a number
// bigger than 0xFFF here.
.equ AUX_IRQ_OFFSET, 0x0 
.equ AUX_ENABLES_OFFSET, 0x4  // enable mini uart on raspberry pi
.equ AUX_MU_IO_REG_OFFSET, 0x40 // used to read/write 8 bits of data to the FIFO, all the other bits other ignored or cleared to 0

.equ AUX_MU_IER_REG_OFFSET, 0x44
.equ AUX_MU_IIR_REG_OFFSET, 0x48 // clear T/C FIFO

.equ AUX_MU_LCR_REG_OFFSET, 0x4c // bottom 2 bits says whether there are 7 or 8 data bits after the start bit. note that there is no 'stop' bit in raspberry pi uart
.equ AUX_MU_MCR_REG_OFFSET, 0x50
.equ AUX_MU_LSR_REG_OFFSET, 0x54 // tells us whether there is any data inside the fifo(both for reciever/transmitter), and if there was any reciever overrun(overflow)
.equ AUX_MU_MSR_REG_OFFSET, 0x58
.equ AUX_MU_SCRATCH_OFFSET, 0x5c
.equ AUX_MU_CNTL_REG_OFFSET, 0x60 // bit 0 is 'receiver enable', bit 1 is 'transmitter enable', these bits are set on reset
.equ AUX_MU_STAT_REG_OFFSET, 0x64 // similiar to AUX_MU_LSR_REG, but gives us more information(i.e how many 'bits' are available in T/R FIFO?)
.equ AUX_MU_BAUD_REG_OFFSET, 0x68 // configure the baud rate using the equation : system_clock_freq(*10^6, because we need this in HZ not MHZ) / (8*(AUX_MU_BAUD_REG + 1))

// receive a byte from muart receive fifo
.macro muart_receive_byte, u32_dest:req 
.wait_receive_fifo_\@ :
    ldr \u32_dest, [u64_muart_base_addr, AUX_MU_LSR_REG_OFFSET]
    tbz \u32_dest, 5, .wait_receive_fifo_\@
    ldr \u32_dest, [u64_muart_base_addr, AUX_MU_IO_REG_OFFSET]
.endm

.macro convert_ascii_to_hex,  u32_dest, u32_ascii
    cmp \u32_ascii, 'A'
    sub u32_r1, \u32_ascii, 0x37 // if ascii ex >= 0xA
    sub u32_r0, \u32_ascii, 0x30 // if ascii hex < 0xA
    csel \u32_dest, u32_r1, u32_r0, ge
.endm

.globl _start
_start:
    b skip

code_loaded_by_bootloader:
    
.space 0x400000-0x80004, 0 // initialized to 0

skip:
    mov sp, #0x080000
    bl notmain
branch_to_code: b code_loaded_by_bootloader

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

// wait for certain amount of cycles.
.globl wait
wait : 

.globl muart_receive_8 // TODO(gh) make this to also detect the receiver fifo overrun?
muart_receive_8 :
    ldr w4, [x0] // load aux_mu_lsr_reg
    tbz w4, 0, muart_receive_8 // check the first bit to see if there is anything to receive

receive_byte:
    ldr w0, [x1] // load aux_mu_io_reg, read the bottom 8bits
    and w0, w0, 0xff
    res

// transmit 4 byte with mini uart(8 bit data)
.globl muart_transmit_32
muart_transmit_32:

u32_data .req w0
u64_uart_lsr_reg_addr .req x1
u64_AUX_MU_IO_REG_addr .req x2

u32_bitshift_amount .req w8 // double-check the arm64 abi, how many input parameters there can be?

u32_uart_lsr_reg .req w9
u64_uart_lsr_reg .req x9
u32_nibble .req w10

    mov u32_bitshift_amount, 32

// check if the transmitter FIFO is empty, so it's safe to write another byte
spinloop_until_fifo_emtpy : 
    ldr u32_uart_lsr_reg, [u64_uart_lsr_reg_addr]
    tbz u32_uart_lsr_reg, 5, spinloop_until_fifo_emtpy

// trasnsmit byte worth of info
transmit_byte :
    sub u32_bitshift_amount, u32_bitshift_amount, 4

    lsr u32_nibble, u32_data, u32_bitshift_amount
    and u32_nibble, u32_nibble, 0xf

    add u32_nibble, u32_nibble, 0x30 // convert to ascii by adding 0x30
    str u32_nibble, [u64_AUX_MU_IO_REG_addr]

    cbnz u32_bitshift_amount, spinloop_until_fifo_emtpy

    // print 'space' as a last character
last_spinloop_until_fifo_emtpy : 
    ldr u32_uart_lsr_reg, [u64_uart_lsr_reg_addr]
    tbz u32_uart_lsr_reg, 5, last_spinloop_until_fifo_emtpy

    mov u32_nibble, 0x20
    str u32_nibble, [u64_AUX_MU_IO_REG_addr] 

    ret

/* ********************************************************************************
   loads S1 line of the srec file, which has an s1 line of the 
*/
.globl  srec_load_s1_line
srec_load_s1_line :
    // scratch registers, always be ware that there might be garbage data inside
    u32_r0 .req w29
    u64_r0 .req x29
    u32_r1 .req w28
    u32_r2 .req w27
    u32_r3 .req w26

#if 0
    sf32_r0 .req s29
    sf32_r1 .req s28
    sf32_r2 .req s27
    sf32_r3 .req s26
#endif

    // intermediate registers, these can work across the labels, but will not have effect
    // in a long term
    u32_i0 .req w25
    u32_i1 .req w24
    u32_i2 .req w23
    u32_i3 .req w22

    // local registers, these should be persistent across the function
    // these normally have names because we either wanna return these,
    // or these would be used across the function
    u32_total_checksum .req  w21
    u64_muart_base_addr .req x20
    u32_s1_byte_count .req w19
    u32_s1_address .req w18
    u32_s1_data_address .req w17
    // this is where the code that is loaded by the bootloader would be going.
    // for now, this is 0x80004
    u32_code_address .req w16 
    u64_code_address .req x16

    u32_data_byte .req w15

initialization : 
    mov u32_total_checksum, 0
    mov u64_muart_base_addr, 0x5000
    movk u64_muart_base_addr, 0x3f21, lsl 16

    // make a base code address of 0x80004
    mov u32_code_address, 0x4
    movk u32_code_address, 0x8, lsl 16 

get_byte_count0 :
    muart_receive_byte u32_r0
    convert_ascii_to_hex u32_i0, u32_r0
    
get_byte_count1 :
    muart_receive_byte u32_r0
    convert_ascii_to_hex u32_i1, u32_r0

    orr u32_s1_byte_count, u32_i1, u32_i0, lsl 4

get_address0: 
    muart_receive_byte u32_r0
    convert_ascii_to_hex u32_i0, u32_r0

get_address1:
    muart_receive_byte u32_r0
    convert_ascii_to_hex u32_i1, u32_r0

    orr u32_i2, u32_i1, u32_i0, lsl 4

get_address2: 
    muart_receive_byte u32_r0
    convert_ascii_to_hex u32_i0, u32_r0

get_address3:
    muart_receive_byte u32_r0
    convert_ascii_to_hex u32_i1, u32_r0

    orr u32_i3, u32_i1, u32_i0, lsl 4

get_data_address : 
    orr u32_s1_address, u32_i2, u32_i3, lsl 8 // get the address(4 hex)
    add u32_s1_data_address, u32_code_address, u32_s1_address

    mov u32_i2, 0 // loop counter, TODO(gh) we can do better by directly using the byte_count variable here
load_data_byte : 
    muart_receive_byte u32_r0
    convert_ascii_to_hex u32_i0, u32_r0

    muart_receive_byte u32_r0
    convert_ascii_to_hex u32_i1, u32_r0

    orr u32_data_byte, u32_i1, u32_i0, lsl4
    
    strb u32_data_byte, [u64_code_address], 1 // post-index, increment the address by 1 after the instruction
    add u32_total_checksum, u32_total_checksum, u32_data_byte

    add u32_i2, u32_i2, 1
    cmp u32_i2, u32_s1_byte_count
    b.ne load_data_byte

finish_s1_line: // finish s1 line by moving to a next line
finish_s1_line_output_0xa:
    ldr u32_r0, [u64_muart_base_addr, AUX_MU_LSR_REG_OFFSET]
    tbz u32_r0, 0, finish_s1_line_output_0xa
    mov u32_r0, 0xa
    str u32_r0, [u64_muart_base_addr, AUX_MU_IO_REG_OFFSET]

finish_s1_line_output_0xd:
    ldr u32_r0, [u64_muart_base_addr, AUX_MU_LSR_REG_OFFSET]
    tbz u32_r0, 0, finish_s1_line_output_0xd
    mov u32_r0, 0xd
    str u32_r0, [u64_muart_base_addr, AUX_MU_IO_REG_OFFSET]

    ret // return to x30 by default, you can also specify other register for this(which might make this a indirect branch?)

// ********************************************************************************

    



    











    

    









    

