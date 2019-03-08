#include "mbed.h"
#include <math.h>
#include "SPO2.h"
#include "OB1203.h"
//#include "SoftI2C.h"

Mutex data_buffer_mutex;

extern Serial pc;

void do_algorithm()
{
    static float hr_avgs[NUM_HR_AVGS];
    static float sum_hr = 0;
    static uint16_t hr_avg_ind = 0;
    static uint16_t spo2_avg_ind =0;
    static float spo2_avgs[NUM_SPO2_AVGS];
    static float sum_spo2 =0;
    static float num_hr_avgs = 0;
    static float num_spo2_avgs = 0;

    spo2.get_alg_params();
  
    if(spo2.sample_count>spo2.array_length)
    {
        spo2.copy_data();
        
        spo2.removeDC();
        
        spo2.get_rms();
    
        spo2.calc_R();   
        
        spo2.calc_spo2(); //<----write this function
        
        spo2.find_max_corr(spo2.irAC, spo2.sample_length, spo2.offset_guess);
        
        spo2.calc_hr();
        
        if(spo2.final_correl < 0) //hr bonked
        {
            pc.printf("alg fail\r\n");
            spo2.prev_valid = 0;
            spo2.current_hr = 0;
            spo2.first_hr = 1;
        }
    }
    else
    {
        //pc.printf("collecting data\r\n");
        spo2.current_hr = 0;
        spo2.prev_valid=0;
        spo2.current_spo2 = 0;
        spo2.first_hr = 1;
        spo2.first_spo2 = 1;
    }
    
    //pc.printf("pre concensus %.1f, %.1f\r\n",spo2.current_spo2, spo2.current_hr);
    spo2.consensus(); //filter out crap data
    //pc.printf("post concensus %.1f, %.1f, first? %d, %d\r\n",spo2.current_spo2, spo2.current_hr, spo2.first_hr, spo2.first_spo2);
    
    if(spo2.current_hr)
    {
        if(spo2.first_hr)//initalize average
        {
            //pc.printf("init hr avg\r\n");
            sum_hr = 0;
            num_hr_avgs =1;
            spo2.first_hr = 0;
            hr_avg_ind = 0;
        }
        else //increment index and num avgs
        {
            num_hr_avgs++;
            hr_avg_ind++;
            hr_avg_ind %= NUM_HR_AVGS;
        }
        if(num_hr_avgs > NUM_HR_AVGS) //subtract oldest sample if number of averages is greater than max
        {
            num_hr_avgs = NUM_HR_AVGS;
            sum_hr -= hr_avgs[hr_avg_ind];
        }
        hr_avgs[hr_avg_ind] = spo2.current_hr;    //overwrite oldest sample with new sample
        sum_hr += hr_avgs[hr_avg_ind]; //add new sample to sum
        spo2.avg_hr = (num_hr_avgs > 0) ? (sum_hr / num_hr_avgs) : sum_hr;
    }
    else
    {
        spo2.avg_hr = 0;
    }
    if(spo2.current_spo2)
    {
        if(spo2.first_spo2)//initalize average
        {
            //pc.printf("init spo2 avg\r\n");
            sum_spo2 = 0;
            num_spo2_avgs = 1;
            spo2.first_spo2 = 0;
            spo2_avg_ind = 0;
            
        }
        else //increment index and num avgs
        {
            num_spo2_avgs++;
            spo2_avg_ind++;
            spo2_avg_ind %= NUM_SPO2_AVGS;
        }
        if(num_spo2_avgs > NUM_SPO2_AVGS) //subtract oldest sample if number of averages is greater than max
        {
            num_spo2_avgs = NUM_SPO2_AVGS;
            sum_spo2 -= spo2_avgs[spo2_avg_ind];
        }
        spo2_avgs[spo2_avg_ind] = spo2.current_spo2;    //overwrite oldest sample with new sample
        sum_spo2 += spo2_avgs[spo2_avg_ind]; //add new sample to sum
        spo2.avg_spo2 = (num_spo2_avgs > 0) ? (sum_spo2 / num_spo2_avgs) : sum_spo2;
    }
    else
    {
        spo2.avg_spo2 = 0;
    }
    pc.printf("%.1f, %.1f; %.1f, %.1f, %.4f\r\n",spo2.avg_spo2,spo2.avg_hr,spo2.current_spo2,spo2.current_hr,spo2.R);
}



SPO2::SPO2()
{
    min_offset = MIN_OFFSET;
    max_offset = MAX_OFFSET;
    prev_valid = 0;
    prev_sample_length =0;
    sample_count = 0;
    first_hr = 1;
    first_spo2 =1;
}

float SPO2::abs_float(float val)
{
    return (val < 0 )? -val : val;
}

void SPO2::consensus()
{//Compares 3 values. If latest 2 samples have big differense, it carries over previous avg. If prev difference is large it uses latest average.
//If both differences are large, it uses the latest value. If only the middle value is off, it drops it.
    static float hr_rms_buffer[3];
    static float spo2_rms_buffer[3];
    static uint8_t hr_rms_ind = 0;
    static uint8_t spo2_rms_ind =0;
    static uint8_t n_hr_samples = 0;
    static uint8_t n_spo2_samples = 0;
    uint8_t ind2;
    uint8_t ind1;
    float prev_diff;
    float prev_avg;
    static float hr_diff;
    static float spo2_diff;
    static float hr_avg;
    static float spo2_avg;
    float diff_from_oldest;
    float oldest_avg;
    uint8_t carryover;
    //pc.printf("n_samples = %d, %d\r\n",n_hr_samples,n_spo2_samples);
    
    
    //begin spo2 analysis
    if(n_spo2_samples==0 ) spo2_rms_ind = 2;
    spo2_rms_ind = (spo2_rms_ind+1)%3;
    spo2_rms_buffer[spo2_rms_ind] = current_spo2;
    if(n_spo2_samples ==1)
    {
        ind2 = (hr_rms_ind+2)%3;
        spo2_diff = spo2_rms_buffer[spo2_rms_ind] - spo2_rms_buffer[ ind2];
        spo2_avg = spo2_rms_buffer[ind2] + spo2_diff/2;
        if( (spo2_diff /spo2_avg) < MAX_SPO2_DROP )  //only consider large drops becuase large rises are biologically possible in one heart beat
        {
            first_spo2 =1; //reset with current value
        }
    }
    else if(n_spo2_samples == 2)
    {
        prev_diff = spo2_diff;
        prev_avg = spo2_avg;
        ind2 = (spo2_rms_ind+2)%3;
        ind1 = (spo2_rms_ind+1)%3;
        spo2_diff = spo2_rms_buffer[spo2_rms_ind] - spo2_rms_buffer[ind2]; //newest - previous
        spo2_avg = spo2_rms_buffer[ind2] + spo2_diff/2;
        diff_from_oldest = spo2_rms_buffer[spo2_rms_ind] - spo2_rms_buffer[ind1]; //newest - oldest
        oldest_avg = spo2_rms_buffer[ind1] + diff_from_oldest/2;
        
        if ( (prev_avg == 0) || (hr_avg == 0) )
        {
            first_spo2 = 1;
            //pc.printf("use latest spo2--00s, ");
        }
        else
        {
            if(spo2_diff/spo2_avg < MAX_SPO2_DROP)
            {
                if( (prev_diff/prev_avg) > MAX_HR_CHANGE)
                {
                    carryover = USE_LATEST; //use last sample
                    first_spo2 = 1;
                    //pc.printf("use latest spo2, ");
                }
                else
                {
                     carryover = USE_PREV; //use first sample
                     current_spo2 = prev_avg;  
                     //pc.printf("drop latest spo2, ");  
                }
                if( (carryover == USE_LATEST) && ( (diff_from_oldest/oldest_avg) < MAX_HR_CHANGE) )
                {
                    carryover = DROP_MIDDLE; // drop middle sample
                    current_spo2 = (spo2_rms_buffer[spo2_rms_ind] + spo2_rms_buffer[ind1] ) /2; //avg of oldest and newest
                    first_spo2 = 1;
                    //pc.printf("and first spo2, ");
                }  
            }
            else if( (prev_diff/prev_avg) > MAX_HR_CHANGE)
            {
                carryover = USE_LATEST_AVG;
                current_spo2 = spo2_avg;
                first_spo2 = 1;
                //pc.printf("drop first spo2, ");
            }
            else
            {
                carryover = USE_ALL;
                //pc.printf("use all spo2, ");
            }
        }//end not zero case
    }//end 3 samples case
    n_spo2_samples++;
    n_spo2_samples = (n_spo2_samples > 2) ? 2 : n_spo2_samples;  //limit to 2   
    //end spo2 analysis
    
    
    //begin HR analysis
    if(n_hr_samples==0) hr_rms_ind = 2;
    hr_rms_ind = (hr_rms_ind+1)%3;
    hr_rms_buffer[hr_rms_ind] = current_hr;//add to three sample buffer

    if(n_hr_samples ==1)
    {
        ind2 = (hr_rms_ind+2)%3;
        hr_diff = hr_rms_buffer[hr_rms_ind] - hr_rms_buffer[ind2];
        hr_avg = hr_rms_buffer[ind2] + hr_diff/2;
        hr_diff = abs_float(hr_diff); //absolute value
        if( (hr_diff /hr_avg) > MAX_HR_CHANGE ) 
        {
            first_hr =1; //reset with current value
        }
    }
    else if(n_hr_samples == 2)
    {
        prev_diff = hr_diff;
        prev_avg = hr_avg;
        ind2 = (hr_rms_ind+2)%3;
        ind1 = (hr_rms_ind+1)%3;
        hr_diff = hr_rms_buffer[hr_rms_ind] - hr_rms_buffer[ind2]; //newest - previous
        diff_from_oldest = hr_rms_buffer[hr_rms_ind] - hr_rms_buffer[ind1]; //newest - oldest
        hr_avg = hr_rms_buffer[ind2] + hr_diff/2;
        oldest_avg = hr_rms_buffer[ind1] + diff_from_oldest/2;
        hr_diff = abs_float(hr_diff); //absolute value
        diff_from_oldest = abs_float(diff_from_oldest);
        
        if( (prev_avg == 0) || (hr_avg == 0) )
        {
            first_hr = 1;
            //pc.printf("use latest hr--00s\r\n");
            if(hr_avg == 0); //attempt to reaquire a lost signal with a typical guess.
        }
        else
        {
            if( (hr_diff/hr_avg) > MAX_HR_CHANGE)
            {
                if( (prev_diff/prev_avg) > MAX_HR_CHANGE)
                {
                    carryover = USE_LATEST; //use most recent sample
                    first_hr = 1;
                    //pc.printf("use latest hr");
                }
                else
                {
                    //pc.printf("drop latest hr, ");
                    carryover = USE_PREV; //use first sample (drops current from average)
                    current_hr = prev_avg;    
                }
                if( (carryover == USE_LATEST) && ( (diff_from_oldest/oldest_avg) < MAX_HR_CHANGE) )
                {
                    carryover = DROP_MIDDLE; // drop middle sample
                    current_hr = (hr_rms_buffer[hr_rms_ind] + hr_rms_buffer[ind1] ) /2; //avg of oldest and newest
                    first_hr = 1;
                   // pc.printf("and first hr");
                }  
            }
            else if( (prev_diff/prev_avg) > MAX_HR_CHANGE)
            {
                //pc.printf("drop first hr");
                carryover = USE_LATEST_AVG;
                current_hr = hr_avg;
                first_hr = 1;
            }
            else
            {
                //pc.printf("use all hr");
                carryover = USE_ALL;
            }
            //pc.printf("\r\n");
        }//end not zero case
    }//end 3 samples case
    n_hr_samples++;
    n_hr_samples = (n_hr_samples > 2) ? 2 : n_hr_samples;  //limit to 2   
    //end hr analysis    
}//end consensus


void SPO2::init_running_avgs(uint32_t init_ir_val, uint32_t init_r_val)
{
    for (uint16_t n=0; n<=AVG_BUFF_MASK; n++) 
    {
        r_avg_buffer[n] = init_r_val;
        ir_avg_buffer[n] = init_ir_val;
    }
    ir_running_sum = init_ir_val<<3;
    r_running_sum = init_r_val<<3;
    avg_ptr = 0;
}


void SPO2::get_sum_squares()
{
    uint32_t start_val = -sample_length;
    for (uint16_t n=0;n<array_length;n++)
    {
        ind[n] = start_val;
        start_val++;
    }
    
    for(uint16_t n=0; n<sample_length; n++) 
    {
        sum_squares += ind[n]*ind[n];
    }
    sum_squares = sum_squares<<1; //multiply by two
}


void SPO2::get_alg_params()
{
    offset_guess = DEFAULT_GUESS;
    sample_length = 1.6*offset_guess;
    sample_length &= (DATA_BUFF_MASK>>1)-1; //max sample length is less than half the buffer minus 1
    //sample_length |= 1; //make sure it is odd
    start_point = DATA_BUFF_SIZE; //add data buffer size so I don't go negative when I subtract the sample size
    start_point += data_ptr; //starting from data_ptr+DATA_BUFF_SIZE
    array_length = (sample_length<<1)+1;
    start_point -= array_length; 
    start_point &= DATA_BUFF_MASK; //fast mod to cut down to less than buffer length
    max_offset = (sample_length > max_offset) ? max_offset : sample_length;
//    pc.printf("max_offset in get alg params = %d\r\n",max_offset);
}

void SPO2::copy_data()
{
    data_buffer_mutex.lock();
    //array length = sample_length*2 + 1;
    for(uint16_t n=0; n<array_length; n++) 
    {
        irAC[n] = ir_buffer[(start_point+n) & DATA_BUFF_MASK];
        rAC[n] = r_buffer[(start_point+n) & DATA_BUFF_MASK];
    }
    data_buffer_mutex.unlock();
}


void SPO2::removeDC()
{
    if (prev_sample_length != sample_length) {
        get_sum_squares();
    }

    prev_sample_length = sample_length; //update for next time

    ir_mean = 0;
    r_mean = 0;
    
    for (uint16_t n=0; n<array_length; n++) {
        ir_mean += irAC[n];
        r_mean += rAC[n];
    }
    ir_mean /= array_length;
    r_mean /= array_length;
    
    //pc.printf("ir mean = %d, r mean = %d\r\n",ir_mean,r_mean);
    //pc.printf("avg removed:\r\n");
    for (uint16_t n=0; n<array_length; n++) {
        irAC[n] -= ir_mean;
        rAC[n] -= r_mean;
        //pc.printf("%d, %d\r\n",irAC[n],rAC[n]);
    }//this works
    
    ir_slope = 0;
    r_slope = 0;
    for (uint16_t n=0; n<array_length; n++) {
        ir_slope += irAC[n]*ind[n];
        r_slope += rAC[n]*ind[n];
    }
    //pc.printf("ir_slope sum= %f, r_slope  sum= %f, sumsquares = %d\r\n",ir_slope,r_slope, sum_squares);

    ir_slope /= sum_squares;
    r_slope /= sum_squares;
    //pc.printf("ir_slope = %.2f, r_slope = %.2f\r\n",ir_slope,r_slope);

//    pc.printf("slope removed\r\n");
    for (uint16_t n=0; n<array_length; n++) {
        irAC[n] -= ind[n]*ir_slope;
        rAC[n] -= ind[n]*r_slope;
//        pc.printf("%d,%d\r\n",irAC[n],rAC[n]);
    }
    
    
}

void SPO2::get_rms()
{
    //assume DC has been removed
    int32_t ir_var = 0;
    int32_t r_var =0;
    for (uint16_t n=0; n<array_length; n++) {
        ir_var += irAC[n]*irAC[n];
        r_var += rAC[n]*rAC[n];
    }
    ir_rms  = sqrt( (float)ir_var/(float)array_length );
    r_rms = sqrt( (float)r_var/(float)array_length );
}

void SPO2::calc_R()
{
    R = (r_rms/(float)r_mean)/(ir_rms/(float)ir_mean);
}

void SPO2::calc_spo2()
{
    float c[6] = {-26639, 80950, -97713, 58474,  -17367, 2147};
    current_spo2 =0;
    for(uint16_t n=0;n<6;n++)
    {
        current_spo2 += c[n]*pow(R,5-n);
    }
    current_spo2 = (current_spo2 > 100) ? 100 : current_spo2;
    current_spo2 =  (current_spo2 <80) ? 80 :current_spo2;
}

void SPO2::calc_hr()
{
    if(final_offset == 0)
    {
        current_hr = 0;
    }
    else
    {
        current_hr = SAMPLE_RATE / final_offset * 60;
    }
}

int32_t SPO2::corr(int32_t *x, uint16_t start_ptr, int16_t len, uint16_t offset)
{
    int32_t result = 0;
    for (uint16_t n=0; n<len; n++) {
        result += x[start_ptr+n] * x[(start_ptr+n+offset) & DATA_BUFF_MASK];
    }
    //pc.printf("offset = %d,corr = %d\r\n",offset,result);
    return result;
}


void SPO2::add_sample(uint32_t ir_data, uint32_t r_data)
{
    //run init_running_avgs() first
    ir_running_sum += ir_data; //add new sample to sum
    r_running_sum += r_data;
    ir_running_sum -= ir_avg_buffer[avg_ptr]; //subtract the old sample
    r_running_sum -= r_avg_buffer[avg_ptr];
    ir_avg_buffer[avg_ptr] = ir_data; //add new sample to average buffer
    r_avg_buffer[avg_ptr] = r_data;
    avg_ptr++; //increment avg pointer
    avg_ptr &= AVG_BUFF_MASK; //roll over
    ir_buffer[data_ptr] = ir_running_sum>>AVG_BITS;
    r_buffer[data_ptr] = r_running_sum>>AVG_BITS;
    prev_data_ptr = data_ptr;
    data_ptr++;
    data_ptr &= DATA_BUFF_MASK; //roll over
//    #ifdef DEBUG
//        if(data_ptr == DATA_BUFF_MASK)
//        {
//            pc.printf("rollover\r\n");
//        }
//    #endif
}


void SPO2::fine_search(int32_t *x, uint16_t len, uint32_t start_offset, int32_t start_correl, uint32_t search_step)
{
    //fine search for correlation peak
    uint16_t offset = start_offset;
    int32_t c1;
    int32_t high_side;
    int32_t low_side;
    
    //make sure we start in bounds
    if(offset<min_offset) offset = 1.2*min_offset; 
    if(offset>max_offset) offset = 0.8*max_offset;
    
    final_correl = start_correl;     //initialize
    final_offset = start_offset;
    while(1) { //search toward lower offset, higher frequency
        offset -= search_step;
        if (offset>=min_offset) {
            c1 = corr(x,start_ptr,len,offset);
            if (c1<=final_correl) {
                break; //Getting worse. Stop.
            } else {
                high_side = final_correl; //load prev sample as new larger offset sample for fit
                final_correl = c1; //better, use new result; keep going
                final_offset = offset;
            }
        } else {
            c1 = start_correl;
            break;
        }
        pc.printf("c= %.0f, %d \r\n",final_offset, final_correl/2500);
    }
    low_side = c1;  //the low-side sample is the one we exited the loop on

    if (final_correl == start_correl) { //didn't find something bigger at lower offsets, check higher
        offset = start_offset;
        while(1) {
            offset += search_step;
            if (offset<max_offset) {
                c1 = corr(x,start_ptr,len,offset); //search lower frequency
                if (c1 <= final_correl) {
                    break; //getting worse; stop
                } else {
                    low_side = final_correl;
                    final_correl = c1; //better, use new results; keep going
                    final_offset = offset;
                }
            } else {
                break; //out of range; stop
            }
            pc.printf("%.0f, %d\r\n",final_offset, final_correl/2500);
        }
        high_side=c1; //the high side sample is the one we exited the loop on.
    }
    //pc.printf("before interp: %0.1f, (%d %d %d)\r\n", final_offset,low_side,final_correl,high_side);
    if( (final_correl <=0) || (low_side <=0) || (high_side <=0) ) //no where near a real maximum
    {
        pc.printf("adjacents negative--false peak\r\n");
        final_offset = min_offset; //force the algorithm to bonk at max found check.
    }
    else
    { 
        if(final_offset <= min_offset)
        {
            final_offset = min_offset; //force the algorithm to bonk at max found check.
        }
        else if(final_offset >=max_offset)
        {
            final_offset = max_offset; //force the algorithm to bonk at max found check.
        }
        else //only run this if final_offset is not one of the boundaries
        {
            final_offset = final_offset + 0.5 *search_step / (float)final_correl * ( ((float)low_side + (float)high_side - 2*(float)final_correl) ); //interpolate a better answer
        }
    }
}


bool SPO2::check4max(int32_t *x, uint16_t len,uint16_t start_offset, int32_t start_correl)
{
    //check for a max in the correlation in a region
    bool max_found = 0;
    fine_search(x,len,start_offset,start_correl, MID_STEP);
    if ( final_offset != min_offset) {
        if(final_offset != max_offset) {
            max_found = 1;
            pc.printf("start = %d, success at %.1f, correl = %.1f\r\n", start_offset, final_offset, final_correl);
        }
    } else {
        pc.printf("start = %d, fail at %.1f, correl = %.1f\r\n", start_offset, final_offset, final_correl);
    }
    return max_found;
}


void SPO2::dither(int32_t *x, uint16_t len, uint16_t offset, const float *rel_vals, uint16_t num_vals, int32_t *correls, float *offsets)
{
    /*dither will run the correlation function for offsets specified relative to
    a nominal offset.
    x: the array to autocorrelate
    len: the length of the array to use
    rel_vals: an array with scale factors to use for the offset, e.g. (0.7.0.9, 1, 1.1, 1.3)
    Make sure correls and offsets are appropriate arrays with enough size
    */

    //condition inputs
    if (offset>len) {
        offset=len-1;
    }

    for (uint16_t n=0; n<num_vals; n++) {
        offsets[n] = floor(rel_vals[n]*offset);
        if (offsets[n] < min_offset) {
            offsets[n] = min_offset;
        } else if (offsets[n] > max_offset) {
            offsets[n] = max_offset;
        }
        correls[n] = corr(x,start_ptr,len,offsets[n]);
    }
    for (int n=0; n<NUM_COARSE_POINTS; n++)
    {
        pc.printf("%d ",correls[n]/2500);
    }
    pc.printf("\r\n");
}

void SPO2::get_corr_slope(int32_t *x, uint16_t len, uint16_t offset0, uint16_t offset1)
{
    c0 = corr(x,start_ptr,len,offset0);
    int32_t c1 = corr(x,start_ptr, len, offset1);
    m = (float)(c1-c0) / (float)(offset1-offset0);
    //pc.printf("c0 %d, c1 %d, m %f\r\n",c0,c1,m);
}

bool SPO2::find_max_corr(int32_t *x, uint16_t max_length, uint16_t offset_guess)
{
    /*run get_corr_slope before this function if prev_valid == 0 to get the
    right length of sample and a decent start_offset
    heart_rate_guess/sample_rate = start_offset;
    max_HR/sample_rate = min_offset;
    min_HR/sample_rate = max_offset;
    prev_valid means use heart rate guess to optimize sample length
    correlation peak which is considered to be a valid measurement
    rms is the average rms signals of IR and R (after DC and slope removal)
    Here is the algorithm:
    If no previous valid sample
    Get a target use "get_corr_slope"--this is the secret sauce of the algorithm
    Then run a spread of nearby values to jump closer
    Then do a fine search.
    */
    int32_t start_correl;
    bool max_found;
    
    if(prev_valid) prev_offset = (uint16_t)final_offset;
    
    //conform inputs
    if (max_offset > max_length) { //sanity check
        max_offset = floor(0.9 * max_length);
        //pc.printf("max_offset = %d,max_length = %d\r\n",max_offset, max_length);
    }

    if (min_offset < 0.1*max_length) { //sanity check
        min_offset = floor(0.1*max_length); //max 10 beats per sample period
    }

    if (offset_guess > max_offset)
        offset_guess = max_offset;
    else if (offset_guess < min_offset) {
        offset_guess = min_offset;
    }
    //pc.printf("initial max offset %d, min offset %d, offset guess %d, prev_valid %d\r\n",max_offset,min_offset,offset_guess,prev_valid);

    uint16_t samples2use = max_offset;

 
    //determine how fast autocorrelation is dropping
    get_corr_slope(x,max_offset,0,OFFSET_FOR_SLOPE_CALC);
    //pc.printf("max_offset after get_corr_slope = %d\r\n",max_offset);
    offset_guess = 4 * c0 /(-m);
    if(offset_guess < MIN_OFFSET) offset_guess = DEFAULT_GUESS; 
    if(offset_guess > MAX_OFFSET) offset_guess = DEFAULT_GUESS;
    //pc.printf("new offset_guess %d\r\n",offset_guess);
    //pc.printf("before dither %d, ", offset_guess);
    //coarse search--break out of local maximum
    const float coarse_search[NUM_COARSE_POINTS] = {0.85,0.92,1,1.1,1.2};
    int32_t correls[NUM_COARSE_POINTS];
    float offsets[NUM_COARSE_POINTS];
    dither(x,samples2use,offset_guess,coarse_search,NUM_COARSE_POINTS,correls,offsets);
    
    uint16_t max_ind = 0;

    for(uint16_t n=1; n<NUM_COARSE_POINTS; n++) {
        if (correls[n] > start_correl) {
            start_correl = correls[n];
            max_ind = n;
        }
    }
    pc.printf("max_ind =%d\r\n",max_ind);
    offset_guess = offsets[max_ind];

    //pc.printf("after dither %d, ",offset_guess);
    max_found = check4max(x, samples2use, offset_guess, start_correl);
    if (prev_valid & !max_found) { //go ahead and try the previous value if the other bonked.
    //note you can't just rely on this or you could lock into a harmonic.
        pc.printf("trying prev sol'n\r\n");
        max_found = check4max(x, samples2use, prev_offset, start_correl);
    }
    
    if(!max_found) {
        final_correl = 0;
        final_offset = 0;
        prev_valid = 0;
    }
    else {
        prev_valid = 1;
    }
    //pc.printf("final_offset %0.1f\r\n",final_offset);

    return max_found;
}