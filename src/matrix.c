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
Tue Nov  4 14:14:26 WET 2003

------------------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "matrix.h"
#include "mtype.h"

/*============================================================================*/

static void Error(char *prefixErrMesg, char *errMesg)
	{
	fprintf(stderr, "%s: %s.\n", prefixErrMesg, errMesg);
	}

/*============================================================================*/

Matrix *CreateMatrix(int nRows, int nCols, int dataType)

	{
	char *prefixErrMesg = "Error (CreateMatrix)";
	int row;
	Matrix *matrix;

	if(nRows <= 0 || nCols <= 0)
		{
		Error(prefixErrMesg, "wrong parameters");
		return(NULL);
		}

	if(!(matrix = (Matrix *)calloc(1, sizeof(Matrix))))
		{
		Error(prefixErrMesg, "unable to allocate memory");
		return(NULL);
		}

	switch(dataType)
		{
		case DATA_TYPE_C:
			if(!(matrix->data.C = (uChar **)calloc(nRows, sizeof(uChar *))))
				{
				Error(prefixErrMesg, "unable to allocate memory");
				return(NULL);
				}

			for(row = 0 ; row < nRows ; row++)
				if(!(matrix->data.C[row] = (uChar *)calloc(nCols,
				  sizeof(uChar))))
					{
					Error(prefixErrMesg, "unable to allocate memory");
					return(NULL);
					}

			break;

		case DATA_TYPE_S:
			if(!(matrix->data.S = (uShort **)calloc(nRows, sizeof(uShort *))))
				{
				Error(prefixErrMesg, "unable to allocate memory");
				return(NULL);
				}

			for(row = 0 ; row < nRows ; row++)
				if(!(matrix->data.S[row] = (unsigned short *)calloc(nCols,
				  sizeof(unsigned short))))
					{
					Error(prefixErrMesg, "unable to allocate memory");
					return(NULL);
					}

			break;

		case DATA_TYPE_I:
			if(!(matrix->data.I = (int **)calloc(nRows, sizeof(int *))))
				{
				Error(prefixErrMesg, "unable to allocate memory");
				return(NULL);
				}

			for(row = 0 ; row < nRows ; row++)
				if(!(matrix->data.I[row] = (int *)calloc(nCols, sizeof(int))))
					{
					Error(prefixErrMesg, "unable to allocate memory");
					return(NULL);
					}

				break;

		case DATA_TYPE_D:
			if(!(matrix->data.D = (double **)calloc(nRows, sizeof(double *))))
				{
				Error(prefixErrMesg, "unable to allocate memory");
				return(NULL);
				}

			for(row = 0 ; row < nRows ; row++)
				if(!(matrix->data.D[row] = (double *)calloc(nCols,
				  sizeof(double))))
					{
					Error(prefixErrMesg, "unable to allocate memory");
					return(NULL);
					}

				break;

		case DATA_TYPE_X:
			if(!(matrix->data.X = (Complex **)calloc(nRows, sizeof(Complex *))))
				{
				Error(prefixErrMesg, "unable to allocate memory");
				return(NULL);
				}

			for(row = 0 ; row < nRows ; row++)
				if(!(matrix->data.X[row] = (Complex *)calloc(nCols,
				  sizeof(Complex))))
					{
					Error(prefixErrMesg, "unable to allocate memory");
					return(NULL);
					}

				break;

		default:
			Error(prefixErrMesg, "invalid data type");
			return(NULL);
			break;

		}

	matrix->nRows = nRows;
	matrix->nCols = nCols;
	matrix->dataType = dataType;
	return(matrix);
	}

/*============================================================================*/

void *GetPointerToMatrixElement(Matrix *matrix, int row, int col)

	{

	switch(matrix->dataType)
		{
		case DATA_TYPE_C:
			return((void *)(&matrix->data.C[row][col]));
			break;

		case DATA_TYPE_S:
			return((void *)(&matrix->data.S[row][col]));
			break;

		case DATA_TYPE_I:
			return((void *)(&matrix->data.I[row][col]));
			break;

		case DATA_TYPE_D:
			return((void *)(&matrix->data.D[row][col]));
			break;

		case DATA_TYPE_X:
			return((void *)(&matrix->data.X[row][col]));
			break;

		}

	return(NULL);
	}

/*============================================================================*/

void DestroyMatrix(Matrix *matrix)

	{
	int row;

	switch(matrix->dataType)
		{
		case DATA_TYPE_C:
			for(row = 0 ; row < matrix->nRows ; row++)
				free((void *)matrix->data.C[row]);

			free((void *)matrix->data.C);
			break;

		case DATA_TYPE_S:
			for(row = 0 ; row < matrix->nRows ; row++)
				free((void *)matrix->data.S[row]);

			free((void *)matrix->data.S);
			break;

		case DATA_TYPE_I:
			for(row = 0 ; row < matrix->nRows ; row++)
				free((void *)matrix->data.I[row]);

			free((void *)matrix->data.I);
			break;

		case DATA_TYPE_D:
			for(row = 0 ; row < matrix->nRows ; row++)
				free((void *)matrix->data.D[row]);

			free((void *)matrix->data.D);
			break;

		case DATA_TYPE_X:
			for(row = 0 ; row < matrix->nRows ; row++)
				free((void *)matrix->data.X[row]);

			free((void *)matrix->data.X);
			break;

		}

	free((void *)matrix);
	}

/*============================================================================*/

Matrix *TransposeMatrix(Matrix *src, Matrix *dest)

	{
	char *prefixErrMesg = "Error (TransposeMatrix)";
	int row, col;

	if(!dest)
		{
		if(!(dest = CreateMatrix(src->nCols, src->nRows, src->dataType)))
			return(NULL);

		}

	else
		{
		if(dest->nRows != src->nCols || dest->nCols != src->nRows ||
		  dest->dataType != src->dataType)
			{
			Error(prefixErrMesg, "sizes of matrices do not match");
			return(NULL);
			}

		}

	if(src == dest)
		{
		Error(prefixErrMesg, "in-place calculations not supported");
		return(NULL);
		}

    switch(src->dataType)
		{
		case DATA_TYPE_C:
			for(row = 0 ; row < src->nRows ; row++)
				for(col = 0 ; col < src->nCols ; col++)
					dest->data.C[col][row] = src->data.C[row][col];

			break;

		case DATA_TYPE_S:
			for(row = 0 ; row < src->nRows ; row++)
				for(col = 0 ; col < src->nCols ; col++)
					dest->data.S[col][row] = src->data.S[row][col];

			break;

		case DATA_TYPE_I:
			for(row = 0 ; row < src->nRows ; row++)
				for(col = 0 ; col < src->nCols ; col++)
					dest->data.I[col][row] = src->data.I[row][col];

			break;

		case DATA_TYPE_D:
			for(row = 0 ; row < src->nRows ; row++)
				for(col = 0 ; col < src->nCols ; col++)
					dest->data.D[col][row] = src->data.D[row][col];

			break;

		case DATA_TYPE_X:
			for(row = 0 ; row < src->nRows ; row++)
				for(col = 0 ; col < src->nCols ; col++)
					dest->data.X[col][row] = src->data.X[row][col];

			break;

		}

	return(dest);
	}

/*============================================================================*/

void FillMatrixWithRandomValues(Matrix *matrix, double minVal, double maxVal)

	{
	int row, col;

	switch(matrix->dataType)
		{
		case DATA_TYPE_C:
			if(minVal < 0)
				minVal = 0;

			if(maxVal > 255)
				maxVal = 255;

			for(row = 0 ; row < matrix->nRows ; row++)
				for(col = 0 ; col < matrix->nCols ; col++)
					matrix->data.C[row][col] = (uChar)((maxVal - minVal) *
					  drand48() + minVal);

			break;

		case DATA_TYPE_S:
			for(row = 0 ; row < matrix->nRows ; row++)
				for(col = 0 ; col < matrix->nCols ; col++)
					matrix->data.S[row][col] = (uShort)((maxVal - minVal) *
					  drand48() + minVal);

			break;

		case DATA_TYPE_I:
			for(row = 0 ; row < matrix->nRows ; row++)
				for(col = 0 ; col < matrix->nCols ; col++)
					matrix->data.I[row][col] = (int)((maxVal - minVal) *
					  drand48() + minVal);

			break;

		case DATA_TYPE_D:
			for(row = 0 ; row < matrix->nRows ; row++)
				for(col = 0 ; col < matrix->nCols ; col++)
					matrix->data.D[row][col] = (maxVal - minVal) *
					  drand48() + minVal;

			break;

		case DATA_TYPE_X:
			for(row = 0 ; row < matrix->nRows ; row++)
				for(col = 0 ; col < matrix->nCols ; col++)
					{
					matrix->data.X[row][col].re = (maxVal - minVal) *
					  drand48() + minVal;
					matrix->data.X[row][col].im = (maxVal - minVal) *
					  drand48() + minVal;
					}

			break;

		}

	}

/*============================================================================*/

Matrix *MultiplyMatrices(Matrix *matrix1, Matrix *matrix2, Matrix *result)

	{
	char *prefixErrMesg = "Error (MultiplyMatrices)";
	int row, col, k;
	Mtype x1, x2, y, t;

	if(matrix1->nCols != matrix2->nRows)
		{
		Error(prefixErrMesg, "sizes of matrices don't match");
		return(NULL);
		}

	if(!result)
		{
		if(!(result = CreateMatrix(matrix1->nRows, matrix2->nCols,
		  matrix1->dataType > matrix2->dataType ? matrix1->dataType :
		  matrix2->dataType)))
			return(NULL);

		}

	else
		{
		if(result->nRows != matrix1->nRows || result->nCols != matrix2->nCols)
			{
			Error(prefixErrMesg, "sizes of matrices don't match");
			return(NULL);
			}

		}

	if(result == matrix1 || result == matrix2)
		{
		Error(prefixErrMesg, "in-place calculations not supported");
		return(NULL);
		}

	x1.dataType = matrix1->dataType;
	x2.dataType = matrix2->dataType;
	y.dataType = result->dataType;
	if(x1.dataType != DATA_TYPE_X && x2.dataType != DATA_TYPE_X)
		{
		double tmp, sum;

		t.data = (void *)(&tmp);
		t.dataType = DATA_TYPE_D;
		for(row = 0 ; row < result->nRows ; row++)
			for(col = 0 ; col < result->nCols ; col++)
				{
				sum = 0;
				y.data = GetPointerToMatrixElement(result, row, col);
				for(k = 0 ; k < matrix1->nCols ; k++)
					{
					x1.data = GetPointerToMatrixElement(matrix1, row, k);
					x2.data = GetPointerToMatrixElement(matrix2, k, col);
					MultiplyMtype(&x1, &x2, &t);
					sum += tmp; /* Same as "sum += *(double *)(t.data)" */
					}

				DoubleToMtype(&y, tmp);
				}

		}

	else
		{
		Complex tmp, sum;

		t.data = (void *)(&tmp);
		t.dataType = DATA_TYPE_X;
		for(row = 0 ; row < result->nRows ; row++)
			for(col = 0 ; col < result->nCols ; col++)
				{
				sum.re = 0;
				sum.im = 0;
				y.data = GetPointerToMatrixElement(result, row, col);
				for(k = 0 ; k < matrix1->nCols ; k++)
					{
					x1.data = GetPointerToMatrixElement(matrix1, row, k);
					x2.data = GetPointerToMatrixElement(matrix2, k, col);
					MultiplyMtype(&x1, &x2, &t);
					sum.re += tmp.re;
					/* Same as "sum.re += ((Complex *)(t.data))->re" */
					sum.im += tmp.im;
					/* Same as "sum.im += ((Complex *)(t.data))->im" */
					}

				ComplexToMtype(&y, tmp);
				}

		}

	return(result);
	}

/*==============================================================================
	Calculates the 2D DCT of a block having nRows x nCols elements.
	For ``inv == 1'' calculates the direct transform.
	For ``inv == -1'' calculates the inverse transform.
------------------------------------------------------------------------------*/

Matrix *Dct2D(Matrix *srcMatrix, Matrix *dstMatrix, int inv)

	{
	int R, C, r, c;
	double val, constR0, constC0, constRnot0, constCnot0;

	if(srcMatrix->dataType != DATA_TYPE_D || dstMatrix->dataType != DATA_TYPE_D)
		{
		fprintf(stderr, "Error (Dct2D): wrong data type\n");
		return(NULL);
		}

	if(srcMatrix->nRows != dstMatrix->nRows ||
	  srcMatrix->nCols != dstMatrix->nCols)
		{
		fprintf(stderr, "Error (Dct2D): blocks have different sizes\n");
		return(NULL);
		}

	constR0 = sqrt(1.0 / srcMatrix->nRows);
	constRnot0 = sqrt(2.0 / srcMatrix->nRows);
	constC0 = sqrt(1.0 / srcMatrix->nCols);
	constCnot0 = sqrt(2.0 / srcMatrix->nCols);

	if(inv == 1)
		{
		for(R = 0 ; R < dstMatrix->nRows ; R++)
			for(C = 0 ; C < dstMatrix->nCols ; C++)
				{
				val = 0;
				for(r = 0 ; r < srcMatrix->nRows ; r++)
					for(c = 0 ; c < srcMatrix->nCols ; c++)
						val += srcMatrix->data.D[r][c] *
						  cos((2 * c + 1) * C * M_PI / (2 * srcMatrix->nCols)) *
						  cos((2 * r + 1) * R * M_PI / (2 * srcMatrix->nRows));

				dstMatrix->data.D[R][C] = val *
				  (R == 0 ? constR0 : constRnot0) *
				  (C == 0 ? constC0 : constCnot0);
				}

		}

	else
		{
		for(r = 0 ; r < dstMatrix->nRows ; r++)
			for(c = 0 ; c < dstMatrix->nCols ; c++)
				{
				val = 0;
				for(R = 0 ; R < srcMatrix->nRows ; R++)
					for(C = 0 ; C < srcMatrix->nCols ; C++)
						val += srcMatrix->data.D[R][C] *
						  (R == 0 ? constR0 : constRnot0) *
						  (C == 0 ? constC0 : constCnot0) *
						  cos((2 * c + 1) * C * M_PI / (2 * srcMatrix->nCols)) *
						  cos((2 * r + 1) * R * M_PI / (2 * srcMatrix->nRows));

				dstMatrix->data.D[r][c] = val;
				}

		}

	return(dstMatrix);
	}

/*==============================================================================
    Apply a Gaussian filter to a matrix.
------------------------------------------------------------------------------*/
/*
 *	If "segmentLen < filterLen" it performs a linear interpolation
 *	of the points in "segment", to get an intermediate value.
 */
static double getValue(double *segment, int n, double segmentLen,
  int filterLen, int nPoints)

	{
	double x;
	int i;

	x = n * segmentLen / filterLen + nPoints - segmentLen;
	i = (int)x;
	if(i == x)
		return(segment[i]);

	else
		return((segment[i+1] - segment[i]) * (x - i) + segment[i]);

	}

/*
 */
static void ComputeFilter(int filterLen, double cutPoint, double *filter)

	{
	int n;
	double x, dx, sum = 0;

	dx = cutPoint / filterLen;
	x = -cutPoint;
	for(n = 0 ; n <= 2 * filterLen ; n++)
		{
		filter[n] = 1.0/(sqrt(2.0 * M_PI)) * exp(- x * x / 2.0);
		sum += filter[n];
		x += dx;
		}

	for(n = 0 ; n <= 2 * filterLen ; n++)
		filter[n] /= sum;

	}

/*
 *	"cutPoint" defines the point, measured in units of sigma, from
 *	where the value of the function is considered zero.
 *	"2 * filterLen + 1" defines the number of points used to
 *	sample the filter.
 */
Matrix *GaussianFilter2D(Matrix *src, Matrix *dst, double sigma)

	{
	int n, row, col, filterLen, nPoints;
	double *filter, *segment, cutPoint, segmentLen;
	Matrix *tmp;

	cutPoint = 4;
	filterLen = 5;

	if(src->nRows != dst->nRows || src->nCols != dst->nCols)
		{
		fprintf(stderr, "Error (GaussianFilter2D): sizes don't match.\n");
		return(NULL);
		}

	if(src->dataType != DATA_TYPE_D || dst->dataType != DATA_TYPE_D)
		{
		fprintf(stderr,
		  "Error (GaussianFilter2D): only data type D supported.\n");
		return(NULL);
		}

	segmentLen = sigma * cutPoint;
	nPoints = (int)segmentLen;
	if(nPoints != segmentLen)
		nPoints++;

    if(nPoints > filterLen)
		filterLen = nPoints;

    if(!(filter = (double *)calloc(2 * filterLen + 1, sizeof(double))))
		{
		fprintf(stderr,
		  "Error (GaussianFilter2D): unable to allocate memory.\n");
		return(NULL);
		}

	if(!(segment = (double *)calloc(2 * nPoints + 1, sizeof(double))))
		{
		fprintf(stderr,
		  "Error (GaussianFilter2D): unable to allocate memory.\n");
		return(NULL);
		}

	if(!(tmp = CreateMatrix(src->nRows, src->nCols, DATA_TYPE_D)))
		return(NULL);

	ComputeFilter(filterLen, cutPoint, filter);

	/*
	 *	Apply filter to rows.
	 */
	for(row = 0 ; row < src->nRows ; row++)
		for(col = 0 ; col < src->nCols ; col++)
			{
			double sum = 0;

			for(n = -nPoints ; n <= nPoints ; n++)
				{
				if(col + n < 0 || col + n >= src->nCols)
					{
					if(col + n < 0)
						segment[n + nPoints] = src->data.D[row][0];

					else
						segment[n + nPoints] = src->data.D[row][src->nCols - 1];

					}

				else
					segment[n + nPoints] = src->data.D[row][col + n];

				}

			for(n = 0 ; n <= 2 * filterLen ; n++)
				sum += filter[n] * getValue(segment, n, segmentLen,
				  filterLen, nPoints);

			tmp->data.D[row][col] = sum;
			}

	/*
	 *	Apply filter to cols.
	 */
	for(col = 0 ; col < src->nCols ; col++)
		for(row = 0 ; row < src->nRows ; row++)
			{
			double sum = 0;

			for(n = -nPoints ; n <= nPoints ; n++)
				{
				if(row + n < 0 || row + n >= src->nRows)
					{
					if(row + n < 0)
						segment[n + nPoints] = tmp->data.D[0][col];

					else
						segment[n + nPoints] = tmp->data.D[src->nRows - 1][col];

					}

				else
					segment[n + nPoints] = tmp->data.D[row + n][col];

				}

			for(n = 0 ; n <= 2 * filterLen ; n++)
				sum += filter[n] * getValue(segment, n, segmentLen,
				  filterLen, nPoints);

			dst->data.D[row][col] = sum;
			}

	return(dst);
	}

#ifdef COMMENT

/*============================================================================*/

Matrix *MultiplyScalarByMatrix(Matrix *matrix, double scalar, Matrix *result)

	{
	char *prefixErrMesg = "Error (MultiplyScalarByMatrix)";
	int row, col;

	if(!result)
		{
		if(!(result = CreateMatrix(matrix->nRows, matrix->nCols)))
			return(NULL);

		}

	else
		{
		if(result->nRows != matrix->nRows || result->nCols != matrix->nCols)
			{
			Error(prefixErrMesg, "sizes of matrices do not match");
			return(NULL);
			}

		}

	for(row = 0 ; row < result->nRows ; row++)
		for(col = 0 ; col < result->nCols ; col++)
			result->data[row][col] = matrix->data[row][col] * scalar;

	return(result);
	}

/*============================================================================*/

Matrix *AddMatrices(Matrix *matrix1, Matrix *matrix2, Matrix *result)

	{
	char *prefixErrMesg = "Error (AddMatrices)";
	int row, col;

	if(matrix1->nRows != matrix2->nRows || matrix1->nCols != matrix2->nCols)
		{
		Error(prefixErrMesg, "sizes of matrices do not match");
		return(NULL);
		}

	if(!result)
		{
		if(!(result = CreateMatrix(matrix1->nRows, matrix1->nCols)))
			return(NULL);

		}

	else
		{
		if(result->nRows != matrix1->nRows || result->nCols != matrix1->nCols)
			{
			Error(prefixErrMesg, "sizes of matrices do not match");
			return(NULL);
			}

		}

	for(row = 0 ; row < result->nRows ; row++)
		for(col = 0 ; col < result->nCols ; col++)
			result->data[row][col] = matrix1->data[row][col] +
			  matrix2->data[row][col];

	return(result);
	}

/*============================================================================*/

Matrix *SubtMatrices(Matrix *matrix1, Matrix *matrix2, Matrix *result)

	{
	char *prefixErrMesg = "Error (SubtMatrices)";
	int row, col;

	if(matrix1->nRows != matrix2->nRows || matrix1->nCols != matrix2->nCols)
		{
		Error(prefixErrMesg, "sizes of matrices do not match");
		return(NULL);
		}

	if(!result)
		{
		if(!(result = CreateMatrix(matrix1->nRows, matrix1->nCols)))
			return(NULL);

		}

	else
		{
		if(result->nRows != matrix1->nRows || result->nCols != matrix1->nCols)
			{
			Error(prefixErrMesg, "sizes of matrices do not match");
			return(NULL);
			}

		}

	for(row = 0 ; row < result->nRows ; row++)
		for(col = 0 ; col < result->nCols ; col++)
			result->data[row][col] = matrix1->data[row][col] -
			  matrix2->data[row][col];

	return(result);
	}

/*============================================================================*/

Matrix *CopyMatrices(Matrix *src, Matrix *dest)

	{
	char *prefixErrMesg = "Error (CopyMatrices)";
	int row, col;

	if(!dest)
		{
		if(!(dest = CreateMatrix(src->nRows, src->nCols)))
			return(NULL);

		}

	else
		{
		if(src->nRows != dest->nRows || src->nCols != dest->nCols)
			{
			Error(prefixErrMesg, "sizes of matrices do not match");
			return(NULL);
			}

		}

	for(row = 0 ; row < src->nRows ; row++)
		for(col = 0 ; col < src->nCols ; col++)
			dest->data[row][col] = src->data[row][col];

	return(dest);
	}

/*============================================================================*/

double MatrixSquareNorm(Matrix *matrix)

	{
	double sqrNorm = 0;
	int row, col;

	for(row = 0 ; row < matrix->nRows ; row++)
		for(col = 0 ; col < matrix->nCols ; col++)
			sqrNorm += matrix->data[row][col] * matrix->data[row][col];

	return(sqrNorm);
	}

/*==============================================================================

	Returns a LMS solution of the equation Ax = b.

------------------------------------------------------------------------------*/

Matrix *SolveLMS(Matrix *A, Matrix *x, Matrix *b, double gain,
  double minChange, int maxIter)

	{
	char *prefixErrMesg = "Error (SolveLMS)";
	char *prefixWarningMesg = "Warning (SolveLMS)";
	int nIter, row;
	double MSE, lastMSE = DBL_MAX, controlFactor = 1.25;
	Matrix *AT, *B, *lastB, *deltaX, *lastDeltaX, *gainVector;

	if(A->nRows != b->nRows || b->nCols != 1)
		{
		Error(prefixErrMesg, "sizes of matrices do not match");
		return(NULL);
		}

	if(!x)
		{
		if(!(x = CreateMatrix(A->nCols, 1)))
			return(NULL);

		}

	else
		{
		if(x->nRows != A->nCols || x->nCols != 1)
			{
			Error(prefixErrMesg, "sizes of matrices do not match");
			return(NULL);
			}

		}

	if(!(AT = TransposeMatrix(A, NULL)))
		return(NULL);

	if(!(B = CreateMatrix(A->nRows, 1)))
		return(NULL);

	if(!(lastB = CreateMatrix(A->nRows, 1)))
		return(NULL);

	if(!(deltaX = CreateMatrix(A->nCols, 1)))
		return(NULL);

	if(!(gainVector = CreateMatrix(A->nCols, 1)))
		return(NULL);

	for(row = 0 ; row < gainVector->nRows ; row++)
		gainVector->data[row][0] = gain;

	if(!(lastDeltaX = CreateMatrix(A->nCols, 1)))
		return(NULL);

	nIter = 0;
	do
		{
		if(!MultMatrices(A, x, B))
			return(NULL);

		if(!SubtMatrices(b, B, B))
			return(NULL);

		MSE = MatrixSquareNorm(B) / B->nRows;

		if(!MultMatrices(AT, B, deltaX))
			return(NULL);

		for(row = 0 ; row < gainVector->nRows ; row++)
			{
			if(deltaX->data[row][0] * lastDeltaX->data[row][0] > 0)
				{
				gainVector->data[row][0] *= controlFactor;
				}

			else
				{
				gainVector->data[row][0] /= (1.5 * controlFactor);
				}

			deltaX->data[row][0] *= gainVector->data[row][0];
			lastDeltaX->data[row][0] = deltaX->data[row][0];
			}

		if(!AddMatrices(x, deltaX, x))
			return(NULL);

/*
	printf("%3d (%.5f) [ ", nIter, MSE);
	for(row = 0 ; row < gainVector->nRows ; row++)
		printf("%.3f ", gainVector->data[row][0]);

	printf("] < ");
	for(row = 0 ; row < x->nRows ; row++)
		printf("%.3f ", x->data[row][0]);

	printf(">\n");
*/
		if(++nIter == maxIter)
			{
			Error(prefixWarningMesg,"non-convergent after limit of iterations");
			break;
			}

		} while(MatrixSquareNorm(deltaX) / deltaX->nRows > minChange);

	DestroyMatrix(AT);
	DestroyMatrix(B);
	DestroyMatrix(lastB);
	DestroyMatrix(deltaX);
	DestroyMatrix(lastDeltaX);
	return(x);
	}

#endif /* COMMENT */

