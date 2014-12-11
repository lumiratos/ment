/*------------------------------------------------------------------------------

Copyright 2006 Armando J. Pinho (ap@det.ua.pt), All Rights Reserved.
	  2014 Luis M. O. Matos (luismatos@ua.pt)

These programs are supplied free of charge for research purposes only,
and may not be sold or incorporated into any commercial product. There is
ABSOLUTELY NO WARRANTY of any sort, nor any undertaking that they are
fit for ANY PURPOSE WHATSOEVER. Use them at your own risk. If you do
happen to find a bug, or have modifications to suggest, please report
the same to Armando J. Pinho, ap@det.ua.pt. The copyright notice above
and this statement of conditions must remain an integral part of each
and every copy made of these files.

------------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <assert.h>
#include "bitio.h"
#include "arith.h"
#include "arith_aux.h"
#include "common.h"
#include "defs.h"

/*----------------------------------------------------------------------------*/

void DecodeBitPlane(Image *recImg, int imgPlanes, int plane, CModel *cModel,
  CTemplate *cTemplateIntra, CTemplate *cTemplateInter, PModel *pModel,
  CPattern *cPattern, FILE *fpInp, int targetBytes)

	{
	int row, col, pModelIdx, symbol;

	ResetCModelCounters(cModel);
	for(row = 0 ; row < recImg->nRows ; row++)
		for(col = 0 ; col < recImg->nCols ; col++)
			{
			pModelIdx = GetPModelIdx(recImg, row, col, plane, imgPlanes,
			  cModel, cTemplateIntra, cTemplateInter, cPattern);
			ComputePModel(cModel, pModel, pModelIdx);
			symbol = ArithDecodeSymbol(cModel->nSymbols, pModel->freqs,
			  pModel->sum, fpInp);
			PutGrayPixel(recImg, row, col, GetGrayPixel(recImg, row, col) |
			  symbol << plane);
			UpdateCModelCounter(cModel, pModelIdx, symbol);

			if(_bytes_input > targetBytes)
				break;

			}

	}

/*----------------------------------------------------------------------------*/

void DecodeBitPlane2(Image *recImg, int imgPlanes, int plane, CModel *cModel,
  CTemplate *cTemplateIntra, CTemplate *cTemplateInter, PModel *pModel,
  CPattern *cPattern, FILE *fpInp, int targetBytes, Image *mask, char BF)

	{
	int row, col, pModelIdx, symbol, maskPixel;

	ResetCModelCounters(cModel);
	for(row = 0 ; row < recImg->nRows ; row++)
		for(col = 0 ; col < recImg->nCols ; col++)
			{
			maskPixel = GetGrayPixel(mask, row, col);
			if( ((maskPixel == 0xFF) && (BF == 'F')) || ((maskPixel == 0x00) && (BF == 'B')) ) 
				{
				pModelIdx = GetPModelIdx(recImg, row, col, plane, imgPlanes,
			 	 cModel, cTemplateIntra, cTemplateInter, cPattern);
				ComputePModel(cModel, pModel, pModelIdx);
				symbol = ArithDecodeSymbol(cModel->nSymbols, pModel->freqs,
				  pModel->sum, fpInp);
				PutGrayPixel(recImg, row, col, GetGrayPixel(recImg, row, col) |
				  symbol << plane);
				UpdateCModelCounter(cModel, pModelIdx, symbol);

				if(_bytes_input > targetBytes)
					break;
				}

			}

	}

/*----------------------------------------------------------------------------*/

Image *GetOriginalImage(Image *fImg, Image *bImg, Image *mask, unsigned int te)
	{
	int row, col, pixel, maskPixel;
	Image *img;
	
	if(!(img = CreateImage(mask->nRows, mask->nCols, DATA_TYPE_S, 1)))
		{
		fprintf(stderr, "Error: unable to create image!\n");
		exit(1);
		}
	
	// Loop the mask image and fill the values of output image
	for(row = 0; row < mask->nRows; row++)
		{
			for(col = 0; col < mask->nCols; col++)
				{
				maskPixel = GetGrayPixel(mask, row, col);
				pixel = ((maskPixel == 0xFF) ? (GetGrayPixel(fImg, row, col) + te) : GetGrayPixel(bImg, row, col));
				PutGrayPixel(img, row, col, pixel);
				}
		}
	return img;
	}
	

/*----------------------------------------------------------------------------*/

int main(int argc, char *argv[])

	{
	//int n, p, row, col, nRows, nCols, imgPlanes1, imgPlanes2, codingPlanes1, codingPlanes2,
	int n, p, nRows, nCols, imgPlanes1, imgPlanes2, codingPlanes1, codingPlanes2,
	  fileSize, targetBytes, deltaNum, deltaDen, maxCount, plane;
	double decodedFraction = 1;
	char progressIndicator = 'y', verbose = 'n';
	unsigned int te;
	Image *recImg1, *recImg2, *maskImg=NULL, *img;
	FILE *fpInp;
	CModel *cModel1, *cModel2;
	CTemplate *cTemplateIntra = InitTemplate(TEMPLATE_INTRA);
	CTemplate *cTemplateInter = InitTemplate(TEMPLATE_INTER);
	PModel *pModel;
	CPattern *cPattern1, *cPattern2;
	char outFileName[FILENAME_MAX]="";	

	if(argc == 1)
		{
		fprintf(stderr,"Usage: %12s [ -f decodedFraction ]\n", argv[0]);
		//fprintf(stderr,"Usage: Micro3DDec [ -f decodedFraction ]\n");
		//fprintf(stderr,"                  [ -nopi (no progress indicator) ]\n");
		fprintf(stderr,"                    [ -v (verbose) ]\n");
		fprintf(stderr,"                    [ -o image (decoded image) ]\n");
		fprintf(stderr,"		    [ -m mask (mask image file)\n");
		fprintf(stderr,"                    codeFile\n");
		return 1;
		}

	for(n = 1 ; n < argc ; n++)
		if(!strcmp("-f", argv[n]))
			{
			decodedFraction = atof(argv[n+1]);
			break;
			}

	for(n = 1 ; n < argc ; n++)
		if(!strcmp("-nopi", argv[n]))
			{
			progressIndicator = 'n';
			break;
			}

	for(n = 1 ; n < argc ; n++)
		if(!strcmp("-v", argv[n]))
			{
			verbose = 'y';
			progressIndicator = 'n';
			break;
			}
	for(n = 1 ; n < argc ; n++)
		if(!strcmp("-m", argv[n]))
			{
			maskImg = ReadImageFile(argv[n + 1]);
			break;
			}

	for(n = 1 ; n < argc ; n++)
		if(!strcmp("-o", argv[n]))
			{
			strcpy(outFileName,argv[n+1]);
			break;
			}


	if(maskImg == NULL)
	{
		fprintf(stderr, "Error: binary mask was not specified!\n");
		fprintf(stderr, "Please specify the binary mask image using the -m option!\n");
		return 1;
	}

	if(maskImg->dataType != DATA_TYPE_C)
	{
		fprintf(stderr, "Error: mask must be of DATA_TYPE_C (Unsigened char)\n");
		return 1;
	}

	if(!(fpInp = fopen(argv[argc - 1], "r")))
		{
		fprintf(stderr, "Error: couldn't open code file\n");
		return 1;
		}

	fseek(fpInp, 0, SEEK_END);
	fileSize = ftell(fpInp);
	rewind(fpInp);
	printf("Code file has %d bytes\n", fileSize);
	targetBytes = (int)(fileSize * decodedFraction + 0.5);
	if(fileSize != targetBytes)
		printf("Decoding the first %d bytes\n", targetBytes);

	startinputtingbits();
	start_decode(fpInp);

	/* Get the number of rows and cols */
	nRows = ReadNBits(STORAGE_BITS_N_ROWS, fpInp);
	nCols = ReadNBits(STORAGE_BITS_N_COLS, fpInp);
	
	/* Threshold value */
	te = ReadNBits(STORAGE_BITS_THRESHOLD, fpInp);

	if((nRows != maskImg->nRows) || (nCols != maskImg->nCols))
	{
		fprintf(stderr, "Error: size between encoded image and binary mask mismatch!\n");
		fprintf(stderr, "Encoded image dimensions:     %d x %d\n", nRows, nCols);
		fprintf(stderr, "Binary mask image dimensions: %d x %d\n", maskImg->nRows, maskImg->nCols);
		return 1;
	}
		

	/* Get the number of image bit-planes (1..16) */
	imgPlanes1 = ReadNBits(STORAGE_BITS_N_IMG_PLANES, fpInp) + 1;
	imgPlanes2 = ReadNBits(STORAGE_BITS_N_IMG_PLANES, fpInp) + 1;

	/* Get the number of encoded bit-planes (1..16) */
	codingPlanes1 = ReadNBits(STORAGE_BITS_N_COD_PLANES, fpInp) + 1;
	codingPlanes2 = ReadNBits(STORAGE_BITS_N_COD_PLANES, fpInp) + 1;

	deltaNum = ReadNBits(STORAGE_BITS_PMODEL_DELTA_NUM, fpInp) + 1;
	deltaDen = ReadNBits(STORAGE_BITS_PMODEL_DELTA_DEN, fpInp) + 1;
	maxCount = ReadNBits(STORAGE_BITS_PMODEL_MAX_COUNT, fpInp);
	if(maxCount == 0 || maxCount > DEFAULT_PMODEL_MAX_COUNT)
		fprintf(stderr, "Warning (maxCount): counters may overflow\n");

	printf("Background image has %d rows, %d cols and %d bit-planes\n", nRows, nCols, imgPlanes1);
	printf("Foreground image has %d rows, %d cols and %d bit-planes\n", nRows, nCols, imgPlanes2);
	
	printf("Decoding the %d most significant bit-planes of the background image\n", codingPlanes1);
	printf("Decoding the %d most significant bit-planes of the foreground image\n", codingPlanes2);
	printf("Delta: %d/%d, MaxCount: %d\n", deltaNum, deltaDen, maxCount);

	if(!(recImg1 = CreateImage(nRows, nCols, imgPlanes1 <= 8 ? DATA_TYPE_C : DATA_TYPE_S, 1)))
		return 1;
	
	if(!(recImg2 = CreateImage(nRows, nCols, imgPlanes2 <= 8 ? DATA_TYPE_C : DATA_TYPE_S, 1)))
		return 1;


	if(verbose == 'y')
		{
		printf("Using intra template:\n\n");
		ShowTemplate(cTemplateIntra);
		putchar('\n');
		printf("Using inter template:\n\n");
		ShowTemplate(cTemplateInter);
		putchar('\n');
		}

	cModel1 = CreateCModel(TEMPLATE_MAX_SIZE, N_SYMBOLS, N_CTX_SYMBOLS, deltaNum, deltaDen, maxCount);
	cModel2 = CreateCModel(TEMPLATE_MAX_SIZE, N_SYMBOLS, N_CTX_SYMBOLS, deltaNum, deltaDen, maxCount);
	pModel = CreatePModel(N_SYMBOLS);
	cPattern1 = CreateCPattern(imgPlanes1, MAX(cTemplateIntra->size, cTemplateInter->size));
	cPattern2 = CreateCPattern(imgPlanes2, MAX(cTemplateIntra->size, cTemplateInter->size));

	// Background image
	for(plane = imgPlanes1 - 1 ; plane >= imgPlanes1 - codingPlanes1 ; plane--)
		{
		/* Get the context pattern for this bit-plane */
		for(p = plane ; p < imgPlanes1 ; p++)
			{
			cPattern1->size[p] = 0;
			for(n = 0 ; n < (p == plane ? cTemplateIntra->size : cTemplateInter->size) ; n++)
				if((cPattern1->pattern[p][n] = ReadNBits(1, fpInp)))
					cPattern1->size[p]++;
			}

		if(verbose == 'y')
			{
			fprintf(stderr, "Plane %d: ", plane);
			for(n = plane ; n < imgPlanes1 ; n++)
				fprintf(stderr, "%d ", cPattern1->size[n]);

			fprintf(stderr, "\n");
			}

		DecodeBitPlane2(recImg1, imgPlanes1, plane, cModel1, cTemplateIntra, cTemplateInter, pModel, cPattern1, fpInp, targetBytes, maskImg, 'B');

		if(_bytes_input > targetBytes)
			break;

		if(progressIndicator == 'y')
			fprintf(stderr, "Plane %d decoded\n", plane);

		}
	
	//WriteImageFile("decodedBackgroundImage.pgm", recImg1);		
	
	// Foreground image
	for(plane = imgPlanes2 - 1 ; plane >= imgPlanes2 - codingPlanes2 ; plane--)
		{
		/* Get the context pattern for this bit-plane */
		for(p = plane ; p < imgPlanes2 ; p++)
			{
			cPattern2->size[p] = 0;
			for(n = 0 ; n < (p == plane ? cTemplateIntra->size : cTemplateInter->size) ; n++)
				if((cPattern2->pattern[p][n] = ReadNBits(1, fpInp)))
					cPattern2->size[p]++;
			}

		if(verbose == 'y')
			{
			fprintf(stderr, "Plane %d: ", plane);
			for(n = plane ; n < imgPlanes2 ; n++)
				fprintf(stderr, "%d ", cPattern2->size[n]);

			fprintf(stderr, "\n");
			}

		DecodeBitPlane2(recImg2, imgPlanes2, plane, cModel2, cTemplateIntra, cTemplateInter, pModel, cPattern2, fpInp, targetBytes, maskImg, 'F');

		if(_bytes_input > targetBytes)
			break;

		if(progressIndicator == 'y')
			fprintf(stderr, "Plane %d decoded\n", plane);

		}	

	/*
	for(plane = imgPlanes - codingPlanes - 1 ; plane >= 0 ; plane--)
		for(row = 0 ; row < recImg->nRows ; row++)
			for(col = 0 ; col < recImg->nCols ; col++)
				if(_bytes_input <= targetBytes)
					PutGrayPixel(recImg, row, col, GetGrayPixel(recImg,
					  row, col) | (ReadNBits(1, fpInp) << plane));
	*/
	printf("Decoded %"PRIu64" bytes\n", (uint64_t)_bytes_input);
	finish_decode();
	doneinputtingbits();

	fclose(fpInp);

		
	img = GetOriginalImage(recImg2, recImg1, maskImg, te);
	//WriteImageFile("!micro3DImage.pgm.gz", recImg);
	if(strcmp(outFileName, "") == 0)
		{
		strcat(outFileName, argv[argc-1]);
		strcat(outFileName, ".dec");
		}
	WriteImageFile(outFileName, img);
	//WriteImageFile("!micro3DImage.pgm.gz", img);
	putchar('\n');
	return 0;
	}

