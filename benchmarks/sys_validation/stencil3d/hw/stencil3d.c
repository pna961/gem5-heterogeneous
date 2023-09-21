/*
Implementation based on algorithm described in:
"Stencil computation optimization and auto-tuning on state-of-the-art multicore architectures"
K. Datta, M. Murphy, V. Volkov, S. Williams, J. Carter, L. Oliker, D. Patterson, J. Shalf, K. Yelick
SC 2008
*/

#include "hw_defines.h"

void stencil3d() {
    TYPE    * C        = (TYPE    *)C_VAR;
    TYPE    * orig     = (TYPE    *)ORIG;
    TYPE    * sol      = (TYPE    *)SOL;

    int i, j, k;
    TYPE sum0, sum1, mul0, mul1;

    // Handle boundary conditions by filling with original values
    height_bound_col :
    #pragma nounroll
    // #pragma unroll
    for(j=0; j<col_size; j++) {
        height_bound_row :
        #pragma unroll 2
        // #pragma nounroll
        for(k=0; k<row_size; k++) {
            sol[INDX(row_size, col_size, k, j, 0)] = orig[INDX(row_size, col_size, k, j, 0)];
            sol[INDX(row_size, col_size, k, j, height_size-1)] = orig[INDX(row_size, col_size, k, j, height_size-1)];
        }
    }
    col_bound_height :
    #pragma nounroll
    // #pragma unroll
    for(i=1; i<height_size-1; i++) {
        col_bound_row :
        #pragma unroll 2
        // #pragma nounroll
        for(k=0; k<row_size; k++) {
            sol[INDX(row_size, col_size, k, 0, i)] = orig[INDX(row_size, col_size, k, 0, i)];
            sol[INDX(row_size, col_size, k, col_size-1, i)] = orig[INDX(row_size, col_size, k, col_size-1, i)];
        }
    }
    row_bound_height :
    #pragma nounroll
    // #pragma unroll
    for(i=1; i<height_size-1; i++) {
        row_bound_col :
        #pragma unroll 2
        // #pragma nounroll
        for(j=1; j<col_size-1; j++) {
            sol[INDX(row_size, col_size, 0, j, i)] = orig[INDX(row_size, col_size, 0, j, i)];
            sol[INDX(row_size, col_size, row_size-1, j, i)] = orig[INDX(row_size, col_size, row_size-1, j, i)];
        }
    }

    // Stencil computation
    loop_height :
    // #pragma unroll 2
    #pragma nounroll
    for(i = 1; i < height_size - 1; i++){
        loop_col :
        // #pragma unroll 2
        #pragma nounroll
        for(j = 1; j < col_size - 1; j++){
            loop_row :
            // #pragma unroll 2
            #pragma unroll
            for(k = 1; k < row_size - 1; k++){
                sum0 = orig[INDX(row_size, col_size, k, j, i)];
                sum1 = orig[INDX(row_size, col_size, k, j, i + 1)] +
                       orig[INDX(row_size, col_size, k, j, i - 1)] +
                       orig[INDX(row_size, col_size, k, j + 1, i)] +
                       orig[INDX(row_size, col_size, k, j - 1, i)] +
                       orig[INDX(row_size, col_size, k + 1, j, i)] +
                       orig[INDX(row_size, col_size, k - 1, j, i)];
                mul0 = sum0 * C[0];
                mul1 = sum1 * C[1];
                sol[INDX(row_size, col_size, k, j, i)] = mul0 + mul1;
            }
        }
    }
}
