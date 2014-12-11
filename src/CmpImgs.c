/*
 *
 * Compares two images and gives several measures of the difference between
 * them (PSNR, maximum error,...).
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "image.h"

int main(int argc, char *argv[])

	{
	Image *img1, *img2, *diff, *sign;
	ImageErr imgErr;
	RGBColor c1, c2;
	int row, col, p1, p2, diffFirstRow=-1, diffFirstCol=-1;
	char showFirstDiff = 'n';

	if(argc < 3)
		{
		fprintf(stderr, "Usage: CmpImgs img1 img2\n");
		exit(1);
		}

	if(!(img1 = ReadImageFile(argv[argc - 2])))
		exit(1);

	if(!(img2 = ReadImageFile(argv[argc - 1])))
		exit(1);

	if(!CalcImageErr(img1, img2, &imgErr))
		exit(1);

	if(img1->dataType == DATA_TYPE_C)
		{
		if((diff = CreateImage(img1->nRows,img1->nCols,DATA_TYPE_C,1)) == NULL)
			exit(1);

		}

	else
		{
		if((diff = CreateImage(img1->nRows,img1->nCols,DATA_TYPE_S,1)) == NULL)
			exit(1);

		}


	if((sign = CreateImage(img1->nRows, img1->nCols, DATA_TYPE_C, 1)) == NULL)
	    exit(1);

	if(img1->nPlanes == 1 && img2->nPlanes == 1)
		for(row = 0; row <img1->nRows; row++)
			for(col = 0; col < img1->nCols; col++)
				{
				p1 = GetGrayPixel(img1, row, col);
				p2 = GetGrayPixel(img2, row, col);
				PutGrayPixel(diff, row, col, abs(p1 - p2));
				if (abs(p1 - p2) > 0)
					{
					if(diffFirstRow < 0) diffFirstRow = row;
					if(diffFirstCol < 0) diffFirstCol = col;
					}

				if(p1 - p2 <= 0)
					PutGrayPixel(sign, row, col, 0);
				else
					PutGrayPixel(sign, row, col, 1);
					
				}

	else
		for(row = 0; row <img1->nRows; row++)
			for(col = 0; col < img1->nCols; col++)
				{
				c1 = GetRGBPixel(img1, row, col);
				c2 = GetRGBPixel(img2, row, col);
				if(c1.plane[0] != c2.plane[0] || c1.plane[1] != c2.plane[1] ||
				  c1.plane[2] != c2.plane[2])
					{
					PutGrayPixel(diff, row, col, 255);
					if(diffFirstRow < 0) diffFirstRow = row;
					if(diffFirstCol < 0) diffFirstCol = col;
					}
				else
					PutGrayPixel(diff, row, col, 0);
					
				}
	
	if(diffFirstRow >= 0 && diffFirstCol >= 0 && showFirstDiff == 'y')
	{
		printf("The first difference occurs at row %d and col %d\n", diffFirstRow, diffFirstCol);
	}

	if(img1->nPlanes == 3 && img2->nPlanes == 3)
		{
		printf("Color[0] - L2 Error: %.3f ; Max Error: %d ; PSNR: %.1f dB ;",
		  imgErr.rmsErr[0], (int)imgErr.maxErr[0], imgErr.psnr[0]);
		printf(" L1 Error: %.3f\n", imgErr.mabErr[0]);
		printf("Color[1] - L2 Error: %.3f ; Max Error: %d ; PSNR: %.1f dB ;",
		  imgErr.rmsErr[1], (int)imgErr.maxErr[1], imgErr.psnr[1]);
		printf(" L1 Error: %.3f\n", imgErr.mabErr[1]);
		printf("Color[2] - L2 Error: %.3f ; Max Error: %d ; PSNR: %.1f dB ;",
		  imgErr.rmsErr[2], (int)imgErr.maxErr[2], imgErr.psnr[2]);
		printf(" L1 Error: %.3f\n", imgErr.mabErr[2]);
		printf("Average  - L2 Error: %.3f ; Max Error: %d ; PSNR: %.1f dB ;",
		  imgErr.rmsErr[3], (int)imgErr.maxErr[3], imgErr.psnr[3]);
		printf(" L1 Error: %.3f\n", imgErr.mabErr[3]);
		}

	else
		{
		printf("L2 Error: %.3f ; Max Error: %d ; PSNR: %.1f dB ;",
		  imgErr.rmsErr[0], (int)imgErr.maxErr[0], imgErr.psnr[0]);
		printf(" L1 Error: %.3f\n", imgErr.mabErr[0]);
		}

	/*
	if(!WriteImageFile("!diffImageAbs.pgm.gz", diff))
		exit(1);

	if(!WriteImageFile("!diffImageSign.pgm.gz", sign))
		exit(1);
	*/

	exit(0);
	}

