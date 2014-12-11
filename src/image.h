/*------------------------------------------------------------------------------

Copyright 2000-2004 Armando J. Pinho (ap@det.ua.pt), All Rights Reserved.

These programs are supplied free of charge for research purposes only,
and may not be sold or incorporated into any commercial product. There is
ABSOLUTELY NO WARRANTY of any sort, nor any undertaking that they are
fit for ANY PURPOSE WHATSOEVER. Use them at your own risk. If you do
happen to find a bug, or have modifications to suggest, please report
the same to Armando J. Pinho, ap@det.ua.pt. The copyright notice above
and this statement of conditions must remain an integral part of each
and every copy made of these files.

Modified:
Thu Nov 25 23:03:49 WET 2004
Fri Oct  8 11:49:06 WEST 2004
Tue Nov  4 14:09:59 WET 2003

------------------------------------------------------------------------------*/

#ifndef IMAGE_H_INCLUDED
#define IMAGE_H_INCLUDED

#include <stdio.h>
#include "mtype.h"
#include "matrix.h"


#define COPY_DEFAULT   0
#define COPY_REAL_PART 1
#define COPY_IMAG_PART 2
#define COPY_PHASE     3
#define COPY_MODULUS   4

typedef struct
    {
	int nRows;
	int nCols;
	int nPlanes; /* 1 or 3 */
	int dataType;
	int maxIntensity;
	union
		{
		uChar **C;
		uShort **S;
		} data[3];
	}
Image;

typedef struct
	{
	double mabErr[4]; /* L1 error */
	double rmsErr[4]; /* L2 error */
	double maxErr[4]; /* Maximum error */
	double psnr[4];
	}
ImageErr;

typedef struct
	{
	int row;
	int col;
	}
ImageCoords;

typedef struct
	{
	int row;
	int col;
	union
		{
		uChar *C;
		uShort *S;
		} val[3];
	}
Pixel;

/* 0 -> R, 1 -> G, 2 -> B */
typedef struct
	{
	int plane[3];
	}
RGBColor;

void FreeImage(Image *img);
Image *CreateImage(int nRows, int nCols, int type, int nPlanes);
Image *ReadImageFile(char *fileName);
Image *WriteImageFile(char *fileName, Image *img);
Image *CopyImage(Image *, Image *);
Image *CalcImageErr(Image *, Image *, ImageErr *);
Image *MatrixToImage(Matrix *src, Image *dst, int imgPlane);
Image *ImageToMatrix(Image *src, Matrix *dst, int imgPlane);
Image *MergeImages(Image *srcImg1, Image *srcImg2, char mode);
int ReadPNMImageFileHeader(FILE *fp, int *nRows, int *nCols,
  int *maxIntensity);
FILE *OpenImageFile(char *fileName, char *mode, char *fpType);
void CloseImageFile(FILE *fp, char fpType);
int ImageMaxIntensity(Image *img);
int GetGrayPixel(Image *img, int row, int col);
void PutGrayPixel(Image *img, int row, int col, int pixel);
int GetColor(Image *img, int row, int col, int component);
double ComputeImgVariation(Image *img);
int ReadGrayPixel(int pixDataType, char inpDataType, FILE *fp);
void WriteGrayPixel(int pixel, int pixDataType, char outDataType, FILE *fp);
RGBColor GetRGBPixel(Image *img, int row, int col);
void PutRGBPixel(Image *img, int row, int col, RGBColor *color);
int CmpRGBColors(RGBColor *rgb1, RGBColor *rgb2);
unsigned int GetBlockValueFromImage(Image *img, int row, int col, int nRSize, int nCSize, int plane);

#endif /* IMAGE_H_INCLUDED */

