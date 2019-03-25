#include "mbed.h"
#include <math.h>
#include "SPO2.h"
#include "OB1203.h"
#include <iostream>
//#include "SoftI2C.h"


/*new plan is to use a fixed sample length of 150 (1.5 seconds)
keep a runing average of the DC level (only needs to be 150 long) and buffer samples with DC already subtracted
this subtracts the DC level no matter what it is doing: linear or quadratic or whatever.
The sample buffers are then int16's and data will be operated on in-place.
*/

extern Serial pc;
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
    pc.printf("sample_count %d\r\n",sample_count);
    if(sample_count>=ARRAY_LENGTH) {
        pc.printf("do alg part 1\r\n");
        get_rms(); //subtract residual slopes and calculate RMS
    }
}

void SPO2::do_algorithm_part2()
{
    static uint16_t hr_avgs[NUM_HR_AVGS];
    static uint16_t sum_hr = 0;
    static uint8_t hr_avg_ind = 0;
    static uint8_t spo2_avg_ind =0;
    static uint16_t spo2_avgs[NUM_SPO2_AVGS];
    static uint16_t sum_spo2 =0;
    static uint8_t num_hr_avgs = 0;
    static uint8_t num_spo2_avgs = 0;

    offset_guess = DEFAULT_GUESS;
//    pc.printf("sample_count %d\r\n",sample_count);
    if(sample_count>=ARRAY_LENGTH) {
        pc.printf("doing algorithm part 2\r\n");
        calc_R();
        calc_spo2();

        find_max_corr(AC1f,(uint16_t)SAMPLE_LENGTH, offset_guess); //AC1f is the IR sample after get_rms is run

        calc_hr();

        pc.printf("pre concensus %.1f, %.1f\r\n",(float)current_spo21f/(float)(1<<FIXED_BITS), (float)current_hr1f/(float)(1<<FIXED_BITS));
        consensus(); //filter out crap data
        pc.printf("post concensus %.1f, %.1f, first? %d, %d\r\n",(float)current_spo21f/(float)(1<<FIXED_BITS), (float)current_hr1f/(float)(1<<FIXED_BITS), first_hr, first_spo2);
    } else {
        //pc.printf("collecting data\r\n");
        current_hr1f = 0;
        prev_valid=0;
        current_spo21f = 0;
        first_hr = 1;
        first_spo2 = 1;
    }


    if(current_hr1f) {
        if(first_hr) { //initalize average
            //pc.printf("init hr avg\r\n");
            sum_hr = 0;
            num_hr_avgs =1;
            first_hr = 0;
            hr_avg_ind = 0;
        } else { //increment index and num avgs
            num_hr_avgs++;
            hr_avg_ind++;
            hr_avg_ind %= NUM_HR_AVGS;
        }
        if(num_hr_avgs > NUM_HR_AVGS) { //subtract oldest sample if number of averages is greater than max
            num_hr_avgs = NUM_HR_AVGS;
            sum_hr -= hr_avgs[hr_avg_ind];
        }
        hr_avgs[hr_avg_ind] = current_hr1f;    //overwrite oldest sample with new sample
        sum_hr += hr_avgs[hr_avg_ind]; //add new sample to sum
        avg_hr = (num_hr_avgs > 0) ? (sum_hr / num_hr_avgs) : sum_hr;
    } else {
        avg_hr = 0;
    }
    if(current_spo21f) {
        if(first_spo2) { //initalize average
            //pc.printf("init spo2 avg\r\n");
            sum_spo2 = 0;
            num_spo2_avgs = 1;
            first_spo2 = 0;
            spo2_avg_ind = 0;

        } else { //increment index and num avgs
            num_spo2_avgs++;
            spo2_avg_ind++;
            spo2_avg_ind %= NUM_SPO2_AVGS;
        }
        if(num_spo2_avgs > NUM_SPO2_AVGS) { //subtract oldest sample if number of averages is greater than max
            num_spo2_avgs = NUM_SPO2_AVGS;
            sum_spo2 -= spo2_avgs[spo2_avg_ind];
        }
        spo2_avgs[spo2_avg_ind] = current_spo21f;    //overwrite oldest sample with new sample
        sum_spo2 += spo2_avgs[spo2_avg_ind]; //add new sample to sum
        avg_spo2 = (num_spo2_avgs > 0) ? (sum_spo2 / num_spo2_avgs) : sum_spo2;
    } else {
        avg_spo2 = 0;
    }
    pc.printf("%.2f, %.2f, %.2f, %.2f, %.4f\r\n",(float)avg_spo2/(float)(1<<FIXED_BITS),(float)avg_hr/(float)(1<<FIXED_BITS),(float)current_spo21f/float(1<<FIXED_BITS),(float)current_hr1f/(float)(1<<FIXED_BITS),R);
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
}




void SPO2::consensus()
{
    //Compares 3 values. If latest 2 samples have big differense, it carries over previous avg. If prev difference is large it uses latest average.
//If both differences are large, it uses the latest value. If only the middle value is off, it drops it.
    static int32_t hr_rms_buffer[3];
    static int32_t spo2_rms_buffer[3];
    static uint8_t hr_rms_ind = 0;
    static uint8_t spo2_rms_ind =0;
    static uint8_t n_hr_samples = 0;
    static uint8_t n_spo2_samples = 0;
    uint8_t ind2;
    uint8_t ind1;
    int32_t prev_diff;
    int32_t prev_avg;
    static int32_t hr_diff;
    static int32_t spo2_diff;
    static int32_t hr_avg;
    static int32_t spo2_avg;
    int32_t diff_from_oldest;
    int32_t oldest_avg;
    uint8_t carryover;
    //pc.printf("n_samples = %d, %d\r\n",n_hr_samples,n_spo2_samples);


    //begin spo2 analysis
    if(n_spo2_samples==0 ) spo2_rms_ind = 2;
    spo2_rms_ind = (spo2_rms_ind+1)%3;
    spo2_rms_buffer[spo2_rms_ind] = current_spo21f;
    if(n_spo2_samples ==1) {
        ind2 = (hr_rms_ind+2)%3;
        spo2_diff = spo2_rms_buffer[spo2_rms_ind] - spo2_rms_buffer[ ind2];
        spo2_avg = spo2_rms_buffer[ind2] + spo2_diff/2;
        if( (spo2_diff /spo2_avg) < MAX_SPO2_DROP1f ) { //only consider large drops becuase large rises are biologically possible in one heart beat
            first_spo2 =1; //reset with current value
        }
    } else if(n_spo2_samples == 2) {
        prev_diff = spo2_diff;
        prev_avg = spo2_avg;
        ind2 = (spo2_rms_ind+2)%3;
        ind1 = (spo2_rms_ind+1)%3;
        spo2_diff = spo2_rms_buffer[spo2_rms_ind] - spo2_rms_buffer[ind2]; //newest - previous
        spo2_avg = spo2_rms_buffer[ind2] + spo2_diff/2;
        diff_from_oldest = spo2_rms_buffer[spo2_rms_ind] - spo2_rms_buffer[ind1]; //newest - oldest
        oldest_avg = spo2_rms_buffer[ind1] + diff_from_oldest/2;

        if ( (prev_avg == 0) || (hr_avg == 0) ) {
            first_spo2 = 1;
            //pc.printf("use latest spo2--00s, ");
        } else {
            if(spo2_diff/spo2_avg < MAX_SPO2_DROP1f) {
                if( (prev_diff/prev_avg) > MAX_HR_CHANGE1f) {
                    carryover = USE_LATEST; //use last sample
                    first_spo2 = 1;
                    //pc.printf("use latest spo2, ");
                } else {
                    carryover = USE_PREV; //use first sample
                    current_spo21f = prev_avg;
                    //pc.printf("drop latest spo2, ");
                }
                if( (carryover == USE_LATEST) && ( (diff_from_oldest/oldest_avg) < MAX_HR_CHANGE1f) ) {
                    carryover = DROP_MIDDLE; // drop middle sample
                    current_spo21f = (spo2_rms_buffer[spo2_rms_ind] + spo2_rms_buffer[ind1] ) /2; //avg of oldest and newest
                    first_spo2 = 1;
                    //pc.printf("and first spo2, ");
                }
            } else if( (prev_diff/prev_avg) > MAX_HR_CHANGE1f) {
                carryover = USE_LATEST_AVG;
                current_spo21f = spo2_avg;
                first_spo2 = 1;
                //pc.printf("drop first spo2, ");
            } else {
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
    hr_rms_buffer[hr_rms_ind] = current_hr1f;//add to three sample buffer

    if(n_hr_samples ==1) {
        ind2 = (hr_rms_ind+2)%3;
        hr_diff = hr_rms_buffer[hr_rms_ind] - hr_rms_buffer[ind2];
        hr_avg = hr_rms_buffer[ind2] + hr_diff/2;
        hr_diff = abs(hr_diff); //absolute value
        if( (hr_diff /hr_avg) > MAX_HR_CHANGE1f ) {
            first_hr =1; //reset with current value
        }
    } else if(n_hr_samples == 2) {
        prev_diff = hr_diff;
        prev_avg = hr_avg;
        ind2 = (hr_rms_ind+2)%3;
        ind1 = (hr_rms_ind+1)%3;
        hr_diff = hr_rms_buffer[hr_rms_ind] - hr_rms_buffer[ind2]; //newest - previous
        diff_from_oldest = hr_rms_buffer[hr_rms_ind] - hr_rms_buffer[ind1]; //newest - oldest
        hr_avg = hr_rms_buffer[ind2] + hr_diff/2;
        oldest_avg = hr_rms_buffer[ind1] + diff_from_oldest/2;
        hr_diff = abs(hr_diff); //absolute value
        diff_from_oldest = abs(diff_from_oldest);

        if( (prev_avg == 0) || (hr_avg == 0) ) {
            first_hr = 1;
            //pc.printf("use latest hr--00s\r\n");
            if(hr_avg == 0); //attempt to reaquire a lost signal with a typical guess.
        } else {
            if( (hr_diff/hr_avg) > MAX_HR_CHANGE1f ) {
                if( (prev_diff/prev_avg) > MAX_HR_CHANGE1f) {
                    carryover = USE_LATEST; //use most recent sample
                    first_hr = 1;
                    //pc.printf("use latest hr");
                } else {
                    //pc.printf("drop latest hr, ");
                    carryover = USE_PREV; //use first sample (drops current from average)
                    current_hr1f = prev_avg;
                }
                if( (carryover == USE_LATEST) && ( (diff_from_oldest/oldest_avg) < MAX_HR_CHANGE1f) ) {
                    carryover = DROP_MIDDLE; // drop middle sample
                    current_hr1f = (hr_rms_buffer[hr_rms_ind] + hr_rms_buffer[ind1] ) /2; //avg of oldest and newest
                    first_hr = 1;
                    // pc.printf("and first hr");
                }
            } else if( (prev_diff/prev_avg) > MAX_HR_CHANGE1f) {
                //pc.printf("drop first hr");
                carryover = USE_LATEST_AVG;
                current_hr1f = hr_avg;
                first_hr = 1;
            } else {
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


//int16_t SPO2::fir( int16_t (*x)[2][NUM_FILTER_TAPS], uint8_t channel, uint16_t ptr, const int32_t *coefs, const uint8_t bit_prec) {
//    int32_t filtered_data = 0;
//    for(uint8_t n=0;n<NUM_FILTER_TAPS;n++) {
////            pc.printf("x = %d\r\n",(*x)[channel][ptr]);
//            filtered_data += coefs[n]*(int32_t)(*x)[channel][ptr]; //incrementing throught the coeficients as we go back through samples.
//            if (ptr==0) {
//                ptr = NUM_FILTER_TAPS-1; //wrap index
//            }
//            else {
//                ptr--;
//            }
//    }
//    filtered_data = filtered_data >> bit_prec;
//    return (int16_t)filtered_data;
//}


void SPO2::copy_data(uint8_t channel)
{
    /*copies all data from the dc_data buffers to temporary buffer and subtracts the DC level
    Output is AC1f-->extended precision array*/
    for (int n=0; n<ARRAY_LENGTH; n++) {
        AC1f[n] = (dc_data[channel][idx[n]]<<FIXED_BITS) - mean1f[channel]; //load the ~11bit or less AC data into an array with fixed precision for DC removal, etc.
    }
}


void SPO2::get_idx()  //creates an array of index pointers mapping 0 to the oldest sample,1 to the next oldest sample, ... ARRAY_LENGTH to the most recent sample.
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
//        pc.printf("channel = %d, DC_data:\r\n",channel);
        mean1f[channel] = 0; //get the running mean
        for (uint16_t n=0; n<ARRAY_LENGTH; n++) {
            mean1f[channel] += dc_data[channel][n];
//            pc.printf("%d\r\n",dc_data[channel][n]);
        }
        mean1f[channel] = (mean1f[channel]<<FIXED_BITS)/ARRAY_LENGTH;
//        pc.printf("channel %d mean = %d\r\n",channel,mean1f[channel]);
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
//        pc.printf("AC1f for channel %d\r\n",channel);
//        avg8Samples();

        //remove slope
        slope1f = 0;
        ind = -(int16_t)SAMPLE_LENGTH;
        for (uint16_t n=0; n<ARRAY_LENGTH; n++) {
            //calc slope
            slope1f += (int32_t)AC1f[n]*(int32_t)ind;
            ind++;
        }
//        pc.printf("slop1ef before divide = %d\r\n",slope1f);
        slope1f /= sum_squares;
//        pc.printf("slope1f = %d\r\n",slope1f);
//        pc.printf("AC1f[n] with slope removed\r\n");
        ind = -(int16_t)SAMPLE_LENGTH;
        for (uint16_t n=0; n<ARRAY_LENGTH; n++) {
            AC1f[n] -= (int32_t)ind*slope1f;
            ind++;
//            pc.printf("%d, %d, %d\r\n",ind,idx[n],AC1f[n]);
        }
        for (uint16_t n=0; n<ARRAY_LENGTH; n++) {
            /*Test whether we need to do the entire array or not.
            Could reduce this if HR is fast.
            If it is too long we risk drift affecting the RMS reading.
            */
            var1f += ( (uint32_t)abs(AC1f[n]>>2 ))*( (uint32_t)abs(AC1f[n]>>2)); //getting 2 bit shifts here, taking out extra bit shift for overhead
//            pc.printf("var1f = %d\r\n",var1f);
        }
        #ifdef PRINT_AC1F
            if(channel == IR) { //print filtered data for IR channel
                for (int n = 0; n<ARRAY_LENGTH; n++) {
                    pc.printf("%d\r\n",AC1f[n]);
                }
            }
        #endif
        pc.printf("var1f = %d\r\n",var1f);
        rms1f[channel] = uint_sqrt(var1f/(uint32_t)ARRAY_LENGTH ); //square root halfs the bit shifts back to 2, so this is more like RMS0.5f -- OH WELL (it is 4x bigger, not 16x bigger)
        pc.printf("channel %d, mean1f = %d, rms1f = %d\r\n",channel,mean1f[channel],rms1f[channel]);
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
    pc.printf("R=%f\r\n",R);
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
//        pc.printf("n = %d, c[n] = %f, Rs[n] = %f, term = %f\r\n, current_spo2 = %f",n,c[n],Rs[n],Rs[n]*c[n], spo2);
    }

    pc.printf("spo2 = %.2f\r\n",spo2);
    current_spo21f = (int16_t)(spo2*(1<<FIXED_BITS));
    current_spo21f = ( (current_spo21f>>FIXED_BITS) > 100) ? 100<<FIXED_BITS : current_spo21f;
    current_spo21f = ( (current_spo21f>>FIXED_BITS) < 80) ? 80<<FIXED_BITS : current_spo21f;
}


void SPO2::calc_hr()
{
    if(final_offset1f == 0) {
        current_hr1f = 0;
    } else {
        current_hr1f = ((SAMPLE_RATE_MIN<<FIXED_BITS)<<FIXED_BITS) / final_offset1f;
    }
    pc.printf("HR = %f, %d\r\n",(float)current_hr1f/(float)(1<<FIXED_BITS), current_hr1f);
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
    #ifdef PRINT_RAW
        pc.printf("%d, %d\r\n",dc_data[IR][data_ptr],dc_data[RED][data_ptr]);
    #endif
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
//    pc.printf("offset = %d,corr = %d\r\n",offset,result);
    return result;
}

void SPO2::fine_search(int16_t *x, uint16_t len, uint32_t start_offset, int32_t start_correl, uint32_t search_step)
{
    /*fine search for correlation peak using defined step size (index units).
    Finds peak and interpolates the maximum and saves the answer with fixed precision.
    */
    pc.printf("fine search at %d\r\n",start_offset);

    uint16_t offset = start_offset;
    int32_t c;
    int32_t high_side;
    int32_t low_side;
    int32_t lowest;
    final_correl = start_correl;     //initialize
    final_offset = start_offset;
    while(1) { //search toward lower offset, higher frequency
        offset -= search_step;
        if (offset>=min_offset) {
            c = corr(x,len,offset);
            pc.printf("%d, %d \r\n",offset, c/2500);
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
                pc.printf("%d, %d \r\n",offset, c/2500);
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
//                pc.printf("dtot = %d, d_HL = %d\r\n", final_correl-lowest,high_side-low_side);
            final_offset1f = (final_offset<<FIXED_BITS) + ( ( (((int32_t)search_step)<<FIXED_BITS) /2)*(high_side - low_side) ) / (final_correl-lowest); //interpolate a better answer
        }
    }
//    }
}


bool SPO2::check4max(int16_t *x, uint16_t len,uint16_t start_offset, int32_t start_correl)
{
    //check for a max in the correlation in a region
    bool max_found = 0;
    fine_search(x,len,start_offset,start_correl, MID_STEP);
    if ( final_offset1f != (min_offset<<FIXED_BITS)) {
        if(final_offset1f != (max_offset<<FIXED_BITS)) {
            max_found = 1;
            pc.printf("start = %d, success at %.2f, correl = %d\r\n", start_offset, (float)final_offset1f/(float)(1<<FIXED_BITS), final_correl);
        }
    } else {
        pc.printf("start = %d, fail at %.2f correl = %d\r\n", start_offset, (float)final_offset1f/(float)(1<<FIXED_BITS), final_correl);
    }
    return max_found;
}


bool SPO2::find_max_corr(int16_t *x, uint16_t max_length, uint16_t offset_guess)
{
    /*Overview: Gets a target using "get_corr_slope"--this is the secret sauce of the algorithm
    Then run a spread of nearby values to jump closer
    Then do a fine search.
    If that bonks, try something near the previous solution (can't start with this or we could get stuck in a local maximum)
    */
    pc.printf("start find max correl\r\n");
//    pc.printf("len = %d, offset_guess = %d\r\n",max_length, offset_guess);
    int32_t start_correl;
    bool max_found;
    
    if(prev_valid) {
        prev_offset = final_offset;
        prev_correl = final_correl;
    }
    
    uint16_t samples2use = max_length;

    //determine how fast autocorrelation is dropping
    //get_corr_slope(x,max_length,0,OFFSET_FOR_SLOPE_CALC);

    //pc.printf("max_offset after get_corr_slope = %d\r\n",max_offset);
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
    c1 = corr(x,samples2use,try_offset);
    try_offset +=BIG_STEP;
    c2 = corr(x,samples2use,try_offset);
    rising = (c2 > c1) ? 1 : 0; //skip to search for a peak if rising, else look for a minimum and double it.
    pc.printf("%d, %d\r\n%d, %d\r\n", MIN_OFFSET-BIG_STEP,c1,MIN_OFFSET,c2);
    if(!rising) {
        while(!rising) { //keep going until you find a minimum
            pc.printf("searching for min\r\n");
            try_offset += BIG_STEP;
            if(try_offset> MAX_OFFSET) {
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
            pc.printf("%d %d\r\n",try_offset,c3);
        }
        if(!fail) {
            highest = (c1 > c3) ? c1 : c3;
            if(c2-highest == 0) {
                offset_guess  = try_offset-BIG_STEP;
            } else {
//                pc.printf("dtot = %d, d_HL = %d\r\n", final_correl-lowest,high_side-low_side);
                d2 =  ( ( ( (int32_t)BIG_STEP<<FIXED_BITS)*(c1 - c3) ) / (highest-c2) )>>FIXED_BITS; //twice the delta
                pc.printf("d2 = %d\r\n",d2);
                offset_guess = (uint16_t)( ( ( (int16_t)try_offset-(int16_t)BIG_STEP)<<1) + (int16_t)d2) ; //interpolate a better answer 
                pc.printf("guessing double the min at %d\r\n",offset_guess);
            }
        }
    } else {//already rising
        while(rising) {//keep going until you find a drop
            pc.printf("searching for max\r\n");
            try_offset += BIG_STEP;
            if(try_offset>MAX_OFFSET) {
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
                pc.printf("%d %d\r\n",try_offset,c3);
            }
        }
        if(!fail) {
            lowest = (c3 < c1) ? c3 : c1;
            if(c2-lowest == 0) {
                offset_guess  = try_offset-BIG_STEP;
            } else {
//                pc.printf("dtot = %d, d_HL = %d\r\n", final_correl-lowest,high_side-low_side);
                d2 = ( ( ( (int32_t)BIG_STEP<<FIXED_BITS)*(c3 - c1) ) / (c2-lowest) )>>(FIXED_BITS+1);
                pc.printf("d2 = %d\r\n",d2);
                offset_guess = (uint16_t)( ( (int16_t)try_offset-(int16_t)BIG_STEP) + (int16_t)d2) ; //interpolate a better answer 
                pc.printf("guessing near the max at %d\r\n",offset_guess);
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
        pc.printf("trying prev sol'n\r\n");
        max_found = check4max(x, samples2use, prev_offset, prev_correl);
    }

    if(!max_found) {
        final_correl = 0;
        final_offset1f = 0;
        prev_valid = 0;
    } else {
        prev_valid = 1;
    }
//pc.printf("final_offset %0.1f\r\n",final_offset);
    return max_found;
}