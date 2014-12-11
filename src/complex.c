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
Tue Nov  4 14:12:22 WET 2003

------------------------------------------------------------------------------*/

#include "complex.h"
#include <math.h>

/*==============================================================================
	Returns the conjugate of a complex number.
------------------------------------------------------------------------------*/

Complex ConjComplex(Complex x)

	{
	x.im = -x.im;

	return(x);
	}

/*==============================================================================
	Returns the sum of two complex numbers.
------------------------------------------------------------------------------*/

Complex AddComplex(Complex x, Complex y)

	{
	x.re += y.re;
	x.im += y.im;

	return(x);
	}

/*==============================================================================
	Returns the difference of two complex numbers.
------------------------------------------------------------------------------*/

Complex SubComplex(Complex x, Complex y)

	{
	x.re -= y.re;
	x.im -= y.im;

	return(x);
	}

/*==============================================================================
	Returns the multiplication of two complex numbers.
------------------------------------------------------------------------------*/

Complex MultComplex(Complex x, Complex y)

	{
	Complex z;

	z.re = x.re * y.re - x.im * y.im;
	z.im = x.re * y.im + x.im * y.re;

	return(z);
	}

/*==============================================================================
	Returns the exponential of a complex number.
------------------------------------------------------------------------------*/

Complex ExpComplex(Complex x)

	{
	Complex z;

	z.re = z.im = exp(x.re);
	z.re *= cos(x.im);
	z.im *= sin(x.im);

	return(z);
	}

/*==============================================================================
	Returns the square module of a complex number.
------------------------------------------------------------------------------*/

double SqrModComplex(Complex x)

	{
	return(x.re * x.re + x.im * x.im);
	}

/*==============================================================================
	Returns the module of a complex number.
------------------------------------------------------------------------------*/

double ModComplex(Complex x)

	{
	return(sqrt(SqrModComplex(x)));
	}

/*==============================================================================
	Returns the phase of a complex number.
------------------------------------------------------------------------------*/

double PhaseComplex(Complex x)

	{
	return(atan2(x.im, x.re));
	}

/*==============================================================================
	Returns the quotient of two complex numbers (x/y).
------------------------------------------------------------------------------*/

Complex DivComplex(Complex x, Complex y)

	{
	Complex z;
	double mod2y;

	z = MultComplex(x, ConjComplex(y));
	mod2y = SqrModComplex(y);
	z.re /= mod2y;
	z.im /= mod2y;

	return(z);
	}

