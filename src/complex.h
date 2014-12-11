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
Tue Nov  4 14:12:34 WET 2003

------------------------------------------------------------------------------*/

#ifndef COMPLEX_INCLUDED
#define COMPLEX_INCLUDED

typedef struct
	{
	double re;
	double im;
	}
Complex;

Complex ConjComplex(Complex);
Complex AddComplex(Complex, Complex);
Complex SubComplex(Complex, Complex);
Complex MultComplex(Complex, Complex);
Complex ExpComplex(Complex);
double  SqrModComplex(Complex);
double  ModComplex(Complex);
double  PhaseComplex(Complex);
Complex DivComplex(Complex, Complex);

#endif /* COMPLEX_INCLUDED */

