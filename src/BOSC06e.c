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

#include <math.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "image.h"
#include "bitio.h"
#include "arith.h"
#include "defs.h"
#include "context95.h"
#include "defs.h"

#define NCONTEXT 2 << 23

void WriteNBits(int bits, int nBits, FILE *oFp)

	{
	while(nBits--)
		{
		if((bits >> nBits) & 0x0001)
			arithmetic_encode(1, 2, 2, oFp);

		else
			arithmetic_encode(0, 1, 2, oFp);

		}

	}

double Log2(double value)

	{
	return(log(value) / M_LN2);
	}


int main(int argc, char **argv)

	{
	int i, n = 0, r, c, j, ctxInc = 1, lastBpp = 0, ctxMax = 65536;
	Image *imgOrig, *img;
	char reset = 'y', verbose = 'n', viip = 'n', icip = 'n', all = 'n';
	unsigned int **C;
	double bits, planeBpp;
	FILE *fp=NULL;
	int imgPlanes, nPlanes = 0, plane, nPlanesJbig = 0;


	if(argc < 2)
		{
		printf("Usage %16s [ -noreset]\n", argv[0]);
		printf("                       [ -v ] (verbose)\n");
		printf("                       [ -viip ] \n");
		printf("                       [ -icip ] \n");
		printf("                       [ -jbig np ] (encoder only)\n");
		printf("                       [ -all ] (Encode all planes)\n");
		printf("                       [ -np planes (def. img_planes)]\n");
		printf("                       [ -ctxInc inc (def. 1)]\n");
		printf("                       [ -ctxMax max (def. 65536)]\n");
		printf("                       img\n");
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
		if(strcmp(argv[j], "-o") == 0)
			{
			if( !(fp = fopen(argv[j+1], "wb")) )
				{
				fprintf(stderr, "Error: can't open file '%s'\n", argv[j+1]);
				return EXIT_FAILURE;
				}
			break;
			}

	for(j = 1 ; j < argc ; j++)
		if(strcmp(argv[j], "-viip") == 0)
			{
			viip = 'y';
			break;
			}

	for(j = 1 ; j < argc ; j++)
		if(strcmp(argv[j], "-icip") == 0)
			{
			icip = 'y';
			break;
			}

	for(j = 1 ; j < argc ; j++)
		if(strcmp(argv[j], "-jbig") == 0)
			{
			nPlanesJbig = atoi(argv[j + 1]);
			break;
			}

	for(j = 1 ; j < argc ; j++)
		if(strcmp(argv[j], "-all") == 0)
			{
			all = 'y';
			break;
			}

	for(j = 1 ; j < argc ; j++)
		if(strcmp(argv[j], "-np") == 0)
			{
			nPlanes = atoi(argv[j + 1]);
			break;
			}

	for(j = 1 ; j < argc ; j++)
		if(strcmp(argv[j], "-ctxInc") == 0)
			{
			ctxInc = atoi(argv[j + 1]);
			break;
			}

	for(j = 1 ; j < argc ; j++)
		if(strcmp(argv[j], "-ctxMax") == 0)
			{
			ctxMax = atoi(argv[j + 1]);
			break;
			}

	if(!(imgOrig = ReadImageFile(argv[argc - 1])))
	        return EXIT_FAILURE;

	C = (unsigned int**)calloc(NCONTEXT, sizeof(unsigned int*));
	for(n = 0 ; n < NCONTEXT ; n++)
		C[n] = (unsigned int*)calloc(3, sizeof(unsigned int));

	/* Force re-calculation of the image maximum intensity */
	imgOrig->maxIntensity = 0;
	imgOrig->maxIntensity = ImageMaxIntensity(imgOrig);
	imgPlanes = (int)(log(imgOrig->maxIntensity) / M_LN2) + 1;

	if(nPlanes == 0)
		nPlanes = imgPlanes;

	/* Create the image with the reduced number of bit-planes */
	if(!(img = CreateImage(imgOrig->nRows, imgOrig->nCols,
	  imgOrig->dataType, 1)))
		return EXIT_FAILURE;

	for(r = 0 ; r < imgOrig->nRows ; r++)
		for(c = 0 ; c < imgOrig->nCols ; c++)
			PutGrayPixel(img, r, c, GetGrayPixel(imgOrig, r, c) >>
			  (imgPlanes - nPlanes));

	/* Open file to write data */
	if(fp == NULL)
		{
		if( !(fp = fopen("encMicroImg06.dat", "wb")) )
			{
			fprintf(stderr, "Error: can't open file '%s'\n", "encMicroImg06.dat");
			return EXIT_FAILURE;
			}
		}


	startoutputtingbits();
	start_encode();

	/* store the number of rows */
	WriteNBits(img->nRows, 32, fp);

	/* store the number of cols */
	WriteNBits(img->nCols, 32, fp);

	/* store the context increment */
	WriteNBits(ctxInc, 8, fp);

	/* store the context maxCount */
	WriteNBits(ctxMax, 32, fp);

	/* store the contex flag */
	if(viip == 'y')
		WriteNBits(0, 2, fp);

	else if(icip == 'y')
		WriteNBits(1, 2, fp);

	else
		WriteNBits(2, 2, fp);
	

	/* store the all planes coded flag */
	if(all == 'y')
		WriteNBits(1, 1, fp);

	else
		WriteNBits(0, 1, fp);

	WriteNBits(nPlanes - 1, 4, fp);

	WriteNBits(imgPlanes - 1, 4, fp);	

	for(j = 0 ; j < NCONTEXT ; j++)
		{
		C[j][0] = 1;
		C[j][1] = 1;
		C[j][2] = 2;
		}

	bits = 0.0;
	for(plane = nPlanes - 1 ; plane >= 0  ; plane--) 
		{
		if(verbose == 'y')
			{
			printf("Encoding plane %2d ...", plane);
			fflush(stdout);
			}

		if(reset == 'y')
			for(j = 0 ; j < NCONTEXT ; j++)
				{
				C[j][0] = 1;
				C[j][1] = 1;
				C[j][2] = 2;
				}

		for(r = 0 ; r < img->nRows ; r++)
		for(c = 0 ; c < img->nCols ; c++)
			{
			if(nPlanesJbig > 0 && plane > nPlanes - nPlanesJbig)
				i = GetContextJBIG(img, plane, nPlanes, r, c);

			else if(viip == 'y')
				i = GetContextViip(img, plane, nPlanes, r, c);

			else if(icip == 'y')
				i = GetContextIcip(img, plane, nPlanes, r, c);

			else
				i = GetContext(img, plane, nPlanes, r, c);

			if(((GetGrayPixel(img, r, c) >> plane) & 0x01) == 1)
				{
				arithmetic_encode(C[i][0], C[i][2], C[i][2], fp);
				bits -= Log2((double)C[i][1] / C[i][2]);
				C[i][1] += ctxInc;
				C[i][2] += ctxInc;

				}

			else
				{
				arithmetic_encode(0, C[i][0], C[i][2], fp);
				bits -= Log2((double)C[i][0] / C[i][2]);
				C[i][0] += ctxInc;
				C[i][2] += ctxInc;
				}

			if(C[i][2] > ctxMax)
				{
				C[i][0] = C[i][1] = ctxInc;
				C[i][2] = ctxInc * 2;
				}

			}

		planeBpp = (double)(_bytes_output - lastBpp) * 8 /
		  (img->nRows * img->nCols);
		lastBpp = _bytes_output;
		if(verbose == 'y')
			printf(" used %1.3f bpp.\n", planeBpp);

		if(planeBpp > 1.001 && all == 'n' &&
		  !(nPlanesJbig > 0 && plane > nPlanes - nPlanesJbig))
			{
			nPlanes -= plane;
			break;
			}

		}

	for(n = imgPlanes - nPlanes - 1 ; n >= 0 ; n--)
		{
		for(r = 0 ; r < imgOrig->nRows ; r++)
			for(c = 0 ; c < imgOrig->nCols ; c++)
				WriteNBits(GetGrayPixel(imgOrig, r, c) >> n, 1, fp);

		if(verbose == 'y')
			printf("Encoding plane %2d RAW.\n", n);

		}

	finish_encode(fp);
	doneoutputtingbits(fp);

	printf("Number of bytes of the encoded image: %"PRIu64" bytes ( %.3f bpp)\n",
	 (uint64_t)_bytes_output, (double)_bytes_output * 8 / (img->nRows * img->nCols));

	return EXIT_SUCCESS;
	}

