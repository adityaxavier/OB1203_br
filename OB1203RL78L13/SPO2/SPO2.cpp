#if defined(DEBUG)
#include <stdio.h>
#endif
#include <string.h>
#include <math.h>
#include "SPO2.h"
#include "OB1203.h"
#include "KALMAN.h"
//#include "SoftI2C.h"


/*This algorithm was developed by Dan Allen, IDT-Renesas 2019 and updated 2020 to add breath detection.
Beat to beat detection porting from Matlab is in progress.
The main program fetches data from OB1203 at 100Hz for both red and IR.
It arranges the bytes from the FIFO and calls add_sample(). Add sample keeps a 
running average of 4 samples (12Hz) and loads the filtered data
into dc_buffer. The DC buffer is long enough for two samples at the slowest heart
rate of ~40Hz, plus an extra sample to make the sum of squares easy and also an
even number of samples to be used as the maximum moving average filter for the heart rate samples.
Main calls get_sum_squares() one, which calculates some constants for algorithm.
It only needs to be run once.
Once each second main calls do_algorithm_part1(). This checks to make sure enough 
samples have been acquired then calls get rms(). 
get_rms() calls get_DC() which calculates the mean levels of IR and red over the entire buffer. 
Note any variable with a 1f ( "one eff" not "LF" - font can be confusing) means it has an 
additional 4 bits of fixed precision, set by FIXED_BITS, so mean1f is 16 times the mean. 
Then get_rms() calls copy_data() which does for each channel:
-subtracts the means
-store the AC residuals with fixed precision of 4 bits in AC1f[]
-calculate the slope: slope1f
-remove the slope: slope1f
-calculate the variance var1f
-calculate the RMS rms1f
This is about half the computational burden of the program.
The main program takes a break to collect more samples then calls do_algorithm_part2().
Part 2 calls calc_R() to get the R value (Red_AC/Red_mean) / (IR_AC/IR_mean). 
This has a floating point operation.
Then it calls calc_spo2() which has more floating point operations. 
This is really slow on the 16bit processor (~several ms). 
The spo2 calculation uses a polynomial in R to caculate the Spo2 level. 
It only uses half the buffer: one "sample_length" of 1.5 seconds. 
If you use a longer value you pick up error due to curvature and that leads 
to a low SpO2 level, since similar curvature increases the RMS more for the smaller red AC signal. 
Higher R is lower SpO2. Also any noise or motion increases R. So SpO2 isn't 
returned by do_algorith_part2() if a valid heart rate isn't obtained (returns 0).
Once the Spo2 is done we have the IR values stored in AC1f. 
We use those for the heart rate calculation in do_algorithm_part2(). 
The following steps are performed.
-filter the data a lot. This is necessary to flatten out the dichrotic notch 
which otherwise leads to false peaks (erroniously high heart rate). 
By default the maximum filter length is applied. If a previous valid heart 
rate average is availble it will use less filtering for faster heart rates. 
-run find_max_corr() to find the first maximum of the autocorrelation. 
This is a robust way to align to find the period of a signal with huge amounts 
of noise as every sample is included in every point in the autocorrelation function, 
so it is very smooth, especially compared to differentiating and peak finding 
using zero crossings of the derivative.
-calculate the heart rate from the correlation peak X using 
60 sec/min * 100 samples/sec / X samples per period = BPM. This is retained in fixed point precision.
-run a Kalman filter to look throw out outliers in both SpO2 and HR.
-update the filter length (samples2avg).
For all peak estimattions fast quadratic fits are used to obtain resolution higher
than the sample rate. 
After part 2 runs, additional samples are collected and then part 3 runs. This is the
breathing detection algorithm. A buffer is kept of recent heart rate measurements.
Part 3 runs an autocorrelation of the heart rate measurements, looking for periodicity.
The sinus arrhythmia is a periodic variation in the heart rate. Breathing in heart rate
speeds up due to smaller volume for the heart to expand. Breathing out, heart rate slows 
down due to larger volume. 
The autocorrelation is similar to an FFT, but faster for 16bit processors. 
Two autocorrelations are performed. One for a short time (preferred) and one
over a long time (used only when a short time correlation can't be found due to
a long time between breaths). A Kalman filter seeks to throw out outliers which
are expected due to sources of variation such as motion.
This algorithm cannot practically detect breathing more than about 1.5x faster than the heart rate. 
This is a practical limitation similar to a Nyquist criterion. It also does not detect
breathing rates slower than the maximum length of the autocorrelation. 
If variable leneght autocorrelation is implemented this may be possible, however
the dual range autocorrelation helps with the spontaneous nature of breathing which 
can change quickly from slow to fast.
A lag of about 20 seconds occurs in the breathing measurements due to the long time
required to collect enough heart rate samples to perform the autocorrelation.
Autocorrelation is used instead of traditional peak/valley detection because it is
more robust against noise and spurious measurements. Every single data point contributes to 
determining the periodic, not just the peak and valley points. (The signal is its own matched filter.)
For breathing rate detection the algorithm is running faster than previous versions with HR and SPO2.
Those versions ran at 1Hz. This runs at 1/0.7 = ~1.5Hz. That helps detect faster breathing.
Note that the Kalman filter has not been optimized and minimal verification has been performed.
This is a beta release.
*/

//#define PRINT_RAW //print raw, filtered data
//#define PRINT_AC1F //print DC and slope removed

typedef unsigned long long      uint64_t;
typedef signed long long        int64_t;


int32_t sum_squares;
int32_t sum_quads;


SPO2::SPO2()
{/*class initializer*/  
  min_offset = MIN_OFFSET;
  max_offset = MAX_OFFSET;
  prev_valid = 0;
  data_ptr=0;
  final_offset = 0;
  reset_kalman_hr = 1;
  reset_kalman_spo2 = 1;
  avg_hr1f = 0;
  avg_spo21f = 0;
  samples2avg = MAX_FILTER_LENGTH;
  downsampled_array_length = ((ARRAY_LENGTH-1)>>DOWNSAMPLE_BITS);
  ds_start = 0;
  alg_count = 0;
  peak_count = 0;
  peak_valley_buffer = 0;
  alg_start_sample_count = 0;
  prev_alg_start_sample_count = 0;
  if(downsampled_array_length & 0x0001) {//odd case
    ds_start = 1<<(DOWNSAMPLE_BITS-1);
  } else { //even case
    downsampled_array_length++;
  }
  downsampled_max_centered_index =  (downsampled_array_length-1)>>1;
  hr_data_buffer_ind = 0;
  
#if 0
  memset(dc_data, 0, sizeof(dc_data));
  memset(idx, 0, sizeof(idx));
  memset(idx2, 0, sizeof(idx2));
  memset(AC1f, 0, sizeof(AC1f));
  memset(mean1f, 0, sizeof(mean1f));
  memset(rms1f, 0, sizeof(rms1f));
  memset(rms_float, 0, sizeof(rms_float));
#endif
}

 
//void SPO2::test_algorithm_part3() {
//  const uint16_t array_len = 24;
//  uint32_t hr_buf[array_len];
//  int32_t autocorr[array_len];
//  int test_cnt = 0;
//  int32_t per1f;
//  int buf_ptr = 0; //test starting in the middle of the array
//  uint8_t error;
//  int32_t lin_buf[BREATH_ARRAY_LENGTH];
//  uint32_t test_array[2*array_len] = {
//  1100, 1200, 1300, 1400, 
//  1500, 1600, 1500, 1400, 
//  1300, 1200, 1100, 1000,
//  1250, 1500, 1550, 1350,
//  1100, 1050, 1200, 1400, 
//  1600, 1500, 1300, 1100,
//  1100, 1200, 1300, 1400, 
//  1500, 1600, 1500, 1400, 
//  1300, 1200, 1100, 1000,
//  1250, 1500, 1550, 1350,
//  1100, 1050, 1200, 1400, 
//  1600, 1500, 1300, 1100};
//  for (int m = 1; m<20; m++) {//try different delays
//    test_cnt = 0;
//    buf_ptr = m;
//    //check subtract mean
//    for (int n = 0; n<2*array_len; n++) {
//      buf_ptr++;
//      if (buf_ptr == BREATH_ARRAY_LENGTH) {
//        buf_ptr = 0;
//      }  
//      test_cnt++;
//      hr_buf[buf_ptr] = test_array[buf_ptr]; 
//      
//      
//      if(test_cnt>=BREATH_ARRAY_LENGTH) {
//        
////        LOG(LOG_INFO,"buf_ptr = %u, unwrapped =\r\n",buf_ptr);
//        
//        unwrap_buffer_to_int(hr_buf,lin_buf,BREATH_ARRAY_LENGTH,buf_ptr);
//        //      for (int n=0;n<BREATH_ARRAY_LENGTH;n++) {
//        //        int index1 = buf_ptr -n;
//        //        if (index1<0) index1+=BREATH_ARRAY_LENGTH;
//        //        LOG(LOG_INFO,"%d, %u, %lu, %ld\r\n",n, buf_ptr, hr_buf[index1],lin_buf[n]);
//        //      }
//        subtract_mean(lin_buf,BREATH_ARRAY_LENGTH);
//        //      for (int n=0;n<BREATH_ARRAY_LENGTH;n++) {
//        //        int index1 = buf_ptr -n;
//        //        if (index1<0) index1+=BREATH_ARRAY_LENGTH;
//        //        LOG(LOG_INFO,"%d, %u, %lu, %ld\r\n",n, buf_ptr, hr_buf[index1],lin_buf[n]);
//        //      }
//        //Consider removing slope here also. This would help with breath detection when in recovery from exercise.
//        get_autocorrelation(lin_buf,autocorr,BREATH_ARRAY_LENGTH,MAX_BREATH_OFFSET);
//        for (int n=0;n<MAX_BREATH_OFFSET;n++) {
//          LOG(LOG_INFO,"%d, %ld\r\n",n, autocorr[n]);
//        }
//        error = get_period_from_array(autocorr, MAX_BREATH_OFFSET, &per1f, USE_MIN);
//        
//        LOG(LOG_INFO,"buf_ptr = %d, err = %u, per1f = %d\r\n",buf_ptr,error, per1f);
//        break;
//      }
//    }
//  }
//}

//void SPO2::test_kalman(void) {
//  
//   
//  const uint32_t test_input = 1048576;
//  uint32_t test_output = hr_filter->uint_sqrt(test_input); //correct output is 1024.
//  
//  uint32_t hr_test_data[18] = {64, 72, 73, 74, 69, 82, 41, 68, 62, 63, 61, 68, 72, 100, 76, 58, 68, 65};
//  uint32_t spo2_test_data[18] ={89, 90, 92, 91, 96, 98, 94, 90, 88, 87, 75, 84, 81, 80, 93, 99, 97, 95};
//  const int num_test = 18;
//  
//  test_output = hr_filter->get_avg(hr_test_data,num_test); //test avg function (69 or 68.9)
//  test_output = hr_filter->get_std(hr_test_data,num_test); //test std dev function (6 or 5.9)
// 
////  uint32_t test_hr_array[num_test];
////  uint32_t test_spo2_array[num_test];
////  uint8_t hr_outlier_cnt = 0;
////  uint8_t spo2_outlier_cnt = 0;
//  
//  for (int n=0; n<num_test; n++) {
//    hr_filter->run_kalman(hr_test_data[n]);  //outlier at index 4 and 15
//    test_hr_array[n] = hr_filter->kalman_avg;
//    hr_outlier_cnt = hr_filter->outlier_cnt;
//    
//    spo2_filter->run_kalman(spo2_test_data[n]); //outlier at 10 and not at 14
//    test_spo2_array[n] = spo2_filter->kalman_avg;
//    spo2_outlier_cnt = spo2_filter->outlier_cnt;
//  }
//
//}

void SPO2::get_sum_squares()
{/*This function calculates the sum of squares used in the first order regression
  in get_rms(). It is run once at the beginning of the program unless the sample
  length is changed. Currently it is fixed. The method assumes the indices are centered
  on zero and calculates only one half and doubles it. For this reason we use an odd
  number of samples. Otherwise this would be fraction armithmetic as the indices
  would be -1/2, 1/2....
  This function also calculates the square values and stores them in idx2 and
  uses those squares to calculated the sum of fourth power terms used in the 
  calculation of the second order regression. See the Matlab code for notes.
  */
  LOG(LOG_INFO,"get sum squares\r\n");
  get_idx();
  sum_squares = 0;
  sum_quads = 0;
  //do the single-sided sum of squares and fourth power terms
  for (int n=0; n<downsampled_max_centered_index ; n++) {
    sum_squares += idx2[n];
    sum_quads += idx2[n]*idx2[n];
  }
  sum_squares = abs(sum_squares<<1); //make the sum double sided
  sum_quads = abs(sum_quads<<1) - sum_squares*sum_squares/(downsampled_array_length); //make the sum double sided and subtract the square term
}


void SPO2::do_algorithm_part1()
{/*This gets the mean levels, copies AC values into arrays and does DC and slope removal
  variance or RMS calculation for SpO2 if there are enough samples.*/
  LOG(LOG_INFO, "sample_count %d\r\n",sample_count);
  if(sample_count>=ARRAY_LENGTH)
  {       
    LOG(LOG_DEBUG, "do alg part 1\r\n");
    get_rms(); //subtract residual slopes and calculate RMS
  }
}


void SPO2::do_algorithm_part2() {
  /*This runs the SpO2 calculation and heart rate calculation. The algorithm
  was split in half to allow a non-RTOS machine to take a break to collect samples
  from the OB1203 buffer. SpO2 is calculated with the R values REDrms/REDmean / IRrms/IRmean.
  Heart rate is calculated by finding the first peak of the autocorrelation using
  variable stepping to find the minimum and then doing fine search at twice the minimum.
  Heart rate-dependent filtering is applied to the AC IR data.
  The stepping in the search algorithm is small for small offsets and large for
  large offsets to keep accuracy proportional and minimize the number of steps.
  The correlations is sample_length multply+add operations, so it takes time.
  Time limits have been applied to time_out in time to get samples from the buffer.
  A key feature is the Kalman filter which throws out outliers by comparing the recent sample
  standard deviation to the next sample. Samples twice the stdev are tossed and not
  included in the kalman running average estimation, but are included in the variance 
  calculation. If too many algorithm fails or outliers are incurred the Kalman resets.*/  
  extern uint16_t t_read(void);
  /*
  static uint32_t hr_samples[HR_DATA_LENGTH];
  static uint8_t hr_ptr = 0;
  static uint8_t spo2_ptr =0;
  static uint32_t spo2_samples[SPO2_DATA_LENGTH];
  static uint8_t num_hr_samples = 0;
  static uint8_t num_spo2_samples = 0;
  static uint8_t kalman_hr_ptr = 0;
  static uint8_t kalman_spo2_ptr = 0;
  static uint8_t kalman_hr_length = 0;
  static uint8_t kalman_spo2_length = 0;
  static uint8_t hr_outlier_cnt =0;
  static uint8_t hr_alg_fail_cnt = 0;
  static uint8_t spo2_outlier_cnt =0;
  static uint8_t spo2_alg_fail_cnt = 0;*/
  
  p2_start_time = t_read();
  offset_guess = DEFAULT_GUESS;
//  LOG(LOG_DEBUG,"sample_count %d\r\n",sample_count);
  if(sample_count>=ARRAY_LENGTH) {
//    LOG(LOG_DEBUG,"doing algorithm part 2\r\n");
    calc_R();
    calc_spo2();
    avgNsamples(AC1f,samples2avg); //smooth the data more to avoid detecting peaks from the dichrotic notch
    //        fastAvg2Nsamples(AC1f); //smooth the data with a 32 sample average
    find_max_corr(AC1f,(uint16_t)SAMPLE_LENGTH, offset_guess); //AC1f is the IR sample after get_rms is run
    
    calc_hr();
    if(current_hr1f == 0) {//don't take valid spo2 samples if we can't find a heart rate
      current_spo21f = 0;
    }
    
    
    corr_filter->run_kalman(current_hr1f); //this is an optional short duration filter designed to snag bad correlation function inputs and replace them with reasonably close values in time. 
    //It is effective because the breathing rate cauases a cyclic variation in the heart rate. If we get bad data, we don't want to the use averge value the nearest valid measurement.
    
    hr_filter->run_kalman(current_hr1f); //this is a traditional 8 second average of the heart rate that produces a medically relevant value. 
    //The effective filter length is slightly longer when we apply the correlation filter, which does some pre-averaging. To first order add the filter lengths in quadrature sqrt(8^2+3^3) = 8.5 sec

//    LOG(LOG_INFO,"SPO2: ");
    
    
    spo2_filter->run_kalman(current_spo21f); //this is a traditional several second flat average of the heart rate that produces a medically relevant value. 9 seconds matches the reference meter best.
    
  } else {
    LOG(LOG_DEBUG,"collecting data\r\n");
    avg_hr1f = 0;
    prev_valid=0;
    avg_spo21f = 0;
  }
  LOG(LOG_INFO,"%.2f, %.2f, %.2f, %.2f, %.4f\r\n",(float)spo2_filter->kalman_avg/(float)(1<<FIXED_BITS),(float)hr_filter->kalman_avg/(float)(1<<FIXED_BITS),(float)current_spo21f/float(1<<FIXED_BITS),(float)current_hr1f/(float)(1<<FIXED_BITS),R);
  display_spo2 = ((avg_spo21f>>FIXED_BITS) * 10) + (((0x0008 & avg_spo21f) == 0) ? 0 : 5);
  display_hr   = avg_hr1f>>FIXED_BITS;
  //reduce averaging for fast heart rates. Ramp from MAX_FILTER_LENGTH down to MIN_FILTER_LENGTH from max HR to min heart rate
  if(avg_hr1f > 0) {
    samples2avg = MAX_FILTER_LENGTH - ((uint32_t)(MAX_FILTER_LENGTH - MIN_FILTER_LENGTH)*((uint32_t)(avg_hr1f>>FIXED_BITS)-(uint32_t)40))/(uint32_t)160; 
    LOG(LOG_INFO,"filter = %u\r\n",samples2avg);
  }
}


uint32_t SPO2::get_std(uint32_t *array,uint8_t length, uint32_t avg) {/* calculates standard deviation for the Kalman filter */
  uint64_t * temp = new uint64_t [length];
  uint64_t curr_avg = 0;
  
  for(uint8_t itr = 0; itr < length; itr++)
  {
    /* Distance to mean */
    temp[itr] = abs((int64_t)array[itr]-avg);
    /* Square */
    temp[itr] *= temp[itr];
    temp[itr] = (temp[itr]/length);
    /* Sum of the square of the distance over length */
    curr_avg += temp[itr];
  }
  
  delete(temp);
  
  uint32_t ret_val = uint_sqrt(curr_avg);
  return ret_val;
}


void SPO2::get_idx() {
  LOG(LOG_INFO,"\r\n idx \r\n");
  int16_t val;
  for (uint16_t n = 0; n<downsampled_array_length; n++) {
    val = -downsampled_max_centered_index+n;
    idx[n] = val;
    LOG(LOG_DEBUG,"%d\r\n",idx[n]);
    idx2[n] = val*val;
  }
}                                      


uint32_t SPO2::get_avg(uint32_t *array,uint8_t length) {/*calculates the average for the st_dev calculation for the Kalman filter*/
  uint32_t avg = 0;
  for (uint16_t n = 0; n<length; n++) 
  {
    avg += (array[n]/length);
  }
  
  return avg;
}


void SPO2::copy_data(uint8_t channel) {
  /*copies all data from the dc_data buffers to temporary buffer and subtracts the DC level
  Output is AC1f-->extended precision array*/
  prev_alg_start_sample_count = alg_start_sample_count;
  alg_start_sample_count = total_sample_count-ARRAY_LENGTH;
  uint16_t indx = data_ptr;
  for (int n=0; n<ARRAY_LENGTH; n++) {
    AC1f[n] = (dc_data[channel][indx]<<FIXED_BITS) - mean1f[channel]; //load the ~11bit or less AC data into an array with fixed precision for DC removal, etc.
    indx++;
    if (indx == ARRAY_LENGTH) indx = 0; //loop back
  }
}


void SPO2::get_DC() {
  /*calculates the mean DC level being subtracted in mean and residual DC level
  for each channel and stores is in res_dc. The mean is used for SpO2 calculations.
  There is a lag in the mean and rms but this should not be significant as the mean
  is quite constant and changes in SpO2 are usually due to rms.*/
  for(uint8_t channel=0; channel<2; channel++) {
    LOG(LOG_DEBUG,"channel = %d, DC_data:\r\n",channel);
    mean1f[channel] = 0; //get the running mean
    for (uint16_t n=0; n<ARRAY_LENGTH; n++) {
      mean1f[channel] += dc_data[channel][n];
      LOG(LOG_DEBUG,"%lu\r\n",dc_data[channel][n]);
    }
    mean1f[channel] = (mean1f[channel]<<FIXED_BITS)/ARRAY_LENGTH;
    //        LOG(LOG_INFO,"channel %u mean = %ld\r\n",channel,mean1f[channel]);
  }
}


uint32_t SPO2::uint_sqrt(uint32_t val)
{
  //integer sqrt function from http://www.azillionmonkeys.com/qed/sqroot.html
  uint32_t temp, g=0, b = 0x8000, bshft = 15;
  do {
    if (val >= (temp = (((g << 1) + b)<<bshft--))) {
      g += b;
      val -= temp;
    }
  } while (b >>= 1);
  return g;
}


uint32_t SPO2::uint_sqrt(uint64_t val)
{
  //integer sqrt function from http://www.azillionmonkeys.com/qed/sqroot.html
  uint64_t temp, g=0, b = 0x80000000, bshft = 31;
  do {
    if (val >= (temp = (((g << 1) + b)<<bshft--))) {
      g += b;
      val -= temp;
    }
  } while (b >>= 1);
  return g;
}


void SPO2::get_rms(){
/*This takes samples from the IR and Red DC data buffers and calculates the rms and
  mean values needed for the SpO2 calculation. It reuses the AC1f data array for both
  Red and IR to same memory.*/
  int32_t slope1f = 0;
  int32_t ind;
  uint32_t var1f;
  int32_t parabolic4f = 0;
  get_DC(); //calculate residual DC level
  for (int channel = 0; channel<2; channel++) {
    
    var1f = 0;
    copy_data(channel); //copies data to AC1f[n] array (extended precision) and removes DC
    //LOG(LOG_DEBUG,"AC1f for channel %d\r\n",channel);
    LOG(LOG_DEBUG,"AC1f for channel %d\r\n",channel);
    //    ind = -((ARRAY_LENGTH-1)>>1);
    //    for (uint16_t n=0; n<ARRAY_LENGTH; n++) {
    //      LOG(LOG_DEBUG,"%ld, %d\r\n",ind,AC1f[n]);
    //      ind++;
    //    }
    
    //remove slope
    slope1f = 0;
    ind = -(int32_t)downsampled_max_centered_index;
    
    //calc slope
    LOG(LOG_DEBUG,"slope calc\r\n");
    for (uint16_t n=0; n<downsampled_array_length; n++ ) {
      slope1f += (int32_t)AC1f[ds_start+(1<<DOWNSAMPLE_BITS)*n]*ind;
      LOG(LOG_DEBUG,"%ld, %d, %ld\r\n",ind,AC1f[ds_start+(1<<DOWNSAMPLE_BITS)*n],slope1f);
      ind++;
    }
    LOG(LOG_DEBUG,"slope1f = %ld, %ld, %ld\r\n",slope1f, slope1f/sum_squares, slope1f/sum_squares/(1<<DOWNSAMPLE_BITS) );
    slope1f /= sum_squares; //normalize
    slope1f /= 1<<DOWNSAMPLE_BITS; //reduce slope by downsampling ratio
    
    //remove linear slope
    LOG(LOG_DEBUG,"AC1f[n] with slope removed\r\n");
    ind = -(int16_t)((ARRAY_LENGTH-1)>>1);
    for (uint16_t n=0; n<ARRAY_LENGTH; n++) {
      AC1f[n] -= ind*slope1f;
      LOG(LOG_DEBUG,"%ld, %d\r\n",ind,AC1f[n]);
      ind++;
    }
    
    //calc quadratic term
    ind = 0;
    LOG(LOG_DEBUG,"calc parabolic\r\n");
    for (uint16_t n = 0; n<downsampled_array_length; n++ ){
      parabolic4f += (int32_t)idx2[n]*(int32_t)AC1f[ds_start+(1<<DOWNSAMPLE_BITS)*n];
      LOG(LOG_DEBUG,"%d,%d,%ld,%ld,%ld\r\n",idx[n], idx2[n],(int32_t)AC1f[ds_start+(1<<DOWNSAMPLE_BITS)*n],(int32_t)idx2[n]*(int32_t)AC1f[ds_start+(1<<DOWNSAMPLE_BITS)*n],parabolic4f);
      ind++;
    }
    parabolic4f = (int32_t)((((int64_t)parabolic4f)<<(FIXED_BITS*3))/(int64_t)sum_quads); //use 12 more bits fixed precision and int64s for the intermediate calculation
    parabolic4f /= (1<<DOWNSAMPLE_BITS)*(1<<DOWNSAMPLE_BITS);
    
    LOG(LOG_DEBUG,"parabolic4f = %ld\r\n",parabolic4f);
    
    //remove quadratic term
    ind = -(int16_t)((ARRAY_LENGTH-1)>>1);
    for (uint16_t n=0; n<ARRAY_LENGTH; n++) {
      AC1f[n] -= (int32_t)(((int64_t)((int32_t)ind*(int32_t)ind)*(int64_t)parabolic4f)>>(FIXED_BITS*3));
      LOG(LOG_DEBUG,"%ld, %d\r\n",ind,AC1f[n]);
      ind++;
      
    }
    
    //calculate residual dc after parabolic removal
    int32_t res_sum = 0;
    for (uint16_t n=0; n<ARRAY_LENGTH; n++) {
      res_sum += AC1f[n]; 
    }
    res_sum /= ARRAY_LENGTH;
    
    //remove residual dc after parabolic removal
    //LOG(LOG_DEBUG,"finished AC1f\r\n");
    ind = -(int16_t)((ARRAY_LENGTH-1)>>1);
    for (uint16_t n=0; n<ARRAY_LENGTH; n++) {
      AC1f[n] -= res_sum;
      //LOG(LOG_DEBUG,"%ld, %d\r\n",ind,AC1f[n]);
      //ind++;
    }
    
    //calculate variance
    for (uint16_t n=MAX_FILTER_LENGTH; n<SAMPLE_LENGTH+MAX_FILTER_LENGTH; n++) {
      var1f += ( (uint32_t)abs(AC1f[n]>>2 ))*( (uint32_t)abs(AC1f[n]>>2)); //getting 2 bit shifts here, taking out extra bit shift for overhead
      LOG(LOG_DEBUG,"var1f = %lu\r\n",var1f);
    }
    
    if(channel == IR) { //print filtered data for IR channel
      for (int n = 0; n<ARRAY_LENGTH; n++) {
        LOG(LOG_DEBUG_NEED,"%d\r\n",AC1f[n]);
      }
    }
    
    //        LOG(LOG_INFO,"var1f = %lu\r\n",var1f);
    rms1f[channel] = uint_sqrt(var1f/(uint32_t)SAMPLE_LENGTH ); //square root halfs the bit shifts back to 2, so this is more like RMS0.5f -- OH WELL (it is 4x bigger, not 16x bigger)
//    LOG(LOG_INFO,"channel %d, mean1f = %lu, rms1f = %lu\r\n",channel,mean1f[channel],rms1f[channel]);
  }//end channel loop
}


void SPO2::calc_R() {
  /*an error of 0.01 in R is an error of about 0.25% in SpO2. So we want to keep
  at least 8 bits of precision in R
  for a huge RMS value like 2056 for IR and 1024 for R.
  IR: (11 bits+4bits fp=15bits) we  shift 17 bits to 32 bits
  R: (10 bits+4bits fp=14bits) we shift 17 bits to 31 bits
  then we divide by the mean ~2e5, (18bits) which leaves 14 bits for IR and 13 for red.
  We can then shift the red that by 12 bits before we divide by the denominator all 13 bits.
  Now consider a minimum rms of 32 (5 bits) for R and 64 bits for IR
  IR: 6bit+4bits FP=10bits. We shift by 17 bits to 27 bits
  R: 5bit+4bits FP=9bits. We shift by 17 bits to 26 bits
  then we divide by 18 bits which leaves 8 bits. This is bare bones
  a 3 fixed point (nibble) shifts*/
  
  
  R = ((float)rms1f[RED]/(float)mean1f[RED])  / ((float)rms1f[IR]/(float)mean1f[IR]); //3ms
  LOG(LOG_INFO,"R=%f\r\n",R);
}


void SPO2::calc_spo2() {
  float spo2 = 0;
  //trying to be very efficient with floating point multiplication here
  float Rs[5];
  //float c[6] = {-26639, 80950, -97713, 58474,  -17367, 2147};
  float c[6] = {0,0,0,-28.149, -10.293, 107.36};
  float Rsq = R*R;
  float R4th = Rsq*Rsq;
  Rs[0] = R4th*R; //R^5
  Rs[1] = R4th; //R^4
  Rs[2] = Rsq*R; //R^3
  Rs[3] = Rsq; //R^2
  Rs[4] = R; //R
  spo2 = c[5];//constant term
  for(uint8_t n=0; n<5; n++) {
    spo2 += c[n]*Rs[n];
    LOG(LOG_DEBUG,"n = %d, c[n] = %f, Rs[n] = %f, term = %f\r\n, current_spo2 = %f",n,c[n],Rs[n],Rs[n]*c[n], spo2);
  }
  
  LOG(LOG_INFO,"spo2 = %.2f\r\n",spo2);
  current_spo21f = (int16_t)(spo2*(1<<FIXED_BITS));
  current_spo21f = ( (current_spo21f>>FIXED_BITS) > 100) ? 100<<FIXED_BITS : current_spo21f;
  current_spo21f = ( (current_spo21f>>FIXED_BITS) < 80) ? 80<<FIXED_BITS : current_spo21f;
}


void SPO2::calc_hr(){
/*Calculates heart rate from a beat periodicity (samples per period aka "offset" in fixed precision*/
  if(final_offset1f == 0) {
    current_hr1f = 0;
  } else {
    current_hr1f = ((uint32_t)((uint32_t)SAMPLE_RATE_MIN<<FIXED_BITS)<<FIXED_BITS) / final_offset1f;
  }
  LOG(LOG_INFO,"HR = %.3f, %lu\r\n",(float)current_hr1f/(float)(1<<FIXED_BITS), current_hr1f);
}


void SPO2::add_sample(uint32_t ir_data, uint32_t r_data) {
/*Called by main to load new samples into the algorithm's buffer. Provides additional
  filtering with a moving average filter. This is not equivalent to increasing 
  averaging on the OB1203 because that reduces data rate. This does not. We need
  that resolution for accuracy at high heart rates.*/
  const uint8_t num2avg=8; //changing this from 8 to 4 based on simulation results from raw data in Matlab
  static uint16_t num_samples = 0;
  static uint8_t buffer_index = 0;
  static uint32_t avg_buffer[2][num2avg];
  static uint32_t running_sum[2] = {0,0};
  if (num_samples == num2avg) {
    running_sum[IR] -= avg_buffer[IR][buffer_index];
    running_sum[RED] -= avg_buffer[RED][buffer_index];
  } else {
    num_samples++;
  }
  running_sum[IR] += ir_data;
  running_sum[RED] += r_data;
  avg_buffer[IR][buffer_index] = ir_data;
  avg_buffer[RED][buffer_index] = r_data;
  dc_data[IR][data_ptr] = running_sum[IR]/num_samples;
  dc_data[RED][data_ptr] = running_sum[RED]/num_samples;
  
  LOG(LOG_DEBUG_RAW,"%lu, %lu\r\n",dc_data[IR][data_ptr],dc_data[RED][data_ptr]);
  
  buffer_index++;
  buffer_index = (buffer_index==num2avg) ? 0 : buffer_index;
  prev_data_ptr = data_ptr; //keeping track of this in case I want to print the data
  data_ptr++;
  data_ptr = (data_ptr>=ARRAY_LENGTH)? 0 : data_ptr; //index roll over
  
}


int32_t SPO2::corr(int16_t *x, uint16_t len, uint16_t offset){
/*this does a correlation of the data with time-offset data and returns the dot 
  product of the two arrays. It takes arrays of ACdata with fixed precision.
  I modified it to start after max filter length since the lag is linearly increasing
  in the region and it can cause some error in the HR estimate.*/
  int32_t result = 0;
  offset = (offset > SAMPLE_LENGTH) ? SAMPLE_LENGTH : offset; //conform inputs to limits
  len = (len > SAMPLE_LENGTH) ? SAMPLE_LENGTH : len;
  for (uint16_t n = MAX_FILTER_LENGTH; n < (len+MAX_FILTER_LENGTH); n++) {
    result += ((int32_t) x[n] >> 4) * ((int32_t) x[n + offset] >> 4); //cast as int32_t for the multiplication to avoid overflow,
    //remove 4 bits of precision to keep same precision, remove 4 more for overhead.
  }
  LOG(LOG_DEBUG,"offset = %u,corr = %ld\r\n",offset,result);
  return result;
}

void SPO2::fine_search(int16_t *x, uint16_t len, uint32_t start_offset, int32_t start_correl, uint32_t max_search_step) {
/*This program looks for the peak of the autocorrelation function by doing correlations and stepping 
  down then up in offset until it finds a peak. If it exits at the extrema of the range
  then this is considered a fail. That doesn't quite work well with the variable stepping
  at long hear rates, however the kalman can sometimes throw out a bad low heart rate*/
  extern uint16_t t_read(void);
  /*fine search for correlation peak using defined step size (index units).
  Finds peak and interpolates the maximum and saves the answer with fixed precision.
  */
//  LOG(LOG_INFO,"fine search at %lu\r\n",start_offset);
  uint32_t search_step = start_offset/30; //dynamically stepping to get better resolution at high heart rates
  search_step = (search_step > max_search_step) ? max_search_step : search_step;
  uint16_t offset = start_offset;
  int32_t c;
  int32_t high_side;
  int32_t low_side;
  int32_t lowest;
  final_correl = start_correl;     //initialize
  final_offset = start_offset;
  uint16_t elapsed_time = t_read() - p2_start_time;
  int32_t y[3];
  if ( elapsed_time >= 15) {
    final_correl = 0;
    final_offset = 0;
    final_offset1f = 0;
  } else {
    
    while(1) { //search toward lower offset, higher frequency
      offset -= search_step;
      if (offset>=min_offset) {
        c = corr(x,len,offset);
//        LOG(LOG_INFO,"%u, %ld \r\n",offset, c>>11);
        if (c<=final_correl) {
          break; //Getting worse. Stop.
        } else {
          high_side = final_correl; //load prev sample as new larger offset sample for fit
          final_correl = c; //better, use new result; keep going
          final_offset = offset;
        }
      } else {
        c = start_correl;
        break;
      }
      
    }
    low_side = c;  //the low-side sample is the one we exited the loop on
    
    if (final_correl == start_correl) { //didn't find something bigger at lower offsets, check higher
      offset = start_offset;
      while(1) {
        offset += search_step;
        if (offset<max_offset) {
          c = corr(x,len,offset); //search lower frequency
//          LOG(LOG_INFO,"%u, %ld \r\n",offset, c>>11);
          if (c <= final_correl) {
            break; //getting worse; stop
          } else {
            low_side = final_correl;
            final_correl = c; //better, use new results; keep going
            final_offset = offset;
          }
        } else {
          break; //out of range; stop
        }
      }
      high_side=c; //the high side sample is the one we exited the loop on.
    }
    if(final_offset <= min_offset) {
      final_offset1f = min_offset<<FIXED_BITS; //force the algorithm to bonk at max found check.
    } else if(final_offset >=max_offset) {
      final_offset1f = max_offset<<FIXED_BITS; //force the algorithm to bonk at max found check.
    } else { //only run this if final_offset is not one of the boundaries
      lowest = (high_side < low_side) ? high_side : low_side;
      if(final_correl-lowest == 0) {
        final_offset1f = final_offset<<FIXED_BITS;
      } else {
        LOG(LOG_DEBUG,"dtot = %ld, d_HL = %ld\r\n", final_correl-lowest,high_side-low_side);
        //                final_offset1f = (final_offset<<FIXED_BITS) + ( ( (((int32_t)search_step)<<FIXED_BITS) /2)*(high_side - low_side) ) / (final_correl-lowest); //interpolate a better answer
        y[0] = low_side;
        y[1] = final_correl;
        y[2] = high_side;
        simple_peak_find(y, &final_offset1f, &fit_correl, (uint16_t)final_offset, (int32_t)search_step);
      }
    }
  }
}


bool SPO2::check4max(int16_t *x, uint16_t len,uint16_t start_offset, int32_t start_correl) {
  //check for a max in the correlation in a region
  bool max_found = 0;
  fine_search(x,len,start_offset,start_correl,MAX_FINE_STEP);
  if ( final_offset1f != (min_offset<<FIXED_BITS)) {
    if(final_offset1f != (max_offset<<FIXED_BITS)) {
      max_found = 1;
//      LOG(LOG_INFO,"start = %u, success at %.2f, correl = %lu\r\n", start_offset, (float)final_offset1f/(float)(1<<FIXED_BITS),fit_correl>>11);
    }
  } else {
    LOG(LOG_INFO,"start = %u, fail at %.2f correl = %lu\r\n", start_offset, (float)final_offset1f/(float)(1<<FIXED_BITS), final_correl>>11);
  }
  return max_found;
}


bool SPO2::find_max_corr(int16_t *x, uint16_t max_length, uint16_t offset_guess) {
/*This is the brains of the heart rate detection. It looks for the minimum of the
  autocorrelation using large steps, interpolates the minimum, doubles it and does a fine
  search near the peak. This is usually faster than an FFT, but not if it has to run
  a lot of values. Anyway, it is quite robust.*/
  extern uint16_t t_read(void);
  
  bool rising = 0;
  uint8_t fail = 0;
  uint16_t try_offset =MIN_OFFSET-BIG_STEP ;
  uint16_t offsets[3];
  offsets[0] = try_offset;
  int32_t offset_extremum1f;
  int32_t c[3];
  //    int32_t d2;
  int32_t prev_correl;
  int32_t min_correl;
  
  
//  LOG(LOG_INFO,"start find max correl\r\n");
//  LOG(LOG_DEBUG,"len = %d, offset_guess = %d\r\n",max_length, offset_guess);
  int32_t start_correl;
  bool max_found;
  
  if(prev_valid) {
    prev_offset = final_offset;
    prev_correl = final_correl;
  }
  
  uint16_t samples2use = max_length;
  
  //determine how fast autocorrelation is dropping
  //get_corr_slope(x,max_length,0,OFFSET_FOR_SLOPE_CALC);
  
  LOG(LOG_DEBUG,"max_offset after get_corr_slope = %d\r\n",max_offset);
  
  c[0] = corr(x,samples2use,try_offset);
  try_offset +=BIG_STEP;
  offsets[1] = try_offset;
  c[1] = corr(x,samples2use,try_offset);
  rising = (c[1] > c[0]) ? 1 : 0; //skip to search for a peak if rising, else look for a minimum and double it.
//  LOG(LOG_INFO,"%d, %ld\r\n%d, %ld\r\n", MIN_OFFSET-BIG_STEP,c[0],MIN_OFFSET,c[1]);
  uint16_t step = BIG_STEP; //start with big step
  const uint8_t max_step = 18;
  if(!rising) {
//    LOG(LOG_INFO,"searching for min\r\n");
    while(!rising) { //keep going until you find a minimum  
      step += (step < max_step) ? 1 : 0; //increment step size if less than max step
      uint16_t elapsed_time = t_read() - p2_start_time;
      
      try_offset += step; //increment by step size            
      if(try_offset> MAX_OFFSET || (elapsed_time >= 15)) {
        fail = 1; //still falling and ran out of samples
        break;
      }
      c[2] = corr(x,samples2use,try_offset);
      if (c[2]>c[1]) {
        rising  = 1;
      } else {
        c[0] = c[1];   
        c[1] = c[2];
        offsets[0] = offsets[1];
        offsets[1] = try_offset;
      }
//      LOG(LOG_INFO,"%u %ld\r\n",try_offset,c[2]);
    }
    if(!fail) {
      //            highest = (c1 > c3) ? c1 : c3;
      
      //            if(c2-highest == 0) {
      //                offset_guess  = try_offset-BIG_STEP;
      //            } else {
      //                d2 =  ( ( ( (int32_t)BIG_STEP<<FIXED_BITS)*(c1 - c3) ) / (highest-c2) )>>FIXED_BITS; //twice the delta
      //                LOG(LOG_INFO,"d2 = %ld\r\n",d2);
      //                offset_guess = (uint16_t)( ( ( (int16_t)try_offset-(int16_t)BIG_STEP)<<1) + (int16_t)d2) ; //interpolate a better answer
      offsets[2] = try_offset; 
//      LOG(LOG_INFO,"min fit at: %u %u %u \r\n",offsets[0],offsets[1],offsets[2]);
//      LOG(LOG_INFO,"for values: %ld %ld %ld \r\n",c[0],c[1],c[2]);
      peak_find(offsets,c,&offset_extremum1f,&min_correl); 
      offset_guess = (uint16_t)(offset_extremum1f>>(FIXED_BITS-1));
//      LOG(LOG_INFO,"guessing double the min at %d\r\n",offset_guess);
      //            }
    }
  } else {//already rising
//    LOG(LOG_INFO,"searching for max\r\n");
    while(rising) {//keep going until you find a drop
      uint16_t elapsed_time = t_read() - p2_start_time;
      
      try_offset += step;
      step += (step < max_step) ? 1 : 0; //increment step size if less than max step
      if((try_offset>MAX_OFFSET) ||((elapsed_time >= 15))) {
        fail = 2; //still rising and ran out of samples
        break;
      } else {
        c[2] = corr(x,samples2use,try_offset);
        if (c[2]<c[1]) {
          rising  = 0;
        } else {
          c[0]= c[1];  
          c[1]= c[2];
          offsets[0] = offsets[1];
          offsets[1] = try_offset;
        }
//        LOG(LOG_INFO,"%u %ld\r\n",try_offset,c[2]);
      }
    }
    if(!fail) {
      //            lowest = (c3 < c1) ? c3 : c1;
      //            if(c2-lowest == 0) {
      //                offset_guess  = try_offset-BIG_STEP;
      //            } else {
      //                d2 = ( ( ( (int32_t)BIG_STEP<<FIXED_BITS)*(c3 - c1) ) / (c2-lowest) )>>(FIXED_BITS+1);
      //                LOG(LOG_INFO,"d2 = %ld\r\n",d2);
      //                offset_guess = (uint16_t)( ( (int16_t)try_offset-(int16_t)BIG_STEP) + (int16_t)d2) ; //interpolate a better answer
      
      //            }
      offsets[2] = try_offset; 
//      LOG(LOG_INFO,"max fit at: %u %u %u \r\n",offsets[0],offsets[1],offsets[2]);
//      LOG(LOG_INFO,"for values: %ld %ld %ld \r\n",c[0],c[1],c[2]);
      peak_find(offsets,c,&offset_extremum1f,&min_correl); 
      offset_guess = (uint16_t)(offset_extremum1f>>FIXED_BITS);
//      LOG(LOG_INFO,"guessing near the max at %d\r\n",offset_guess);
    } else {
      offset_guess = DEFAULT_GUESS;
    }
  }
  //condition inputs
  if(offset_guess < MIN_OFFSET) offset_guess = DEFAULT_GUESS;
  if(offset_guess > MAX_OFFSET) offset_guess = DEFAULT_GUESS;
  
  start_correl = corr(x,samples2use,offset_guess);
//  LOG(LOG_INFO,"%u, %lu\r\n",offset_guess,start_correl>>11);
  max_found = check4max(x, samples2use, offset_guess, start_correl);
  if (prev_valid & !max_found) { //go ahead and try the previous value if the other bonked.
    //note you can't just rely on this or you could lock into a harmonic.
//    LOG(LOG_INFO,"trying prev sol'n\r\n");
    max_found = check4max(x, samples2use, prev_offset, prev_correl);
  }
  
  if(!max_found) {
    final_correl = 0;
    final_offset1f = 0;
    prev_valid = 0;
  } else {
    prev_valid = 1;
  }
  LOG(LOG_INFO,"final_offset %lu\r\n",final_offset);
  return max_found;
}


void SPO2::peak_find(uint16_t *x0, int32_t *y, int32_t *x_fit1f, int32_t *y_fit) {
/*This is a peak (or valley) finder that takes arbitrary x and y data. The y 
  data here is not kept in fixed precision so it assumes it is dealing with large
  numbers like the correlation dot products. It is more general than the simple finder
  which assumes equal spacing in the x data (offset steps)
  Solve this linear equation
  a*x[0]^2 + b*x[0] + c = y[0]
  a*x[1]^2 + b*x[1] + c = y[1]
  a*x[2]^2 + b*x[2] + c = y[2]
  
  using x2 = {x[0]^2, x[1]^2, x[2]^2}
  subtract x[1] from x's to get centered values, y[1] = C, reduces to system of 2 equations in for constants y_i-C
  */
  
  int32_t x[2] = {(int32_t)x0[0]-(int32_t)x0[1], (int32_t)x0[2]-(int32_t)x0[1]}; //get centered x data
  int32_t x2[2] = {x[0]*x[0], x[1]*x[1]}; //get squared x data
  int32_t C = y[1]; //second equation in system is trivial
  int32_t yp[2] = {y[0]-C,y[2]-C}; //get centered y values
  int32_t D;
  int32_t Dx;
  int32_t Dy;
  int32_t A;
  int32_t B;
  //using Cramer's rule
  D =  x2[0]* x[1] - x[0]*x2[1]; //get determinant
  Dx = yp[0]*x[1] - x[0]*yp[1]; //get det with constants (y) replacing col 1
  Dy = x2[0]*yp[1] - yp[0]*x2[1]; //get det with constants (y) replacing col 2
  LOG(LOG_DEBUG,"D = %ld, Dx = %ld, Dy = %ld\r\n",D,Dx,Dy);
  A = Dx/D; //not using fixed precision because y is so large
  B = Dy/D;
  
  *x_fit1f = -(B<<FIXED_BITS)/(2*A) + (x0[1]<<FIXED_BITS); //peak is where slope = 0, namely dy/dx = 2ax+b = 0, so x_peak = -b/2a and add back in offset x0[1]
  *y_fit  = C - (((B/4)*B)/A); //plug x = -b/2a into y = ax^2 + bx +c
  //if a > 0 it is a peak
  //if a < 0 it is a valley
  LOG(LOG_DEBUG,"A = %ld, B = %ld, C = %ld, b^2/a = %ld\r\n",A,B,C, B*B/4/A);
}


void SPO2::simple_peak_find(int32_t *y, int32_t *x_fit1f, int32_t *y_fit, uint16_t x_center, int32_t step){
/*This is a fast peak finder for equally spaced data in x. Y is not calculated with
  additional fixed precision so the estimated peak value is accurate for large values of y only.
  For numerical stability let x[1] = 0, namely the middle point (high or low) is zero
  To simplify the math we measure in units of the step size, so xfit1f is fixed precision in units of the step size
  
  Solve this linear equation
  a*x[0]^2 + b*x[0] + c = y[0]
  a*x[1]^2 + b*x[1] + c = y[1]
  a*x[2]^2 + b*x[2] + c = y[2]
  
  using x =[-1,0,1}, and x2 = {x[0]^2, x[1]^2, x[2]^2} = {1,0,1]
  //solve for a, b, and c using x2, x and c as constants
  */
  int32_t A; //a
  int32_t B; //b
  int32_t C; //c
  //using Cramer's rule
  C = y[1]; //middle equation in system is trivial. Problem reduces to 2x2 determinant.
  /*
  a*x[0]^2 + b*x[0] = y[0]-c
  a*x[2]^2 + b*x[2] = y[2]-c
  */
  //let yp[0] = y[0]-C and yp[2] = y[2]-C;
  //D = 2; //determinant of {{1, -1},{1, 1}}
  //Da = yp[0]*1 - (-1)*yp[1] = yp[0] + yp[1] = y[0]-C + y[2]-C = y[0] + y[2] -2*C;
  //Dy = 1*yp[2] - yp[0]*1 = yp[2] - yp[1] = y[2] -C - (y[0] -C) = y[2] - y[0];
  A = (y[0]+y[2]-(C*2) )/2; // Dx/D
  B = (y[2] - y[0])/2; // Dy/D
  *x_fit1f =  (-((B<<FIXED_BITS)/A)*(int32_t)step)/2 + (x_center<<FIXED_BITS)  ; // -b/2a * step + x_center
  *y_fit = C - (((B>>2)*B)/A); //C - b^2/4a
  LOG(LOG_DEBUG,"A = %ld, B = %ld, C = %ld, b^2/4a = %ld\r\n",A,B, C, B*B/4/A);
}

void SPO2::ext_prec_peak_find(int32_t *y, int32_t *x_fit1f, int32_t *y_fit, uint16_t x_center, int32_t step){
/*This is a fast peak finder for equally spaced data in x. Y is not calculated with
  additional fixed precision so the estimated peak value is accurate for large values of y only.
  For numerical stability let x[1] = 0, namely the middle point (high or low) is zero
  To simplify the math we measure in units of the step size, so xfit1f is fixed precision in units of the step size
  
  Solve this linear equation
  a*x[0]^2 + b*x[0] + c = y[0]
  a*x[1]^2 + b*x[1] + c = y[1]
  a*x[2]^2 + b*x[2] + c = y[2]
  
  using x =[-1,0,1}, and x2 = {x[0]^2, x[1]^2, x[2]^2} = {1,0,1]
  //solve for a, b, and c using x2, x and c as constants
  */
  int32_t A; //a
  int32_t B; //b
  int32_t B1f; //b with addition bits of precisions
  int32_t C; //c
  //using Cramer's rule
  C = y[1]; //middle equation in system is trivial. Problem reduces to 2x2 determinant.
  /*
  a*x[0]^2 + b*x[0] = y[0]-c
  a*x[2]^2 + b*x[2] = y[2]-c
  */
  //let yp[0] = y[0]-C and yp[2] = y[2]-C;
  //D = 2; //determinant of {{1, -1},{1, 1}}
  //Da = yp[0]*1 - (-1)*yp[1] = yp[0] + yp[1] = y[0]-C + y[2]-C = y[0] + y[2] -2*C;
  //Dy = 1*yp[2] - yp[0]*1 = yp[2] - yp[1] = y[2] -C - (y[0] -C) = y[2] - y[0];
  A = (y[0]+y[2]-(C*2) )/2; // Dx/D
  B = (y[2] - y[0])/2; // Dy/D
  B1f = ((y[2]*(int32_t)(1<<FIXED_BITS)) - (y[0]*(int32_t)(1<<FIXED_BITS)))/2; // Dy/D
  *x_fit1f =  (-(B1f/A)*(int32_t)step)/2 + (x_center<<FIXED_BITS)  ; // -b/2a * step + x_center
  *y_fit = C - (((B>>2)*B)/A); //C - b^2/4a
  LOG(LOG_DEBUG,"A = %ld, B = %ld, B1f = %ld, C = %ld, b^2/4a = %ld\r\n",A,B, B1f, C, B*B/4/A);
}

void SPO2::avgNsamples(int16_t *x, uint8_t number2avg) {
  /*Runs a moving average filter over the data in the array.
  The filter length ramps up to the desired length so the first few samples
  don't have the same lag as latter ones. For this reason the rms calc starts after 
  the max sample length*/
  int32_t avg_buffer[MAX_FILTER_LENGTH];
  uint16_t buffer_ind =0;
  uint8_t avg_length = 0;
  int32_t running_sum = 0;
  for (uint16_t n = 0; n<ARRAY_LENGTH; n++) {
    if(avg_length<number2avg) {
      avg_length++;
    } else {
      running_sum -= avg_buffer[buffer_ind];
    }
    avg_buffer[buffer_ind] = (int32_t)x[n];
    running_sum += avg_buffer[buffer_ind];
    x[n] = (int16_t)(running_sum/avg_length);
    buffer_ind++;
    if(buffer_ind==number2avg) buffer_ind = 0;
  }
}


//void SPO2::set_filters(KALMAN* filter1, KALMAN* filter2, KALMAN* filter3, KALMAN* filter4, KALMAN* filter5, KALMAN* filter6 ) {
void SPO2::set_filters(KALMAN* filter1, KALMAN* filter2, KALMAN* filter3, KALMAN* filter4) {
  corr_filter = filter1;
  hr_filter = filter2;
  spo2_filter = filter3;
  //rr_filter = filter4;
  rr_filter = filter4;
  //short_breath_filter = filter5;
  //breath_filter = filter6;
}


void SPO2::fastAvg2Nsamples(int16_t *x) { 
  /*runs filters for multipes of 2 defined by FILTER_BITS 
  Ramps up the filter length for teh first few samples, then runs fast.*/
  uint16_t buffer_ind =0;
  int32_t avg_buffer[1<<FILTER_BITS];
  uint8_t avg_length = 0;
  int32_t running_sum = 0;
  uint16_t filter_length = 1<<FILTER_BITS;
  uint16_t buffer_mask = filter_length -1;
  for (uint16_t n = 0; n<filter_length; n++) {//do slow method until the buffer is full
    avg_buffer[buffer_ind] = (int32_t)x[n]; //load new sample in buffer
    running_sum += avg_buffer[buffer_ind]; //update running sum
    x[n] = (int16_t)(running_sum/avg_length); //load filtered sample
    buffer_ind++; //increment buffer index
  }
  for (uint16_t n=filter_length; n<ARRAY_LENGTH; n++) {//do fast method once buffer is full
    running_sum -= avg_buffer[buffer_ind]; //remove old sample
    avg_buffer[buffer_ind] = (int32_t)x[n]; //load new sample in buffer
    running_sum += avg_buffer[buffer_ind]; //update running sum
    x[n] = (int16_t)(running_sum>>FILTER_BITS);
    buffer_ind++; //increment buffer index
    buffer_ind &= buffer_mask; //loop index
  }
}


int SPO2::get_direction(uint32_t data1, uint32_t data2) {
  int dir;
  if (data2 > data1) {
    dir = 1;
  } else if (data2 < data1) {
    dir = -1;
  } else {
    dir = 0;
  }
  return dir;
}


void SPO2::findminmax(int32_t* data, uint16_t start_ind, uint16_t stop_ind, int32_t* c, int* extreme, int* type){
  uint16_t ind = start_ind;
  c[1] = data[ind];
  ind = ind+1;
  c[2] = data[ind];
  bool change_dir = 0;
  int dir1;
  int dir2;
  dir1 = get_direction(c[1],c[2]); //-1 is falling, 1 is rising
  ind = ind+1;
  *extreme = -1; //default = fail condition
  type = 0;
  while (!change_dir && (ind<=stop_ind)) {
    c[0] = c[1];
    c[1] = c[2];
    c[2] = data[ind];
    dir2 = get_direction(c[1],c[2]);
    if (dir1*dir2 < 0) {
      change_dir = 1;
      *type = dir1; //1 = peak, -1 = valley
      break;
    }
    ind = ind+1;
  }
  *extreme = ind-1; //this is the last point we verified is or is not an extremum. Start next search at this point.
}


int32_t SPO2::get_peak_height (int32_t *peak_xdata, int32_t *peak_ydata) {//********UNFINISHED***********//
  //x and y are 3 points arrays with the extreme at y[1]
  int32_t dx1;
  int32_t dx;
  int32_t y0;
  int32_t dx2;
  
  dx1 = peak_xdata[1]-peak_xdata[0];
  dx2 = peak_xdata[2]-peak_xdata[1];
  dx = dx1 + dx2;
  //get estimated baseline at peak position
  y0 = peak_ydata[0] + dx1/dx * (peak_ydata[2] - peak_ydata[0]); //add an amount proportional to the difference between point 1 and 3
  //difference between peak and valley
  return y0;
} //end get_peak_height

//BEAT TO BEAT PORTING IN PROGRESS
//void SPO2::peak2peak(int32_t* ac_data) {//********UNFINISHED***********//
//  static int peaks_found = 0;
//  static int valleys_found = 0;
//  int array_end = SAMPLE_RATE + 2;
//  int dir = 0;
//  int32_t c[3];
//  int type = 0;
//  static int32_t last_peak_x1f = 0;
//  static int32_t last_peak_y = 0;
//  static int32_t last_valley_x1f = 0;
//  static int32_t last_valley_y = 0;
//  static int last_found_type = 0;
//  int32_t y[3];
//  int32_t x1f; //latest peak position
//  int32_t yfit;
//  int32_t peak_amp;
//  uint16_t rr_time;
//  int32_t step = 1;
//  int extreme;
//  int array_ind = 0;
//  array_end = SAMPLE_RATE+1; //extreme on exit (no peak) will be at SAMPLE_RATE (100) and next search starts here. We search 101 samples and overlap 1 sample.
//  while (1) {
//    //look for extrema
//    findminmax(ac_data, array_ind, array_end, c, &extreme, &type);
//    array_ind = extreme; //update start position for next time;
//    if (type !=0) { //peak found
//      if (type == 1) peaks_found++;
//      if (type == -1) valleys_found++;
//      y[0] = ac_data[extreme-1];
//      y[1] = ac_data[extreme];
//      y[2] = ac_data[extreme+1];
//      simple_peak_find(y, &x1f, &yfit, extreme, step);
//      if ( (peaks_found > 1) && (type ==1 ) ) {//enough peaks to calc R-R time
//        //THEN USE GET_PEAK_HEIGHT TO GET PEAK AMPLITUDE AND SEE IF IT WORKS BETTER THAN RMS FOR 
//        rr_time1f = x1f+alg_start_sample_count<<FIXED_BITS - last_peak_x1f;
//        //ALSO FIGURE OUT HOW TO REPORT THIS--PERHAPS AN ISR?
//        int32_t xdata[3] = {last_valley_x1f,last_peak_x1f,x1f};
//        int32_t ydata[3] = {last_valley_y,last_peak_y,yfit};
//        peak_amp = get_peak_height(xdata,ydata);
//      }
//      if (type == 1) {
//        last_peak_x1f = x1f+alg_start_sample_count<<FIXED_BITS;
//        last_peak_y = yfit;
//      }
//      if (type == -1) {
//        last_valley_x1f = x1f + alg_start_sample_count<<FIXED_BITS;
//        last_valley_y = yfit;
//      }
//    } else { //ran out of points
//      break; //exit loop
//    }
//  }
//} //end peak2peak


void SPO2::unwrap_buffer_to_int(uint32_t *circular_buffer, int32_t *unwrapped_buffer, int length, int buffer_ind) {
  //unwrap buffer
  int ind;
  for (int n=0;n<length;n++) { //newest sample first, oldest sample last
    ind = buffer_ind-n;
    if (ind<0) ind += length;
    unwrapped_buffer[n] = (int32_t)circular_buffer[ind];
  } 
}

void SPO2::subtract_mean(int32_t* array,int length) {
  //subtract the mean
  int32_t mean=0;  
  for (int n =0;n<length;n++) {
    mean += array[n];
  }
  mean /= length;
  for (int n=0;n<length;n++) {
    array[n] -= mean;
  }
}

void SPO2::get_autocorrelation(int32_t * array, int32_t * autocorrelation,int array_length, int max_offset) { 
  //Note if we really wanted to save processing we could update only the change in the autocorrelation with each new data point #lazy I do it all every time because it is fast and stable 
  //If we don't find a peak, but we do find a minimum, the option "use_min" will allow the algorithm
  //to peak_find (trough find) the minimum and double that to estimate the period.
  if (max_offset<0) max_offset *= -1;
  int64_t product;
  for (int offset = 0;offset<max_offset;offset++) {//sweep offset
    autocorrelation[offset] = 0; //initialize
    for (int n=0;n<max_offset;n++) { //dot product of offset vectors
      if (n+offset < array_length) {//only do it if the array is long enough
        product =  (int64_t)array[n]*(int64_t)array[n+offset];
        autocorrelation[offset] += (int32_t)(product/((int64_t)(1<<FIXED_BITS))); //try to limit the size of the autocorrelation
      }
    }
  }
}

uint8_t SPO2::get_period_from_array(int32_t* array, int max_offset, int32_t* period1f, bool use_min) {
  int n=1; //starting at 1 instead of zero because occasionally a small offset is greater than 0, especially if data is sloping.
  int m;
  uint8_t err = 0;
  *period1f = max_offset<<FIXED_BITS;
  int32_t ydat[3];
  int32_t xfit1f;
  int32_t yfit;
  
  
  while ( 1 ) {//look for a minimum starting at offset 1 
    //n is array minimum
    if ( (n+1) <= max_offset) { //not reached end
      if  ( array[n+1] <= array[n] ){ //going down
        n=n+1;
      }
      else {//turned up
        break;
      }
    }
    else {//reached end
      break;
    }
  }
  if (n == max_offset) {//breathing rate descended continuously
    *period1f = max_offset<<FIXED_BITS; //didn't find minimum. Choose default minimum detectable breathing rate 
    err = 1; //no minimum found
    return err;
  }
  //m is array maximum
  m = n;
  while (1) { //start search for max
    if ( (m+1) <= max_offset ) {//not reached end
      if (array[m+1] >= array[m]) {//going up
        m = m+1;
      }
      else {//passed peak
        break;
      }
    }
    else {//reached end
      break;
    }
  }//exit loop
  
  //calculate period
  if (m == max_offset) { //error: breathing rate lower than detecable limit
    err = 2; //min found, but couldn't find max
    if (use_min) {
      //double the minimum to estimate the maximum
      if (m < 3/4*max_offset){ //make sure this minimum is occuring early enough in the dataset to be relevant
        ydat[0] = array[n-1];
        ydat[1] = array[n];
        ydat[2] = array[n+1];
        simple_peak_find(ydat, &xfit1f, &yfit, (uint16_t)m, 1); //xfit1f is fixed precision period     
        *period1f = xfit1f*2; //double the min to guess the peak
      }
      else {
        *period1f = max_offset<<FIXED_BITS;
      }
    }
    else {
      *period1f = max_offset<<FIXED_BITS;
    }
    
  }
  else {//not max--peak found
    if (m <= 2){ //error too fast
      if (m>1) {//go ahead and try to fit the peak
        ydat[0] = array[m-1];
        ydat[1] = array[m];
        ydat[2] = array[m+1];
        simple_peak_find(ydat, &xfit1f, &yfit, (uint16_t)m, 1); //xfit1f is fixed precision period     
        *period1f = xfit1f;
      }
      else {//maxed out
        *period1f = 3<<FIXED_BITS; 
        err = 3;
      }
    }
    else {
      err = 0; //found a peak
      ydat[0] = array[m-1];
      ydat[1] = array[m];
      ydat[2] = array[m+1];
      simple_peak_find(ydat, &xfit1f, &yfit, (uint16_t)m, 1); //xfit1f is fixed precision period     
//      LOG(LOG_INFO,"m = %u, period for length %u is %ld\r\n",m, max_offset,xfit1f);
      *period1f = xfit1f;
    }
  } //end found max case
  return err;
}

void SPO2::do_algorithm_part3() {
  /*
  This function does the breathing rate detection with autocorrelation of the sinus arrythmia.
  */
  alg_count++; //increment the algorithm run count. Like a 1 second tick. Keeps track of peak and valley positions.
  int32_t lin_buffer[BREATH_ARRAY_LENGTH];
  int32_t autocorrelation[BREATH_ARRAY_LENGTH];
  int32_t short_period1f;
  int32_t long_period1f;
  int32_t current_br_period1f;
  uint8_t short_err = 0;
  uint8_t long_err = 0;
  bool choice;
  //this part is going to use the correlation-based heart rate while beat-to-beat is in development
  //put in some protection from bad data--like skip it or something.
  
  hr_data_buffer[hr_data_buffer_ind] = corr_filter->kalman_avg;
  
  if(alg_count>SHORT_BREATH_ARRAY_LENGTH) {
    
    unwrap_buffer_to_int(hr_data_buffer,lin_buffer,SHORT_BREATH_ARRAY_LENGTH,hr_data_buffer_ind);
    subtract_mean(lin_buffer,SHORT_BREATH_ARRAY_LENGTH);
    //Consider removing slope here also. This would help with breath detection when in recovery from exercise.
    get_autocorrelation(lin_buffer,autocorrelation,SHORT_BREATH_ARRAY_LENGTH,SHORT_MAX_BREATH_OFFSET);
    LOG(LOG_INFO,"autocorrelation:\r\n");
//    for (int n=0;n<SHORT_BREATH_ARRAY_LENGTH;n=n+4) {
//      LOG(LOG_INFO,"%ld %ld %ld %ld\r\n",autocorrelation[n*4],autocorrelation[n*4+1],autocorrelation[n*4+2],autocorrelation[n*4+3]);
//    }
    short_err =get_period_from_array(autocorrelation, SHORT_MAX_BREATH_OFFSET, &short_period1f, USE_MIN);
  }
  else {
    LOG(LOG_INFO,"Collecting breath samples...\r\n");
  }
  
  if(alg_count>BREATH_ARRAY_LENGTH) {
    
    unwrap_buffer_to_int(hr_data_buffer,lin_buffer,BREATH_ARRAY_LENGTH,hr_data_buffer_ind);
    subtract_mean(lin_buffer,BREATH_ARRAY_LENGTH);
    //Consider removing slope here also. This would help with breath detection when in recovery from exercise.
    get_autocorrelation(lin_buffer,autocorrelation,BREATH_ARRAY_LENGTH,MAX_BREATH_OFFSET);
    long_err = get_period_from_array(autocorrelation, MAX_BREATH_OFFSET, &long_period1f, USE_MIN);
  }
  
  //filter data and decide which data to use
  //so long as we have data, run the Kalman filter

  
  //if(short_period1f > 0) short_breath_filter->run_kalman((uint32_t)short_period1f);
  //if(long_period1f > 0) breath_filter->run_kalman((uint32_t)long_period1f);
  
  //if short has peak and long doesn't, use short
  if ( (short_err == 0) && (long_err != 0) ) {
    choice = USE_SHORT;
  }
  //if short finds a peak and long does too, use short
  else if ( (short_err == 0) && (long_err == 0) ) {
    choice = USE_SHORT;
  }
  //if short doesn't find a peak and long does, use long
  else if ( (short_err !=0) && (long_err == 0) ) {
    choice = USE_LONG;
  }
  //if neither finds a peak, but long found a min, use double the min
  else if ( (short_err !=0) && (long_err == 2) ) {
    choice = USE_LONG;
  }
  //if long found no min, but short did, use 2x the min
  else if ( (long_err == 1) && (short_err == 2) ) {
    choice = USE_SHORT;
  }
  //if long and short found no min, use long
  else if ( (long_err == 1) && (short_err == 1) ) {
    choice = USE_LONG;
  }
  //if short and long both found a very fast peak , use short
  else if ( (short_err == 3) && (long_err == 3) ) {
    choice = USE_SHORT;
  }
  else {
    choice = USE_LONG;
  }
  
  current_br_period1f = (choice ? long_period1f : short_period1f);

  rr_filter->run_kalman(current_br_period1f);

  
  //below here isn't updated yet
  breathing_rate1f = (( ( ((uint32_t)60)<<FIXED_BITS)* (uint32_t)INTERVAL)/ (uint32_t)SAMPLE_RATE)<<FIXED_BITS / (uint32_t)rr_filter->kalman_avg;
  
  LOG(LOG_INFO,"avg br %0.2f, cur br %0.2f\r\n",(float)(((int32_t)60*(1<<FIXED_BITS))*(int32_t)SAMPLE_RATE/(int32_t)INTERVAL)/(float)rr_filter->kalman_avg,(float)(((int)60*(1<<FIXED_BITS))*(int32_t)SAMPLE_RATE/(int32_t)INTERVAL)/(float)(current_br_period1f) ) ;
  LOG(LOG_INFO,"choice = %u, s/l err = %u/ %u, s/l per = %ld / %ld\r\n", choice, short_err, long_err, short_period1f, long_period1f);
  
  //increment buffer index
  hr_data_buffer_ind++;
  if (hr_data_buffer_ind >= BREATH_ARRAY_LENGTH) { 
    hr_data_buffer_ind = 0;
  }
}