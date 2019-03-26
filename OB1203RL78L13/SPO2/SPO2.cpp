#if defined(DEBUG)
#include <stdio.h>
#endif
#include <math.h>
#include "SPO2.h"
#include "OB1203.h"
//#include "SoftI2C.h"


/*new plan is to use a fixed sample length of 150 (1.5 seconds)
keep a runing average of the DC level (only needs to be 150 long) and buffer samples with DC already subtracted
this subtracts the DC level no matter what it is doing: linear or quadratic or whatever.
The sample buffers are then int16's and data will be operated on in-place.
*/

//#define PRINT_RAW //print raw, filtered data
//#define PRINT_AC1F //print DC and slope removed

//int32_t sum_squares=2272550; //(for 150 sample array) sum of n^2 for -sample length to + sample length, with sample length = 150 and array length = 301 or 2*sum(1^2+ 2^2...150^2)

//int32_t sum_squares=676700;  //(for 100 sample array) sum of n^2 for -sample length to + sample length, with sample length = 100 and array length = 201 or 2*sum(1^2+ 2^2...100^2)

int32_t sum_squares;

//const int32_t  h[32] = {3,1,-3,-8,-8,2,16,22,11,-19,-48,-47,4,100,208,279,279,208,100,4,-47,-48,-19,11,22,16,2,-8,-8,-3,1,3}; //10 bits of fixed precision included
//const int32_t h[12] = {-12,-13,16,89,185,254,254,185,89,16,-13,-12};
//const int32_t h[18] = {21,74,158,242,280,241,137,18,-60,-74,-41,-2,17,14,3,-4,-3,1};
//const int32_t h[12] = {-11,-24,-6,72,194,290,290,194,72,-6,-24,-11};
//const uint8_t h_bit_prec = 10; //real filter coefficients are 2^bit_prec smaller
//const int32_t h[NUM_FILTER_TAPS] = {1,1,1,1,1,1,1,1}; //running average
//const uint8_t h_bit_prec = 3;

void SPO2::get_sum_squares()
{
    sum_squares = 0;
    for (int n=1; n<SAMPLE_LENGTH; n++) {
        sum_squares += n*n;
    }
    sum_squares = sum_squares<<1;
}


void SPO2::do_algorithm_part1()
{
    LOG(LOG_INFO, "sample_count %d\r\n",sample_count);
    if(sample_count>=ARRAY_LENGTH)
    {       
        LOG(LOG_INFO, "do alg part 1\r\n");
        get_rms(); //subtract residual slopes and calculate RMS
    }
}


void SPO2::do_algorithm_part2()
{
    extern uint16_t t_read(void);
    static uint32_t hr_samples[NUM_HR_AVGS];
    static uint8_t hr_ptr = 0;
    static uint8_t spo2_ptr =0;
    static uint32_t spo2_samples[NUM_SPO2_AVGS];
    static uint8_t num_hr_samples = 0;
    static uint8_t num_spo2_samples = 0;
    static uint8_t kalman_hr_ptr = 0;
    static uint8_t kalman_spo2_ptr = 0;
    static uint8_t kalman_hr_length = 0;
    static uint8_t kalman_spo2_length = 0;

    p2_start_time = t_read();
    offset_guess = DEFAULT_GUESS;
    LOG(LOG_DEBUG,"sample_count %d\r\n",sample_count);
    if(sample_count>=ARRAY_LENGTH) {
        LOG(LOG_INFO,"doing algorithm part 2\r\n");
        calc_R();
        calc_spo2();
        find_max_corr(AC1f,(uint16_t)SAMPLE_LENGTH, offset_guess); //AC1f is the IR sample after get_rms is run

        calc_hr();
        if(current_hr1f == 0) {//don't take valid spo2 samples if we can't find a heart rate
            current_spo21f = 0;
        }

        LOG(LOG_DEBUG,"pre kalman %.1f, %.1f\r\n",(float)current_spo21f/(float)(1<<FIXED_BITS), (float)current_hr1f/(float)(1<<FIXED_BITS));
//        consensus(); //filter out crap data
        LOG(LOG_INFO,"HR: ");
        kalman(kalman_hr_array,&kalman_hr_length,&kalman_hr_ptr,hr_samples,&num_hr_samples,&hr_ptr,current_hr1f,&reset_kalman_hr,&avg_hr1f);
        LOG(LOG_INFO,"SPO2: ");
        kalman(kalman_spo2_array,&kalman_spo2_length,&kalman_spo2_ptr,spo2_samples,&num_spo2_samples,&spo2_ptr,current_spo21f,&reset_kalman_spo2,&avg_spo21f);
        LOG(LOG_INFO,"post kalman %.1f, %.1f\r\n",(float)avg_hr1f/(float)(1<<FIXED_BITS), (float)avg_spo21f/(float)(1<<FIXED_BITS) );

    } else {
        LOG(LOG_DEBUG,"collecting data\r\n");
        avg_hr1f = 0;
        prev_valid=0;
        avg_spo21f = 0;
        first_hr = 1;
        first_spo2 = 1;
    }
    LOG(LOG_INFO,"%.2f, %.2f, %.2f, %.2f, %.4f\r\n",(float)avg_spo21f/(float)(1<<FIXED_BITS),(float)avg_hr1f/(float)(1<<FIXED_BITS),(float)current_spo21f/float(1<<FIXED_BITS),(float)current_hr1f/(float)(1<<FIXED_BITS),R);
}




SPO2::SPO2()
{
    min_offset = MIN_OFFSET;
    max_offset = MAX_OFFSET;
    prev_valid = 0;
    sample_count = 0;
    first_hr = 1;
    first_spo2 =1;
    data_ptr=0;
    final_offset = 0;
    reset_kalman_hr = 1;
    reset_kalman_spo2 = 1;
    avg_hr1f = 0;
    avg_spo21f = 0;
}


uint32_t SPO2::get_std(uint32_t *array,uint8_t length, uint32_t avg)
{
    int32_t var = 0;
    for (uint16_t n = 0; n< length; n++) {
        var += ((int32_t)array[n]-(int32_t)avg)*((int32_t)array[n]-(int32_t)avg);
    }
    var /= length;
    return uint_sqrt((uint32_t)abs(var));
}

uint32_t SPO2::get_avg(uint32_t *array,uint8_t length)
{
    uint32_t avg = 0;
    for (uint16_t n = 0; n<length; n++) {
        avg += array[n];
    }
    avg /= length;
    return avg;
}

void SPO2::kalman(uint32_t *kalman_array, uint8_t *kalman_length, uint8_t *kalman_ptr, uint32_t *data_array, uint8_t *data_array_length, uint8_t *data_ptr, uint32_t new_data, volatile bool *reset_kalman, uint32_t *kalman_avg)
{
    /*Decide whether to trust the new data or not.
        Base decision on difference between new data and Kalman average and data variance.
        If data variance is large over Kalman length, admit wide variation samples.
        If data variance is small over Kalman length, admit small variation.
        Throw out other samples as outliers.
        Kalman estimate does not include outliers.
        To do: if data is zero that means the algorithm bonked.
        Don't use zero in the data_array variance calculation.
        Simply skip and don't increment data or Kalman pionters
        Instead increment the bad data counter.
        If 3 bad data in a row or 3 outliers, then reset the filter
        */
    uint32_t avg;
    
    LOG(LOG_INFO,"kal ");
    for (int n= 0; n<MAX_KALMAN_LENGTH; n++) {
        LOG(LOG_INFO,"%04lu ",kalman_array[n]);
    }
    LOG(LOG_INFO,"\r\ndat ");
    for (int n= 0; n<NUM_HR_AVGS; n++) {
        LOG(LOG_INFO,"%04lu ",data_array[n]);
    }    
    static uint8_t outlier_cnt =0;
    static uint8_t alg_fail_cnt = 0;
    uint32_t data_std;
    LOG(LOG_DEBUG,"\r\nk_length = %u, k_ptr = %u, d_length = %u, "\
                  "d_ptr = %u, new_data = %lu, reset_kalman = %d, kalman_avg = %lu\r\n",
                  *kalman_length,       *kalman_ptr,    *data_array_length,
                  *data_ptr,    new_data,       *reset_kalman,  *kalman_avg);
    if (*kalman_avg == 0) *reset_kalman = 1; //reset if the previous average was invalid
    if(*reset_kalman) {//re-initialize
        LOG(LOG_INFO,"resetting Kalman\r\n");
        *kalman_ptr = 0;
        *kalman_length = 0;
        alg_fail_cnt = 0;
        outlier_cnt = 0;
        *reset_kalman = 0;
        if (new_data != 0) { //if sample is valid
            kalman_array[*kalman_ptr] = new_data; //put new data in array
            (*kalman_length)++; //increment the array length
            *kalman_avg = new_data; //output new data as new average
            (*kalman_ptr)++; //increment the array index
            data_array[*data_ptr] = new_data;
            (*data_ptr)++;
            if(*data_ptr >= NUM_HR_AVGS) *data_ptr = 0;
            if(*data_array_length < NUM_HR_AVGS) (*data_array_length)++;
        } else {
            *kalman_avg = 0;
        }
    } else if (new_data != 0) {//not resetting and valid sample: update the filter and the sample array
        avg = get_avg(data_array,*data_array_length); //get average of existing (previous) samples)
        LOG(LOG_INFO,"avg %lu\r\n", avg);
        data_std = get_std(data_array, *data_array_length, avg);//get data variance
        data_std = (data_std < MIN_DATA_STD) ? MIN_DATA_STD : data_std; //constrain data variance to a mininum value
        LOG(LOG_INFO,"std = %lu\r\n", data_std);
        if ( abs((int32_t)(new_data-*kalman_avg)) > (uint32_t)KALMAN_THRESHOLD*data_std ) {//outlier case, don't update the filter
            LOG(LOG_INFO,"outlier %lu\r\n", new_data);
            outlier_cnt++;
        } else { //valid data case: update the Kalman avg
            if(*kalman_length < MAX_KALMAN_LENGTH)(*kalman_length)++;
            LOG(LOG_INFO,"keeping %lu\r\n", new_data);
            kalman_array[*kalman_ptr] = new_data; //add new data to the kalman array
            *kalman_avg = get_avg(kalman_array,*kalman_length);// get new kalman average
            alg_fail_cnt = 0; //zero the consecutive algorithm fails
            outlier_cnt = 0; //zero the consecutive outlier fails
            (*kalman_ptr)++;
            if(*kalman_ptr == MAX_KALMAN_LENGTH) *kalman_ptr = 0; //loop index in buffer
        }
        
        //load new sample int data_array for next time
        if(*data_array_length<NUM_HR_AVGS) {//increment number of samples in the average up to the max NUM_HR_AVGS
            (*data_array_length)++;
        }
        data_array[*data_ptr] = new_data;
        (*data_ptr)++;//increment data pointer
        if (*data_ptr>=NUM_HR_AVGS) *data_ptr = 0; //loop index

    } else if (new_data == 0) {//not resetting and invalid sample
        LOG(LOG_INFO,"not valid sample\r\n");
        alg_fail_cnt++;
        if(alg_fail_cnt == ALG_FAIL_TOLERANCE) {//too many bad data points: next time reset the filter
            LOG(LOG_INFO,"reset: too many alg fails\r\n");
            /*i.e., can't latch on to nothing*/
            *reset_kalman = 1;
        }
        if(outlier_cnt >= OUTLIER_DATA_TOLERANCE) {//too many outliers: next time reset the filters
            /*
            This is to handle the case where the Kalman filter latches onto an outlier
            or harmonic/subharmonic and needs to break out.
            */
            LOG(LOG_INFO,"reset: too many outliers\r\n");
            *reset_kalman = 1;
        }
    }
}



void SPO2::copy_data(uint8_t channel)
{
    /*copies all data from the dc_data buffers to temporary buffer and subtracts the DC level
    Output is AC1f-->extended precision array*/
    for (int n=0; n<ARRAY_LENGTH; n++) {
        AC1f[n] = (dc_data[channel][idx[n]]<<FIXED_BITS) - mean1f[channel]; //load the ~11bit or less AC data into an array with fixed precision for DC removal, etc.
    }
}


void SPO2::get_idx()   //creates an array of index pointers mapping 0 to the oldest sample,1 to the next oldest sample, ... ARRAY_LENGTH to the most recent sample.
{
    uint16_t new_idx = data_ptr;
    for (uint16_t n = 0; n < (uint16_t) ARRAY_LENGTH; n++) {
        if (new_idx == 0) { //wrap the index
            new_idx = ARRAY_LENGTH - 1;
        } else {
            new_idx--;
        }
        idx[n] = new_idx; //oldest sample is the next sample to replace
    }
}


void SPO2::get_DC()
{
    /*calculates the mean DC level being subtracted in mean and residual DC level
    for each channel and stores is in res_dc. THe mean is used for SpO2 calculations.
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
        LOG(LOG_INFO,"channel %u mean = %ld\r\n",channel,mean1f[channel]);
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


void SPO2::get_rms()
{
    int32_t slope1f = 0;
    int16_t ind;
    uint32_t var1f;
    get_idx(); //fill an array with indices
    get_DC(); //calculate residual DC level
    for (int channel = 0; channel<2; channel++) {

        var1f = 0;
        copy_data(channel); //copies data to AC1f[n] array (extended precision) and removes DC
        LOG(LOG_DEBUG,"AC1f for channel %d\r\n",channel);
//        avg8Samples();

        //remove slope
        slope1f = 0;
        ind = -(int16_t)SAMPLE_LENGTH;
        for (uint16_t n=0; n<ARRAY_LENGTH; n++) {
            //calc slope
            slope1f += (int32_t)AC1f[n]*(int32_t)ind;
            ind++;
        }
        LOG(LOG_DEBUG,"slop1ef before divide = %ld\r\n",slope1f);
        slope1f /= sum_squares;
        LOG(LOG_DEBUG,"slope1f = %ld\r\n",slope1f);
        LOG(LOG_DEBUG,"AC1f[n] with slope removed\r\n");
        ind = -(int16_t)SAMPLE_LENGTH;
        for (uint16_t n=0; n<ARRAY_LENGTH; n++) {
            AC1f[n] -= (int32_t)ind*slope1f;
            ind++;
            LOG(LOG_DEBUG,"%d, %d, %d\r\n",ind,idx[n],AC1f[n]);
        }
        for (uint16_t n=0; n<SAMPLE_LENGTH; n++) {
            /*Test whether we need to do the entire array or not.
            Could reduce this if HR is fast.
            If it is too long we risk drift affecting the RMS reading.
            */
            var1f += ( (uint32_t)abs(AC1f[n]>>2 ))*( (uint32_t)abs(AC1f[n]>>2)); //getting 2 bit shifts here, taking out extra bit shift for overhead
            LOG(LOG_DEBUG,"var1f = %lu\r\n",var1f);
        }
        
        if(channel == IR) { //print filtered data for IR channel
            for (int n = 0; n<ARRAY_LENGTH; n++) {
                    LOG(LOG_DEBUG_NEED,"%d\r\n",AC1f[idx[n]]);
            }
        }
        
        LOG(LOG_INFO,"var1f = %lu\r\n",var1f);
        rms1f[channel] = uint_sqrt(var1f/(uint32_t)SAMPLE_LENGTH ); //square root halfs the bit shifts back to 2, so this is more like RMS0.5f -- OH WELL (it is 4x bigger, not 16x bigger)
        LOG(LOG_INFO,"channel %d, mean1f = %lu, rms1f = %lu\r\n",channel,mean1f[channel],rms1f[channel]);
    }//end channel loop
}


void SPO2::calc_R()
{
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


void SPO2::calc_spo2()
{
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


void SPO2::calc_hr()
{
    if(final_offset1f == 0) {
        current_hr1f = 0;
    } else {
        current_hr1f = ((uint32_t)((uint32_t)SAMPLE_RATE_MIN<<FIXED_BITS)<<FIXED_BITS) / final_offset1f;
    }
    LOG(LOG_INFO,"HR = %f, %lu\r\n",(float)current_hr1f/(float)(1<<FIXED_BITS), current_hr1f);
}


void SPO2::add_sample(uint32_t ir_data, uint32_t r_data)
{
    const uint8_t num2avg=8;
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


int32_t SPO2::corr(int16_t *x, uint16_t len, uint16_t offset)
{
    int32_t result = 0;
    offset = (offset > SAMPLE_LENGTH) ? SAMPLE_LENGTH : offset; //conform inputs to limits
    len = (len > SAMPLE_LENGTH) ? SAMPLE_LENGTH : len;
    for (uint16_t n = 0; n < len; n++) {
        result += ((int32_t) x[n] >> 4) * ((int32_t) x[n + offset] >> 4); //cast as int32_t for the multiplication to avoid overflow,
        //remove 4 bits of precision to keep same precision, remove 4 more for overhead.
    }
    LOG(LOG_DEBUG,"offset = %u,corr = %ld\r\n",offset,result);
    return result;
}

void SPO2::fine_search(int16_t *x, uint16_t len, uint32_t start_offset, int32_t start_correl, uint32_t search_step)
{
    extern uint16_t t_read(void);
    /*fine search for correlation peak using defined step size (index units).
    Finds peak and interpolates the maximum and saves the answer with fixed precision.
    */
    LOG(LOG_INFO,"fine search at %lu\r\n",start_offset);

    uint16_t offset = start_offset;
    int32_t c;
    int32_t high_side;
    int32_t low_side;
    int32_t lowest;
    final_correl = start_correl;     //initialize
    final_offset = start_offset;
    uint16_t elapsed_time = t_read() - p2_start_time;
    if ( elapsed_time >= 15) {
      final_correl = 0;
      final_offset = 0;
      final_offset1f = 0;
    } else {
        
        while(1) { //search toward lower offset, higher frequency
            offset -= search_step;
            if (offset>=min_offset) {
                c = corr(x,len,offset);
                LOG(LOG_INFO,"%u, %ld \r\n",offset, c/2500);
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
                    LOG(LOG_INFO,"%u, %ld \r\n",offset, c/2500);
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
                final_offset1f = (final_offset<<FIXED_BITS) + ( ( (((int32_t)search_step)<<FIXED_BITS) /2)*(high_side - low_side) ) / (final_correl-lowest); //interpolate a better answer
            }
        }
    }
}


bool SPO2::check4max(int16_t *x, uint16_t len,uint16_t start_offset, int32_t start_correl)
{
    //check for a max in the correlation in a region
    bool max_found = 0;
    fine_search(x,len,start_offset,start_correl, MID_STEP);
    if ( final_offset1f != (min_offset<<FIXED_BITS)) {
        if(final_offset1f != (max_offset<<FIXED_BITS)) {
            max_found = 1;
            LOG(LOG_INFO,"start = %u, success at %.2f, correl = %lu\r\n", start_offset, (float)final_offset1f/(float)(1<<FIXED_BITS), final_correl);
        }
    } else {
        LOG(LOG_INFO,"start = %u, fail at %.2f correl = %lu\r\n", start_offset, (float)final_offset1f/(float)(1<<FIXED_BITS), final_correl);
    }
    return max_found;
}


bool SPO2::find_max_corr(int16_t *x, uint16_t max_length, uint16_t offset_guess)
{
    extern uint16_t t_read(void);
    
    bool rising = 0;
    uint8_t fail = 0;
    uint16_t try_offset =MIN_OFFSET-BIG_STEP ;
    int32_t c1;
    int32_t c2;
    int32_t c3;
    int32_t highest;
    int32_t lowest;
    int32_t d2;
    int32_t prev_correl;
    
    LOG(LOG_INFO,"start find max correl\r\n");
    LOG(LOG_DEBUG,"len = %d, offset_guess = %d\r\n",max_length, offset_guess);
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

    c1 = corr(x,samples2use,try_offset);
    try_offset +=BIG_STEP;
    c2 = corr(x,samples2use,try_offset);
    rising = (c2 > c1) ? 1 : 0; //skip to search for a peak if rising, else look for a minimum and double it.
    LOG(LOG_INFO,"%d, %ld\r\n%d, %ld\r\n", MIN_OFFSET-BIG_STEP,c1,MIN_OFFSET,c2);
    uint16_t step = BIG_STEP; //start with big step
    const uint16_t max_step = 16;
    if(!rising) {
        while(!rising) { //keep going until you find a minimum
          uint16_t elapsed_time = t_read() - p2_start_time;
            LOG(LOG_INFO,"searching for min\r\n");
            try_offset += step; //increment by step size
            step += (step < max_step) ? 1 : 0; //increment step size if less than max step
            if(try_offset> MAX_OFFSET || (elapsed_time >= 15)) {
                fail = 1; //still falling and ran out of samples
                break;
            }
            c3 = corr(x,samples2use,try_offset);
            if (c3>c2) {
                rising  = 1;
            } else {
                c2 = c3;
                c1 = c2;
            }
            LOG(LOG_INFO,"%u %ld\r\n",try_offset,c3);
        }
        if(!fail) {
            highest = (c1 > c3) ? c1 : c3;
            if(c2-highest == 0) {
                offset_guess  = try_offset-BIG_STEP;
            } else {
                d2 =  ( ( ( (int32_t)BIG_STEP<<FIXED_BITS)*(c1 - c3) ) / (highest-c2) )>>FIXED_BITS; //twice the delta
                LOG(LOG_INFO,"d2 = %ld\r\n",d2);
                offset_guess = (uint16_t)( ( ( (int16_t)try_offset-(int16_t)BIG_STEP)<<1) + (int16_t)d2) ; //interpolate a better answer
                LOG(LOG_INFO,"guessing double the min at %d\r\n",offset_guess);
            }
        }
    } else {//already rising
        while(rising) {//keep going until you find a drop
          uint16_t elapsed_time = t_read() - p2_start_time;
            LOG(LOG_INFO,"searching for max\r\n");
            try_offset += step;
            step += (step < max_step) ? 1 : 0; //increment step size if less than max step
            if((try_offset>MAX_OFFSET) ||((elapsed_time >= 15))) {
                fail = 2; //still rising and ran out of samples
                break;
            } else {
                c3 = corr(x,samples2use,try_offset);
                if (c3<c2) {
                    rising  = 0;
                } else {
                    c2= c3;
                    c1= c2;
                }
                LOG(LOG_INFO,"%u %ld\r\n",try_offset,c3);
            }
        }
        if(!fail) {
            lowest = (c3 < c1) ? c3 : c1;
            if(c2-lowest == 0) {
                offset_guess  = try_offset-BIG_STEP;
            } else {
                d2 = ( ( ( (int32_t)BIG_STEP<<FIXED_BITS)*(c3 - c1) ) / (c2-lowest) )>>(FIXED_BITS+1);
                LOG(LOG_INFO,"d2 = %ld\r\n",d2);
                offset_guess = (uint16_t)( ( (int16_t)try_offset-(int16_t)BIG_STEP) + (int16_t)d2) ; //interpolate a better answer
                LOG(LOG_INFO,"guessing near the max at %d\r\n",offset_guess);
            }
        } else {
            offset_guess = DEFAULT_GUESS;
        }
    }
    //condition inputs
    if(offset_guess < MIN_OFFSET) offset_guess = DEFAULT_GUESS;
    if(offset_guess > MAX_OFFSET) offset_guess = DEFAULT_GUESS;

    start_correl = corr(x,samples2use,offset_guess);
    max_found = check4max(x, samples2use, offset_guess, start_correl);
    if (prev_valid & !max_found) { //go ahead and try the previous value if the other bonked.
        //note you can't just rely on this or you could lock into a harmonic.
        LOG(LOG_INFO,"trying prev sol'n\r\n");
        max_found = check4max(x, samples2use, prev_offset, prev_correl);
    }

    if(!max_found) {
        final_correl = 0;
        final_offset1f = 0;
        prev_valid = 0;
    } else {
        prev_valid = 1;
    }
    LOG(LOG_DEBUG,"final_offset %lu\r\n",final_offset);
    return max_found;
}


void SPO2::peak_find(uint16_t *x0, int32_t *y, int32_t *x_fit1f, int32_t *y_fit)
{
    /*
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


void SPO2::simple_peak_find(int32_t *y, int32_t *x_fit1f, int32_t *y_fit, uint16_t x_center, uint16_t step)
{
    /* For numerical stability let x[1] = 0, namely the middle point (high or low) is zero
    To simplify the math we measure in units of the step size, so xfit1f is fixed precision in units of the step size

    Solve this linear equation
    a*x[0]^2 + b*x[0] + c = y[0]
    a*x[1]^2 + b*x[1] + c = y[1]
    a*x[2]^2 + b*x[2] + c = y[2]

    using x2 = {x[0]^2, x[1]^2, x[2]^2}
    in this case x[0] = -1, x[1] = 0; x[2] = 1;
    */
    int32_t A;
    int32_t B;
    int32_t C;
    //using Cramer's rule
    C = y[1]; //middle equation in system is trivial. Reduces to 2x2 determinant.
    //let yp[2] = {y[0]-C,y[2]-C};
    //D = 2; //determinant of {{1, -1},{1, 1}}
    //Dx = yp[0]*1 - (-1)*yp[2] = yp[0] + y[p] = y[0]-C + y[2]-C = y[0] + y[2] -2*C;
    //Dy = 1*yp[2] - yp[0]*1 = yp[2] - yp[1] = y[2] -C - (y[0] -C) = y[2] - y[0];
    A = (y[0]+y[2]-(C<<1) )>>1; // Dx/D
    B = (y[2]-y[0])>>1; // Dy/D
    *x_fit1f =  (-((B<<FIXED_BITS)/A)>>1)*step + (x_center<<FIXED_BITS)  ; // -b/2a * step + x_center
    *y_fit = C - (((B>>2)*B)/A); //C - b2/4a
    LOG(LOG_DEBUG,"A = %ld, B = %ld, C = %ld, b^2/a = %ld\r\n",A,B,C, B*B/4/A);
}