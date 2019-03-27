#ifndef __SPO2_H__
#define __SPO2_H__

#include "mbed.h"
//#define NUM_FILTER_TAPS 32
//#define NUM_FILTER_TAPS 18
#define NUM_FILTER_TAPS 8
//#define NUM_FILTER_TAPS 12
#define SAMPLE_LENGTH 149 //40 BMP at 100sps, also MAX_OFFSET 
#define MAX_OFFSET 149 //
#define MIN_OFFSET  32 //180 BPM at 100sps
#define ARRAY_LENGTH 299 //twice sample length plus 1
#define SAMPLE_RATE 100 //sps
#define SAMPLE_RATE_MIN 6000 //spm
#define DEFAULT_GUESS 75 //80 BPM at 100sps
#define NUM_HR_AVGS 8
#define NUM_SPO2_AVGS 8
#define NUM_COARSE_POINTS 5
#define SMALL_STEP 1
#define MID_STEP 2
#define BIG_STEP 5
#define FIXED_BITS 4 //for basic fixed precision enhancement. Don't change this without looking carefully at spo2.corr function >>2 instances and potential overflows.
#define IR 1 //channel definitions. Can't switch these (might seem illogical because IR data comes in first) because when we cycle through channels the last one to be analyzed with DC removal is IR, which is used for HR calculations
#define RED 0
#define MAX_KALMAN_LENGTH 8 //set this to same as NUM_HR_AVGS
#define MIN_DATA_STD 4<<FIXED_BITS
#define ALG_FAIL_TOLERANCE 3 //number of algorith fails we tolerate before resetting the kalman filter
#define OUTLIER_DATA_TOLERANCE 3 //number of outlier samples we reject before resetting the kalman filter
#define KALMAN_THRESHOLD 2 //the multiplier for the standard deviation to use for the kalman filter

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
    int32_t corr(int16_t *x, uint16_t len, uint16_t offset);
    void add_sample(uint32_t ir_data, uint32_t r_data);
    void fine_search(int16_t *x, uint16_t len, uint32_t start_offset, int32_t start_correl, uint32_t search_step);
    bool check4max(int16_t *x, uint16_t len,uint16_t start_offset, int32_t start_correl);
    //void dither(int16_t *x, uint16_t len, uint16_t offset, const uint16_t *rel_vals, uint16_t num_vals, int32_t *correls, uint16_t *offsets);
//    void get_corr_slope(int16_t *x, uint16_t len, uint16_t offset0, uint16_t offset1);
    bool find_max_corr(int16_t *x, uint16_t max_length, uint16_t offset_guess);
    void copy_data(void);
    void calc_hr(void);
    uint32_t uint_sqrt(uint32_t val);
    void do_algorithm_part1();
    void do_algorithm_part2();
    void get_sum_squares();
    void peak_find(uint16_t *x0, int32_t *y, int32_t *x_fit1f, int32_t *y_fit) ;
    void simple_peak_find(int32_t *y, int32_t *x_fit1f, int32_t *yfit, uint16_t x_center, uint16_t step);
    void kalman(uint32_t *kalman_array,uint8_t *kalman_length,uint8_t *kalman_ptr,uint32_t *data_array,uint8_t *data_array_length,uint8_t *data_ptr, uint32_t new_data,volatile bool *reset_kalman,uint32_t *kalman_avg);
    uint32_t get_std(uint32_t *array,uint8_t length, uint32_t avg);
    uint32_t get_avg(uint32_t *array,uint8_t length);
    //variables
   
    uint16_t data_ptr;  
    uint16_t prev_data_ptr;  
    int16_t AC1f[ARRAY_LENGTH]; //including 4 bits fixed point
//    int16_t fir_buffer[2][NUM_FILTER_TAPS];
//    uint16_t buffer_ptr;
    uint32_t dc_data[2][ARRAY_LENGTH];
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
    uint32_t final_offset;
    int32_t final_offset1f;
    int32_t final_correl;
    uint16_t offset_guess;
    uint16_t prev_offset;
    int32_t prev_correl;
    
    int32_t c0;
    
    uint32_t current_spo21f;
    uint32_t current_hr1f;
    uint32_t avg_hr1f;
    uint32_t avg_spo21f;
    uint16_t sample_count;
    
    bool first_hr;
    bool first_spo2;
    volatile bool reset_kalman_hr;
    volatile bool reset_kalman_spo2;
    uint32_t kalman_hr_array[MAX_KALMAN_LENGTH];
    uint32_t kalman_spo2_array[MAX_KALMAN_LENGTH];
    
private:
//functions

    
//variables
};

extern SPO2 spo2;
#endif