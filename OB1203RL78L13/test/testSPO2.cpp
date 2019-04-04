/* Copyright (c) 2010 James Grenning and Contributed to Unity Project
 * ==========================================
 *  Unity Project - A Test Framework for C
 *  Copyright (c) 2007 Mike Karlesky, Mark VanderVoord, Greg Williams
 *  [Released under MIT License. Please refer to license.txt for details]
 * ========================================== */

#include "unity_fixture.h"

#include "SPO2.h"

#define DIMENSION_OF(x)         (sizeof(x)/sizeof(x[0]))

   

typedef unsigned long long      uint64_t;
typedef signed long long        int64_t;

TEST_GROUP(SPO2_TG1);

TEST_SETUP(SPO2_TG1)
{
    
}

TEST_TEAR_DOWN(SPO2_TG1)
{
    
}

TEST(SPO2_TG1, test_Initialization)
{
    SPO2 spo2;
    uint32_t zero_initialized_array[ARRAY_LENGTH];
    
    for(uint16_t itr = 0; itr < DIMENSION_OF(zero_initialized_array); itr++)
    {
      zero_initialized_array[itr] = 0;
    }
    
    /* If any of these tests fail, it implies that the SPO2 object is not sufficiently initialized */
    TEST_ASSERT_EQUAL_MEMORY_ARRAY(zero_initialized_array, &spo2.dc_data[0][0], sizeof(spo2.dc_data[0][0]),DIMENSION_OF(spo2.dc_data[0]));
    TEST_ASSERT_EQUAL_MEMORY_ARRAY(zero_initialized_array, &spo2.dc_data[1][0], sizeof(spo2.dc_data[1][0]),DIMENSION_OF(spo2.dc_data[1]));
    TEST_ASSERT_EQUAL_MEMORY_ARRAY(zero_initialized_array, &spo2.idx[0], sizeof(spo2.idx[0]),DIMENSION_OF(spo2.idx));
    TEST_ASSERT_EQUAL_MEMORY_ARRAY(zero_initialized_array, &spo2.AC1f[0], sizeof(spo2.AC1f[0]),DIMENSION_OF(spo2.AC1f));
    TEST_ASSERT_EQUAL_MEMORY_ARRAY(zero_initialized_array, &spo2.mean1f[0], sizeof(spo2.mean1f[0]),DIMENSION_OF(spo2.mean1f));
    TEST_ASSERT_EQUAL_MEMORY_ARRAY(zero_initialized_array, &spo2.rms1f[0], sizeof(spo2.rms1f[0]),DIMENSION_OF(spo2.rms1f));
    TEST_ASSERT_EQUAL_MEMORY_ARRAY(zero_initialized_array, &spo2.rms_float[0], sizeof(spo2.rms_float[0]),DIMENSION_OF(spo2.rms_float));
}

TEST(SPO2_TG1, test_uint_sqrt32)
{
    SPO2 spo2;
    const uint16_t count = 64;
    const uint16_t starting_integer = ((~0)>>1);
    
    for(uint16_t itr = starting_integer; itr >= (uint32_t)(starting_integer-count); itr--)
    {
      uint32_t result = ((uint32_t)itr*(uint32_t)itr);
      uint32_t computed = spo2.uint_sqrt(result);
      char str_itr[sizeof(uint32_t)*2+1];
      sprintf(str_itr, "%u", itr);
      TEST_ASSERT_EQUAL_UINT32_MESSAGE(itr, computed, str_itr);
    }
}

TEST(SPO2_TG1, test_uint_sqrt64)
{
    SPO2 spo2;
    const uint16_t count = 64;
    const uint32_t starting_integer = ((~0)>>1);
    
    for(uint32_t itr = starting_integer; itr >= (uint32_t)(starting_integer-count); itr--)
    {
      uint64_t result = (uint64_t)itr*(uint64_t)itr;
      uint32_t computed = spo2.uint_sqrt(result);
      char str_itr[sizeof(uint64_t)*2+1];
      sprintf(str_itr, "%lu", itr);
      TEST_ASSERT_EQUAL_UINT32_MESSAGE(itr, computed, str_itr);
    }
}
TEST(SPO2_TG1, test_get_avg)
{
    SPO2 spo2;
    
    /* Using =RANDBETWEEN(0, 4294967296) in an excel sheet for 8 columns and =AVERAGE(B4:I4) in 9th column */
    const uint32_t rand_numbers_w_avg[][NUM_HR_AVGS+1] = 
    {
        {1732641646	, 969794403	, 1788135717	, 3039241019	, 3992486493	, 859647062	, 425516952	, 3094657926		, 1987765152},
        {2817749843	, 3660776999	, 1012869876	, 1010947202	, 304279251	, 1976862489	, 36222553	, 45843774		, 1358193998},
        {1402399809	, 504378864	, 2376768310	, 3819220913	, 949276726	, 2685722204	, 3270030872	, 646770985		, 1956821085},
        {1738435820	, 553428617	, 1027705189	, 3758625595	, 1410103895	, 4210721430	, 327153211	, 1463154904		, 1811166083},
        {3081812015	, 3800394799	, 3395788889	, 1441854106	, 896698314	, 1312742488	, 1000468338	, 1702584987		, 2079042992},
        {1302029506	, 1826459612	, 3481737244	, 1062604250	, 3435361614	, 2344233912	, 2160662924	, 3020440299		, 2329191170},
        {645662190	, 1448571980	, 1789523607	, 1949034181	, 2256324953	, 399046789	, 4064573426	, 933213973		, 1685743887},
        {142147486	, 1426058929	, 126429488	, 379266228	, 2025667392	, 991260195	, 2948401915	, 3914381025		, 1494201582},
        {1783444984	, 862659769	, 752492526	, 3429414699	, 1801215177	, 3562251481	, 445638482	, 479924746		, 1639630233},
        {2218020969	, 2940765372	, 3855011547	, 436791128	, 4059715834	, 2876237399	, 2934814332	, 4264101648		, 2948182279},
        {746819167	, 2844586958	, 1356668412	, 1625820940	, 2412593293	, 3029616488	, 2422243167	, 3762397592		, 2275093252},
        {3355011326	, 2452645464	, 1093525512	, 2752157424	, 541548158	, 3663790675	, 4071784689	, 882279747		, 2351592874},
        {2015077976	, 1360408428	, 4270419326	, 2079536273	, 2335969761	, 536553684	, 196530168	, 4036756486		, 2103906513},
        {196079575	, 375426116	, 731989780	, 1978163108	, 1004311435	, 2567202045	, 680176107	, 1848100525		, 1172681086},
        {1421518936	, 2280065471	, 3311389112	, 466799725	, 3697162542	, 4286432239	, 23682548	, 389374924		, 1984553187},
        {4096614922	, 519118867	, 1011880435	, 10419714	, 3115482742	, 3807874951	, 3826725297	, 4203456273		, 2573946650},
        {4258502544	, 499632546	, 1463639506	, 1109332135	, 2770475433	, 3511100030	, 2035307025	, 3061548061		, 2338692160},
        {1596112203	, 2340340361	, 304113623	, 2661152668	, 3296454898	, 3194440682	, 1340087160	, 2424074310		, 2144596988},
        {3606252928	, 1339005330	, 3774632458	, 2603378551	, 3941368155	, 4258214549	, 4077255993	, 4179525895		, 3472454232},
        {2673375489	, 1392390664	, 2794742827	, 2077388333	, 716391892	, 3235979981	, 1329727895	, 3052346711		, 2159042974},
        {2953785900	, 3129283634	, 519677850	, 186535277	, 2284930296	, 1313933531	, 4211088540	, 2955672081		, 2194363389},
        {2281032715	, 2046338037	, 1173739063	, 2681572490	, 3226643566	, 1507414846	, 3032967631	, 3834382087		, 2473011304},
        {4110573960	, 3770362211	, 703412087	, 889383805	, 649902039	, 1111677021	, 2198152690	, 1001096923		, 1804320092}, 
    };
    
    for(uint16_t itr = 0; itr < DIMENSION_OF(rand_numbers_w_avg); itr++)
    {
      uint32_t result = rand_numbers_w_avg[itr][NUM_HR_AVGS];
      uint32_t computed = spo2.get_avg((uint32_t*)&rand_numbers_w_avg[itr][0], NUM_HR_AVGS);
      char str_itr[sizeof(uint32_t)*2+1];
      sprintf(str_itr, "%u", itr);
      /* The function will always return a value within tolerance because we lose bits in integer math. */
      TEST_ASSERT_LESS_OR_EQUAL_UINT32_MESSAGE(result, computed, str_itr);
      TEST_ASSERT_UINT32_WITHIN_MESSAGE((0.5*(NUM_HR_AVGS)+1), result, computed, str_itr);
    }
}

TEST(SPO2_TG1, test_get_std)
{
    SPO2 spo2;
    
    /* Using =RANDBETWEEN(0, 4294967296) in an excel sheet for 8 columns with =AVERAGE(B4:I4) in 9th column and =STDEVA(B1:I1) in 10th column, and Evaluation using integeres in 11th column */
    const uint32_t rand_numbers_w_std[][NUM_HR_AVGS+3] = 
    {
        { 487188663	, 1223984574	, 3593552139	, 2625025422	, 1008064974	, 3456393511	, 2593704069	, 759444138	, 1968419686	, 1242656445		, 1162398666 },
        { 2209436255	, 2356141772	, 3751237606	, 2145232735	, 2542513403	, 3660989542	, 2434041847	, 4005983040	, 2888197025	, 775798381		, 725692935  },
        { 1033747177	, 2243277365	, 4153859383	, 824166751	, 656811702	, 2328595129	, 1204772413	, 1451607847	, 1737104721	, 1153061854		, 1078590601 },
        { 470566318	, 1877436429	, 476294737	, 671295888	, 1697951559	, 3959442890	, 4218444208	, 2408682186	, 1972514277	, 1485859221		, 1389894033 },
        { 361956121	, 745775758	, 2632348884	, 3064043490	, 3962302957	, 3506085741	, 4041817177	, 3811400677	, 2765716351	, 1447503495		, 1354015536 },
        { 2271501059	, 2689111133	, 527345443	, 2314236430	, 2113658174	, 3104917556	, 2033096434	, 1135344934	, 2023651395	, 827565499		, 774116641  },
        { 1890197587	, 2125451452	, 2457005600	, 1464026985	, 1332739064	, 3419906043	, 3264028307	, 1169128360	, 2140310425	, 854893917		, 799680035  },
        { 4265956869	, 2693529133	, 3005600497	, 1982778167	, 857541271	, 3206994789	, 2441888708	, 3931882347	, 2798271473	, 1083556102		, 1013573924 },
        { 66542426	, 1240625084	, 120010194	, 2209903500	, 3481674217	, 390546149	, 1902806102	, 2173537737	, 1448205676	, 1212009900		, 1133731449 },
        { 1497140431	, 3982599623	, 1616992327	, 735455063	, 679282259	, 4010917700	, 3262243439	, 4118459205	, 2487886256	, 1506855147		, 1409533923 },
        { 2664539592	, 1869037241	, 3616506095	, 1144688949	, 2926651006	, 2148131697	, 2562071717	, 3350086997	, 2535214162	, 804546373		, 752584220  },
        { 2096866998	, 3761406060	, 1163245162	, 915615547	, 698823928	, 223549952	, 2507141735	, 3265660316	, 1829038712	, 1279625137		, 1196979712 },
        { 3803248284	, 258400765	, 3337685524	, 1250824271	, 2802711217	, 175967586	, 1728290966	, 1147534901	, 1813082939	, 1369462736		, 1281015090 },
        { 105117816	, 592470490	, 826660666	, 2923027360	, 2160504528	, 3775801578	, 2567393668	, 2812042464	, 1970377321	, 1306794550		, 1222394370 },
        { 946537964	, 2448934611	, 1939082307	, 90475044	, 4150288845	, 3758797917	, 2264483410	, 104217225	, 1962852165	, 1527431897		, 1428781711 },
        { 874939686	, 2130997930	, 3597760193	, 4279346966	, 3888193775	, 1869214897	, 3557454757	, 2631481953	, 2853673770	, 1171563388		, 1095897201 },
        { 939527389	, 3005672348	, 3360354945	, 4115585050	, 630683654	, 3569294645	, 715038778	, 3756441463	, 2511574784	, 1485284943		, 1389356844 },
        { 326478517	, 3965562804	, 1675953447	, 1559253098	, 3283786372	, 1783694046	, 1348991377	, 1660758670	, 1950559791	, 1145027338		, 1071075000 },
        { 2698714703	, 2803276465	, 120535023	, 2431910193	, 2368336339	, 2295388576	, 1304327925	, 4061160999	, 2260456278	, 1150463318		, 1076159893 },
        { 3309398766	, 1433618218	, 2935560467	, 19145087	, 1788399038	, 2401516190	, 1800989874	, 2387262803	, 2009486305	, 1016241031		, 950606440  },
        { 3744602505	, 1392329690	, 842981273	, 3292928399	, 2047002633	, 2193780106	, 536108780	, 3938418463	, 2248518981	, 1302161249		, 1218060314 },
        { 2009872249	, 4148619344	, 3409799792	, 1720266710	, 548513630	, 241935445	, 2933943855	, 2003680851	, 2127078985	, 1342871364		, 1256141140 },
        { 1710228761	, 330828461	, 2909129778	, 1536844172	, 2285802212	, 4187230267	, 773509020	, 1511780471	, 1905669143	, 1221722576		, 1142816825 },
    };
    
    for(uint16_t itr = 0; itr < DIMENSION_OF(rand_numbers_w_std); itr++)
    {
      uint32_t result = rand_numbers_w_std[itr][NUM_HR_AVGS+2];
      uint32_t computed = spo2.get_std((uint32_t*)&rand_numbers_w_std[itr][0], NUM_HR_AVGS, rand_numbers_w_std[itr][NUM_HR_AVGS]);
      const char str_fix[] = "failed with itr = %u";
      char str_itr[sizeof(str_fix) + sizeof(uint32_t)*2+1];
      sprintf(str_itr, str_fix, itr);
      /* The function will always return a value within tolerance because we lose bits in integer math. */
      TEST_ASSERT_LESS_OR_EQUAL_UINT32_MESSAGE(result, computed, str_itr);
      TEST_ASSERT_UINT32_WITHIN_MESSAGE((0.5*(NUM_HR_AVGS)+1), result, computed, str_itr);
    }
}