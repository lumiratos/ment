/*------------------------------------------------------------------------------

Copyright 2005 Antonio Neves (an@ieeta.pt), All Rights Reserved.

These programs are supplied free of charge for research purposes only,
and may not sold or incorporated into any commercial product. There is
ABSOLUTELY NO WARRANTY of any sort, nor any undertaking that they are
fit for ANY PURPOSE WHATSOEVER. Use them at your own risk. If you do
happen to find a bug, or have modifications to suggest, please report
the same to Antonio Neves, an@ieeta.pt. The copyright notice above and
this statement of conditions must remain an integral part of each and
every copy made of these files.

------------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>

#include "image.h"
#include "arith.h"
#include "bitio.h"
#include "context95.h"

#define NCONTEXT 2 << 23  /* The maximum context size is 19 pixels */

int ReadNBits(int nBits, FILE *iFp)

	{
	int bits = 0, target, count[2];

	count[0] = count[1] = 1;
	while(nBits--)
		{
		bits <<= 1;
		target = arithmetic_decode_target(2);
		if(target < 1)
			{
			bits |= 0;
			arithmetic_decode(0, 1, 2, iFp);
			}

		else
			{
			bits |= 1;
			arithmetic_decode(1, 2, 2, iFp);
			}

		}

	return bits;
	}


int main(int argc, char **argv)

	{
	FILE *fp;
	Image *img = NULL;
	int r, c, n = 0, imgPlanes, nRows, nCols, fileSize;	
	int plane, i, j, pixel, nPlanes = 8, mode = 0, targetBytes;
	int lastBpp = 0, ctxInc, ctxMax;
	unsigned int target;
	unsigned int **C;
	char reset = 'y', verbose = 'n', all = 'n';
	double decodedFraction = 1, planeBpp;
	unsigned int np = 16, count = 0, outFile = 0;

	if (argc < 2) 
		{
		printf("Usage %16s [ -noreset]\n", argv[0]);
		printf("                       [ -v ] (verbose)\n");
		printf("                       [ -f decodedFraction (0-1)]\n");
		printf("		       [ -np numberPlane ]  (number of planes to decode)\n");
		printf("		       [ -o outputFile ] \n");
		printf("                       encodedFile\n");
		return EXIT_FAILURE;
		}

	for(j = 1 ; j < argc ; j++)
		if(strcmp(argv[j], "-noreset") == 0)
			{
			reset = 'n';
			break;
			}

	for(j = 1 ; j < argc ; j++)
		if(strcmp(argv[j], "-v") == 0)
			{
			verbose = 'y';
			break;
			}

	for(j = 1 ; j < argc ; j++)
		if(strcmp(argv[j], "-f") == 0)
			{
			decodedFraction = atof(argv[j + 1]);
			break;
			}
	for(j = 1 ; j < argc ; j++)
		if(strcmp(argv[j], "-np") == 0)
			{
			np = atoi(argv[j + 1]);
			break;
			}
	for(j = 1 ; j < argc ; j++)
		if(strcmp(argv[j], "-o") == 0)
			{
			outFile = j+1;
			break;
			}

	fp = fopen(argv[argc - 1], "rb");
	if(fp == NULL)
		return EXIT_FAILURE;

	C = (unsigned int**)calloc(NCONTEXT, sizeof(unsigned int*));
	for(n = 0 ; n < NCONTEXT ; n++)
		C[n] = (unsigned int*)calloc(3, sizeof(unsigned int));

	fseek(fp, 0, SEEK_END);
	fileSize = ftell(fp);
	rewind(fp);
	printf("Code file has %d bytes\n", fileSize);
	targetBytes = fileSize * decodedFraction;
	printf("Decoding the first %d bytes\n", targetBytes);

	for(j = 0 ; j < NCONTEXT ; j++)
		{
		C[j][0] = 1;
		C[j][1] = 1;
		C[j][2] = 2;
		}

	startinputtingbits();
	start_decode(fp);

	/* Get the number of rows and cols */
	nRows = ReadNBits(32, fp);
	nCols = ReadNBits(32, fp);

	/* Get the context information */
	ctxInc = ReadNBits(8, fp);
	ctxMax = ReadNBits(32, fp);

	/* Get the contex flag */
	mode = ReadNBits(2, fp);

	/* Get the all planes flag */
	all = ReadNBits(1, fp);

	/* Get the number of planes coded*/
	nPlanes = ReadNBits(4, fp) + 1;

	/* Get the number of image bit-planes (1..16) */
	imgPlanes = ReadNBits(4, fp) + 1;

	printf("rows= %d,cols= %d,mode= %d,all= %d,nPlanes= %d,imgPlanes= %d\n",
	  nRows, nCols, mode, all, nPlanes, imgPlanes);
	printf("ctxInc= %d, ctxMax= %d\n", ctxInc, ctxMax);

	if(imgPlanes > 8)
		{
		if ((img = CreateImage(nRows, nCols, DATA_TYPE_S, 1)) == NULL) 
			return EXIT_FAILURE;

		}

	else
		{
		if ((img = CreateImage(nRows, nCols, DATA_TYPE_C, 1)) == NULL) 
			return EXIT_FAILURE;

		}

	for(r = 0 ; r < img->nRows ; r++)
		for(c = 0 ; c < img->nCols ; c++)
			PutGrayPixel(img, r, c, 0);

	// Update the number of planes in case of passing a higher number than the number of planes encoded
	if (np > (nPlanes - 1))
		np = nPlanes - 1;

	for(plane = nPlanes - 1 ; plane >= 0  ; plane--) 
		{
		if(verbose == 'y')
			{
			printf("Decoding plane %2d ...", plane);
			fflush(stdout);
			}

		if(reset == 'y')
			for(j = 0 ; j < NCONTEXT ; j++)
				{
				C[j][0] = 1;
				C[j][1] = 1;
				C[j][2] = 2;
				}

		// Update the current the current decoded planes
		count++;

		for(r = 0 ; r < img->nRows ; r++)
		for(c = 0 ; c < img->nCols ; c++)
			{
			if(mode == 0)
				i = GetContextViip(img, plane, nPlanes, r, c);
			else if(mode == 1)
				i = GetContextIcip(img, plane, nPlanes, r, c);
			else
				i = GetContext(img, plane, nPlanes, r, c);

			pixel = GetGrayPixel(img, r, c);			

			target = arithmetic_decode_target(C[i][2]);

			if(C[i][0] > target)
				{
				arithmetic_decode(0, C[i][0], C[i][2], fp);
				C[i][0] += ctxInc;
				C[i][2] += ctxInc;
				}

			else
				{
				PutGrayPixel(img, r, c, pixel | (0x01 << plane));
				arithmetic_decode(C[i][0], C[i][2], C[i][2], fp);
				C[i][1] += ctxInc;
				C[i][2] += ctxInc;
				}

			if(C[i][2] > ctxMax)
				{
				C[i][0] = C[i][1] = ctxInc;
				C[i][2] = ctxInc * 2;
				}

			if(_bytes_input > targetBytes)
				break;

			}

		planeBpp = (double)(_bytes_input - lastBpp) * 8 /
		  (img->nRows * img->nCols);
		lastBpp = _bytes_input;
		
		if(verbose == 'y')
			printf(" used %1.3f bpp.\n", planeBpp);

		if(planeBpp > 1.001 && !all)
			{
			nPlanes -= plane;
			break;
			}
		//printf("_bytes_input = %u, targetBytes %u\n",_bytes_input, targetBytes);
		if ((count > np) || (_bytes_input > targetBytes) )
			{
				break;	
			}
		}

	//for(n = 0 ; n < imgPlanes - nPlanes ; n++)
	if ((count < np) || (_bytes_input < targetBytes))
		{		
		for(n = imgPlanes - nPlanes - 1 ; n >= 0 ; n--)
			{
			count++;
			for(r = 0 ; r < img->nRows ; r++)
			for(c = 0 ; c < img->nCols ; c++)
				if(_bytes_input > targetBytes)
					PutGrayPixel(img, r, c, GetGrayPixel(img, r, c) << 1);

				else
					PutGrayPixel(img, r, c, GetGrayPixel(img, r, c) |
					  (ReadNBits(1, fp) << n));

			if(verbose == 'y')
				printf("Decoding plane %2d RAW.\n", n);


			if ((count > np) || (_bytes_input > targetBytes) )
				{
				break;	
				}			
			}
		}

	finish_decode();
	doneinputtingbits();

	fclose(fp);
	//WriteImageFile("!decodedMicroarrayImage.pgm", img);
	
	if ((outFile > 0) && (outFile < argc))
		WriteImageFile(argv[outFile], img);
	else
		WriteImageFile("!decMicroImg06.pgm", img);
	

	printf("Image decoded successfully!\n");
	return EXIT_SUCCESS;	
	}

