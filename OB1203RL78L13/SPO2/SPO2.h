#ifndef __SPO2_H__
#define __SPO2_H__

#if defined(__CA78K0R__) || defined(__CCRL__) || defined(__ICCRL78__) 
#include "r_cg_macrodriver.h"
#include "r_cg_iica.h"
#include "r_cg_userdefine.h"
#include "KALMAN.h"
#endif //#if defined(__CA78K0R__) || defined(__CCRL__) || defined(__ICCRL78__) 

#define INTERVAL 70 //10ms timer clicks between algorithm runs. Set faster for breathing rate detection and update filter lengths accordingly.

#define SAMPLE_LENGTH 148 //40.5 BPM at 100sps, also MAX_OFFSET 
#define MAX_OFFSET (SAMPLE_LENGTH) //
#define MIN_OFFSET  30 //200 BPM at 100sps
#define MAX_FILTER_LENGTH 38 //for avgNsamples filter --MUST BE EVEN
#define MIN_FILTER_LENGTH 12 //~<40% of the minimum offset of 32
#define ARRAY_LENGTH (2*SAMPLE_LENGTH+MAX_FILTER_LENGTH+1) //twice sample length + max filter length plus 1 MUST BE ODD for sum of squares calculation and 2*SAMPLE_LENGTH+MAX_FILTER_LENGTH must be divisible by 2^DOWNSAMPLE_BITS
#define SAMPLE_RATE 100//samples per second
//**************************UPDATE SAMPLE RATE FOR PRODUCTION PARTS = 100****************************

#define SAMPLE_RATE_MIN 6000 //samples per minute
#define DEFAULT_GUESS 80 //75 BPM at 100sps
#define DOWNSAMPLE_BITS 2 //number of bits defining the reduction in data rate for slope estimation (e.g. 4x downsampling is 2 bits)

#define SMALL_STEP 1
#define MID_STEP 2
#define BIG_STEP 2
#define FIXED_BITS 4 //for basic fixed precision enhancement. Don't change this without looking carefully at spo2.corr function >>2 instances and potential overflows.
#define IR 1 //channel definitions. Can't switch these (might seem illogical because IR data comes in first) because when we cycle through channels the last one to be analyzed with DC removal is IR, which is used for HR calculations
#define RED 0

#define MIN_DATA_STD 4<<FIXED_BITS
#define ALG_FAIL_TOLERANCE 3 //number of algorith fails we tolerate before resetting the kalman filter
#define OUTLIER_DATA_TOLERANCE 3 //number of outlier samples we reject before resetting the kalman filter

#define CORR_KALMAN_LENGTH 4
#define CORR_DATA_LENGTH 13
#define CORR_MIN_STD_1F 10<<FIXED_BITS
#define CORR_KALMAN_THRESHOLD_1F 40



#define FILTER_BITS 5 //32 (for fastAvg2Nsamples filter)-not currently in use
#define MAX_FINE_STEP 3 //maximum step to be used in fine search for slow heart beats
//#define DEBUG


//now enter in all the constants for the kalman filters
#define NO_JUMPS 0
#define JUMPS_OK 1

//#define HR_KALMAN_LENGTH 9 //number of points to make a running average over
//#define HR_DATA_LENGTH 15
//#define HR_MIN_STD_1F 8<<FIXED_BITS
//#define HR_KALMAN_THRESHOLD_1F  40//the multiplier for the standard deviation to use for the kalman filter
//
//#define SPO2_KALMAN_LENGTH 8
//#define SPO2_DATA_LENGTH 8
//#define SPO2_MIN_STD_1F 4<<FIXED_BITS
//#define SPO2_KALMAN_THRESHOLD_1F 32

#define HR_KALMAN_LENGTH 13 //number of points to make a running average over
#define HR_DATA_LENGTH 21
#define HR_MIN_STD_1F 8<<FIXED_BITS
#define HR_KALMAN_THRESHOLD_1F  40//the multiplier for the standard deviation to use for the kalman filter

#define SPO2_KALMAN_LENGTH 11
#define SPO2_DATA_LENGTH 12
#define SPO2_MIN_STD_1F 4<<FIXED_BITS
#define SPO2_KALMAN_THRESHOLD_1F 32

//#define RR_KALMAN_LENGTH 8
//#define RR_DATA_LENGTH 15
//#define RR_MIN_STD_1F 10<<FIXED_BITS
//#define RR_KALMAN_THRESHOLD_1F 40

#define MAX_OUTLIER_COUNT 3
#define MAX_ALG_FAIL_COUNT 3

#define CONSENSUS_KALMAN_LENGTH 5
#define CONSENSUS_DATA_LENGTH 10
#define CONSENSUS_MIN_STD_1F 4<<FIXED_BITS //this is the breath period in intervals. FOr example 70 internal with 100 sample rate means we are looking for std_dev of less than 4 intervals or about 3 seconds * threshold.
#define CONSENSUS_KALMAN_THRESHOLD_1F 40

//#define BREATH_KALMAN_LENGTH 12
//#define BREATH_DATA_LENGTH 21
//#define BREATH_MIN_STD_1F 4<<FIXED_BITS
//#define BREATH_KALMAN_THRESHOLD_1F 40
//
//#define SHORT_BREATH_KALMAN_LENGTH 9
//#define SHORT_BREATH_DATA_LENGTH 16
//#define SHORT_BREATH_MIN_STD_1F 4<<FIXED_BITS
//#define SHORT_BREATH_KALMAN_THRESHOLD_1F 40

#define SHORT_MAX_BREATH_OFFSET 12
#define SHORT_BREATH_ARRAY_LENGTH 24
#define MAX_BREATH_OFFSET 18
#define BREATH_ARRAY_LENGTH 36

#define USE_MIN 1
#define DONT_USE_MIN 0

#define USE_SHORT 0
#define USE_LONG 1

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
    void calc_R(void);
    void calc_spo2(void);
    void get_idx();
    int32_t corr(int16_t *x, uint16_t len, uint16_t offset);
    void add_sample(uint32_t ir_data, uint32_t r_data);
    void fine_search(int16_t *x, uint16_t len, uint32_t start_offset, int32_t start_correl, uint32_t search_step);
    bool check4max(int16_t *x, uint16_t len,uint16_t start_offset, int32_t start_correl);
    bool find_max_corr(int16_t *x, uint16_t max_length, uint16_t offset_guess);
    void copy_data(void);
    void calc_hr(void);
    uint32_t uint_sqrt(uint32_t val);
    uint32_t uint_sqrt(unsigned long long val);
    void do_algorithm_part1();
    void do_algorithm_part2();
    void do_algorithm_part3();
    void get_sum_squares();
    void peak_find(uint16_t *x0, int32_t *y, int32_t *x_fit1f, int32_t *y_fit) ;
    void simple_peak_find(int32_t *y, int32_t *x_fit1f, int32_t *yfit, uint16_t x_center, int32_t step);
    /*void kalman(uint32_t *kalman_array, uint8_t *kalman_length, uint8_t max_kalman_length, uint8_t *kalman_ptr, 
                  uint32_t *data_array, uint8_t *data_array_length, uint8_t max_data_length, uint8_t *data_ptr, 
                  uint32_t new_data, volatile bool *reset_kalman, uint32_t *kalman_avg, 
                  uint8_t *outlier_cnt, uint8_t *alg_fail_cnt, uint32_t kalman_threshold_1f, uint32_t min_data_std, bool jumps_ok);
    */
    uint32_t get_std(uint32_t *array,uint8_t length, uint32_t avg);
    uint32_t get_avg(uint32_t *array,uint8_t length);
    void fastAvg2Nsamples(int16_t *x); 
    void avgNsamples(int16_t *x, uint8_t number2avg);
    //void set_filters(KALMAN* filter1, KALMAN* filter2, KALMAN* filter3, KALMAN* filter4, KALMAN* filter5, KALMAN* filter6); //utility for populating KALMAN classes within the algorithm SPO2 class after they are instantiated.
    void set_filters(KALMAN* filter1, KALMAN* filter2, KALMAN* filter3, KALMAN* filter4); //utility for populating KALMAN classes within the algorithm SPO2 class after they are instantiated.
    void get_peak_diff(void); 
    int get_direction(uint32_t data1, uint32_t data2);
    void findminmax(int32_t* data, uint16_t start_ind, uint16_t stop_ind, int32_t* c, int* extreme, int* type); //function to find minima and maxima in an array.
    int32_t get_peak_height (int32_t *peak_xdata, int32_t *peak_ydata);
    void peak2peak(int32_t* ac_data);
    void ext_prec_peak_find(int32_t *y, int32_t *x_fit1f, int32_t *y_fit, uint16_t x_center, int32_t step);
    
    void get_autocorrelation(int32_t * array, int32_t * autocorrelation,int array_length, int max_offset);
    void subtract_mean(int32_t* array,int length);
    void unwrap_buffer_to_int(uint32_t *circular_buffer, int32_t *unwrapped_buffer, int length, int ind);
    uint8_t get_period_from_array(int32_t* array, int max_offset, int32_t* period1f, bool use_min);
    
//    void test_algorithm_part3(void);
    
//    void test_kalman(void);
    
    
    
    //variables
    int16_t idx[((ARRAY_LENGTH-1)>>DOWNSAMPLE_BITS)+1];
    int16_t idx2[((ARRAY_LENGTH-1)>>DOWNSAMPLE_BITS)+1];
    uint16_t data_ptr;  
    uint16_t prev_data_ptr;  
    int16_t AC1f[ARRAY_LENGTH]; //including 4 bits fixed point
//    int16_t fir_buffer[2][NUM_FILTER_TAPS];
//    uint16_t buffer_ptr;
    uint32_t dc_data[2][ARRAY_LENGTH];
    
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
    volatile uint16_t sample_count; //keeps track of whether the sample buffer is full or not. If full then alg runs.
    volatile uint32_t total_sample_count; //keeps track of total number of samples collected
    uint32_t alg_start_sample_count; //records the number most recent sample when the algorithm starts
    uint32_t prev_alg_start_sample_count; //remembers the sample count start position of the previous alg run
    
    volatile bool reset_kalman_hr; //flag to reset kalman filter for HR
    volatile bool reset_kalman_spo2; //flag to reset kalman filter for SpO2
    uint32_t kalman_hr_array[HR_KALMAN_LENGTH]; //stores "accepted" values for kalman filter for HR
    uint32_t kalman_spo2_array[SPO2_KALMAN_LENGTH]; //stores "accepted" values for kalman filter for spo2
    uint16_t p2_start_time; //tracking when the algorithm starts to check for timeout
    
    uint16_t display_spo2; //value output to display
    uint16_t display_hr; //value output to display

    int32_t fit_correl; //estimated correlation value from quadratic fit function.
    uint8_t samples2avg; //length of moving average filter for heart rate calculations
    
    uint16_t ds_start; //point in the array where the downsampled data will start;
    uint16_t downsampled_array_length;
    uint16_t downsampled_max_centered_index;
    
    uint16_t rr_time1f; //fixed precision output from peak find beat to beat time --FIGURE OUT A WAY TO PUBLISH THIS DATA. IT IS ASYNCHROUS. 
    int32_t peak_amp; //from simple_peak_find and get_peak_height

    uint32_t hr_data_buffer[2*MAX_BREATH_OFFSET];
    uint8_t hr_data_buffer_ind;
    uint32_t breathing_rate1f; //breaths per minute with fixed precision
    
    int alg_count;
    int peak_count;
    int peak_valley_buffer;
    
private:
//functions

    
//variables
    KALMAN* corr_filter;
    KALMAN* hr_filter;
    KALMAN* spo2_filter;
    //KALMAN* rr_filter;
    KALMAN* consensus_breath_filter;
    //KALMAN* short_breath_filter;
    //KALMAN* breath_filter;
};

extern SPO2 spo2;




#endif