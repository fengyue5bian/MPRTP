
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fec.h"


int
main(int argc, char *argv[])
{
    char buf[256];
    void *code ;
    
    int k = 4, n = 8;
    int i, item, sz = 1;
    
    int index[n], re_index[k] ;


    static u_char **d_original = NULL, **d_src = NULL, **data ;

        d_original = malloc(k * sizeof(void *));
        data = malloc(k * sizeof(void *));
        d_src = malloc(n * sizeof(void *));
        
        for (i = 0 ; i < k ; i++ ) {
            d_original[i] = malloc(sz);
            data[i] = malloc(sz);
        }
        for (i = 0 ; i < n ; i++ ) {
            d_src[i] = malloc(sz);
        }

    code = fec_new(k, n);

    for (i = 0 ; i < k ; i++ ) {
        for (item=0; item < 1; item++) {
            d_original[i][item] = 3+i;
            printf("d_original[%d][%d] = %d, ",i, item, d_original[i][item]);
            printf("\n");
        }
    }

    for( i = 0 ; i < n ; i++ ) {
        index[i] = n-i-1;
        printf("index[%d] = %d \n", i, index[i]);
    }

    for( i = 0 ; i < n ; i++ ) {
        printf("=======================================\n");
        fec_encode(code, d_original, d_src[i], index[i], sz );
        printf("i = %d, d_src = %x, index = %d \n", i, d_src[i][0], index[i]);
        printf("=======================================\n");
    }

    data[0][0] = d_src[0][0];
    data[1][0] = d_src[1][0];
    data[2][0] = d_src[2][0];
    data[3][0] = d_src[3][0];

    re_index[0] = 7;
    re_index[1] = 6;
    re_index[2] = 5;
    re_index[3] = 4;

    for (i = 0 ; i < k ; i++ ) {
        for (item=0; item < 1; item++) {
            printf("before decode data: data[%d][%d] = %d, ",i, item, data[i][item]);
            printf("\n");
        }
    }

    fec_decode(code, data, re_index, sz);

    for (i = 0 ; i < k ; i++ ) {
        for (item=0; item < 1; item++) {
            printf("decode data: data[%d][%d] = %d, ",i, item, data[i][item]);
            printf("\n");
        }
    }

    fec_free(code);
    return 0;
}
