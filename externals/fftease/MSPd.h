#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

/* choose your poison */

#define MSP (0)
#define PD (!MSP)

#if MSP
#include "ext.h"
#include "z_dsp.h"
#define t_floatarg double
#endif

#if PD
#include "m_pd.h"
// #define t_floatarg float // not needed?
#endif

/* because Max and Pd have different ideas of what A_FLOAT is, use t_floatarg
to force consistency. Otherwise functions that look good will fail on some
hardware. Also note that Pd messages cannot accept arguments of type A_LONG. */