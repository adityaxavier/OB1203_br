#ifndef __SPO2_H__
#define __SPO2_H__

#include "mbed.h"

#define DATA_BUFF_MASK 0x01FF
#define DATA_BUFF_SIZE 512
#define AVG_BUFF_MASK 0x0007
#define AVG_BITS 3
#define MIN_OFFSET  32 //180 BPM at 100sps
#define MAX_OFFSET  150 //40 BMP at 100sps
#define SAMPLE_RATE 100 //sps
#define OFFSET_FOR_SLOPE_CALC 16 //half of max offset for 180 bpm
#define DEFAULT_GUESS 75 //80 BPM at 100sps
#define NUM_HR_AVGS 10
#define NUM_SPO2_AVGS 10
#define NUM_COARSE_POINTS 5
#define MID_STEP 2
#define MAX_HR_CHANGE 0.15 //can change 20% per sample
#define MAX_SPO2_DROP -0.05 //can change 5% per sample
#define DROP_MIDDLE 4
#define USE_LATEST 3
#define USE_LATEST_AVG 2
#define USE_PREV 1
#define USE_ALL 0


//#define DEBUG

void do_algorithm();

class SPO2
{


public:
    SPO2();
//functions
    void init_running_avgs(uint32_t init_ir_val, uint32_t init_r_val);
    void get_sum_squares();
    void get_alg_params();
    void removeDC();
    void get_rms();
    void calc_R();
    void calc_spo2();
    int32_t corr(int32_t *x, uint16_t start_ptr, int16_t len, uint16_t offset);
    void add_sample(uint32_t ir_data, uint32_t r_data);
    void fine_search(int32_t *x, uint16_t len, uint32_t start_offset, int32_t start_correl, uint32_t search_step);
    bool check4max(int32_t *x, uint16_t len,uint16_t start_offset, int32_t start_correl);
    void dither(int32_t *x, uint16_t len, uint16_t offset, const float *rel_vals, uint16_t num_vals, int32_t *correls, float *offsets);
    void get_corr_slope(int32_t *x, uint16_t len, uint16_t offset0, uint16_t offset1);
    bool find_max_corr(int32_t *x, uint16_t max_length, uint16_t offset_guess);
    void copy_data();
    void calc_hr();
    void consensus();
    float abs_float(float val);
    
//variables
   
    uint16_t data_ptr;  
    uint16_t prev_data_ptr;  
    uint32_t ir_buffer[512];
    uint32_t r_buffer[512];
    int32_t irAC[512];
    int32_t rAC[512];
    uint16_t start_ptr;
    uint16_t start_point;

    uint32_t ir_avg_buffer[8];
    uint32_t r_avg_buffer[8];
    uint16_t avg_ptr;
    
    uint32_t ir_running_sum;
    uint32_t r_running_sum;
    
    int32_t ir_mean;
    int32_t r_mean;
    
    uint16_t sample_length; //length of correlation sum
    uint16_t array_length;//2*sample_length + 1 --must be odd
    uint16_t max_offset;
    uint16_t min_offset;
    
    float ir_slope;
    float r_slope;
    int32_t sum_squares;
    int32_t ind[2*MAX_OFFSET+1];
    
    uint16_t prev_sample_length;
    
    float ir_rms;
    float r_rms;
    float R;
    
    bool prev_valid;
    float final_offset;
    int32_t final_correl;
    uint16_t offset_guess;
    uint16_t prev_offset;
    
    int32_t c0;
    float m;
    
    float current_spo2;
    float current_hr;
    float avg_hr;
    float avg_spo2;
    uint16_t sample_count;
    
    bool first_hr;
    bool first_spo2;
    float ir_range[2];
    float r_range[2];

    bool quickstart;
    
private:
//functions

    
//variables
};

extern SPO2 spo2;
#endif