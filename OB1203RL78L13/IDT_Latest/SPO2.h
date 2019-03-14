#ifndef __SPO2_H__
#define __SPO2_H__

#include "mbed.h"
//#define NUM_FILTER_TAPS 32
//#define NUM_FILTER_TAPS 18
#define NUM_FILTER_TAPS 8
//#define NUM_FILTER_TAPS 12
#define SAMPLE_LENGTH 150 //40 BMP at 100sps, also MAX_OFFSET
#define MAX_OFFSET 150
#define MIN_OFFSET  32 //180 BPM at 100sps
#define ARRAY_LENGTH 301 //twice sample length plus 1
#define SAMPLE_RATE 100 //sps
#define SAMPLE_RATE_MIN 6000 //spm
#define OFFSET_FOR_SLOPE_CALC 16 //half of max offset for 180 bpm
#define DEFAULT_GUESS 75 //80 BPM at 100sps
#define NUM_HR_AVGS 8
#define NUM_SPO2_AVGS 8
#define NUM_COARSE_POINTS 5
#define SMALL_STEP 1
#define MID_STEP 2
#define MAX_HR_CHANGE1f 2 //can change 20% per sample
#define MAX_SPO2_DROP1f -1 //can change -6.25% per sample (includes 4 bits of precision
#define DROP_MIDDLE 4 //enumerations of methods
#define USE_LATEST 3
#define USE_LATEST_AVG 2
#define USE_PREV 1
#define USE_ALL 0
#define FIXED_BITS 4 //for basic fixed precision enhancement
#define IR 1 //channel definitions. Can't switch these (might seem illogical because IR data comes in first) because when we cycle through channels the last one to be analyzed with DC removal is IR, which is used for HR calculations
#define RED 0


//#define DEBUG



class SPO2
{


public:
    SPO2();
//functions
    void do_algorithm(void);
    void init_running_avgs(uint32_t init_ir_val, uint32_t init_r_val);
    void copy_data(uint8_t channel);
    void get_DC(void);
    void removeDC(void);
    void get_rms(void);
    void get_idx(void); //get the indices to use for the array
    void calc_R(void);
    void calc_spo2(void);
    int32_t corr(int16_t *x, uint16_t start_ptr, uint16_t len, uint16_t offset);
    void add_sample(uint32_t ir_data, uint32_t r_data);
    void fine_search(int16_t *x, uint16_t len, uint32_t start_offset, int32_t start_correl, uint32_t search_step);
    bool check4max(int16_t *x, uint16_t len,uint16_t start_offset, int32_t start_correl);
    void dither(int16_t *x, uint16_t len, uint16_t offset, const float *rel_vals, uint16_t num_vals, int32_t *correls, float *offsets);
    void get_corr_slope(int16_t *x, uint16_t len, uint16_t offset0, uint16_t offset1);
    bool find_max_corr(int16_t *x, uint16_t max_length, uint16_t offset_guess);
    void copy_data(void);
    void calc_hr(void);
    void consensus(void);
    float abs_float(float val);
//    int16_t fir( int16_t (*x)[2][NUM_FILTER_TAPS], uint8_t channel, uint16_t ptr, const int32_t *coefs, const uint8_t bit_prec);
    uint32_t uint_sqrt(uint32_t val);
    void do_algorithm_part1();
    void do_algorithm_part2();
    void avg8Samples();
//variables
   
    uint16_t data_ptr;  
    uint16_t prev_data_ptr;  
    int16_t AC1f[ARRAY_LENGTH]; //including 4 bits fixed point
//    int16_t fir_buffer[2][NUM_FILTER_TAPS];
//    uint16_t buffer_ptr;
    uint32_t dc_data[2][ARRAY_LENGTH];
    uint16_t start_ptr; //for fine search
    uint16_t start_point; //reference point in the array
    uint16_t idx[ARRAY_LENGTH];
    
//    uint32_t running_sum[2]; //used in the averaging of incoming data for bandwidth limiting

    int32_t mean1f[2];
    uint16_t sample_length; //length of correlation sum
    uint16_t array_length;//2*sample_length + 1 --must be odd
    uint16_t max_offset;
    uint16_t min_offset;
    
    float R;
    uint32_t rms1f[2];
    float rms_float[2];
    
    bool prev_valid;
    int32_t final_offset1f;
    int32_t final_correl;
    uint16_t offset_guess;
    uint16_t prev_offset;
    
    int32_t c0;
    int32_t m1f;
    
    uint16_t current_spo21f;
    uint16_t current_hr1f;
    uint16_t avg_hr;
    uint16_t avg_spo2;
    uint16_t sample_count;
    
    bool first_hr;
    bool first_spo2;
//    float ir_range[2];
//    float r_range[2];

    bool quickstart;
    
private:
//functions

    
//variables
};

extern SPO2 spo2;
#endif