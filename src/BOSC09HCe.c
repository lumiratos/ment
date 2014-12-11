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
#include <limits.h>
#include "bitio.h"
#include "arith.h"
#include "arith_aux.h"
#include "common.h"
#include "defs.h"

typedef struct
	{
	double cRow;
	double cCol;
	int nRows;
	int nCols;
	int ulRow;
	int ulCol;
	int lrRow;
	int lrCol;
	}
SearchArea;

/*----------------------------------------------------------------------------*/

void SetSearchArea(Image *img, SearchArea *sa, int argc, char *argv[])

	{
	int n;

	sa->cRow = 0.5;
	sa->cCol = 0.5;
	//sa->nCols = img->nCols;
	//sa->nRows = img->nRows;
	sa->nCols = 256;
	sa->nRows = 256;
	for(n = 1 ; n < argc ; n++)
		if(strcmp("-sa", argv[n]) == 0)
			{
			if(sscanf(argv[n+1], "%dx%d", &sa->nCols, &sa->nRows) != 2)
				{
				//sa->nRows = img->nRows;
				//sa->nCols = img->nCols;
				sa->nRows = 256;
				sa->nCols = 256;
				break;
				}

			break;
			}

	for(n = 1 ; n < argc ; n++)
		if(strcmp("-best", argv[n]) == 0)
			{
				sa->nRows = img->nRows;
				sa->nCols = img->nCols;
				break;
			}

	for(n = 1 ; n < argc ; n++)
		if(strcmp("-sc", argv[n]) == 0)
			{
			if(sscanf(argv[n+1], "%lf,%lf", &sa->cCol, &sa->cRow) != 2)
				{
				sa->cRow = 0.5;
				sa->cCol = 0.5;
				break;
				}

			break;
			}

	sa->ulRow = (int)(sa->cRow * img->nRows) - sa->nRows / 2;
	sa->ulCol = (int)(sa->cCol * img->nCols) - sa->nCols / 2;
	if(sa->ulRow < 0)
		sa->ulRow = 0;

	if(sa->ulCol < 0)
		sa->ulCol = 0;

	sa->lrRow = (int)(sa->cRow * img->nRows) + sa->nRows / 2;
	sa->lrCol = (int)(sa->cCol * img->nCols) + sa->nCols / 2;
	if(sa->lrRow >= img->nRows)
		sa->lrRow = img->nRows - 1;

	if(sa->lrCol >= img->nCols)
		sa->lrCol = img->nCols - 1;

	}

/*----------------------------------------------------------------------------*/

int EvaluateCPattern(Image *img, int plane, int imgPlanes, CModel *cModel,
  CTemplate *cTemplateIntra, CTemplate *cTemplateInter, PModel *pModel,
  CPattern *cPattern, SearchArea *sa)

	{
	int row, col, pModelIdx, symbol;
	double nNats = 0;

	ResetCModelCounters(cModel);
	for(row = sa->ulRow ; row <= sa->lrRow ; row++)
		for(col = sa->ulCol ; col <= sa->lrCol ; col++)
			{
			pModelIdx = GetPModelIdx(img, row, col, plane, imgPlanes,
			  cModel, cTemplateIntra, cTemplateInter, cPattern);
			ComputePModel(cModel, pModel, pModelIdx);
			symbol = (GetGrayPixel(img, row, col) >> plane) & 0x1;
			nNats += PModelSymbolNats(pModel, symbol);
			UpdateCModelCounter(cModel, pModelIdx, symbol);
			}

	return (int)(nNats / M_LN2 + 0.5);
	}

/*----------------------------------------------------------------------------*/

int FindBestCPatternMode0(Image *img, int imgPlanes, int plane, CModel *cModel,
  CTemplate *cTemplateIntra, CTemplate *cTemplateInter, PModel *pModel,
  CPattern *cPattern, SearchArea *sa)

	{
	int nBits, bestNBits = INT_MAX, count = 1, p, newPattern;
	CPattern *workingCPattern = CreateCPattern(imgPlanes,
	  MAX(cTemplateIntra->size, cTemplateInter->size));

	/* Reset cPattern (which always contains the best pattern so far) */
	CopyCPattern(cPattern, workingCPattern);

	/* Evaluate the null pattern */
	bestNBits = EvaluateCPattern(img, plane, imgPlanes, cModel, 
	  cTemplateIntra, cTemplateInter, pModel, workingCPattern, sa);

	do
		{
		newPattern = 0;
		workingCPattern->totalSize++;
		for(p = plane ; p < imgPlanes ; p++)
			{
			if(workingCPattern->size[p] < (p == plane ? cTemplateIntra->size :
			  cTemplateInter->size))
				{
				workingCPattern->pattern[p][workingCPattern->size[p]] = 1;
				workingCPattern->size[p]++;
				nBits = EvaluateCPattern(img, plane, imgPlanes, cModel,
				  cTemplateIntra, cTemplateInter, pModel, workingCPattern, sa);
				if(nBits < bestNBits)
					{
					bestNBits = nBits;
					CopyCPattern(cPattern, workingCPattern);
					newPattern = 1;
					}

				workingCPattern->size[p]--;
				workingCPattern->pattern[p][workingCPattern->size[p]] = 0;
				printf("%d\r", count++);
				fflush(stdout);
				}

			}

		CopyCPattern(workingCPattern, cPattern);
		}
	while(workingCPattern->totalSize < TEMPLATE_MAX_SIZE && newPattern);

	FreeCPattern(workingCPattern);
	return bestNBits;
	}

/*----------------------------------------------------------------------------*/

int FindBestCPatternMode1(Image *img, int imgPlanes, int plane, CModel *cModel,
  CTemplate *cTemplateIntra, CTemplate *cTemplateInter, PModel *pModel,
  CPattern *cPattern, SearchArea *sa)

	{
	int nBits, bestNBits = INT_MAX, count = 1, p, n, newPattern;
	CPattern *workingCPattern = CreateCPattern(imgPlanes,
	  MAX(cTemplateIntra->size, cTemplateInter->size));

	/* Reset cPattern (which always contains the best pattern so far) */
	CopyCPattern(cPattern, workingCPattern);

	/* Evaluate the null pattern */
	bestNBits = EvaluateCPattern(img, plane, imgPlanes, cModel, 
	  cTemplateIntra, cTemplateInter, pModel, workingCPattern, sa);

	do
		{
		newPattern = 0;
		workingCPattern->totalSize++;
		for(p = plane ; p < imgPlanes ; p++)
			if(workingCPattern->size[p] < (p == plane ? cTemplateIntra->size :
			  cTemplateInter->size))
				for(n = 0 ; n < (p == plane ? cTemplateIntra->size :
				  cTemplateInter->size) ; n++)
					if(!workingCPattern->pattern[p][n])
						{
						workingCPattern->pattern[p][n] = 1;
						workingCPattern->size[p]++;
						nBits = EvaluateCPattern(img, plane, imgPlanes, cModel,
						  cTemplateIntra, cTemplateInter, pModel,
						  workingCPattern, sa);
						if(nBits < bestNBits)
							{
							bestNBits = nBits;
							CopyCPattern(cPattern, workingCPattern);
							newPattern = 1;
							}

						workingCPattern->size[p]--;
						workingCPattern->pattern[p][n] = 0;
						printf("%d\r", count++);
						fflush(stdout);
						}

		CopyCPattern(workingCPattern, cPattern);
		}
	while(workingCPattern->totalSize < TEMPLATE_MAX_SIZE && newPattern);

	FreeCPattern(workingCPattern);
	return bestNBits;
	}

/*----------------------------------------------------------------------------*/

int NextCPatternMode2(int plane, int imgPlanes, CTemplate *cTemplateIntra,
  CTemplate *cTemplateInter, CPattern *cPattern)

	{
	int p, n;

	for(p = plane ; p < imgPlanes ; p++)
		{
		cPattern->pattern[p][cPattern->size[p]] = 1;
		cPattern->size[p]++;
		cPattern->totalSize++;
		if(cPattern->size[p] <= (p == plane ? cTemplateIntra->size :
		  cTemplateInter->size) && cPattern->totalSize <= TEMPLATE_MAX_SIZE)
			return 1;

		for(n = 0 ; n < (p == plane ? cTemplateIntra->size :
          cTemplateInter->size) ; n++)
			cPattern->pattern[p][n] = 0;

		cPattern->totalSize -= cPattern->size[p];
		cPattern->size[p] = 0;
		}

	return 0;
	}

/*----------------------------------------------------------------------------*/

int FindBestCPatternMode2(Image *img, int imgPlanes, int plane, CModel *cModel,
  CTemplate *cTemplateIntra, CTemplate *cTemplateInter, PModel *pModel,
  CPattern *cPattern, SearchArea *sa)

	{
	int nBits, bestNBits = INT_MAX, count = 1, totalCount = 0;
	CPattern *workingCPattern = CreateCPattern(imgPlanes,
	  MAX(cTemplateIntra->size, cTemplateInter->size));

	/* Reset cPattern (which will contain the best pattern found) */
	CopyCPattern(cPattern, workingCPattern);

	do
		{
		totalCount++;
		}
	while(NextCPatternMode2(plane, imgPlanes, cTemplateIntra,
	  cTemplateInter, workingCPattern));
	/* Reset workingPattern */
	CopyCPattern(workingCPattern, cPattern);

	do
		{
		nBits = EvaluateCPattern(img, plane, imgPlanes, cModel,
		  cTemplateIntra, cTemplateInter, pModel, workingCPattern, sa);
		if(nBits < bestNBits)
			{
			bestNBits = nBits;
			CopyCPattern(cPattern, workingCPattern);
			}

		printf("%d / %d\r", totalCount, count++);
		fflush(stdout);
		}
	while(NextCPatternMode2(plane, imgPlanes, cTemplateIntra,
	  cTemplateInter, workingCPattern));

	FreeCPattern(workingCPattern);
	return bestNBits;
	}

/*----------------------------------------------------------------------------*/

int EncodeBitPlane(Image *img, int imgPlanes, int plane, CModel *cModel,
  CTemplate *cTemplateIntra, CTemplate *cTemplateInter, PModel *pModel,
  CPattern *cPattern, FILE *fpOut)

	{
	int row, col, pModelIdx, symbol;
	double nNats = 0;

	ResetCModelCounters(cModel);
	for(row = 0 ; row < img->nRows ; row++)
		for(col = 0 ; col < img->nCols ; col++)
			{
			pModelIdx = GetPModelIdx(img, row, col, plane, imgPlanes,
			  cModel, cTemplateIntra, cTemplateInter, cPattern);
			ComputePModel(cModel, pModel, pModelIdx);
			symbol = (GetGrayPixel(img, row, col) >> plane) & 0x1;
			nNats += PModelSymbolNats(pModel, symbol);
			ArithEncodeSymbol(symbol, pModel->freqs, pModel->sum, fpOut);
			UpdateCModelCounter(cModel, pModelIdx, symbol);
			}

	return (int)(nNats / M_LN2 + 0.5);
	}


/*----------------------------------------------------------------------------*/

int main(int argc, char *argv[])
	{
	int n, p, row, col, codingPlanes, imgPlanes, plane, nBits,
	  totalBits = 0, verbosityLevel, optMode, i, oriImgMaxIntensity;
	Image *img = NULL;
	int deltaNum = DEFAULT_PMODEL_DELTA_NUM,
	  deltaDen = DEFAULT_PMODEL_DELTA_DEN,
	  maxCount = DEFAULT_PMODEL_MAX_COUNT;
	//FILE *fpOut = NULL, *tmp;
	FILE *fpOut = NULL;
	CModel *cModel;
	CTemplate *cTemplateIntra = InitTemplate(TEMPLATE_INTRA);
	CTemplate *cTemplateInter = InitTemplate(TEMPLATE_INTER);
	PModel *pModel;
	CPattern *cPattern;
	SearchArea sa;
	Gray *imgGrays = NULL;
	char hr = 'y';
	//unsigned int n1 = 0, n2 = 0;
	//int symbol, nGrays=0, pixel;
	int nGrays=0, pixel;
	//unsigned long long pModelIdx;
	//double nNats = 0;	

	codingPlanes = 16;
	verbosityLevel = 0;
	optMode = 0;
	if(argc == 1)
		{
		fprintf(stderr, "Usage: %16s [ -v (verbose) ]\n", argv[0]);
		//fprintf(stderr, "Usage: Micro3DEncBestHR [ -v (verbose) ]\n");
		//fprintf(stderr, "                      [ -O optMode (def 0) ]\n");
		fprintf(stderr, "                        [ -o outputFile ]\n");
		//fprintf(stderr, "                      [ -delta d/n (def. %d/%d) ]\n",
		  //deltaNum, deltaDen);
		//fprintf(stderr, "                      [ -mc maxCount (def %d) ]\n",
		  //maxCount);
		fprintf(stderr, "                        [ -p codingPlanes (def %d) ]\n",
		  codingPlanes);
		fprintf(stderr,
		  "                        [ -sa nCxnR (search area, def 256x256) ]\n");
		fprintf(stderr,
		  "                        [ -sc cC,cR (search center, def 0.5,0.5) ]\n");
		fprintf(stderr,
		  "                        [ -hr (activate Histrogram Reduction) ]\n");
		fprintf(stderr, 
		  "                        img\n");
		return 1;
		}

	if(!(img = ReadImageFile(argv[argc - 1])))
		return 1;

	for(n = 1 ; n < argc ; n++)
		if(strcmp("-v", argv[n]) == 0)
			verbosityLevel++;

	for(n = 1 ; n < argc ; n++)
		if(strcmp("-O", argv[n]) == 0)
			{
			optMode = atoi(argv[n + 1]);
			if(optMode < 0 || optMode > 2)
				optMode = 0;

			break;
			}

	for(n = 1 ; n < argc ; n++)
		if(strcmp("-o", argv[n]) == 0)
			{
			if(!(fpOut = fopen(argv[n+1], "w")))
				{
				fprintf(stderr, "Error: can't open file\n");
				exit(1);
				}

			break;
			}

	for(n = 1 ; n < argc ; n++)
		if(!strcmp("-delta", argv[n]))
			{
			if(sscanf(argv[n+1], "%d/%d", &deltaNum, &deltaDen) != 2)
				{
				deltaNum = DEFAULT_PMODEL_DELTA_NUM;
				deltaDen = DEFAULT_PMODEL_DELTA_DEN;
				}

			break;
			}

	for(n = 1 ; n < argc ; n++)
		if(!strcmp("-mc", argv[n]))
			{
			maxCount = atoi(argv[n+1]);
			if(maxCount == 0 || maxCount > DEFAULT_PMODEL_MAX_COUNT)
				fprintf(stderr, "Warning (maxCount): counters may overflow\n");

			break;
			}

	for(n = 1 ; n < argc ; n++)
		if(!strcmp("-p", argv[n]))
			{
			codingPlanes = atoi(argv[n+1]);
			if(codingPlanes < 1)
				codingPlanes = 1;

			break;
			}
	for(n = 1 ; n < argc ; n++)
		if(!strcmp("-hr", argv[n]))
		{
			hr = 'y';
		}

	SetSearchArea(img, &sa, argc, argv);

	if(fpOut == NULL)
		fpOut = fopen("/dev/null", "w");

	/* Force re-calculation of the image maximum intensity */
	img->maxIntensity = 0;
	img->maxIntensity = ImageMaxIntensity(img);
	imgPlanes = (int)(log(img->maxIntensity) / M_LN2) + 1;
	
	oriImgMaxIntensity = img->maxIntensity;
	
	if(codingPlanes > imgPlanes)
		codingPlanes = imgPlanes;
	

	/*************************************************************/
	if(hr == 'y')
	{
		/* Array of grays */
		if(!(imgGrays = (Gray *)calloc(img->maxIntensity + 1, sizeof(Gray))))
		{
			fprintf(stderr, "Error(main): no memory!\n");
			return EXIT_FAILURE;
		}
	
		/* Discovering the gray values that are used */
		for(row = 0 ; row < img->nRows ; row++)
			for(col = 0 ; col < img->nCols ; col++)
			{
				imgGrays[GetGrayPixel(img, row, col)].usage = 1;
			}
			
		for(i=0; i < img->maxIntensity + 1; i++)
		{
			imgGrays[i].value = i;
			if(imgGrays[i].usage)
			{
				imgGrays[i].codeword = nGrays;
				nGrays++;
			}
		}
	
		for(row = 0 ; row < img->nRows ; row++)
			for(col = 0 ; col < img->nCols ; col++)
			{
				// Get the pixel value
				pixel = GetGrayPixel(img, row, col);
				// Update the pixel with the new pixel value
				PutGrayPixel(img, row, col, imgGrays[pixel].codeword);				
			}
		
		img->maxIntensity = 0;
		img->maxIntensity = ImageMaxIntensity(img);
		imgPlanes = (int)(log(img->maxIntensity) / M_LN2) + 1;
		if(codingPlanes > imgPlanes)
			codingPlanes = imgPlanes;
		
	}
	/*************************************************************/

	printf("Image has %d rows, %d cols and %d bit-planes\n", img->nRows, img->nCols, imgPlanes);
	printf("Encoding the %d most significant bit-planes\n", codingPlanes);
	printf("Using search area ul=%d,%d lr=%d,%d\n", sa.ulCol, sa.ulRow,
	  sa.lrCol, sa.lrRow);
	printf("Image max intensity: %d\n", img->maxIntensity);
	if(hr == 'y')
	{
		printf("Number of grays: %d\n", nGrays);
	}
	
	printf("Optimization mode: %d\n", optMode);

	printf("Using intra template:\n\n");
	ShowTemplate(cTemplateIntra);
	putchar('\n');
	printf("Using inter template:\n\n");
	ShowTemplate(cTemplateInter);
	putchar('\n');

	cModel = CreateCModel(TEMPLATE_MAX_SIZE, N_SYMBOLS, N_CTX_SYMBOLS,
	  deltaNum, deltaDen, maxCount);
	pModel = CreatePModel(N_SYMBOLS);
	cPattern = CreateCPattern(imgPlanes, MAX(cTemplateIntra->size,
	  cTemplateInter->size));

	startoutputtingbits();
	start_encode();

	/* Store the number of rows and cols */
	WriteNBits(img->nRows, STORAGE_BITS_N_ROWS, fpOut);
	WriteNBits(img->nCols, STORAGE_BITS_N_COLS, fpOut);

	/* Store the number of image bit-planes (1..16) */
	WriteNBits(imgPlanes - 1, STORAGE_BITS_N_IMG_PLANES, fpOut);

	/* Store the number of encoded bit-planes (1..16) */
	WriteNBits(codingPlanes - 1, STORAGE_BITS_N_COD_PLANES, fpOut);
	
	WriteNBits(oriImgMaxIntensity - 1, STORAGE_BITS_MAX_INTENSITY, fpOut);
	
	

	WriteNBits(cModel->deltaNum - 1, STORAGE_BITS_PMODEL_DELTA_NUM, fpOut);
	WriteNBits(cModel->deltaDen - 1, STORAGE_BITS_PMODEL_DELTA_DEN, fpOut);
	WriteNBits(maxCount, STORAGE_BITS_PMODEL_MAX_COUNT, fpOut);
	
	/*
	if(!(tmp = fopen("grays.dat", "w")))
	{
		fprintf(stderr, "Error: can't open file\n");
		exit(1);
	}
	*/
	/*************************************************************/
	if(hr == 'y')
	{
		/* We are using Histogram Reduction */
		WriteNBits(1, 1, fpOut);

		//for(i = 0 ; i < img->maxIntensity + 1; i++)
		for(i = 0 ; i < oriImgMaxIntensity + 1; i++)
		{
			
			if(imgGrays[i].usage)
			{
				WriteNBits(1, 1, fpOut);
				//fprintf(tmp,"1");
			}
			else
			{
				WriteNBits(0, 1, fpOut);
				//fprintf(tmp,"0");
			}
		}
		printf("Gray total bits = %d\n", img->maxIntensity + 1);
	}
	
	/* The HR is not used */
	else
		WriteNBits(0, 1, fpOut);
	/*************************************************************/
	
	
	for(plane = imgPlanes - 1 ; plane >= imgPlanes - codingPlanes ; plane--)
	{
		switch(optMode)
		{
			case 0:
				FindBestCPatternMode0(img, imgPlanes, plane, cModel, cTemplateIntra, cTemplateInter, pModel, cPattern, &sa);
				break;
			case 1:
				FindBestCPatternMode1(img, imgPlanes, plane, cModel, cTemplateIntra, cTemplateInter, pModel, cPattern, &sa);
				break;

			case 2:
				FindBestCPatternMode2(img, imgPlanes, plane, cModel, cTemplateIntra, cTemplateInter, pModel, cPattern, &sa);
				break;
		}

		// Store the context pattern for this bit-plane 
		for(p = plane ; p < imgPlanes ; p++)
			for(n = 0 ; n < (p == plane ? cTemplateIntra->size : cTemplateInter->size) ; n++)
				WriteNBits(cPattern->pattern[p][n], 1, fpOut);

		
		
		nBits = EncodeBitPlane(img, imgPlanes, plane, cModel, cTemplateIntra, cTemplateInter, pModel, cPattern, fpOut);

		totalBits += nBits;
		if(verbosityLevel > 0)
			{
			printf("Plane %2d: %d bits ( %.3f bpp ) %d: ", plane, nBits,
			  (double)nBits / (img->nRows * img->nCols), cPattern->totalSize);
			for(p = imgPlanes - 1 ; p >= plane ; p--)
				printf("%d ", cPattern->size[p]);

			putchar('\n');
			if(verbosityLevel > 1)
				for(p = imgPlanes - 1 ; p >= plane ; p--)
					{
					for(n = 0 ; n < (p == plane ? cTemplateIntra->size : cTemplateInter->size) ; n++)
						printf("%d", cPattern->pattern[p][n]);
					putchar('\n');
					}

			}

		}


 	// Send the remaining bit-planes uncoded.

	for(plane = imgPlanes - codingPlanes - 1 ; plane >= 0 ; plane--)
		for(row = 0 ; row < img->nRows ; row++)
			for(col = 0 ; col < img->nCols ; col++)
			{
				WriteNBits(GetGrayPixel(img, row, col) >> plane, 1, fpOut);	
			}
	
	finish_encode(fpOut);
	doneoutputtingbits(fpOut);

	fclose(fpOut);
	printf("Number of bytes of the encoded image: %"PRIu64" ( %.3f bpp )\n",
	  (uint64_t)_bytes_output, _bytes_output * 8. / (img->nRows * img->nCols));
	return 0;
	}

