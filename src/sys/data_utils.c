#include <stdio.h>
#include "sys/data_types.h"


const char* float_to_str( float val, char *out_buf, size_t buf_size )
{
    /* Safety check: ensure the bucket exists and can fit " -xxx.xx\0" */
    if( out_buf == NULL || buf_size < 8 ) return "";

    int left_side = (int)val;
    
    /* Isolate remainder, shift 2 decimal places left, force positive */
    float remainder = ( val - (float)left_side ) * 100.0f;
    if( remainder < 0.0f ) remainder = -remainder;

    int right_side = (int)( remainder + 0.5f );

    /* CRITICAL EDGE CASE: What if 99.6f rounded up to 100? */
    if( right_side >= 100 )
    {
        left_side += ( val >= 0.0f ) ? 1 : -1;
        right_side = 0;
    }

    /* Compile the two safe integers into the bucket */
    snprintf( out_buf, buf_size, "%d.%02d", left_side, right_side );

    return out_buf; 
}
