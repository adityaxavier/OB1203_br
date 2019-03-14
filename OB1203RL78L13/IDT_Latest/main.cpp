#include "mbed.h"
#include "OB1203.h"
#include "math.h"
#include "SPO2.h"

//normal
I2C i2c(I2C_SDA,I2C_SCL); //instantiate an i2c object from its class
#define intb_pin D3
#define DEBUG

//slave board//
//I2C i2c(D12,PA_7);
//#define intb_pin D10

InterruptIn intb(intb_pin); //declare an interrupt pin for INTB

//setting three 3 pins high to function as pullups for SDA, SCL and INTB
//Connect these pins to SDA, SCL and INTB by 2.7K resistors.
//5K resistors also work usually
//DigitalOut sda_pullup(D10,1);
//DigitalOut scl_pullup(D11,1);
//DigitalOut intb_pullup(D12,1);


//RTOS variables
OB1203 ob1203(&i2c); //instantiate the OB1203 object from its class and pass i2c object
SPO2 spo2;
Serial pc(USBTX, USBRX,256000); //create a serial port for printing data to a pc
Timer t; //use a microsecond timer for time stamping data
//Timer p;
Semaphore sensor_ready(0);
#define PROX_MODE 0 //0 for prox, 1 for bio
#define BIO_MODE 1 //1 for bio mode
const uint32_t PROX_THRESHOLD = 10000; //counts
const uint32_t BIO_THRESHOLD = 100000; // -->for consistency change to AGC low limit
#define MAX_LOW_SAMPLES 50 //one second
#define PROX_DELAY 100
bool samples_ready = 0;

//function declarations
void defaultConfig();
void switch_mode(bool prox_bio_mode);
void regDump(uint8_t Addr, uint8_t startByte, uint8_t endByte);
void get_sensor_data();
void get_prox_data();
void intEvent(void);


//USER CONFIGURABLE*********
bool mode = PROX_MODE; //start in prox mode, then switch to HR mode when we detect proximity
const bool meas_ps = 1;
const bool ppg2 = 1; //0 for HR, 1 for SpO2
const bool meas_temp = 0;
const bool printRaw = 0; //default 0 print raw data
const bool printCurrent = 0;//print LED current and time stamp
const bool redAGC = 1;
const bool irAGC = 1;
const bool trim_oscillator = 0;
//****************************


//internal settings
volatile uint8_t num_low_samples = 0;
void defaultConfig() //populate the default settings here
{
    //high accuracy oscillator trim overwrite option
    ob1203.osc_trim = 0x3F; //max trim code =0x3F
    //temperature sensor settings (hidden registers)
    meas_temp ? ob1203.temp_en = TEMP_ON : ob1203.temp_en = TEMP_OFF;
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
//    pc.printf("ps_pulses = %02X\r\n",ob1203.ps_pulses);
    ob1203.ps_pwidth = PS_PWIDTH(1);
    ob1203.ps_rate = PS_RATE(5); //5 = 100ms
//    pc.printf("ps_rate = %02X\r\n",ob1203.ps_rate);
    ob1203.ps_avg_en = PS_AVG_OFF;
    ob1203.ps_can_ana = PS_CAN_ANA_0;
    ob1203.ps_digital_can = 0;
    ob1203.ps_hys_level = 0;
    meas_ps ? ob1203.ps_current = 0x1FF : ob1203.ps_current = 0x000;
//    ob1203.ps_current = 0;
    ob1203.ps_thres_hi = 0xFF;
    ob1203.ps_thres_lo = 0x00;

    //interrupts
    ob1203.ls_int_sel = LS_INT_SEL_W;
    ob1203.ls_var_mode = LS_THRES_INT_MODE;
    ob1203.ls_int_en = LS_INT_OFF;
    ob1203.ppg_ps_en = PPG_PS_ON;

    ob1203.ps_logic_mode = PS_INT_READ_CLEARS;
    ob1203.ps_int_en = PS_INT_OFF;
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
    //use rate 1 with pulse width 3 and average 4, or rate 3 with pulse width 4 and average 3 for 100 sps (50Hz basis) or 120 sps sample rate (60Hz basis)
    ob1203.ppg_avg = PPG_AVG(4); //2^n averages
    ob1203.ppg_rate = PPG_RATE(1);
    ob1203.ppg_pwidth = PPG_PWIDTH(3);
    ob1203.ppg_freq = PPG_FREQ_50HZ;
//    ob1203.ppg_freq = PPG_FREQ_60HZ;
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
        pc.printf("initial setup: bio\r\n");
        ppg2 ? ob1203.init_spo2() : ob1203.init_hr();
    } else {
        pc.printf("initial setup: ps\r\n");
        ob1203.ppg_int_en = PPG_INT_OFF;
        ob1203.ps_int_en = PS_INT_OFF;
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
        ob1203.ps_int_en = PS_INT_OFF;
        ob1203.init_ps();
    }
    pc.printf("switching to mode %d\r\n", mode);
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

    regData[0] = startByte;
    i2c.write(Addr,regData,1,true);
    i2c.read(Addr, regData, numBytes);
    for(int n=0; n<numBytes; n++) {
        pc.printf("%02X, %02X \r\n", startByte+n, regData[n]);
    }
}


void get_sensor_data()
{
    char samples2Read;
    char maxSamples2Read = 8; //FIFO samples, e.g. 4 samples * 3 bytes = 12 bytes (or 2 SpO2 samples) 16 samples is the entire SpO2 buffer.
    char fifoBuffer[maxSamples2Read*6];
    uint32_t ppgData[maxSamples2Read*2];
    char fifo_reg_data[3];
    char sample_info[3];
    char overflow;
    bool do_reset_fifo;
    if(mode ==  BIO_MODE) { //only if bio mode
//        pc.printf("reading samples\r\n");
        if(ob1203.afull_int_en) { //slow mode--find out how many samples in buffer
            ob1203.getNumFifoSamplesAvailable(fifo_reg_data,sample_info); //read the samples fifo registers and figure out how many samples are left
            samples2Read = (sample_info[1] > maxSamples2Read) ? maxSamples2Read : sample_info[1]; //limit the number of samples to the maximum
//            pc.printf("wr = %02x, read = %02x, overflow = %02x,sample_info[0] = %02x, sample_info[1] = %02x,  samples2Read %d\r\n",ob1203.writePointer,ob1203.readPointer,ob1203.fifoOverflow,sample_info[0],sample_info[1],samples2Read);
    //        pc.printf("sample_info: hr samples %d, ppg_samples %d, overflow %d\r\n",sample_info[0],sample_info[1], sample_info[2]);
        }
        else {
            samples2Read = 1; //read one sample
        }
        ob1203.getFifoSamples(samples2Read<<1,fifoBuffer);
//        pc.printf("%02x %02x %02x %02x %02x %02x\r\n",fifoBuffer[0],fifoBuffer[1],fifoBuffer[2],fifoBuffer[3],fifoBuffer[4],fifoBuffer[5]);
        ob1203.parseFifoSamples(samples2Read<<1,fifoBuffer,ppgData);
//        pc.printf("1st sample: %d %d\r\n",ppgData[0],ppgData[1]);
        if(ob1203.ir_in_range && ob1203.r_in_range) {
            for( int n=0; n<(overflow>>1); n++) {
                spo2.add_sample(ppgData[0],ppgData[1]); //duplicate oldest data to deal with missing (overwritten) samples
                if(spo2.sample_count < ARRAY_LENGTH) spo2.sample_count ++; //number of samples in buffer
            }
            for( int n=0; n<samples2Read; n++) { //add samples
                spo2.add_sample(ppgData[2*n],ppgData[2*n+1]); //add data to sample buffer when data is in range
                if(spo2.sample_count < ARRAY_LENGTH) spo2.sample_count ++; //number of samples in buffer
            }
        } else {
//            pc.printf("out of range\r\n");
            spo2.sample_count = 0;
        }

        if(ppg2) { //print two columns of data with or without current
            //PRINT RAW DATA
            if(printCurrent) {
                for(int n = 0; n<(overflow>>1); n++) {
                    //print oldest sample in the place of missing samples
                    pc.printf("overflow %d, %d, %d, %d, %d\r\n",ppgData[0],ppgData[1],ob1203.ir_current,ob1203.r_current); //(use only with slower data rates or averaging as this slows down the data printing);
                }
                for(int n= 0; n<samples2Read; n++) {
                    //print samples from FIFO
                    pc.printf("%d, %d, %d, %d\r\n",ppgData[2*n],ppgData[2*n+1],ob1203.ir_current,ob1203.r_current); //(use only with slower data rates or averaging as this slows down the data printing);
                }
            } else if (printRaw) {
                for(int n = 0; n<(overflow>>1); n++) {
                    //print oldest sample in the place of missing samples
                    pc.printf("overflow %d, %d\r\n",ppgData[0],ppgData[1]);
                }
                for(int n= 0; n<samples2Read; n++) {
                    //print samples from FIFO
                    pc.printf("%d,    %d\r\n",ppgData[2*n],ppgData[2*n+1]); 
                }
            }//end print if
        }//end SpO2 case
        else { //HR mode print one column of data
            //PRINT RAW DATA
            for(int n=0; n<samples2Read; n++) {
                pc.printf("%d\r\n%d\r\n",ppgData[0],ppgData[1]);
            }
        }//HR case

        if(irAGC) {
            ob1203.do_agc(ppgData[2*(samples2Read-1)],0);  //use the most recent sample in the FIFO
        }//end IR AGC case
        if(ppg2 && redAGC) {
            ob1203.do_agc(ppgData[2*(samples2Read-1)+1],1);  //use the most recent sample in the FIFO
        }//enr R AGC case
        if(ob1203.updateFastMode || ob1203.updateCurrent)
            do_reset_fifo = 1;
        if(ob1203.updateFastMode) {
            pc.printf("switching FastMode to %d\r\n",ob1203.ppg_int_en);
            ob1203.setIntConfig();
            ob1203.updateFastMode = 0;
        }
        if(ob1203.updateCurrent) {
            ob1203.setPPGcurrent();
            ob1203.updateCurrent=0;
        }
        if(do_reset_fifo) {
            ob1203.resetFIFO();
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
        }
    }//end if mode
    else { //prox mode case
        get_prox_data();
    }
    samples_ready = 0;
}//end get_sensor_data

void get_prox_data()
{
    char valid;
    uint32_t ps_data[1]; //array for storing parsed samples
    wait_ms(PROX_DELAY);
    if(mode == PROX_MODE) {
        valid = ob1203.get_ps_data(ps_data);
        pc.printf("prox = %d\r\n",ps_data[0]);
        if(ps_data[0] > PROX_THRESHOLD) { //got signal, switch to bio mode
            mode = BIO_MODE;
            switch_mode (mode);
        }
    }
}

void intEvent(void)
{
    samples_ready = 1;
}


int main()
{   
    i2c.frequency( 400000 ); //always use max speed I2C
    bool do_part2;
//    pc.printf("register settings\r\n");
//    regDump(OB1203_ADDR,0,19);
//    regDump(OB1203_ADDR,20,39);
//    regDump(OB1203_ADDR,40,59);
//    regDump(OB1203_ADDR,60,77);

//    pc.printf("do initial config\r\n");
    defaultConfig(); //do the ASIC configuration now

    pc.printf("register config\r\n");
    regDump(OB1203_ADDR,0,19);
    regDump(OB1203_ADDR,20,39);
    regDump(OB1203_ADDR,40,59);
    regDump(OB1203_ADDR,60,77);

    intb.fall(&intEvent); //attach a falling interrupt
    t.start(); //start microsecond timer for datalogging

    wait(1); //finish regDump

    intb.fall(&intEvent); //set an interrupt on the falling edge
    pc.printf("begin autogain\r\n");
    //while ( (!ob1203.ir_in_range) && (!ob1203.r_in_range) ) {} //wait for AGC complete
    pc.printf("10s SPO2, 10s HR, 1s SPO2, 1s HR, R\r\n");
    while(1) { //main program loop
        t.reset();
        if(ob1203.afull_int_en) {
            spo2.do_algorithm_part1();
        }
        do_part2 = 1;
        while(t.read()<1) {
            if(mode == BIO_MODE) {
                if(samples_ready) { //only read data if available (samples_ready is asserted by ISR and cleard by get_sensor_data)
                    get_sensor_data();
                }
            } else {
                get_sensor_data(); //includes a delay for prox samples
            }
            if(do_part2 && ob1203.afull_int_en) {
                //if we are in bio mode and we haven't done part 2 yet
                spo2.do_algorithm_part2();
                do_part2 = 0;
            }
        }
    }
}//end main