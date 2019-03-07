#include <stddef.h>
#include "OB1203.h"

#if defined(USE_CG)
#warning "Compiling with RL78 Code Generator I2C Driver"
#endif

/* Declare and Initialize the static variables */
volatile bool OB1203::busy = false;

void OB1203::callback(void)
{
  busy = false;
}

#if defined(USE_CG)
OB1203 * OB1203::current = NULL;
void OB1203_callback_tx_complete(void)
{
  OB1203::callback();
}
void OB1203_callback_rx_complete(void)
{
  OB1203::callback();
}

void OB1203_callback_error(MD_STATUS status)
{
  if(NULL != OB1203::current)
  {
    OB1203::current->i2c_error = status;
    R_IICA0_StopCondition();
  }
  OB1203::callback();
}
#else
r_iic_drv_info_t * __near OB1203::current_i2c = NULL;
#endif

#if !defined(USE_CG)
OB1203::OB1203(r_iic_drv_info_t *i2c_obj, uint8_t addr)
#else
OB1203::OB1203(uint8_t addr)
#endif
{
#if defined(__CA78K0R__) || defined(__CCRL__) || defined(__ICCRL78__) 
#if !defined(USE_CG)
  i2c = i2c_obj;
  current_i2c = NULL;
  busy = false;
  ready = false;
  i2c_addr = addr;
  i2c->CallBackFunc = callback;  
  i2c->pSlvAdr        = NULL;
  i2c->pData1st       = NULL;
  i2c->Cnt1st         = 0;
  i2c->pData2nd       = NULL;
  i2c->Cnt2nd         = 0;
  i2c->pDevStatus     = &i2c_DevStatus;  
#else
  busy = false;
  ready = false;
  i2c_addr = addr;
  i2c_error = MD_OK;
#endif
#endif //defined(__CA78K0R__) || defined(__CCRL__) || defined(__ICCRL78__) 
}


void OB1203::reset()
{ 
  current = this;
#if defined(__CA78K0R__) || defined(__CCRL__) || defined(__ICCRL78__)
#if !defined(USE_CG)
  /* Test if Slave Responds */
  {
    i2c->pSlvAdr        = &i2c_addr;
    i2c->pData1st       = NULL;
    i2c->Cnt1st         = 0;
    i2c->pData2nd       = NULL;
    i2c->Cnt2nd         = 0;
    
    /* Test I2C */
    i2c_error = R_IIC_Drv_Advance(i2c);
    
    if(i2c_error == R_IIC_NO_INIT)
    {
      i2c_error = R_IIC_Drv_Init(i2c);
    }
    else if(i2c_error == R_IIC_ERR_NON_REPLY)
    {
      i2c_error = R_IIC_Drv_Reset(i2c);
    }

    if(R_IIC_IDLE==i2c_error)
    {
      i2c_error = R_IIC_Drv_MasterTx(i2c);
      busy = true;
      /* Wait for transmission test to complete */
      while((true == busy) || (i2c_DevStatus == R_IIC_COMMUNICATION));
    }
  }

  /* If I2C is IDLE; Power-On Reset the Device over I2C */
  if((i2c_DevStatus == R_IIC_IDLE)||(i2c_DevStatus == R_IIC_FINISH))
  {
    /* Send POR reset */
    writeRegister(OB1203_ADDR,REG_MAIN_CTRL_0,SW_RESET);
    
    ready = false;
    
    /* Wait for transmission test to complete */
    while((true == busy) || (i2c_DevStatus == R_IIC_COMMUNICATION));
    
    /* A software reset will be triggered immediately and therefore,
      the I�C bus command is NOT answered with �ACK�.  */
    i2c->pSlvAdr        = &i2c_addr;
    i2c->pData1st       = NULL;
    i2c->Cnt1st         = 0;
    i2c->pData2nd       = NULL;
    i2c->Cnt2nd         = 0;
    
    /* Wait for POR to complete. Check if device is back over the I2C bus. */
    do{
        i2c_error = R_IIC_Drv_MasterTx(i2c);
        busy = true;

      /* Wait for transmission test to complete */
      while((true == busy) || (i2c_DevStatus == R_IIC_COMMUNICATION));
    }while((i2c_DevStatus == R_IIC_NACK));
  }

  if((i2c_DevStatus == R_IIC_IDLE)||(i2c_DevStatus == R_IIC_FINISH))
  {
    /* Read the status register */
    uint16_t status_reg_vals = get_status();
    
    /* Part is operational after a typical delay of 10 ms. 
    However, the power on reset bit in STATUS_0 is NOT set. */
    if((status_reg_vals & 0x8000) == 0)
    {
      ready = true;
    }
  }
#else
  /* Test if Slave Responds */
  {
    do
    {
      i2c_error = R_IICA0_Master_Send(i2c_addr, NULL, 0, 1);
      if(i2c_error==MD_OK)
      {
        busy = true;
      }
      
      while(true == busy);
      
    }while((i2c_error==MD_ERROR1) || (i2c_error==MD_ERROR2));
    
    /* If I2C is IDLE; Power-On Reset the Device over I2C */
    if(MD_OK==i2c_error)
    {
      writeRegister(OB1203_ADDR,REG_MAIN_CTRL_0,SW_RESET);
      
      ready = false;
      
      /* Wait for POR to complete. Check if device is back over the I2C bus. */
      {
        do
        {
          i2c_error = R_IICA0_Master_Send(i2c_addr, NULL, 0, 1);
          if(i2c_error==MD_OK)
          {
            busy = true;
          }
          
          while(true == busy);
          
        }while((i2c_error==MD_ERROR1) || 
               (i2c_error==MD_ERROR2) || 
                 (i2c_error==MD_NACK) );
      }
      
      /* Read the status register */
      {
        uint16_t status_reg_vals = get_status();
        
        /* Part is operational after a typical delay of 10 ms. 
        However, the power on reset bit in STATUS_0 is NOT set. */
        if((status_reg_vals & 0x8000) == 0)
        {
          ready = true;
        }
      }
    }
    
    
  }
  
#endif
#endif //defined(__CA78K0R__) || defined(__CCRL__) || defined(__ICCRL78__) 
}


void OB1203::writeRegister(int addr, char reg, char val) 
{
  current = this;
  uint8_t writeRegister_writeData[2];
  /*writes 1 byte to a single register*/
#if defined(__CA78K0R__) || defined(__CCRL__) || defined(__ICCRL78__) 
#if !defined(USE_CG)
  i2c_addr = addr & 0xFFU;
  
  i2c->pSlvAdr        = &i2c_addr;
  
  i2c->pData1st       = (uint8_t*)reg;
  i2c->Cnt1st         = 1;
  
  i2c->pData2nd       = (uint8_t*)val;
  i2c->Cnt2nd         = 1;
  
  /* Wait for previous communication to complete */
  while(R_IIC_COMMUNICATION == i2c_DevStatus);
  
  if((i2c_DevStatus == R_IIC_IDLE)||(i2c_DevStatus == R_IIC_FINISH))
  {
    i2c_error = R_IIC_Drv_MasterTx(i2c);
    
    if(R_IIC_COMMUNICATION == i2c_error)
    {
      /* Successfully started the transfer of bytes */
      busy = true;
      current_i2c = this->i2c;
      
      /* Wait for communication to complete */
      while(R_IIC_COMMUNICATION == i2c_DevStatus);
    }
  }
#else
  i2c_addr = addr & 0xFFU;
  
  writeRegister_writeData[0] = reg;
  writeRegister_writeData[1] = val;
  
  /* Wait for previous transmission to complete */
  while(busy == true);
  
  /* Send out the data */
  do
  {
    i2c_error = R_IICA0_Master_Send(i2c_addr, &writeRegister_writeData[0], 2, 1);
    if(i2c_error==MD_OK)
    {
      busy = true;
    }
    
    while(true == busy);
  }while((i2c_error==MD_ERROR1) || (i2c_error==MD_ERROR2));
#endif
#endif //defined(__CA78K0R__) || defined(__CCRL__) || defined(__ICCRL78__) 
}


void OB1203::writeBlock(int addr, char startReg, char *data, char numBytes) 
{
  current = this;
  /*writes data from an array beginning at the startReg*/
#if defined(DEBUG)
  pc.printf("entering writeBlock with data %02x %02x\r\n",data[0],data[1]);
#endif
  
#if defined(__CA78K0R__) || defined(__CCRL__) || defined(__ICCRL78__) 
#if !defined(USE_CG)
  i2c_addr = addr & 0xFFU;
  
  i2c->pSlvAdr        = &i2c_addr;
  
  i2c->pData1st       = (uint8_t*)&startReg;
  i2c->Cnt1st         = 1;
  
  i2c->pData2nd       = (uint8_t*)&data[0];
  i2c->Cnt2nd         = (uint8_t)numBytes;
  
  /* Wait for any previous communication to complete */
  while(R_IIC_COMMUNICATION == i2c_DevStatus);
 
  if((i2c_DevStatus == R_IIC_IDLE)||(i2c_DevStatus == R_IIC_FINISH))
  {
    i2c_error = R_IIC_Drv_MasterTx(i2c);

    if(R_IIC_COMMUNICATION == i2c_error)
    {
      /* Successfully started the transfer of bytes */
      busy = true;
      current_i2c = this->i2c;
      
      /* Wait for communication to complete */
      while(R_IIC_COMMUNICATION == i2c_DevStatus);
    }
  }
#else
  
#endif
#endif //defined(__CA78K0R__) || defined(__CCRL__) || defined(__ICCRL78__) 
}


void OB1203::readBlock(int addr, char startReg, char *data, int numBytes) 
{
  current = this;
#if defined(__CA78K0R__) || defined(__CCRL__) || defined(__ICCRL78__) 
#if !defined(USE_CG)
  i2c_addr = addr & 0xFFU;
  
  i2c->pSlvAdr        = &i2c_addr;
  
  i2c->pData1st       = (uint8_t*)&startReg;
  i2c->Cnt1st         = 1;
  
  i2c->pData2nd       = (uint8_t*)&data[0];
  i2c->Cnt2nd         = numBytes;
  
  /* Wait for previous communication to complete */
  while(R_IIC_COMMUNICATION == i2c_DevStatus);
  
  if((i2c_DevStatus == R_IIC_IDLE)||(i2c_DevStatus == R_IIC_FINISH))
  {
    i2c_error = R_IIC_Drv_MasterTRx(i2c);
    
    if(R_IIC_COMMUNICATION == i2c_error)
    {
      /* Successfully started the transfer of bytes */
      busy = true;
      current_i2c = this->i2c;
      
      /* Wait for communication to complete */
    while(R_IIC_COMMUNICATION == i2c_DevStatus);
    }
  }
#else
  
#endif
#endif //defined(__CA78K0R__) || defined(__CCRL__) || defined(__ICCRL78__) 
}

uint8_t get_status_writeData[1]; 
uint8_t get_status_readData[2];
uint16_t OB1203::get_status()
{
  current = this; 
#if defined(__CA78K0R__) || defined(__CCRL__) || defined(__ICCRL78__) 
#if !defined(USE_CG)
  char writeData[1]; 
  writeData[0] = REG_STATUS_0;
  
  i2c->pSlvAdr        = &i2c_addr;
  
  i2c->pData1st       = (uint8_t*)&writeData[0];
  i2c->Cnt1st         = 1;
  
  i2c->pData2nd       = (uint8_t*)&data[0];
  i2c->Cnt2nd         = sizeof(data);
  
  if((i2c_DevStatus == R_IIC_IDLE)||(i2c_DevStatus == R_IIC_FINISH))
  {
    i2c_error = R_IIC_Drv_MasterTRx(i2c);
    
    if(R_IIC_COMMUNICATION == i2c_error)
    {
      /* Successfully started the transfer of bytes */
      busy = true;
      current_i2c = this->i2c;
    }
    
    /* Wait for communication to complete */
    while(R_IIC_COMMUNICATION == i2c_DevStatus);
  }
#else
  get_status_writeData[0] = REG_STATUS_0;
  
  /* Send out the data */
  do
  {
    i2c_error = R_IICA0_Master_Send(i2c_addr, 
                                    &get_status_writeData[0], 
                                    sizeof(get_status_writeData), 
                                    1);
    busy = true;
  }while((i2c_error==MD_ERROR1) || (i2c_error==MD_ERROR2));
  
  while(true==busy);
  
  do
  {
    i2c_error = R_IICA0_Master_Receive(i2c_addr, 
                                       get_status_readData, 
                                       sizeof(get_status_readData),
                                       1);
    busy = true;
  }while((i2c_error==MD_ERROR1) || (i2c_error==MD_ERROR2));
  
  while(true==busy);
    
#endif
#endif //defined(__CA78K0R__) || defined(__CCRL__) || defined(__ICCRL78__) 
  return (get_status_readData[0]<<8 | get_status_readData[1]);
}

bool OB1203::dataIsNew()
{
    int status_reg_vals;
    status_reg_vals = get_status();
    return ((status_reg_vals & 0x0100)>>8) || (status_reg_vals & 0x0091);
}

bool OB1203::lsIsNew()
{
    int status_reg_vals;
    status_reg_vals = get_status();
    return ((status_reg_vals & 0x0100)>>8);
}

bool OB1203::psIsNew()
{
    int status_reg_vals;
    status_reg_vals = get_status();
    return (status_reg_vals & 0x0001);
}

bool OB1203::tempIsNew()
{
    int status_reg_vals;
    status_reg_vals = get_status();
    return (status_reg_vals & 0x0080);
}

bool OB1203::bioIsNew()
{
    int status_reg_vals;
    status_reg_vals = get_status();
    return (status_reg_vals & 0x0010);
}

void OB1203::setOscTrim()
{
    char writeData[1];
    writeData[0] = osc_trim;
#if defined(DEBUG)
    pc.printf("writing %02x to REG %02x\r\n",osc_trim,REG_OSC_TRIM);
#endif
    writeBlock(OB1203_ADDR,REG_OSC_TRIM,writeData,1);
}

void OB1203::setMainConfig()
{
    char writeData[2];
    writeData[0] = ls_sai | ls_mode | ls_en; //MAIN_CTRL_0
    writeData[1] = temp_en | ps_sai_en | ppg_ps_mode | ppg_ps_en; //MAIN_CTRL_1
#if defined(DEBUG)
    pc.printf("main config 1 to write %02x\r\n",writeData[1]);
#endif
    writeBlock(OB1203_ADDR,REG_MAIN_CTRL_0,writeData,2); 
}


void OB1203::setIntConfig()
{
    char writeData[3];
    writeData[0] =  ls_int_sel | ls_var_mode | ls_int_en;
    writeData[1] = afull_int_en | ppg_int_en | ps_logic_mode | ps_int_en;
    writeData[2] = ls_persist | ps_persist;
    writeBlock(OB1203_ADDR,REG_INT_CFG_0,writeData,3); //default  
}


void OB1203::setLSthresh()
{
    char writeData[6];
    writeData[0] = (char) (ls_thres_hi & 0x000000FF);
    writeData[1] = (char) ((ls_thres_hi & 0x0000FF00)>>8);
    writeData[2] = (char) ((ls_thres_hi & 0x00FF0000)>>16);
    writeData[3] = (char) (ls_thres_lo & 0x000000FF);
    writeData[4] = (char) ((ls_thres_lo & 0x0000FF00)>>8);
    writeData[5] = (char) ((ls_thres_lo & 0x00FF0000)>>16);
    writeBlock(OB1203_ADDR,REG_LS_THRES_HI,writeData,6); //default  
}


void OB1203::setPSthresh()
{
    char writeData[4];
    writeData[0] = (char) (ps_thres_hi & 0x000FF);
    writeData[1] = (char) ((ps_thres_hi & 0xFF00)>>8);
    writeData[2] = (char) (ps_thres_lo & 0x000FF);
    writeData[3] = (char) ((ps_thres_lo & 0xFF00)>>8);
    writeBlock(OB1203_ADDR,REG_PS_THRES_HI,writeData,4); //default  
}


void OB1203::setPScurrent()
{
    char writeData[2];
    writeData[0] = (char) (ps_current & 0x00FF);
    writeData[1] = (char) ((ps_current & 0xFF00)>>8);
    writeBlock(OB1203_ADDR,REG_PS_LED_CURR,writeData,2);
}


void OB1203::setPPGcurrent()
{
    char writeData[4];
    writeData[0] = (char) (ir_current & 0x00FF);
    writeData[1] = (char) ((ir_current & 0xFF00)>>8);
    writeData[2] = (char) (r_current & 0x00FF);
    writeData[3] = (char) ((r_current & 0xFF00)>>8);
    writeBlock(OB1203_ADDR,REG_PPG_IRLED_CURR,writeData,4);
}


void OB1203::setPPG_PSgain_cfg()
{
    char writeData[2];
    writeData[0] = ppg_ps_gain | ppg_LED_settling | ppg_ALC_track;
    writeData[1] = ppg_pow_save | led_flip | sig_out | diff | alc;
    writeBlock(OB1203_ADDR,REG_PPG_PS_GAIN,writeData,2);
}

void OB1203::setPPGana_can()
{
    char writeData[1];
    writeData[0] = ch1_can_ana | ch2_can_ana;
    writeBlock(OB1203_ADDR,REG_PPG_CAN_ANA,writeData,1);
}

void OB1203::setPPGavg_and_rate()
{
    char writeData[2];
    writeData[0] = ppg_avg | 0x0A; //use standard LED max settings
    writeData[1] = ppg_pwidth | ppg_freq | ppg_rate;
    writeBlock(OB1203_ADDR,REG_PPG_AVG,writeData,2);
}

void OB1203::setBioTrim()
{
    char writeData[1];
    writeData[0] = bio_trim;
    writeBlock(OB1203_ADDR, REG_BIO_TRIM,writeData,1);
}

void OB1203::setLEDTrim()
{
    char writeData[1];
    writeData[0] = led_trim;
    writeBlock(OB1203_ADDR, REG_LED_TRIM,writeData,1);
}

void OB1203::setDigitalCan()
{
    char writeData[2];
    writeData[0] = (char)(ps_digital_can & 0x00FF);
    writeData[1] = (char)((ps_digital_can & 0xFF00)>>8);
    writeBlock(OB1203_ADDR,REG_PS_CAN_DIG,writeData,2);
}


void OB1203::setFifoConfig()
{
    char writeData[1];
    writeData[0] = fifo_rollover_en | fifo_afull_advance_warning;
    writeBlock(OB1203_ADDR,REG_FIFO_CFG,writeData,1);    
}


void OB1203::resetFIFO()
{
    char writeData[2];
    writeData[0]=0;
    writeData[1]=0;
    writeBlock(OB1203_ADDR,REG_FIFO_WR_PTR,writeData,2);//set write and read pointer to zero--next sample is newest
    writeRegister(OB1203_ADDR,REG_MAIN_CTRL_1, (temp_en | ps_sai_en | ppg_ps_mode | 0) ); //turn PPG off MAIN_CTRL_1;
    writeRegister(OB1203_ADDR,REG_MAIN_CTRL_1, (temp_en | ps_sai_en | ppg_ps_mode | ppg_ps_en) ); //MAIN_CTRL_1;
}


void OB1203::init_rgb()
{
    /*Configures ALS/RGB mode. PS and BIO off.
    Use: set class variables using header declarations. Then call this function.*/
    char writeData[2];
    writeBlock(OB1203_ADDR,REG_LS_RES_RATE,writeData,2);
    writeData[0] = ls_res | ls_rate; //LS_RES_RATE
    writeData[1] = ls_gain; //LS_GAIN
    writeBlock(OB1203_ADDR,REG_LS_RES_RATE,writeData,2);  
    setLSthresh(); 
    setIntConfig();
    ppg_ps_en = PPG_PS_OFF;
    ls_en = LS_ON;
    setMainConfig();
}


void OB1203::init_ps()
{
    /*Configures PS mode but not thresholds or interrupts. RGB/ALS and BIO off.
    Use: set class variables using header declarations. Then call this function.*/
    char writeData[2];  
    //PS settings
    setPScurrent();
    writeData[0] = ps_can_ana | ps_pulses | 0x02; //PS_CAN_PULSES -->set hidden max current registers to allow higher LED powers
    writeData[1] = ps_pwidth | ps_rate; //PS_PWIDTH_RATE
    writeBlock(OB1203_ADDR,REG_PS_CAN_PULSES,writeData,2);
    //Digital crosstalk cancellation
    setDigitalCan(); //PS_CAN_DIG
    //set PS moving average and hysteresis
    writeData[0] = ps_avg_en | ps_hys_level; //PS_MOV_AVG_HYS
    writeBlock(OB1203_ADDR,REG_PS_MOV_AVG_HYS,writeData,1);
    //set PS interrupt thresholds
    setPSthresh();
    //interrupt configuration
    ls_int_en = LS_INT_OFF;
    setIntConfig();
    setPPG_PSgain_cfg();
    setPScurrent(); 
    //config PS
    ls_en = LS_OFF;
    ppg_ps_en = 1;
    ppg_ps_mode = PS_MODE;
    setLEDTrim();
    setMainConfig();
}


void OB1203::init_ps_rgb()
{
    reset();
    char writeData[2];
    writeData[0] = ls_res | ls_rate; //LS_RES_RATE
    writeData[1] = ls_gain; //LS_GAIN
    writeBlock(OB1203_ADDR,REG_LS_RES_RATE,writeData,2);
    writeData[0] = ps_can_ana | ps_pulses | 0x02; //PS_CAN_PULSES -->set hidden max current registers to allow higher LED powers
    writeData[1] = ps_pwidth | ps_rate; //PS_PWIDTH_RATE
    writeBlock(OB1203_ADDR,REG_PS_CAN_PULSES,writeData,2);
    setDigitalCan(); //PS_CAN_DIG
    //set PS moving average and hysteresis
    writeData[0] = ps_avg_en | ps_hys_level; //PS_MOV_AVG_HYS
    writeBlock(OB1203_ADDR,REG_PS_MOV_AVG_HYS,writeData,1);
    setIntConfig();
    setPSthresh();
    setPScurrent();   
    setLSthresh();
    ls_en = LS_ON;
    ppg_ps_en = PPG_PS_ON;
    ppg_ps_mode = PS_MODE;
    setLEDTrim();
    setMainConfig();
}


void OB1203::init_hr()
{
    reset();
    ps_int_en = PS_INT_OFF;
    ls_en = LS_OFF;
    setIntConfig(); 
#if defined(DEBUG)    
    char readData[1];
    readBlock(OB1203_ADDR,REG_PS_INT_CFG_1,readData,1);
    pc.printf("int config 1 = %02X\r\n",readData[0]);
#endif
    setPPG_PSgain_cfg();
    setPPGcurrent();
    setPPGana_can();
    setPPGavg_and_rate();
    setFifoConfig(); 
    ppg_ps_mode = HR_MODE;
    setBioTrim();
    setLEDTrim();
    setMainConfig();    
}

void OB1203::init_spo2()
{
    reset();
    ps_int_en = PS_INT_OFF;
    ls_en = LS_OFF;
    setIntConfig(); 
    char readData[1];
    readBlock(OB1203_ADDR,REG_PS_INT_CFG_1,readData,1);
#if defined(DEBUG)    
    pc.printf("int config 1 = %02X\r\n",readData[0]);
#endif
    setPPG_PSgain_cfg();
    setPPGcurrent();
    setPPGana_can();
    setPPGavg_and_rate();
    setFifoConfig(); 
    ppg_ps_mode = SPO2_MODE;
    setLEDTrim();
    setBioTrim();
    setMainConfig();    
}

uint32_t OB1203::bytes2uint32(char *data, int start_byte) 
{
    //coverts a string of 3 bytes with LSB first into unsigned long MSB last
    return ((uint32_t)data[start_byte+2])<<16 | ((uint32_t)data[start_byte+1])<<8 | ((uint32_t)data[start_byte]) ;
}

uint32_t OB1203::twoandhalfBytes2uint32(char *data, int start_byte) 
{
  /* coverts a string of 3 bytes with LSB first into unsigned long MSB last */
  uint32_t out = 0;
  out |= (data[start_byte+2] & 0x0F);
  out <<= 8;
  out |= (data[start_byte+1] & 0x0F);
  out <<= 8;
  out |= (data[start_byte+0] & 0x0F);
  
  return out;
}

char OB1203::get_ls_data(uint32_t *data)
{  
    char byte_data[21];
    readBlock(OB1203_ADDR,REG_STATUS_0,byte_data,21);  
    #ifdef DEBUG
        pc.printf("%02x %02x  %02x %02x  %02x %02x %02x  %02x %02x %02x  %02x %02x %02x  %02x %02x  %02x %02x %02x %02x %02x $02x\r\n",
        byte_data[0],byte_data[1],byte_data[2],byte_data[3],
        byte_data[4],byte_data[5],byte_data[6],byte_data[7],
        byte_data[8],byte_data[9],byte_data[10],byte_data[11],
        byte_data[12],byte_data[13],byte_data[14],byte_data[15],
        byte_data[16],byte_data[17],byte_data[18]),
        byte_data[19],byte_data[20];
    #endif
     

    //byte_data[0] is ps (not populated)
    data[1] = twoandhalfBytes2uint32(byte_data,4); //w
    data[2] = twoandhalfBytes2uint32(byte_data,7); //g
    data[3] = twoandhalfBytes2uint32(byte_data,10); //b
    data[4] = twoandhalfBytes2uint32(byte_data,13); //r
    data[5] = twoandhalfBytes2uint32(byte_data,16); //c
    data[6] =  (uint32_t)((byte_data[20] & 0x0F)<<8) | (uint32_t)byte_data[19] ; //temp data
    return ( (byte_data[0] & LS_NEW_DATA) == 0x01 ? 1 : 0); //return 1 if new data or 0 if old data
}

char OB1203::get_ps_data(uint32_t *data)
{  
    char byte_data[4];
    readBlock(OB1203_ADDR,REG_STATUS_0,byte_data,4);  
    #ifdef DEBUG
        pc.printf( "%02x %02x %02x %02x\r\n", byte_data[0], byte_data[1], byte_data[2], byte_data[3] );
    #endif

    data[0] = ((uint32_t)byte_data[3])<<8 | ((uint32_t)byte_data[2]); //ps data
    return ( (byte_data[1] & PS_NEW_DATA) == 0x01 ? 1 : 0); //return 1 if new data or 0 if old data
}

char OB1203::get_ps_ls_data(uint32_t *data)
{  
    char byte_data[21];
    readBlock(OB1203_ADDR,REG_STATUS_0,byte_data,21);  
    #ifdef DEBUG
        pc.printf("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x $02x\r\n",
        byte_data[0],byte_data[1],byte_data[2],byte_data[3],byte_data[4],
        byte_data[5],byte_data[6],byte_data[7],byte_data[8],byte_data[9],
        byte_data[10],byte_data[11],byte_data[12],byte_data[13],byte_data[14],
        byte_data[15],byte_data[16],byte_data[17],byte_data[18] ),
        byte_data[19],byte_data[20];
    #endif
     
    data[0] = ((uint32_t)byte_data[3])<<8 | ((uint32_t)byte_data[2]); //ps
    data[1] = twoandhalfBytes2uint32(byte_data,4); //w
    data[2] = twoandhalfBytes2uint32(byte_data,7); //g
    data[3] = twoandhalfBytes2uint32(byte_data,10); //b
    data[4] = twoandhalfBytes2uint32(byte_data,13); //r
    data[5] = twoandhalfBytes2uint32(byte_data,16); //c
    data[6] =  (uint32_t)((byte_data[20] & 0x0F)<<8) | (uint32_t)byte_data[19] ; //temp data
    return ( (byte_data[0] & LS_NEW_DATA) == 0x01 ? 1 : 0); //return 1 if new data or 0 if old data
}


void OB1203::getFifoInfo(char *fifo_info)
{
    readBlock(OB1203_ADDR,REG_FIFO_WR_PTR,fifo_info,3);
    writePointer = fifo_info[0];
    readPointer = fifo_info[1];
    fifoOverflow = fifo_info[2];
}


uint8_t OB1203::getNumFifoSamplesAvailable()
{
    uint8_t numSamples = writePointer;
    if (writePointer<readPointer)
    {
        numSamples += 31-readPointer;
    }
    return numSamples;
}


void OB1203::getFifoSamples(uint8_t numSamples, char *fifoData)
{
    readBlock(OB1203_ADDR,REG_FIFO_DATA,fifoData,3*numSamples);
}


void OB1203::parseFifoSamples(char numSamples, char *fifoData, uint32_t *assembledData)
{
    for (int n=0; n<numSamples; n++)
    {
        assembledData[n] = bytes2uint32(fifoData,3*n);
    }
}


char OB1203::get_part_ID(char *data)
{
    readBlock(OB1203_ADDR,REG_PART_ID,data,1);
    return data[0];
}


void OB1203::do_agc(uint32_t data, bool ch)
{
    const uint32_t tol1 = TOL1;
    const uint32_t tol2 = TOL2;
    const uint16_t in_range_persist = IN_RANGE_PERSIST;
    static uint16_t in_range[2] = {0,0};
    const uint16_t maxCurrent[2] = {IR_MAX_CURRENT , R_MAX_CURRENT};
    const uint16_t step = STEP;
    const uint32_t targetCounts[2] = {IR_TARGET_COUNTS, R_TARGET_COUNTS};
     //ch = 0 for IR, 1 for R (channel)
     if( data > targetCounts[ch] + (in_range[ch]>in_range_persist ? tol2: tol1) )
    {       
        if(data>targetCounts[ch] + tol2)
            in_range[ch]=0;
        
       
        if( (ch ? r_current : ir_current)>step)
        {
            (ch ? r_current : ir_current) -= step;
            update = 1;
        }
    }
    else if( data < targetCounts[ch] - (in_range[ch]>in_range_persist ? tol2 : tol1) )
    {
        if(data<targetCounts[ch] - tol2)
            in_range[ch]=0;
        if( (ch ? r_current : ir_current) +step<maxCurrent[ch]) //no need to go to full current
        {
            (ch ? r_current : ir_current) += step;
            update = 1;
        }
    }
    else
    {
        if( (data > (targetCounts[ch]-tol1) ) && (data < (targetCounts[ch]+tol1)) )
            in_range[ch]++;
    }
}