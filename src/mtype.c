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
Tue Nov  4 14:13:20 WET 2003

------------------------------------------------------------------------------*/

#include "mtype.h"
#include "complex.h"
#include <math.h>

/*============================================================================*/

void MtypeToDouble(Mtype *x, double *d)

	{

	switch(x->dataType)
		{
		case DATA_TYPE_C:
			*d = (double)(*((uChar *)x->data));
			return;

		case DATA_TYPE_S:
			*d = (double)(*((uShort *)x->data));
			return;

		case DATA_TYPE_I:
			*d = (double)(*((int *)x->data));
			return;

		case DATA_TYPE_D:
			*d = (double)(*((double *)x->data));
			return;

		}

	}

/*============================================================================*/

void DoubleToMtype(Mtype *x, double d)

	{

	switch(x->dataType)
		{
		case DATA_TYPE_C:
			*((uChar *)x->data) = (uChar)d;
			return;

		case DATA_TYPE_S:
			*((uShort *)x->data) = (uShort)d;
			return;

		case DATA_TYPE_I:
			*((int *)x->data) = (int)d;
			return;

		case DATA_TYPE_D:
			*((double *)x->data) = (double)d;
			return;

		case DATA_TYPE_X:
			((Complex *)x->data)->re = (double)d;
			((Complex *)x->data)->im = 0;
			return;

		}

	}

/*============================================================================*/

void MtypeToComplex(Mtype *x, Complex *c)

	{

	switch(x->dataType)
		{
		case DATA_TYPE_C:
			c->re = (double)(*((uChar *)x->data));
			c->im = 0;
			return;

		case DATA_TYPE_S:
			c->re = (double)(*((uShort *)x->data));
			c->im = 0;
			return;

		case DATA_TYPE_I:
			c->re = (double)(*((int *)x->data));
			c->im = 0;
			return;

		case DATA_TYPE_D:
			c->re = (double)(*((double *)x->data));
			c->im = 0;
			return;

		}

	}

/*============================================================================*/

void ComplexToMtype(Mtype *x, Complex c)

	{

	switch(x->dataType)
		{
		case DATA_TYPE_C:
			*((uChar *)x->data) = (uChar)c.re;
			return;

		case DATA_TYPE_S:
			*((uShort *)x->data) = (uShort)c.re;
			return;

		case DATA_TYPE_I:
			*((int *)x->data) = (int)c.re;
			return;

		case DATA_TYPE_D:
			*((double *)x->data) = (double)c.re;
			return;

		case DATA_TYPE_X:
			((Complex *)x->data)->re = c.re;
			((Complex *)x->data)->im = c.im;
			return;

		}

	}

/*============================================================================*/

void MtypeToMtype(Mtype *x1, Mtype *x2)

	{

	if(x1->dataType != DATA_TYPE_X && x2->dataType != DATA_TYPE_X)
		{
		double d;

		MtypeToDouble(x1, &d);
		DoubleToMtype(x2, d);
		}

	else
		{
		Complex c;

		MtypeToComplex(x1, &c);
		ComplexToMtype(x2, c);
		}

	}

/*============================================================================*/

void *AddMtype(Mtype *x1, Mtype *x2, Mtype *y)

	{

	if(x1->dataType != DATA_TYPE_X && x2->dataType != DATA_TYPE_X)
		{
		double d1, d2;

		MtypeToDouble(x1, &d1);
		MtypeToDouble(x2, &d2);
		DoubleToMtype(y, d1 + d2);
		}

	else
		{
		Complex c1, c2;

		MtypeToComplex(x1, &c1);
		MtypeToComplex(x2, &c2);
		ComplexToMtype(y, AddComplex(c1, c2));
		}

	return(y->data);
	}

/*============================================================================*/

void *MultiplyMtype(Mtype *x1, Mtype *x2, Mtype *y)

	{

	if(x1->dataType != DATA_TYPE_X && x2->dataType != DATA_TYPE_X)
		{
		double d1, d2;

		MtypeToDouble(x1, &d1);
		MtypeToDouble(x2, &d2);
		DoubleToMtype(y, d1 * d2);
		}

	else
		{
		Complex c1, c2;

		MtypeToComplex(x1, &c1);
		MtypeToComplex(x2, &c2);
		ComplexToMtype(y, MultComplex(c1, c2));
		}

	return(y->data);
	}

/*============================================================================*/

void *SubtractMtype(Mtype *x1, Mtype *x2, Mtype *y)

	{

	if(x1->dataType != DATA_TYPE_X && x2->dataType != DATA_TYPE_X)
		{
		double d1, d2;

		MtypeToDouble(x1, &d1);
		MtypeToDouble(x2, &d2);
		DoubleToMtype(y, d1 - d2);
		}

	else
		{
		Complex c1, c2;

		MtypeToComplex(x1, &c1);
		MtypeToComplex(x2, &c2);
		ComplexToMtype(y, SubComplex(c1, c2));
		}

	return(y->data);
	}

/*============================================================================*/

void *DivideMtype(Mtype *x1, Mtype *x2, Mtype *y)

	{

	if(x1->dataType != DATA_TYPE_X && x2->dataType != DATA_TYPE_X)
		{
		double d1, d2;

		MtypeToDouble(x1, &d1);
		MtypeToDouble(x2, &d2);
		DoubleToMtype(y, d1 / d2);
		}

	else
		{
		Complex c1, c2;

		MtypeToComplex(x1, &c1);
		MtypeToComplex(x2, &c2);
		ComplexToMtype(y, DivComplex(c1, c2));
		}

	return(y->data);
	}

