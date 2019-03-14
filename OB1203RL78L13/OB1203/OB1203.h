#ifndef __OB1203_H__
#define __OB1203_H__

#if defined(__CA78K0R__) || defined(__CCRL__) || defined(__ICCRL78__) 
#include "r_cg_macrodriver.h"
#include "r_cg_iica.h"
#include "r_cg_userdefine.h"
#endif //#if defined(__CA78K0R__) || defined(__CCRL__) || defined(__ICCRL78__) 
//#define DEBUG

#define OB1203_ADDR 0xA6

//Define registers
#define REG_STATUS_0        0x00
#define REG_STATUS_1        0x01
#define REG_PS_DATA         0x02
#define REG_LS_W_DATA       0x04
#define REG_LS_G_DATA       0x07
#define REG_LS_B_DATA       0x0A
#define REG_LS_R_DATA       0x0D
#define REG_LS_C_DATA       0x10
#define REG_TEMP_DATA       0x13
#define REG_MAIN_CTRL_0     0x15
#define REG_MAIN_CTRL_1     0x16
#define REG_PS_LED_CURR     0x17
#define REG_PS_CAN_PULSES   0x19
#define REG_PS_PWIDTH_RATE  0x1A
#define REG_PS_CAN_DIG      0x1B
#define REG_PS_MOV_AVG_HYS  0x1D
#define REG_PS_THRES_HI     0x1E
#define REG_PS_THRES_LO     0x20
#define REG_LS_RES_RATE     0x22
#define REG_LS_GAIN         0x23
#define REG_LS_THRES_HI     0x24
#define REG_LS_THRES_LO     0x27
#define REG_LS_THRES_VAR    0x2A
#define REG_INT_CFG_0       0x2B
#define REG_PS_INT_CFG_1    0x2C
#define REG_INT_PST         0x2D
#define REG_PPG_PS_GAIN     0x2E
#define REG_PPG_PS_CFG      0x2F
#define REG_PPG_IRLED_CURR  0x30
#define REG_PPG_RLED_CURR   0x32
#define REG_PPG_CAN_ANA     0x34
#define REG_PPG_AVG         0x35
#define REG_PPG_PWIDTH_RATE 0x36
#define REG_FIFO_CFG        0x37
#define REG_FIFO_WR_PTR     0x38
#define REG_FIFO_RD_PTR     0x39
#define REG_FIFO_OVF_CNT    0x3A
#define REG_FIFO_DATA  0x3B
#define REG_PART_ID         0x3D
#define REG_DEVICE_CFG      0x4D
#define REG_OSC_TRIM        0x3E
#define REG_LED_TRIM        0x3F
#define REG_BIO_TRIM        0x40


//Define settings
//STATUS_0
#define POR_STATUS          0x80
#define LS_INT_STATUS       0x02
#define LS_NEW_DATA         0x01
//STATUS_1
#define LED_DRIVER_STATUS   0x40
#define FIFO_AFULL_STATUS   0x20
#define PPG_DATA_STATUS     0x10
#define PS_LOGIC_STATUS     0x04
#define PS_INT_STATUS       0x02
#define PS_NEW_DATA         0x01
//MAIN_CTRL_0
#define SW_RESET            0x01<<7
#define LS_SAI_ON           0x01<<3
#define LS_SAI_OFF          0x00
#define ALS_MODE            0x00
#define RGB_MODE            0x01<<1
#define LS_OFF              0x00
#define LS_ON               0x01
//MAIN_CTRL_1
#define PS_SAI_ON           0x01<<3
#define PS_SAI_OFF          0x00
#define PS_MODE             0x00
#define HR_MODE             0x01<<1
#define SPO2_MODE           0x02<<1
#define PPG_PS_ON           0x01
#define PPG_PS_OFF          0x00
#define TEMP_ON             0x01<<7
#define TEMP_OFF            0x00
//PS_CAN_PULSES
#define PS_CAN_ANA_0        0x00 //off
#define PS_CAN_ANA_1        0x01<<6 //50% of FS
#define PS_CAN_ANA_2        0x02<<6 //100% of FS
#define PS_CAN_ANA_3        0x03<<6 //150% of FS
#define PS_PULSES(x)    (( (x) & (0x07) )<<3) //where x = 0..5 and num pulses = 2^x        
//PS_PWIDTH_RATE
#define PS_PWIDTH(x)   (x & 0x03)<<4 //where x = 0..3
#define PS_RATE(x)          (x & 0x07) //where x = 0..7      
//PS_MOV_AVG_HYS
#define PS_AVG_ON           1<<7
#define PS_AVG_OFF          0
#define PS_HYS_LEVEL(x)     x>>1 //where x=0..256
//LS_RES_RATE
#define LS_RES(x)           (x & 0x07)<<4 //where x=0..7
#define LS_RATE(x)          (x & 0x07) //where x=0..7        

#define LS_RES_20          0x00
#define LS_RES_19          0x01
#define LS_RES_18          0x02
#define LS_RES_17          0x03
#define LS_RES_16         0x04
#define LS_RES_13          0x05

//LS_GAIN
#define LS_GAIN(x)          (x & 0x03)
#define LS_GAIN_1           0x00
#define LS_GAIN_3           0x01
#define LS_GAIN_6           0x10
#define LS_GAIN_20          0x11

//LS_THRES_VAR
#define LS_THRES_VAR(x)     (x & 0x07)
//INT_CFG_0
#define LS_INT_SEL_W        0
#define LS_INT_SEL_G        1<<4
#define LS_INT_SEL_R        2<<4
#define LS_INT_SEL_B        3<<4
#define LS_THRES_INT_MODE  0
#define LS_VAR_INT_MODE     1<<1
#define LS_INT_ON           1
#define LS_INT_OFF          0
//INT_CFG_1
#define AFULL_INT_ON        1<<5
#define AFULL_INT_OFF       0
#define PPG_INT_ON          1<<4
#define PPG_INT_OFF          0
#define PS_INT_READ_CLEARS  0<<1
#define PS_INT_LOGIC 1
#define PS_INT_ON           1
#define PS_INT_OFF          0

//INT_PST
#define LS_PERSIST(x)       (x & 0x0F)<<4
#define PS_PERSIST(x)       (x & 0x0F)
//PPG_PS_GAIN
#define PPG_PS_GAIN_1       0
#define PPG_PS_GAIN_1P5     1<<4
#define PPG_PS_GAIN_2       2<<4
#define PPG_PS_GAIN_4       3<<4
#define PPG_LED_SETTLING(x) (x & 0x03)<<2 // 0=0us, 1 = 5us, 2 = 10us (Default), 3=20us
#define PPG_ALC_TRACK(x)    (x & 0x03) // 0 = 10us, 1 (20us) DEFAULT ,2 = 30us, 3 = 60us
//PPG_PS_CFG
#define PPG_POW_SAVE_ON     1<<6
#define PPG_POW_SAVE_OFF    0
#define LED_FLIP_ON         1<<3
#define LED_FLIP_OFF        0
#define DIFF_OFF            2
#define ALC_OFF             1
#define DIFF_ON             0
#define ALC_ON              0
#define SIGNAL_OUT          1<<2
#define OFFSET_OUT          0
//PPG_CAN_ANA
#define PPG_CH1_CAN(x)      (x & 0x03)<<2
#define PPG_CH2_CAN(x)      (x & 0x03)
//PPG_AVG
#define PPG_AVG(x)          (x & 0x07)<<4
//PPG_PWIDTH_RATE
#define PPG_PWIDTH(x)       (x & 0x07)<<4
#define PPG_FREQ_60HZ       0
#define PPG_FREQ_50HZ       1<<3
#define PPG_RATE(x)         (x & 0x07)
//FIFO_CFG
#define FIFO_ROLL_ON        1<<4
#define FIFO_ROLL_OFF       0
#define AFULL_ADVANCE_WARNING(x)   (x & 0x0F)





#define POR_TIME_MS 5 //a guess


#define IR_TARGET_COUNTS 196608
#define R_TARGET_COUNTS 196608
#define TOL1 6000
#define TOL2 40000
#define STEP 8
#define IN_RANGE_PERSIST 4
#define IR_MAX_CURRENT 0x02AF
#define R_MAX_CURRENT 0x01FF


class OB1203
{
  uint8_t i2c_addr;
public:
  
  static OB1203 * current;
  MD_STATUS i2c_error;

  bool ready;
  
  static volatile bool busy;
  static void callback(void);
  
  char osc_trim;
  char ls_res;
  char ls_rate;
  char ls_gain;
  uint32_t ls_thres_hi;
  uint32_t ls_thres_lo;
  char ls_sai;
  char ls_mode;
  char ls_en;
  char ls_int_sel;
  char ls_var_mode;
  char ps_sai_en;
  char temp_en;
  char ppg_ps_mode;
  char ppg_ps_en;
  char ps_can_ana;
  char afull_int_en;
  char ppg_int_en;
  char ps_logic_mode;
  uint16_t ps_digital_can;
  char ps_int_en;
  char ls_persist;
  char ps_persist;
  uint16_t ps_thres_hi;
  uint16_t ps_thres_lo;
  uint16_t ps_current;
  uint16_t ir_current;
  uint16_t r_current;
  char ppg_ps_gain;
  char ppg_pow_save;
  char led_flip;
  char ch1_can_ana;
  char ch2_can_ana;
  char ppg_avg;
  char ppg_pwidth;
  char ppg_freq;
  char ppg_rate;
  char ppg_LED_settling;
  char ppg_ALC_track;
  char ps_pulses;
  char ps_pwidth;
  char ps_rate;
  char ps_avg_en;
  char ps_hys_level;
  char ls_int_en;
  char fifo_rollover_en;
  char fifo_afull_advance_warning;
  char writePointer;
  char readPointer;
  char fifoOverflow;
  char bio_trim;
  char led_trim;
  char diff;
  char alc;
  char sig_out;
  volatile bool ir_in_range;
  volatile bool r_in_range;
  volatile bool prev_in_range;
  volatile bool updateFastMode;
  volatile bool updateCurrent;

  OB1203 (uint8_t addr = OB1203_ADDR);

  // Low-level operations
  void reset();
  uint16_t get_status();
  void writeRegister(int, char, char);
  void writeBlock(int, char, char *, char);
  void readBlock(int, char, char *, int);
  uint32_t bytes2uint32(char *, int);
  uint32_t twoandhalfBytes2uint32(char *, int); 
  
  // High-level operations
  void setOscTrim();
  bool dataIsNew();
  bool lsIsNew();
  bool psIsNew();
  bool tempIsNew();
  bool bioIsNew();
  void setMainConfig();
  void setIntConfig();
  void setLSthresh();
  void setPSthresh();
  void setPScurrent();
  void setPPGcurrent();
  void setPPG_PSgain_cfg();
  void setPPGana_can();
  void setDigitalCan();
  void setPPGavg_and_rate();
  void setFifoConfig();
  void setBioTrim();
  void setLEDTrim();
  char get_ps_data(uint32_t *);
  char get_ls_data(uint32_t *);
  char get_ps_ls_data(uint32_t *);
  void resetFIFO();
  void init_ps();
  void init_rgb();
  void init_ps_rgb();
  void init_hr();
  void init_spo2();
  void getFifoInfo(char *fifo_info);
  uint8_t getNumFifoSamplesAvailable();
  void getNumFifoSamplesAvailable(char *fifo_info, char *sample_info);

  void getFifoSamples(uint8_t, char *);
  void parseFifoSamples(char, char *, uint32_t *);
  char get_part_ID(char *);
  
  
  //agc functions
  void do_agc(uint32_t,bool);
  
  //variables
  uint16_t rate;
  char res;
  char gain;
  uint32_t data_max;
  uint32_t reg_max;
  
  
  //    const uint32_t targetCounts[2];  
};


#endif