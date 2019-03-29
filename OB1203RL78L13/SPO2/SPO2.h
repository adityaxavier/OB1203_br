#ifndef __SPO2_H__
#define __SPO2_H__

#if defined(__CA78K0R__) || defined(__CCRL__) || defined(__ICCRL78__) 
#include "r_cg_macrodriver.h"
#include "r_cg_iica.h"
#include "r_cg_userdefine.h"
#endif //#if defined(__CA78K0R__) || defined(__CCRL__) || defined(__ICCRL78__) 

#define SAMPLE_LENGTH 148 //40.5 BPM at 100sps, also MAX_OFFSET 
#define MAX_OFFSET (SAMPLE_LENGTH) //
#define MIN_OFFSET  30 //200 BPM at 100sps
#define MAX_FILTER_LENGTH 32 //for avgNsamples filter --MUST BE EVEN
#define MIN_FILTER_LENGTH 12 //~<40% of the minimum offset of 32
#define ARRAY_LENGTH (2*SAMPLE_LENGTH+MAX_FILTER_LENGTH+1) //twice sample length + max filter length plus 1 MUST BE ODD for sum of squares calculation
#define SAMPLE_RATE 100 //samples per second
#define SAMPLE_RATE_MIN 6000 //samples per minute
#define DEFAULT_GUESS 80 //75 BPM at 100sps
#define NUM_HR_AVGS 8
#define NUM_SPO2_AVGS 8
#define SMALL_STEP 1
#define MID_STEP 2
#define BIG_STEP 2
#define FIXED_BITS 4 //for basic fixed precision enhancement. Don't change this without looking carefully at spo2.corr function >>2 instances and potential overflows.
#define IR 1 //channel definitions. Can't switch these (might seem illogical because IR data comes in first) because when we cycle through channels the last one to be analyzed with DC removal is IR, which is used for HR calculations
#define RED 0
#define MAX_KALMAN_LENGTH 8 //set this to same as NUM_HR_AVGS. This is 8 seconds for most medical grade equipment.
#define MIN_DATA_STD 4<<FIXED_BITS
#define ALG_FAIL_TOLERANCE 3 //number of algorith fails we tolerate before resetting the kalman filter
#define OUTLIER_DATA_TOLERANCE 3 //number of outlier samples we reject before resetting the kalman filter
#define KALMAN_THRESHOLD 2 //the multiplier for the standard deviation to use for the kalman filter
#define FILTER_BITS 5 //32 (for fastAvg2Nsamples filter)-not currently in use
#define MAX_FINE_STEP 3 //maximum step to be used in fine search for slow heart beats
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
    void simple_peak_find(int32_t *y, int32_t *x_fit1f, int32_t *yfit, uint16_t x_center, int32_t step);
    void kalman(uint32_t *kalman_array,uint8_t *kalman_length,uint8_t *kalman_ptr,uint32_t *data_array,uint8_t *data_array_length,uint8_t *data_ptr, uint32_t new_data,volatile bool *reset_kalman,uint32_t *kalman_avg,uint8_t *outlier_cnt, uint8_t *alg_fail_cnt );
    uint32_t get_std(uint32_t *array,uint8_t length, uint32_t avg);
    uint32_t get_avg(uint32_t *array,uint8_t length);
    void fastAvg2Nsamples(int16_t *x); 
    void avgNsamples(int16_t *x, uint8_t number2avg);
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
    uint16_t max_offset; //determines minimum heart rate
    uint16_t min_offset; //determins maximum heart rate
    
    float R; //R value used for SpO2 calculation
    uint32_t rms1f[2]; //rms value in fixed precision
    float rms_float[2]; //rms value in float for R calculation
    
    bool prev_valid; //did the previous algorithm yeild a valid heart rate?
    uint32_t final_offset; //integer value nearest the peak
    int32_t final_offset1f; //fixed precision estimation of the correlation peak
    int32_t final_correl; //value of the correlation near the peak
    uint16_t offset_guess; //trial point to find the correlation peak
    uint16_t prev_offset; //previous values to use in the case of algorithm bonk
    int32_t prev_correl; 
    
    uint32_t current_spo21f; //most recent SpO2 calculation in percent and fixed precision
    uint32_t current_hr1f; //most recent heart rate value in BPM and fixed precision
    uint32_t avg_hr1f; //average heart rate determined by Kalman filter to output to user (fixed precision)
    uint32_t avg_spo21f;  //average Spo2 value determined by Kalman filter to output ot user (fixed precision)
    uint16_t sample_count; //keeps track of whether the sample buffer is full or not. If full then alg runs.
    
    volatile bool reset_kalman_hr; //flag to reset kalman filter for HR
    volatile bool reset_kalman_spo2; //flag to reset kalman filter for SpO2
    uint32_t kalman_hr_array[MAX_KALMAN_LENGTH]; //stores "accepted" values for kalman filter for HR
    uint32_t kalman_spo2_array[MAX_KALMAN_LENGTH]; //stores "accepted" values for kalman filter for spo2
    uint16_t p2_start_time; //tracking when the algorithm starts to check for timeout
    
    uint16_t display_spo2; //value output to display
    uint16_t display_hr; //value output to display

    int32_t fit_correl; //estimated correlation value from quadratic fit function.
    uint8_t samples2avg; //length of moving average filter for heart rate calculations

private:
//functions

    
//variables
};

extern SPO2 spo2;
#endif