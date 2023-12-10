inline void
muart_transmit_byte(u32 c)
{
    while(1)
    {
        u32 u32_lsr = *AUX_MU_LSR_REG;
        if(u32_lsr & (1<<5))
        {
            break;
        }
    }

    *AUX_MU_IO_REG = c;
}

void 
muart_transmit_byte_as_number(u32 b)
{
    u32 h1 = (b >> 4) & 0xf;
    while(1)
    {
        u32 u32_lsr = *AUX_MU_LSR_REG;
        if(u32_lsr & (1<<5))
        {
            break;
        }
    }
    if(h1 >= 0xa)
    {
        h1 += 7;
    }
    h1 += 0x30;

    *AUX_MU_IO_REG = h1;

    u32 h0 = (b & 0xf);
    while(1)
    {
        u32 u32_lsr = *AUX_MU_LSR_REG;
        if(u32_lsr & (1<<5))
        {
            break;
        }
    }
    if(h0 >= 0xa)
    {
        h0 += 7;
    }
    h0 += 0x30;
    *AUX_MU_IO_REG = h0;
}

inline u32 
muart_receive_byte()
{
    u32 xu32_result;
    while(1)
    {
        // check if there's anything to read
        u32 xu32_check = *AUX_MU_LSR_REG; 
        if((xu32_check & 1))
        {
            xu32_result = *AUX_MU_IO_REG;
            break;
        }
    }

    return xu32_result;
}

// takes two hex characters into a single byte
// c0 is the upper hex, c1 is the lower hex
inline u32
convert_hex01_into_byte(u32 xu32_c0, u32 xu32_c1)
{
    // TODO(gh) assert for c0 c1 boundary? 

    // if the ascii character is bigger than 0x41(A),
    // we should subtract 0x37 instead of 0x30
    u32 xu32_sub0 = (xu32_c0 >= 0x41) ? 0x37 : 0x30; 
    xu32_c0 -= xu32_sub0;

    u32 xu32_sub1 = (xu32_c1 >= 0x41) ? 0x37 : 0x30; 
    xu32_c1 -= xu32_sub1;

    u32 xu32_result = (xu32_c0 << 4) + xu32_c1;

    return xu32_result;
}
