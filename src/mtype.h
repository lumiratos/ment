/*------------------------------------------------------------------------------

Copyright 2000-2003 Armando J. Pinho (ap@det.ua.pt), All Rights Reserved.

These programs are supplied free of charge for research purposes only,
and may not be sold or incorporated into any commercial product. There is
ABSOLUTELY NO WARRANTY of any sort, nor any undertaking that they are
fit for ANY PURPOSE WHATSOEVER. Use them at your own risk. If you do
happen to find a bug, or have modifications to suggest, please report
the same to Armando J. Pinho, ap@det.ua.pt. The copyright notice above
and this statement of conditions must remain an integral part of each
and every copy made of these files.

Modified:
Tue Nov  4 14:13:34 WET 2003

------------------------------------------------------------------------------*/

#ifndef MTYPE_INCLUDED
#define MTYPE_INCLUDED
#include "complex.h"

#define DATA_TYPE_C    1 /* unsigned char */
#define DATA_TYPE_S    2 /* short */
#define DATA_TYPE_I    4 /* int */
#define DATA_TYPE_D    8 /* double */
#define DATA_TYPE_X   16 /* complex */

typedef unsigned char  uChar;
typedef unsigned short uShort;
typedef unsigned int   uInt;

typedef struct {
	void	*data;
	int		dataType;
} Mtype;

void MtypeToDouble(Mtype *x, double *d);
void DoubleToMtype(Mtype *x, double d);
void MtypeToComplex(Mtype *x, Complex *d);
void ComplexToMtype(Mtype *x, Complex d);
void MtypeToMtype(Mtype *x1, Mtype *x2);
void *AddMtype(Mtype *x1, Mtype *x2, Mtype *y);
void *MultiplyMtype(Mtype *x1, Mtype *x2, Mtype *y);
void *SubtractMtype(Mtype *x1, Mtype *x2, Mtype *y);
void *DivideMtype(Mtype *x1, Mtype *x2, Mtype *y);

#endif /* MTYPE_INCLUDED */

