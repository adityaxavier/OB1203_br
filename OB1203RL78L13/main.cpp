#if defined(DEBUG)
#include <stdio.h>
#endif
#include <math.h>
#if defined(__CA78K0R__) || defined(__CCRL__) || defined(__ICCRL78__) 
#include "r_cg_macrodriver.h"
#include "r_cg_iica.h"
#include "r_cg_userdefine.h"
#endif //#if defined(__CA78K0R__) || defined(__CCRL__) || defined(__ICCRL78__) 
#include "OB1203.h"
#include "SPO2.h"
#include "KALMAN.h"
#include "lcd_panel.h"
#include "ppg_lcd.h"

//normal
//I2C i2c(I2C_SDA,I2C_SCL); //instantiate an i2c object from its class
#define intb_pin D3
//#define DEBUG

//slave board//
//I2C i2c(D12,PA_7);
//#define intb_pin D10

typedef enum
{
  LCD_HEART_RATE,
  LCD_OXYGEN_LEVEL,
}lcd_info_t;

lcd_info_t display = LCD_HEART_RATE;

void (*p_IntB_Event)(void) = NULL;
//InterruptIn intb(intb_pin); //declare an interrupt pin for INTB

//setting three 3 pins high to function as pullups for SDA, SCL and INTB
//Connect these pins to SDA, SCL and INTB by 2.7K resistors.
//5K resistors also work usually
//DigitalOut sda_pullup(D10,1);
//DigitalOut scl_pullup(D11,1);
//DigitalOut intb_pullup(D12,1);

//RTOS variables
OB1203 ob1203; //instantiate the OB1203 object from its class and pass i2c object
SPO2 spo2;
KALMAN corr_filter(CORR_KALMAN_LENGTH, CORR_DATA_LENGTH, MAX_OUTLIER_COUNT, MAX_ALG_FAIL_COUNT, CORR_MIN_STD_1F, CORR_KALMAN_THRESHOLD_1F, NO_JUMPS);
KALMAN hr_filter(HR_KALMAN_LENGTH, HR_DATA_LENGTH, MAX_OUTLIER_COUNT, MAX_ALG_FAIL_COUNT, HR_MIN_STD_1F, HR_KALMAN_THRESHOLD_1F, NO_JUMPS);
KALMAN spo2_filter(SPO2_KALMAN_LENGTH, SPO2_DATA_LENGTH, MAX_OUTLIER_COUNT, MAX_ALG_FAIL_COUNT, SPO2_MIN_STD_1F, SPO2_KALMAN_THRESHOLD_1F, JUMPS_OK);
//KALMAN rr_filter(RR_KALMAN_LENGTH, RR_DATA_LENGTH, MAX_OUTLIER_COUNT, MAX_ALG_FAIL_COUNT, RR_MIN_STD_1F, RR_KALMAN_THRESHOLD_1F, NO_JUMPS);
KALMAN consensus_breath_filter(CONSENSUS_KALMAN_LENGTH, CONSENSUS_DATA_LENGTH, MAX_OUTLIER_COUNT, MAX_ALG_FAIL_COUNT, CONSENSUS_MIN_STD_1F, CONSENSUS_KALMAN_THRESHOLD_1F, NO_JUMPS);
//KALMAN short_breath_filter(SHORT_BREATH_KALMAN_LENGTH, SHORT_BREATH_DATA_LENGTH, MAX_OUTLIER_COUNT, MAX_ALG_FAIL_COUNT, SHORT_BREATH_MIN_STD_1F, SHORT_BREATH_KALMAN_THRESHOLD_1F, NO_JUMPS);
//KALMAN breath_filter(BREATH_KALMAN_LENGTH, BREATH_DATA_LENGTH, MAX_OUTLIER_COUNT, MAX_ALG_FAIL_COUNT, BREATH_MIN_STD_1F, BREATH_KALMAN_THRESHOLD_1F, NO_JUMPS);

//Serial pc(USBTX, USBRX,256000); //create a serial port for printing data to a pc
//Timer t; //use a microsecond timer for time stamping data
//Timer p;

#define PROX_MODE 0 //0 for prox, 1 for bio
#define BIO_MODE 1 //1 for bio mode
//const uint32_t PROX_THRESHOLD = 10000; //counts
const uint32_t BIO_THRESHOLD = 100000; // -->for consistency change to AGC low limit
#define MAX_LOW_SAMPLES 50 //one second
#define PROX_DELAY 100
volatile bool samples_ready = 0;
volatile bool just_woke_up = 0;
//function declarations
void defaultConfig();
void switch_mode(bool prox_bio_mode);
void regDump(uint8_t Addr, uint8_t startByte, uint8_t endByte);
void get_sensor_data();
void get_prox_data();
void dataEvent(void); //interrupt for data
void proxEvent(void); //interrupt for prox


//USER CONFIGURABLE*********
bool mode = PROX_MODE; //start in prox mode, then switch to HR mode when we detect proximity
const bool meas_ps = 1;
const bool ppg2 = 1; //0 for HR, 1 for SpO2
const bool redAGC = 1;
//const bool printCurrent = 0;
//const bool printRaw = 1;
const bool irAGC = 1;
const bool trim_oscillator = 0;
//****************************


//internal settings
//uint32_t prev_total_samples=0;
//uint32_t samples_collected=0;
//int32_t avg_sample_count=0;
volatile uint8_t num_low_samples = 0;
void defaultConfig() //populate the default settings here
{
    //high accuracy oscillator trim overwrite option
    ob1203.osc_trim = 0x3F; //max trim code =0x3F
    //temperature sensor settings (hidden registers)
    ob1203.temp_en = TEMP_OFF;
    //LS settings
    ob1203.ls_res = LS_RES(2); //2= 18bit 100ms, 0= max res
    ob1203.ls_rate = LS_RATE(2); //2 =100ms, 4 = 500ms
    ob1203.ls_gain = LS_GAIN(3); //gain 3 default (range)
    ob1203.ls_thres_hi = 0x000FFFFF;
    ob1203.ls_thres_lo = 0;
    ob1203.ls_sai = LS_SAI_OFF;
    ob1203.ls_mode = RGB_MODE;
    ob1203.ls_en = LS_ON;
    //PS and PPG settings
    ob1203.ps_sai_en = PS_SAI_OFF;
//    ob1203.ps_sai_en = PS_SAI_ON;
    if(mode) {
        ppg2 ? ob1203.ppg_ps_mode = SPO2_MODE : ob1203.ppg_ps_mode = HR_MODE;
    } else {
        ob1203.ppg_ps_mode = PS_MODE;
    }
    ob1203.ps_pulses = PS_PULSES(3);
//    LOG(LOG_DEBUG,"ps_pulses = %02X\r\n",ob1203.ps_pulses);
    ob1203.ps_pwidth = PS_PWIDTH(1);
    ob1203.ps_rate = PS_RATE(5); //5 = 100ms
//    LOG(LOG_DEBUG,"ps_rate = %02X\r\n",ob1203.ps_rate);
    ob1203.ps_avg_en = PS_AVG_OFF;
    ob1203.ps_can_ana = PS_CAN_ANA_0;
    ob1203.ps_digital_can = 0;
    ob1203.ps_hys_level = 0;
    meas_ps ? ob1203.ps_current = 0x0A8 : ob1203.ps_current = 0x000;
//    ob1203.ps_current = 0;
    ob1203.ps_thres_hi = 0x3A98; //15,000 decimal
    ob1203.ps_thres_lo = 0x00;

    //interrupts
    ob1203.ls_int_sel = LS_INT_SEL_W;
    ob1203.ls_var_mode = LS_THRES_INT_MODE;
    ob1203.ls_int_en = LS_INT_OFF;
    ob1203.ppg_ps_en = PPG_PS_ON;

    ob1203.ps_logic_mode = PS_INT_READ_CLEARS;
    ob1203.ps_int_en = PS_INT_ON;
    ob1203.ls_persist = LS_PERSIST(2);
    ob1203.ps_persist = PS_PERSIST(2);


    //BIO SETTINGS
    //int
    ob1203.afull_int_en = AFULL_INT_OFF;
    ob1203.ppg_int_en = PPG_INT_ON;
    //PPG
    ob1203.ir_current = 0x1AF; //max 1023. 3FF
    if (ppg2) {
//        ob1203.r_current = 0x0FF;
        ob1203.r_current = 0x1AF; //max 511. 1FF
    } else {
        ob1203.r_current = 0;
    }
    ob1203.ppg_ps_gain = PPG_PS_GAIN_1;
    ob1203.ppg_pow_save = PPG_POW_SAVE_OFF;
    ob1203.led_flip = LED_FLIP_OFF;
    ob1203.ch1_can_ana = PPG_CH1_CAN(0);
    ob1203.ch2_can_ana = PPG_CH2_CAN(0);
    //use rate 1 with pulse width 3 and average 4, or rate 3 with pulse width 4 and average 3 for 100 sps (60Hz) basis on new samples
    ob1203.ppg_avg = PPG_AVG(4); //2^n averages
    ob1203.ppg_rate = PPG_RATE(1);
    ob1203.ppg_pwidth = PPG_PWIDTH(3);
    ob1203.ppg_freq = PPG_FREQ;
    ob1203.bio_trim = 3; //max 3 --this dims the ADC sensitivity, but reduces noise
    ob1203.led_trim = 0x00; //can use to overwrite trim setting and max out the current
    ob1203.ppg_LED_settling = PPG_LED_SETTLING(2); //hidden regstier for adjusting LED setting time (not a factor for noise)
    ob1203.ppg_ALC_track = PPG_ALC_TRACK(2); //hidden register for adjusting ALC track and hold time (not a factor for noise)
    ob1203.diff = DIFF_ON; //hidden register for turning off subtraction of residual ambient light after ALC
    ob1203.alc = ALC_ON;  //hidden register for turning off ambient light cancelleation track and hold circuit
    ob1203.sig_out = SIGNAL_OUT; //hidden register for selecting ambient sample or LED sample if DIFF is off
    ob1203.fifo_rollover_en = FIFO_ROLL_ON;
    ob1203.fifo_afull_advance_warning = AFULL_ADVANCE_WARNING(0x0F); //warn as quickly as possible (after 17 samples with 0x0F)

    //run initialization according to user compile settings

    if(mode == BIO_MODE) {
        LOG(LOG_INFO,"initial setup: bio\r\n");
        ppg2 ? ob1203.init_spo2() : ob1203.init_hr();
    } else {
        LOG(LOG_INFO,"initial setup: ps\r\n");
        ob1203.ppg_int_en = PPG_INT_OFF;
        ob1203.init_ps();
    }
    if(trim_oscillator)
        ob1203.setOscTrim();
}

void switch_mode(bool prox_bio_mode)
{
    if(prox_bio_mode == BIO_MODE) {
        ob1203.afull_int_en = AFULL_INT_OFF;
        ob1203.ppg_int_en = PPG_INT_ON;
        ob1203.init_spo2();
        num_low_samples = 0;
    } else {
        ob1203.afull_int_en = AFULL_INT_OFF;
        ob1203.ppg_int_en = PPG_INT_OFF;
        ob1203.ps_int_en = PS_INT_ON;
        ob1203.init_ps();
    }
    LOG(LOG_INFO,"switching to mode %d\r\n", mode);
}

void regDump(uint8_t Addr, uint8_t startByte, uint8_t endByte)
{
    /*print the values of up to 20 registers--buffer limit, e.g.*/
    char regData[20];
    int numBytes;
    if (endByte>=startByte) {
        numBytes =  (endByte-startByte+1) < 20 ? (endByte-startByte+1) : 20;
    } else {
        numBytes=1;
    }

    ob1203.readRegisters(Addr, startByte, &regData[0], numBytes);
    
    for(int n=0; n<numBytes; n++) {
        LOG(LOG_INFO,"%02X, %02X\r\n", startByte+n, regData[n]);
    }
}

    const char maxSamples2Read = 16; //FIFO samples, e.g. 4 samples * 3 bytes = 12 bytes (or 2 SpO2 samples) 16 samples is the entire SpO2 buffer.
    char fifoBuffer[maxSamples2Read*6];
    uint32_t ppgData[maxSamples2Read*2];
void get_sensor_data()
{
    char samples2Read = 0;

    char fifo_reg_data[3];
    char sample_info[3];
    char overflow =0;
    bool do_reset_fifo =0 ;
//        LOG(LOG_DEBUG,"reading samples\r\n");
//    LOG(LOG_DEBUG,"%d \r\n",t.read_us());
    if(ob1203.afull_int_en) { //slow mode--find out how many samples in buffer
        ob1203.getNumFifoSamplesAvailable(fifo_reg_data,sample_info); //read the samples fifo registers and figure out how many samples are left
        samples2Read = (sample_info[1] > maxSamples2Read) ? maxSamples2Read : sample_info[1]; //limit the number of samples to the maximum
//            LOG(LOG_DEBUG,"wr = %02x, read = %02x, overflow = %02x,sample_info[0] = %02x, sample_info[1] = %02x,  samples2Read %d\r\n",ob1203.writePointer,ob1203.readPointer,ob1203.fifoOverflow,sample_info[0],sample_info[1],samples2Read);
//        LOG(LOG_DEBUG,"s2r = %d, sample_info: hr samples %d, ppg_samples %d, overflow %d\r\n",samples2Read, sample_info[0],sample_info[1], sample_info[2]);
        
        if(samples2Read == 0)
        {
          LOG(LOG_INFO,"samples2Read == 0\r\n");
        }
        
    } else {
        samples2Read = 1; //read one sample
        overflow = 0;
    }
    ob1203.getFifoSamples(samples2Read<<1,fifoBuffer);
//        LOG(LOG_DEBUG,"%02x %02x %02x %02x %02x %02x\r\n",fifoBuffer[0],fifoBuffer[1],fifoBuffer[2],fifoBuffer[3],fifoBuffer[4],fifoBuffer[5]);
    ob1203.parseFifoSamples(samples2Read<<1,fifoBuffer,ppgData);
//        LOG(LOG_DEBUG,"1st sample: %d %d\r\n",ppgData[0],ppgData[1]);
    if(ob1203.ir_in_range && ob1203.r_in_range) {
        for( int n=0; n<(overflow>>1); n++) {
            spo2.add_sample(ppgData[0],ppgData[1]); //duplicate oldest data to deal with missing (overwritten) samples
            spo2.total_sample_count++;
            if(spo2.sample_count < ARRAY_LENGTH) spo2.sample_count ++; //number of samples in buffer
        }
        for( int n=0; n<samples2Read; n++) { //add samples
            spo2.add_sample(ppgData[2*n],ppgData[2*n+1]); //add data to sample buffer when data is in range
            spo2.total_sample_count++;
            if(spo2.sample_count < ARRAY_LENGTH) spo2.sample_count ++; //number of samples in buffer
        }
    } else {
        LOG(LOG_INFO,"out of range\r\n");
        spo2.sample_count = 0;
    }

//    if(ppg2) { //print two columns of data with or without current
//        //PRINT RAW DATA
//        if(printCurrent) {
//            LOG(LOG_DEBUG,"overflow = %d\r\n",overflow);
//            for(int n = 0; n<(overflow>>1); n++) {
//                //print oldest sample in the place of missing samples
//                LOG(LOG_DEBUG,"overflow %d, %d, %d, %d, %d\r\n",ppgData[0],ppgData[1],ob1203.ir_current,ob1203.r_current); //(use only with slower data rates or averaging as this slows down the data printing);
//            }
//            for(int n= 0; n<samples2Read; n++) {
//                //print samples from FIFO
//                LOG(LOG_DEBUG,"%d, %d, %d, %d\r\n",ppgData[2*n],ppgData[2*n+1],ob1203.ir_current,ob1203.r_current); //(use only with slower data rates or averaging as this slows down the data printing);
//            }
//        } else if (printRaw) {
//            for(int n = 0; n<(overflow>>1); n++) {
//                //print oldest sample in the place of missing samples
//                LOG(LOG_DEBUG,"overflow %d, %d\r\n",ppgData[0],ppgData[1]);
//            }
//            for(int n= 0; n<samples2Read; n++) {
//                //print samples from FIFO
//                LOG(LOG_DEBUG,"%d, %d, %d\r\n",n, ppgData[2*n],ppgData[2*n+1]);
//            }
//        }//end print if
//    }//end SpO2 case
//    else { //HR mode print one column of data
//        //PRINT RAW DATA
//        for(int n=0; n<samples2Read; n++) {
//            LOG(LOG_DEBUG,"%d\r\n%d\r\n",ppgData[0],ppgData[1]);
//        }
//    }//HR case

    if(irAGC) {
        ob1203.do_agc(ppgData[2*(samples2Read-1)],0);  //use the most recent sample in the FIFO
    }//end IR AGC case
    if(ppg2 && redAGC) {
        ob1203.do_agc(ppgData[2*(samples2Read-1)+1],1);  //use the most recent sample in the FIFO
    }//enr R AGC case
    if(ob1203.updateFastMode || ob1203.updateCurrent) {
        do_reset_fifo = 1;
    }
    if(ob1203.updateFastMode) {
        LOG(LOG_INFO,"switching FastMode to %d\r\n",ob1203.ppg_int_en);
        ob1203.setIntConfig();
        ob1203.updateFastMode = 0;
        if(ob1203.ppg_int_en == 0) { //not in fastMode -- switching to collecting data
            spo2.reset_kalman_hr = 1; //reset the Kalman filter for SpO2
            spo2.reset_kalman_spo2 = 1; //reset the Kalman filter for SpO2
        }
    }
    if(ob1203.updateCurrent) {
        ob1203.setPPGcurrent();
        ob1203.updateCurrent=0;
    }
    if(do_reset_fifo) {
        ob1203.resetFIFO();
        do_reset_fifo = 0;
    }
    //switch modes if no signal
    if(ppgData[2*(samples2Read-1)]< BIO_THRESHOLD) {
        num_low_samples++;
    } else {
        num_low_samples = 0;
    }
    if (num_low_samples >= MAX_LOW_SAMPLES) {
        mode = PROX_MODE;
        switch_mode(mode);
        just_woke_up = 0;
        p_IntB_Event = &proxEvent; //set an interrupt on the falling edge
    }
    samples_ready = 0;
  
}//end get_sensor_data

//void get_prox_data()
//{
//    char valid;
//    uint32_t ps_data[1]; //array for storing parsed samples
//    wait_ms(PROX_DELAY);
//    if(mode == PROX_MODE) {
//        valid = ob1203.get_ps_data(ps_data);
//        LOG(LOG_INFO,"prox = %lu\r\n",ps_data[0]);
//        if(ps_data[0] > PROX_THRESHOLD) { //got signal, switch to bio mode
//            mode = BIO_MODE;
//            switch_mode (mode);
//        }
//    }
//}

void dataEvent(void)
{
    samples_ready = 1;
}


void proxEvent(void)
{
    just_woke_up = 1;
}

static volatile uint16_t t_time = 0;
void t_callback(void)
{
  /* Called periodically from ISR of timing unit */
  t_time++;
}

void t_start(void)
{
  extern void R_IT_Start(void);
  
  R_IT_Start();
}

void t_reset(void)
{
  extern void R_IT_Stop(void);
  R_IT_Stop();
  t_time = 0;
  t_start();
}

uint16_t t_read(void)
{
  return t_time;
}

void ob1203_spo2_main(void)
{
  //spo2.set_filters(&corr_filter,&hr_filter,&spo2_filter,&consensus_breath_filter,&short_breath_filter,&breath_filter);
  spo2.set_filters(&corr_filter,&hr_filter,&spo2_filter,&consensus_breath_filter);
  // spo2.test_kalman();//test the Kalman filter
  //spo2.test_algorithm_part3();

  
#if defined(MEASURE_PERFORMANCE)
    /* P03 := Logic low */
    P0_bit.no3 = 1;
#endif  
//    i2c.frequency( 400000 ); //always use max speed I2C
    bool do_part2 = false;
    bool samples_processed = false;
    bool do_part3 = false;
//    LOG(LOG_DEBUG,"register settings\r\n");
//    regDump(OB1203_ADDR,0,19);
//    regDump(OB1203_ADDR,20,39);
//    regDump(OB1203_ADDR,40,59);
//    regDump(OB1203_ADDR,60,77);

//    LOG(LOG_DEBUG,"do initial config\r\n");
    defaultConfig(); //do the ASIC configuration now

    LOG(LOG_INFO,"register config\r\n");
    regDump(OB1203_ADDR,0,19);
    regDump(OB1203_ADDR,20,39);
    regDump(OB1203_ADDR,40,59);
    regDump(OB1203_ADDR,60,77);

    p_IntB_Event = &dataEvent; //attach a falling interrupt
    
    t_start(); //start microsecond timer for datalogging

#if 0
    wait(1); //finish regDump
#endif
    
    p_IntB_Event = &proxEvent; //set an interrupt on the falling edge to wake from prox mode
    LOG(LOG_INFO,"begin autogain\r\n");
    //while ( (!ob1203.ir_in_range) && (!ob1203.r_in_range) ) {} //wait for AGC complete
    LOG(LOG_INFO,"10s SPO2, 10s HR, 1s SPO2, 1s HR, R\r\n");
    spo2.get_sum_squares(); //get sum_squares
#if defined(MEASURE_PERFORMANCE)
    /* P03 := Logic low */
    P0_bit.no3 = 0;
#endif  
    while(1) { //main program loop
        if(just_woke_up) {
            LOG(LOG_INFO,"got prox interrupt \r\n");
        }
        if(mode == PROX_MODE) {//prox mode case

            //*******put sleep command here and register wake on interrupt******

            if(just_woke_up) {
                mode = BIO_MODE;
                switch_mode(mode); //starts in fast mode
                p_IntB_Event = &dataEvent; //attach interrupt to data events
                just_woke_up = 0;
            }
        } else { //bio mode case
            t_reset();
            if(ob1203.afull_int_en) {
#if defined(MEASURE_PERFORMANCE)
                /* P03 := Logic high */
                P0_bit.no3 = 1;
#endif
//                samples_collected = spo2.total_sample_count - prev_total_samples;
//                 
//                if (avg_sample_count == 0) {
//                  avg_sample_count = samples_collected;
//                }
//                else {
//                  avg_sample_count += ((int32_t)samples_collected - (int32_t)avg_sample_count)>>3; //IIR with 16 sample tail
//                }
//                LOG(LOG_INFO,"avg spi = %ld, samples = %lu, total = %lu, prev = %lu\r\n",avg_sample_count,samples_collected,spo2.total_sample_count, prev_total_samples);
//                prev_total_samples = spo2.total_sample_count;//store current total sample count for
                spo2.do_algorithm_part1();
                samples_processed = false;
#if defined(MEASURE_PERFORMANCE)
                /* P03 := Logic low */
                P0_bit.no3 = 0;
#endif
            }
            do_part2 = 1;
            //while(t_read()<100) {//change this to adjust rate at which algorithm runs
            while(t_read()<INTERVAL) {//changing to 0.7 seconds to improve breathing rate detection. If you change this you need to update the breathing_filter average lengths and MAX_BREATH_OFFSET, BREATH_ARRAY_LENGTH, etc. to make them equal time
              if(mode == BIO_MODE) {
                if(samples_ready) { //only read data if available (samples_ready is asserted by ISR and cleard by get_sensor_data)
                  get_sensor_data();
                  samples_processed = true;
                }
              } else {
                break; //exit loop and go to sleep
              }
              if(do_part2 && ob1203.afull_int_en && (samples_processed == true)) 
              {
                //if we are in bio slow read mode and we haven't done part 2 yet
#if defined(MEASURE_PERFORMANCE)
                /* P05 := Logic high */
                P0_bit.no5 = 1;
#endif  
                spo2.do_algorithm_part2();
#if defined(MEASURE_PERFORMANCE)
                /* P05 := Logic low */
                P0_bit.no5 = 0;
#endif  
                do_part2 = 0;
                do_part3 = 1;
                samples_processed = false;
                ///**********ADD ALGORITHM PART 3 HERE*****************

                if(display==LCD_HEART_RATE)
                {
                  LCD_DISPLAY_OFF();
                  R_PPG_LCD_Display_SPO2(spo2.display_spo2);
                  display = LCD_OXYGEN_LEVEL;
                }
                else if(display==LCD_OXYGEN_LEVEL)
                {
                  LCD_DISPLAY_OFF();
                  R_PPG_LCD_Display_HRM(spo2.display_hr);
                  display = LCD_HEART_RATE;
                }
                
              }
              //end if do part 2. Run part 3 after processing additional samples.
              if ( (do_part3 == true) && (samples_processed == true) ) {
                P0_bit.no5 = 1;
                P0_bit.no3 = 1;
                spo2.do_algorithm_part3();
                do_part3 = 0;
                samples_processed = false;
                P0_bit.no5 = 0;
                P0_bit.no3 = 0;
              }
            } //end 1sec (or other set interval) loop
        } //end conditional
    }//end while (1)
}//end main