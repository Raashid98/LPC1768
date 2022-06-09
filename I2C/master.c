//I2C Master Mode Read and Write
#include "lpc17xx.h"

#define address_slv (8)
uint8_t tx[6] = {'H','e','l','l','o','\n'};
uint8_t rx;

void SystemInit() {
    LPC_SC->PCONP |= (0x1UL<<26);	//re-enable POWER to I2C_0 if required
    LPC_PINCON->PINSEL0 |= (0x2UL<<20);	//Pin P0.10 allocated to alternate function 2
    LPC_PINCON->PINSEL0 |= (0x2UL<<22);	//Pin P0.11 allocated to alternate function 2
    LPC_SC->PCLKSEL1 |= (0x1UL<<21);	//pclk = cclk/2 (100Mhz/2)
    LPC_GPIO0->FIODIR |= (1<<10);	//Bit P0.10 an output
    LPC_GPIO0->FIODIR |= (1<<11);	//Bit P0.11 an output
    LPC_PINCON->PINMODE0 &= ~(3<<20);	//P0.10 has pull up/down resistor value is 00 enabling internal resistor
    //LPC_PINCON->PINMODE0 |= (2<<20);	//omit to use internal pull up
    LPC_PINCON->PINMODE0 &= ~(3<<22);	//P0.11 has pull up/down resistor
    //LPC_PINCON->PINMODE0 |= (2<<22);	//omit to use internal pull up
    LPC_PINCON->PINMODE_OD0 |= (1<<10);	//Bit P0.10 is open drain
    LPC_PINCON->PINMODE_OD0 |= (1<<11);	//Bit P0.11 is open drain
    LPC_I2C2->I2SCLH = 250;	//100kHz from 50MHz
    LPC_I2C2->I2SCLL = 250;	//100kHz from 50MHz
    LPC_I2C2->I2CONCLR = ((1<<2) | (1<<5) | (1<<6));
}

void I2C2_enable(){	
    LPC_I2C2->I2CONSET |= 1<<6;	//enable I2C2
}

void I2C_Start(){
    	
    LPC_I2C2->I2CONSET = 1<<5;	//START I2C2
    // Wait for complete
    while (!(LPC_I2C2->I2CONSET & 1<<3));
}

void I2C_Stop(){	
    /* Make sure start bit is not active */
	if (LPC_I2C2->I2CONSET & (0x20))//cheking start bit
	{
		LPC_I2C2->I2CONCLR = (1<<5);//clearing start bit
	}
	LPC_I2C2->I2CONSET = (1<<4);//setting stop bit
	LPC_I2C2->I2CONCLR = (1<<3);//clearing int bit
}

static uint32_t SendAdd (uint8_t databyte){

    LPC_I2C2->I2CONCLR = (1<<3);//clearing int bit
    LPC_I2C2->I2DAT = databyte;
    LPC_I2C2->I2CONCLR = 1<<5;	//clear start
	// Wait for complete
	while (!(LPC_I2C2->I2CONSET & (0x08)));

    return 0;
}

static uint32_t SendByte (uint8_t databyte){

    LPC_I2C2->I2DAT = databyte;
    LPC_I2C2->I2CONCLR = 1<< 3;	//clear SI
	// Wait for complete
	while (!(LPC_I2C2->I2CONSET & (0x08)));

    return 0;
}

static uint32_t GetByte (uint8_t *databyte){

    LPC_I2C2->I2CONSET = 1<<2;
    LPC_I2C2->I2CONCLR = 1<<3;	//clear SI
    // Wait for complete
	while (!(LPC_I2C2->I2CONSET & (0x08)));

    *databyte = LPC_I2C2->I2DAT;

    return 0;
}

static uint32_t master_send(){
    
    I2C_Start();
    if ((LPC_I2C2->I2STAT != 0x08) ){
		while(1);
	}
    SendAdd(address_slv<<1);
    if ((LPC_I2C2->I2STAT != 0x18) ){
		while(1);
	}
    SendByte(*tx);
    if ((LPC_I2C2->I2STAT != 0x28) ){
		while(1);
	}
   

    return 0;

}

static uint32_t master_receive(){
    
    I2C_Start();
    if ((LPC_I2C2->I2STAT != 0x08) ){
		while(1);
	}
    SendAdd((address_slv<<1) | 0x01);
    if ((LPC_I2C2->I2STAT != 0x40) ){
		while(1);
	}
    GetByte(&rx);
    if ((LPC_I2C2->I2STAT != 0x50) ){
		while(1);
	}
   
   return 0;

}

int main(void){

    SystemInit();
    
    while(1){
        I2C2_enable();
        master_send();
        I2C_Stop();
        master_receive();
        I2C_Stop();
    }

}