#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "bench.h"
#include "../../../common/m5ops.h"
#include "../gaussian_clstr_hw_defines.h"

gaus_struct ges;


volatile uint8_t  * top   = (uint8_t  *)(TOP + 0x00);
volatile uint32_t * val = (uint32_t *)(TOP + 0x01);

int __attribute__ ((optimize("0"))) main(void) {
	m5_reset_stats();
    uint32_t base = 0x80c00000;
	double *m = (double *)base;
	double *check = (double *)(base+8*N*N);

    volatile int count = 0;
	stage = 0;

    ges.gaus = m;
    ges.n = N;

    printf("Generating data\n");
    genData(&ges);
    printf("Data generated\n");

    *val = (uint32_t)(void *)m;

    // printf("%d\n", *top);
    *top = 0x01;
    while (stage < 1) count++;

    printf("Job complete\n");
#ifdef CHECK
    printf("Checking result\n");
    printf("Running bench on CPU\n");
	bool fail = false;
	int i, j, i_col;
    double sigma=1;
	double sum = 0.0;
	double mult = 0.0;
	for(i=0;i<N;i++) {
        for(j=0;j<N;j++) {
            double x = -((i-N/2)*(i-N/2)+(j-N/2)*(j-N/2))/(2.0*sigma*sigma);
            int ii;
            double result = 1.0;
            double numerator = 1.0, denominator = 1.0;
            for(ii = 0; ii <= 50; ii++)
            {
                numerator *= x;
                denominator *= ii;
                result += numerator / denominator;
            }
            i_col = i * N;
            check[i_col + j] = result;
            sum+=check[i_col + j];
        }
    }
    for(i=0;i<N;i++) {
        for(j=0;j<N;j++) {
            i_col = i * N;
            check[i_col + j] = check[i_col + j]/sum;
        }
    }
    printf("Comparing CPU run to accelerated run\n");
    for(i=0; i<N*N; i++) {
        if(m[i] != check[i]) {
            printf("Expected:%f Actual:%f\n", check[i], m[i]);
            fail = true;
            break;
        }
    }
    if(fail)
        printf("Check Failed\n");
    else
        printf("Check Passed\n");
#endif
	m5_dump_stats();
	m5_exit();
}
