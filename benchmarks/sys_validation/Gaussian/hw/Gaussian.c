#include "hw_defines.h"

void Gaussian()
{
    int i,j;
    double sigma=1;
    double *gussian = (double *)Gaussian_matrix;
    double sum=0.0;

    for(i=0;i<N;i++)
    {
        #pragma unroll
        for(j=0;j<N;j++)
        {
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
            gussian[i*N+j]=result;
            sum+=gussian[i*N+j];
        }
    }

    for(i=0;i<N;i++)
    {
        for(j=0;j<N;j++)
        {
            gussian[i*N+j]=gussian[i*N+j]/sum;
        }
    }
}