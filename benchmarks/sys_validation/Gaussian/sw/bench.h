#include "../defines.h"

extern volatile int stage;

typedef struct {
    double * gaus;
    int n;
} gaus_struct;

void genData(gaus_struct * ges) {
    int r, c;

    for( r=0; r < ges->n; r++ ) {
        for( c=0; c < ges->n; c++ ) {
            ges->gaus[r*ges->n+c] = r*ges->n+c;
        }
    }
}

