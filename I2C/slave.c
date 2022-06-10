// I2C slave read write
#include "lpc17xx.h"

#define own_add (2)

uint8_t tx[6] = {'H','e','l','l','o','\n'};
uint8_t rx;

void SystemInit() {
    //LPC_GPIO0->FIODIR0 |= 0x18000000;	//P0.27 P0.28
    LPC_SC->PCONP |= (0x1UL<<26);	//re-enable POWER to I2C_0 if required
    LPC_PINCON->PINSEL0 |= (0x2UL<<20);	//Pin P0.10 allocated to alternate function 2
    LPC_PINCON->PINSEL0 |= (0x2UL<<22);	//Pin P0.11 allocated to alternate function 2
    LPC_SC->PCLKSEL1 |= (0x1UL<<20);	//pclk = cclk
    LPC_GPIO0->FIODIR |= (1<<10);	//Bit P0.10 an output
    LPC_GPIO0->FIODIR |= (1<<11);	//Bit P0.11 an output
    LPC_PINCON->PINMODE0 &= ~(3<<20);	//P0.10 has pull up/down resistor value is 00 enabling internal resistor
    //LPC_PINCON->PINMODE0 |= (2<<20);	//omit to use internal pull up
    LPC_PINCON->PINMODE0 &= ~(3<<22);	//P0.11 has pull up/down resistor
    //LPC_PINCON->PINMODE0 |= (2<<22);	//omit to use internal pull up
    LPC_PINCON->PINMODE_OD0 |= (1<<10);	//Bit P0.10 is open drain
    LPC_PINCON->PINMODE_OD0 |= (1<<11);	//Bit P0.11 is open drain
    LPC_I2C2->I2SCLH = 520;	//100kHz from 104MHz
    LPC_I2C2->I2SCLL = 520;	//100kHz from 104MHz
    LPC_I2C2->I2CONCLR = ((1<<2) | (1<<5) | (1<<6));
}

void I2C2_enable(){	
    LPC_I2C2->I2CONSET |= 1<<6;	//enable I2C2
}

void set_add(){
    LPC_I2C2->I2ADR0 = ((own_add<<1) | 1);
    LPC_I2C2->I2MASK0 = 0xFE;
}

static uint32_t slave_read(uint8_t *data){
    LPC_I2C2->I2CONSET = (1<<2); //Setting Ack Bit
    LPC_I2C2->I2CONCLR = (1<<3);  //interrupt clear
    while(1){
        if (LPC_I2C2->I2CONSET & (1<<3)){
            switch(LPC_I2C2->I2STAT){
                case 0xF8:      // No status information 
                    LPC_I2C2->I2CONSET = (1<<2);
	    		    LPC_I2C2->I2CONCLR = (1<<3);
	    		    break;
                case 0x60:      // Own SLA+W has been received, ACK has been returned 
                case 0x70:      // Data has been transmitted, ACK has been received 
                    LPC_I2C2->I2CONSET = (1<<2); //Setting Ack Bit
	    			LPC_I2C2->I2CONCLR = (1<<3); //interrupt clear
	    			break;
                case 0x80:      // DATA has been received, ACK has been return 
                case 0x90:      //All data bytes that over-flow the specified receive data length, just ignore them.
                    *data = LPC_I2C2->I2DAT;
                    LPC_I2C2->I2CONSET = (1<<2); //Setting Ack Bit
	    			LPC_I2C2->I2CONCLR = (1<<3); //interrupt clear
	    		    break;
                case 0x88:  // DATA has been received, NACK has been return 
                case 0x98:  // DATA has been received, NACK has been return 
                    LPC_I2C2->I2CONCLR = (1<<3); //interrupt clear
	    			break;
                case 0xA0:  
                    LPC_I2C2->I2CONCLR = (1<<3); //interrupt clear
                    LPC_I2C2->I2CONCLR = (1<<2); //Clearing Ack Bit
                    return 0;
	    			break;
                default :
                    LPC_I2C2->I2CONCLR = (1<<3); //interrupt clear
                    LPC_I2C2->I2CONCLR = (1<<2); //Clearing Ack Bit
                    while(1);

            }
        }
    }
    return 0;
    
}

static uint32_t slave_write(uint8_t data){
    LPC_I2C2->I2CONSET = (1<<2); //Setting Ack Bit
    LPC_I2C2->I2CONCLR = (1<<3);  //interrupt clear
    while(1){
        if (LPC_I2C2->I2CONSET & (1<<3)){
            switch(LPC_I2C2->I2STAT){
                case 0xF8:      /* No status information */
                    LPC_I2C2->I2CONSET = (1<<2);
	    		    LPC_I2C2->I2CONCLR = (1<<3);
	    		    break;
                case 0xA8:      /* Own SLA+R has been received, ACK has been returned */
                case 0xB8:      /* General call address has been received, ACK has been returned */
                    LPC_I2C2->I2DAT = data;
                    LPC_I2C2->I2CONSET = (1<<2); //Setting Ack Bit
	    			LPC_I2C2->I2CONCLR = (1<<3); //interrupt clear
	    			break;
                case 0xC0:
                case 0xC8:  
                    LPC_I2C2->I2CONSET = (1<<2); //Setting Ack Bit
	    			LPC_I2C2->I2CONCLR = (1<<3); //interrupt clear
                    LPC_I2C2->I2CONCLR = (1<<2); //Clearing Ack Bit
                    return 0;
	    			break;
                default :
                    LPC_I2C2->I2CONCLR = (1<<3); //interrupt clear
                    LPC_I2C2->I2CONCLR = (1<<2); //Clearing Ack Bit
                    while(1);

            }
        }
    }
    return 0;
    
}

int main(void){

    SystemInit();
    
    while(1){
        set_add();
        I2C2_enable();
        slave_read(&rx);
        slave_write(*tx);
    }

}