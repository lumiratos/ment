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
Tue Nov  4 14:14:42 WET 2003

------------------------------------------------------------------------------*/

#ifndef MATRIX_H_INCLUDED
#define MATRIX_H_INCLUDED

#define drand48() (rand()*(1./RAND_MAX))

#include "complex.h"


typedef struct matrix {
	int		nRows;
	int		nCols;
	int		dataType;
	union
		{
		unsigned char	**C;
		unsigned short	**S;
		int				**I;
		double			**D;
		Complex			**X;
		}	data;
} Matrix;

Matrix	*CreateMatrix(int nRows, int nCols, int dataType);
void	DestroyMatrix(Matrix *matrix);
Matrix	*TransposeMatrix(Matrix *src, Matrix *dest);
void	FillMatrixWithRandomValues(Matrix *matrix, double minVal,double maxVal);
Matrix	*MultMatrices(Matrix *matrix1, Matrix *matrix2, Matrix *result);
Matrix	*MultScalarByMatrix(Matrix *matrix, double scalar, Matrix *result);
Matrix	*AddMatrices(Matrix *matrix1, Matrix *matrix2, Matrix *result);
Matrix	*SubtMatrices(Matrix *matrix1, Matrix *matrix2, Matrix *result);
Matrix	*CopyMatrices(Matrix *src, Matrix *dest);
void	*GetPointerToMatrixElement(Matrix *matrix, int row, int col);
double	MatrixSquareNorm(Matrix *matrix);
Matrix	*SolveLMS(Matrix *A, Matrix *x, Matrix *b, double gain, double maxMSE,
			int maxIter);
Matrix	*Dct2D(Matrix *srcMatrix, Matrix *dstMatrix, int inv);
Matrix	*GaussianFilter2D(Matrix *src, Matrix *dst, double sigma);

#endif /* MATRIX_H_INCLUDED */

