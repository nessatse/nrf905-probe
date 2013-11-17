#include "lm75.h"
#include "USI_TWI_Master.h"

static unsigned char msg[3];

void lm75_conf(uint8_t addr,unsigned char c)
{
    msg[0] = (addr << 1) | 0;
    msg[1] = LM75_PTR_CONF;
    msg[2] = c;
    USI_TWI_Start_Transceiver_With_Data( msg, 3 );
}

int16_t lm75_temp(uint8_t addr)
{
    int16_t temp;
    msg[0] = (addr << 1) | 0;
    msg[1] = LM75_PTR_TEMP;
//    msg[1] = LM75_PTR_TOS;
    USI_TWI_Start_Transceiver_With_Data( msg, 2 );
    msg[0] = (addr << 1) | 1;
//    do
    USI_TWI_Start_Transceiver_With_Data( msg, 3 );
    if (USI_TWI_Get_State_Info() == USI_TWI_NO_ACK_ON_ADDRESS) {
        return 0xaaaa;
    }
    while (USI_TWI_Get_State_Info() == USI_TWI_NO_ACK_ON_ADDRESS);
    makeint(temp,msg[1],msg[2]);
    return temp>>5;
}
