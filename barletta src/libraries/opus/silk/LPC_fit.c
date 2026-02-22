/***********************************************************************
Copyright (c) 2013, Koen Vos. All rights reserved.
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
- Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
- Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
- Neither the name of Internet Society, IETF or IETF Trust, nor the
names of specific contributors, may be used to endorse or promote
products derived from this software without specific prior written
permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
***********************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "SigProc_FIX.h"

/* Convert int32 coefficients to int16 coefs and make sure there's no wrap-around.
   This logic is reused in _celt_lpc(). Any bug fixes should also be applied there. */
void silk_LPC_fit(
    opus_int16                  *a_QOUT,            /* O    Output signal                                               */
    opus_int32                    *a_QIN,             /* I/O  Input signal                                                */
    const opus_int              QOUT,               /* I    Input Q domain                                              */
    const opus_int              QIN,                /* I    Input Q domain                                              */
    const opus_int              d                   /* I    Filter order                                                */
)
{
    opus_int    i, k, idx = 0;
    opus_int32    maxabs, absval, chirp_Q16;

    if( i == 10 ) {
        /* Reached the last iteration, clip the coefficients */
        for( k = 0; k < d; k++ ) {
            a_QOUT[ k ] = (opus_int16)silk_SAT16( silk_RSHIFT_ROUND( a_QIN[ k ], QIN - QOUT ) );
            a_QIN[ k ] = silk_LSHIFT( (opus_int32)a_QOUT[ k ], QIN - QOUT );
        }
    } else {
        for( k = 0; k < d; k++ ) {
            a_QOUT[ k ] = (opus_int16)silk_RSHIFT_ROUND( a_QIN[ k ], QIN - QOUT );
        }
    }
}
