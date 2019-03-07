/*******************************************************************************
 * DISCLAIMER
 * This software is supplied by Renesas Electronics Corporation and is only
 * intended for use with Renesas products. No other uses are authorized. This
 * software is owned by Renesas Electronics Corporation and is protected under
 * all applicable laws, including copyright laws.
 * THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING
 * THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT
 * LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
 * AND NON-INFRINGEMENT. ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED.
 * TO THE MAXIMUM EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS
 * ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES SHALL BE LIABLE
 * FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR
 * ANY REASON RELATED TO THIS SOFTWARE, EVEN IF RENESAS OR ITS AFFILIATES HAVE
 * BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
 * Renesas reserves the right, without notice, to make changes to this software
 * and to discontinue the availability of this software. By using this software,
 * you agree to the additional terms and conditions found by accessing the
 * following link:
 * http://www.renesas.com/disclaimer
 * Copyright (C) 2019 Renesas Electronics Corporation. All rights reserved.
 ******************************************************************************/
/*******************************************************************************
 * File Name    : ob1203.c
 * Version      : 1.0 <- Optional as long as history is shown below
 * Description  : This module solves all the world's problems
 ******************************************************************************/
/*******************************************************************************
 * History : DD.MM.YYYY Version  Description
 *         : 15.01.2007 1.00     First Release
 ******************************************************************************/

/*******************************************************************************
 Includes   <System Includes> , "Project Includes"
 ******************************************************************************/
#include "ob1203.h"
/*******************************************************************************
 Macro definitions
 ******************************************************************************/
#define I2C_WRITE(x,y,z)       i2c_write(x,y,z,
/*******************************************************************************
 Typedef definitions
 ******************************************************************************/

/*******************************************************************************
 Exported global variables (to be accessed by other files)
 ******************************************************************************/

/*******************************************************************************
 Private global variables and functions
 ******************************************************************************/
extern MD_STATUS R_IICA0_Master_Send(uint8_t adr, 
                                     uint8_t * const tx_buf, 
                                     uint16_t tx_num, 
                                     uint8_t wait);

MD_STATUS (__far_func * const i2c_write)(uint8_t adr, 
                                         uint8_t * const tx_buf, 
                                         uint16_t tx_num, 
                                         uint8_t wait) = R_IICA0_Master_Send;

__far_func void OB1203__writeRegister(uint8_t addr, char reg, char val) 
{
 /*writes 1 byte to a single register*/
    char writeData[2];
    writeData[0] = reg;
    writeData[1] = val;
//    i2c->write(addr,writeData, 2);
}

__far_func void OB1203__writeBlock(uint8_t addr, char startReg, char *data, char numBytes) {
/*writes data from an array beginning at the startReg*/
//    pc.printf("entering writeBlock with data %02x %02x\r\n",data[0],data[1]);
//    char writeData[numBytes+1];
//    writeData[0]=startReg;
//    for(int n=1;n<numBytes+1;n++) {
//        writeData[n]=data[n-1];
//    }
//    i2c->write(addr,writeData,numBytes+1);
}


__far_func void OB1203__readBlock(uint8_t addr, char startReg, char *data, int32_t numBytes) 
{
    char writeData[1];
    writeData[0] = startReg;
//    i2c->write(addr,writeData,1,true);
//    i2c->read(addr,data,numBytes);
//    wait_us(800);
//    for(int n=0;n<numBytes;n++)
//    {
//        i2c->read(addr,data+n,1,1);
//        wait_us(20);
//    }
}

__far_func void OB1203__reset(void)
{

}

uint16_t OB1203__get_status()
{
    char writeData[1]; //declare array of size 1
    writeData[0] = REG_STATUS_0; 
    char data[2]; //declare array of size 1
//    i2c->write(OB1203_ADDR,writeData,1,true);
//    i2c->read(OB1203_ADDR,data,2);
    return (data[0]<<8 | data[1]);
}

//void * reset = &OB1203__reset;


ob1203_ll_fp_t OB1203 =
{
  .reset                = OB1203__reset,
  .get_status	        = OB1203__get_status,	
  .writeRegister 	= OB1203__writeRegister,
  .writeBlock	        = OB1203__writeBlock,
  .readBlock	        = OB1203__readBlock,	  
};

