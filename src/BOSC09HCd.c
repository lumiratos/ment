/*------------------------------------------------------------------------------

Copyright 2006 Armando J. Pinho (ap@ua.pt) 
	  2012 Luis M. O. Matos (luimatos@ua.pt), All Rights Reserved.

These programs are supplied free of charge for research purposes only,
and may not be sold or incorporated into any commercial product. There is
ABSOLUTELY NO WARRANTY of any sort, nor any undertaking that they are
fit for ANY PURPOSE WHATSOEVER. Use them at your own risk. If you do
happen to find a bug, or have modifications to suggest, please report
the same to Armando J. Pinho, ap@ua.pt. The copyright notice above
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

int main(int argc, char *argv[])

	{
	int n, p, row, col, nRows, nCols, imgPlanes, codingPlanes, fileSize, i, gray,
	  targetBytes, deltaNum, deltaDen, maxCount, plane, maxIntensity, nGrays = 0;
	int codeword; 
	double decodedFraction = 1;
	char progressIndicator = 'y', verbose = 'n', hr = 'y';
	char outFileName[FILENAME_MAX]="";
	Image *recImg;
	FILE *fpInp;
	CModel *cModel;
	CTemplate *cTemplateIntra = InitTemplate(TEMPLATE_INTRA);
	CTemplate *cTemplateInter = InitTemplate(TEMPLATE_INTER);
	PModel *pModel;
	CPattern *cPattern;
	Gray *imgGrays = NULL;
	
	
	if(argc == 1)
		{
		fprintf(stderr,"Usage: %13s [ -f decodedFraction ]\n", argv[0]);
		//fprintf(stderr,"                  [ -nopi (no progress indicator) ]\n");
		fprintf(stderr,"                     [ -v (verbose) ]\n");
		fprintf(stderr,"                     [ -o decodedImage ]\n");
		fprintf(stderr,"                     codeFile\n");
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
		if(!strcmp("-o", argv[n]))
		{
		strcpy(outFileName,argv[n+1]);
		break;
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

	/* Get the number of image bit-planes (1..16) */
	imgPlanes = ReadNBits(STORAGE_BITS_N_IMG_PLANES, fpInp) + 1;

	/* Get the number of encoded bit-planes (1..16) */
	codingPlanes = ReadNBits(STORAGE_BITS_N_COD_PLANES, fpInp) + 1;
	maxIntensity = ReadNBits(STORAGE_BITS_MAX_INTENSITY, fpInp) + 1;
	
	deltaNum = ReadNBits(STORAGE_BITS_PMODEL_DELTA_NUM, fpInp) + 1;
	deltaDen = ReadNBits(STORAGE_BITS_PMODEL_DELTA_DEN, fpInp) + 1;
	maxCount = ReadNBits(STORAGE_BITS_PMODEL_MAX_COUNT, fpInp);
	if(maxCount == 0 || maxCount > DEFAULT_PMODEL_MAX_COUNT)
		fprintf(stderr, "Warning (maxCount): counters may overflow\n");

	
	// Decode Histogram reduction flag
	hr = ((ReadNBits(1, fpInp) == 1) ? 'y' : 'n');
	
	if(hr == 'y')
	{
		/* Array of grays */
		if(!(imgGrays = (Gray *)calloc(maxIntensity + 1, sizeof(Gray))))
		{
			fprintf(stderr, "Error(main): no memory!\n");
			return EXIT_FAILURE;
		}
		
		for(i = 0; i < maxIntensity + 1; i++)
		{
			imgGrays[i].usage = ReadNBits(1, fpInp);
			imgGrays[i].value = i;
			if(imgGrays[i].usage == 1)
			{
				imgGrays[i].codeword = nGrays;
				nGrays++;
			}
			else
				imgGrays[i].codeword = -1;
		}
	}


	printf("Image has %d rows, %d cols and %d bit-planes\n", nRows,
	  nCols, imgPlanes);
	printf("Decoding the %d most significant bit-planes)\n", codingPlanes);
	printf("Delta: %d/%d, MaxCount: %d\n", deltaNum, deltaDen, maxCount);

	if(!(recImg = CreateImage(nRows, nCols, imgPlanes <= 8 ?
	  DATA_TYPE_C : DATA_TYPE_S, 1)))
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

	cModel = CreateCModel(TEMPLATE_MAX_SIZE, N_SYMBOLS, N_CTX_SYMBOLS,
	  deltaNum, deltaDen, maxCount);
	pModel = CreatePModel(N_SYMBOLS);
	cPattern = CreateCPattern(imgPlanes, MAX(cTemplateIntra->size,
	  cTemplateInter->size));

	for(plane = imgPlanes - 1 ; plane >= imgPlanes - codingPlanes ; plane--)
		{
		// Get the context pattern for this bit-plane
		for(p = plane ; p < imgPlanes ; p++)
			{
			cPattern->size[p] = 0;
			for(n = 0 ; n < (p == plane ? cTemplateIntra->size :
			  cTemplateInter->size) ; n++)
				if((cPattern->pattern[p][n] = ReadNBits(1, fpInp)))
					cPattern->size[p]++;

			}

		if(verbose == 'y')
			{
			fprintf(stderr, "Plane %d: ", plane);
			for(n = plane ; n < imgPlanes ; n++)
				fprintf(stderr, "%d ", cPattern->size[n]);

			fprintf(stderr, "\n");
			}

		DecodeBitPlane(recImg, imgPlanes, plane, cModel, cTemplateIntra,
          cTemplateInter, pModel, cPattern, fpInp, targetBytes);

		if(_bytes_input > targetBytes)
			break;

		if(progressIndicator == 'y')
			fprintf(stderr, "Plane %d decoded\n", plane);

		}

	for(plane = imgPlanes - codingPlanes - 1 ; plane >= 0 ; plane--)
		for(row = 0 ; row < recImg->nRows ; row++)
			for(col = 0 ; col < recImg->nCols ; col++)
				if(_bytes_input <= targetBytes)
					PutGrayPixel(recImg, row, col, GetGrayPixel(recImg,
					  row, col) | (ReadNBits(1, fpInp) << plane));
					  
	
	printf("Decoded %"PRIu64" bytes\n", (uint64_t)_bytes_input);
	finish_decode();
	doneinputtingbits();

	fclose(fpInp);
	putchar('\n');
	
	
	// After decoding procedure is done, verify if the Histogram reduction was performed or not
	if(hr == 'y')
	{
		for(row = 0 ; row < recImg->nRows ; row++)
			for(col = 0 ; col < recImg->nCols ; col++)
			{
				codeword = GetGrayPixel(recImg, row, col);
				gray = FindGrayFromCodeWord(imgGrays, maxIntensity+1, codeword);
				PutGrayPixel(recImg, row, col, gray);				
			}
	}
	
	if(strcmp(outFileName, "") == 0)
			{
			strcat(outFileName, argv[argc-1]);
			strcat(outFileName, ".dec");
			}
	WriteImageFile(outFileName, recImg);
	//WriteImageFile("!micro3DImage.pgm.gz", recImg);
	
	return 0;
	}


