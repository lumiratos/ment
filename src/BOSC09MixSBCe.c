/*------------------------------------------------------------------------------

Copyright 2006 Armando J. Pinho (ap@ua.pt), 
          2009 Antonio J. R. Neves (an@ua.pt), 
          2014 Luis M. O. Matos (luismatos@ua.pt), All Rights Reserved.

These programs are supplied free of charge for research purposes only,
and may not be sold or incorporated into any commercial product. There is
ABSOLUTELY NO WARRANTY of any sort, nor any undertaking that they are
fit for ANY PURPOSE WHATSOEVER. Use them at your own risk. If you do
happen to find a bug, or have modifications to suggest, please report
the same to Armando J. Pinho, ap@ua.pt or any of the previous mentioned
names in the header. The copyright notice above and this statement of 
conditions must remain an integral part of each and every copy made of 
these files.

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
	//int nBits, bestNBits = INT_MAX, count = 1, p, newPattern;
	int nBits, bestNBits = INT_MAX, p, newPattern;
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
				//printf("%d\r", count++);
				//fflush(stdout);
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
	//int nBits, bestNBits = INT_MAX, count = 1, p, n, newPattern;
	int nBits, bestNBits = INT_MAX, p, n, newPattern;
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
						//printf("%d\r", count++);
						//fflush(stdout);
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
	//int nBits, bestNBits = INT_MAX, count = 1, totalCount = 0;
	int nBits, bestNBits = INT_MAX, totalCount = 0;
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

		//printf("%d / %d\r", totalCount, count++);
		//fflush(stdout);
		}
	while(NextCPatternMode2(plane, imgPlanes, cTemplateIntra,
	  cTemplateInter, workingCPattern));

	FreeCPattern(workingCPattern);
	return bestNBits;
	}
	
/*----------------------------------------------------------------------------*/
	
double EvaluateContextSize(Image *img, Image *expImg, int plane,
	CModel *cModel, CTemplate *cTemplateSBC, PModel *pModel)
		
	{
	int row, col, pModelIdx=0, symbol, y;
	double nNats = 0;
	ResetCModelCounters(cModel);
		
	for(row = 0 ; row < img->nRows ; row++)
		{
		for(col = 0 ; col < img->nCols ; col++)
			{
			pModelIdx = GetKikuchiPModelIdx(expImg, row, col, cTemplateSBC);
			ComputePModel(cModel, pModel, pModelIdx);
			symbol = (GetGrayPixel(img, row, col) >> plane) & 0x1;
								
			nNats += PModelSymbolNats(pModel, symbol);
			UpdateCModelCounter(cModel, pModelIdx, symbol);
				
				
			y = GetGrayPixel(expImg, row, col) + symbol * (0x1 << plane);
			if((plane - 1) >= 0) 
				y -= (0x1 << (plane - 1));
			PutGrayPixel(expImg, row, col, y);
			}
		}
	return (nNats / M_LN2 + 0.5);
	}

/*----------------------------------------------------------------------------*/

int GetBestContextSize(Image *img, Image *expImg, int plane,
	CModel *cModel, CTemplate *cTemplateSBC, PModel *pModel)
		
	{
	Image *cloneExpImg;
	int ctxSize = 0, bestCtxSize = cTemplateSBC->size;
	double bestBits = 0, nBits = 0;
		
	cloneExpImg = CreateImage(expImg->nRows, expImg->nCols, expImg->dataType, expImg->nPlanes);
	if(cloneExpImg == NULL)
		{
		fprintf(stderr, "Error: memory allocation!\n");
		exit(EXIT_FAILURE);
		}
		
	CopyImage(expImg, cloneExpImg);
	bestBits = EvaluateContextSize(img, cloneExpImg, plane, cModel, cTemplateSBC, pModel);
		
	for(ctxSize = cTemplateSBC->size - 1; ctxSize > 0; ctxSize--)
		{
		// Update context size
		cTemplateSBC->size = ctxSize;
			
		// Copy image with the estimated pixel values
		CopyImage(expImg, cloneExpImg);
			
		// Evaluate context size
		nBits = EvaluateContextSize(img, cloneExpImg, plane, cModel, cTemplateSBC, pModel);
		if(nBits < bestBits)
			{
			bestCtxSize = ctxSize;
			bestBits = nBits;
			}			
		}
	return bestCtxSize;
	}

/*----------------------------------------------------------------------------*/
	
int GetGreedyContextSizeV1(Image *img, Image *expImg, int plane, 
	CModel *cModel, CTemplate *cTemplateSBC, PModel *pModel)
		
	{
	Image *cloneExpImg;
	int ctxSize = 0, bestCtxSize = cTemplateSBC->size;
	double bestBits = 0, nBits = 0;
		
	cloneExpImg = CreateImage(expImg->nRows, expImg->nCols, expImg->dataType, expImg->nPlanes);
	if(cloneExpImg == NULL)
		{
		fprintf(stderr, "Error: memory allocation!\n");
		exit(EXIT_FAILURE);
		}
		
	CopyImage(expImg, cloneExpImg);
	bestBits = EvaluateContextSize(img, cloneExpImg, plane, cModel, cTemplateSBC, pModel);
	
	for(ctxSize = cTemplateSBC->size - 1; ctxSize > 0; ctxSize--)
		{
		// Update context size
		cTemplateSBC->size = ctxSize;
		
		// Copy image with the estimated pixel values
		CopyImage(expImg, cloneExpImg);
		
		// Evaluate context size
		nBits = EvaluateContextSize(img, cloneExpImg, plane, cModel, cTemplateSBC, pModel);
		if(nBits < bestBits)
			{
			bestCtxSize = ctxSize;
			bestBits = nBits;
			}
		// The obtained number of bits is worse than the previous best, stop the searching
		// procedure
		else
			break;
		}
	
	return bestCtxSize;
	}
	
/*----------------------------------------------------------------------------*/
	
int GetGreedyContextSizeV2(Image *img, Image *expImg, int plane,
	CModel *cModel, CTemplate *cTemplateSBC, PModel *pModel)
		
	{
	Image *cloneExpImg;
	int ctxSize = 0, maxContextSize = cTemplateSBC->size;
	int middleCtxSize = maxContextSize/2;
	double bestBitsL, bestBitsR, nBits;
	int bestCtxSizeL, bestCtxSizeR;
	
	cloneExpImg = CreateImage(expImg->nRows, expImg->nCols, expImg->dataType, expImg->nPlanes);
	if(cloneExpImg == NULL)
		{
		fprintf(stderr, "Error: memory allocation!\n");
		exit(EXIT_FAILURE);
		}
		
	CopyImage(expImg, cloneExpImg);
	
	// Starting from middleCtxSize to the left
	cTemplateSBC->size = middleCtxSize;
	bestCtxSizeL = bestCtxSizeR = middleCtxSize;
	bestBitsL = bestBitsR = EvaluateContextSize(img, cloneExpImg, plane, cModel, cTemplateSBC, pModel);
		
	for(ctxSize = middleCtxSize-1; ctxSize > 0; ctxSize--)
		{
		// Update context size
		cTemplateSBC->size = ctxSize;
		
		// Copy image with the estimated pixel values
		CopyImage(expImg, cloneExpImg);
		
		// Evaluate context size
		nBits = EvaluateContextSize(img, cloneExpImg, plane, cModel, cTemplateSBC, pModel);	
				
		if(nBits < bestBitsL)
			{
			bestCtxSizeL = ctxSize;
			bestBitsL = nBits;
			}
		// The obtained number of bits is worse than the previous best, stop the searching
		// procedure
		else
			break;
		}	
	
	CopyImage(expImg, cloneExpImg);
	
	// Starting from middleCtxSize to the right
	for(ctxSize = middleCtxSize+1; ctxSize <= maxContextSize; ctxSize++)
		{
		
		// Update context size
		cTemplateSBC->size = ctxSize;
		
		// Copy image with the estimated pixel values
		CopyImage(expImg, cloneExpImg);
		
		// Evaluate context size
		nBits = EvaluateContextSize(img, cloneExpImg, plane, cModel, cTemplateSBC, pModel);	
		
		if(nBits < bestBitsR)
			{
			bestCtxSizeR = ctxSize;
			bestBitsR = nBits;
			}
		// The obtained number of bits is worse than the previous best, stop the searching
		// procedure
		else
			break;
		}	
	
	// Verify wich context size produces less bits
	if(bestBitsL < bestBitsR)
		return bestCtxSizeL;
	return bestCtxSizeR;
	
	}

/*----------------------------------------------------------------------------*/

int GetGreedyContextSizeV3(Image *img, Image *expImg, int plane,
	CModel *cModel, CTemplate *cTemplateSBC, PModel *pModel)
		
	{
	Image *cloneExpImg;
	int ctxSize = 0, maxContextSize = cTemplateSBC->size;
	int middleCtxSize = maxContextSize/2;
	double bestBitsL, bestBitsR, nBits;
	int bestCtxSizeL, bestCtxSizeR;
	
	cloneExpImg = CreateImage(expImg->nRows, expImg->nCols, expImg->dataType, expImg->nPlanes);
	if(cloneExpImg == NULL)
		{
		fprintf(stderr, "Error: memory allocation!\n");
		exit(EXIT_FAILURE);
		}
		
	CopyImage(expImg, cloneExpImg);
	
	// Starting from 1 to middleCtxSize
	cTemplateSBC->size = 1;
	bestCtxSizeL = 1;
	bestBitsL = EvaluateContextSize(img, cloneExpImg, plane, cModel, cTemplateSBC, pModel);
	
	for(ctxSize = 2; ctxSize <= middleCtxSize; ctxSize++)
		{
		// Update context size
		cTemplateSBC->size = ctxSize;
		
		// Copy image with the estimated pixel values
		CopyImage(expImg, cloneExpImg);
		
		// Evaluate context size
		nBits = EvaluateContextSize(img, cloneExpImg, plane, cModel, cTemplateSBC, pModel);	
		
		if(nBits < bestBitsL)
			{
			bestCtxSizeL = ctxSize;
			bestBitsL = nBits;
			}
		// The obtained number of bits is worse than the previous best, stop the searching
		// procedure
		else
			break;
		}	
	
	CopyImage(expImg, cloneExpImg);
		
	// Starting from maxContextSize to middleCtxSize
	cTemplateSBC->size = maxContextSize;
	bestCtxSizeR = maxContextSize;
	bestBitsR = EvaluateContextSize(img, cloneExpImg, plane, cModel, cTemplateSBC, pModel);
		
	// Starting from maxContextSize to middleCtxSize
	for(ctxSize = maxContextSize-1; ctxSize > middleCtxSize; ctxSize--)
		{			
		// Update context size
		cTemplateSBC->size = ctxSize;
		
		// Copy image with the estimated pixel values
		CopyImage(expImg, cloneExpImg);
		
		// Evaluate context size
		nBits = EvaluateContextSize(img, cloneExpImg, plane, cModel, cTemplateSBC, pModel);	
		
		if(nBits < bestBitsR)
			{
			bestCtxSizeR = ctxSize;
			bestBitsR = nBits;
			}
		// The obtained number of bits is worse than the previous best, stop the searching
		// procedure
		else
			break;
		}	
	
	// Verify wich context size produces less bits
	if(bestBitsL < bestBitsR)
		return bestCtxSizeL;
	return bestCtxSizeR;
	
	}
	
/*----------------------------------------------------------------------------*/

int main(int argc, char *argv[])

	{
	char outFileName[FILENAME_MAX]= "";
	//int n, p, row, col, codingPlanes, imgPlanes, plane, nBits,
	//  totalBits = 0, verbosityLevel, optMode;
	int n, p, row, col, codingPlanes, imgPlanes, plane,
		verbosityLevel, optMode;
	
	int nChrs1 = (int)strlen(argv[0]), nChrs2;
	nChrs2 = nChrs1 + (int)strlen("Usage: ");
	
	Image *img, *expImg;
	int deltaNum = DEFAULT_PMODEL_DELTA_NUM,
	  deltaDen = DEFAULT_PMODEL_DELTA_DEN,
	  maxCount = DEFAULT_PMODEL_MAX_COUNT;
	FILE *fpOut = NULL;
	//CModel *cModel;
	CTemplate *cTemplateIntra = InitTemplate(TEMPLATE_INTRA);
	CTemplate *cTemplateInter = InitTemplate(TEMPLATE_INTER);
	CTemplate *cTemplateSBC = InitTemplate(TEMPLATE_KIKUCHI);
	//PModel *pModel;
	CPattern *cPattern;
	SearchArea sa;

	// Variables needed for the model mixture
	int nCModels = 2, ctxMode = KIKUCHI_CONTEXT_GREEDY_MODE, cModel, s;
	//int nCModels = 0, cModel, s, bestWeightCModel, bestCModel = 0, lastBestCModel = 0, worstCModel = 0, lastWorstCModel = 0;
	//int bestCModel = 0, worstCModel = 0, bestWeightCModel, lastBestCModel = 0, lastWorstCModel = 0;
	int bestCModel = 0;
	int tmpSize=0, y;
	CModel **cModels = NULL;
	PModel **pModels = NULL, *mixPModel=NULL;
	FloatPModel *floatPModel = NULL;
	double bestWeight, nats = 0, totalWeight = 0, gamma = 0.99, *cModelsNats,
	  *cModelsWeight, *cModelsProb, bestCModelNats, worstCModelNats, *cModelsGlobalNats,
	  totalNats = 0, bestNats = 0, bestTotalNats = 0, totalPartialNats = 0;
	unsigned *cModelsGlobalUsage;
	int pModelIdx, symbol, oriSBCTemplateSize = cTemplateSBC->size, ctxSize=0;
	unsigned int **modelsCount = NULL;
	unsigned int modelsPerformance[2];
	// **************************************


	codingPlanes = 16;
	verbosityLevel = 0;
	optMode = 0;
	gamma = ((int)(gamma * GAMMA_K)/(double)GAMMA_K);
	
	if(argc == 1)
		{
		fprintf(stderr, "\n");
		fprintf(stderr, "Usage: %*s [ -v (verbose) ]\n", nChrs1, argv[0]);
		fprintf(stderr, "%*s [ -o outputFile ]\n", nChrs2, " ");
		fprintf(stderr, "%*s [ -p codingPlanes (def %d) ]\n", nChrs2, " ", codingPlanes);
		fprintf(stderr, "%*s [ -sa nCxnR (search area, def 256x256) ]\n", nChrs2, " ");
		fprintf(stderr, "%*s [ -best (search area, all image) ]\n", nChrs2, " ");
		fprintf(stderr, "%*s [ -sc cC,cR (search center, def 0.5,0.5) ]\n", nChrs2, " ");
		
		// To be used on the bit-modeling by pixels values estimate
		fprintf(stderr, "%*s [ -ctxMode best/greedy ]\n", nChrs2, " ");
		
		
		fprintf(stderr, "%*s img\n", nChrs2, " ");
		return EXIT_FAILURE;
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
			if(!(fpOut = fopen(argv[n+1], "wb")))
				{
				fprintf(stderr, "Error: can't open file '%s'\n", argv[n+1]);
				return EXIT_FAILURE;
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
		if(strcmp("-ctxMode", argv[n]) == 0)
			{
			if(strcmp("best", argv[n+1]) == 0)
				ctxMode = KIKUCHI_CONTEXT_BEST_MODE;
			if(strcmp("greedy", argv[n+1]) == 0)
				ctxMode = KIKUCHI_CONTEXT_GREEDY_MODE;
			/*
			if(strcmp("greedy2", argv[n+1]) == 0)
				ctxMode = CONTEXT_GREEDY_MODE2;
			if(strcmp("greedy3", argv[n+1]) == 0)
				ctxMode = CONTEXT_GREEDY_MODE3;
			*/
			}
				
	SetSearchArea(img, &sa, argc, argv);


	// ************************************************************************************
	
	// Only two models for this case
	nCModels = 2;
	
	if(!(cModels = (CModel **)calloc(nCModels, sizeof(CModel *))))		
		{
		fprintf(stderr, "Error: out of memory\n"); 
		return EXIT_FAILURE; 
		}
	
	if(!(pModels = (PModel **)calloc(nCModels, sizeof(PModel *))))		
		{		
		fprintf(stderr, "Error: out of memory\n");		
		return EXIT_FAILURE;		
		}	
		
	if(!(cModelsNats = (double *)calloc(nCModels, sizeof(double))))		
		{
		fprintf(stderr, "Error: out of memory\n");		
		return EXIT_FAILURE;		
		}
		
	if(!(cModelsWeight = (double *)calloc(nCModels, sizeof(double))))		
		{		
		fprintf(stderr, "Error: out of memory\n");		
		return EXIT_FAILURE;		
		}	
		
	if(!(cModelsProb = (double *)calloc(nCModels, sizeof(double))))		
		{
		fprintf(stderr, "Error: out of memory\n");		
		return EXIT_FAILURE;		
		}
			
	if(!(cModelsGlobalNats = (double *)calloc(nCModels, sizeof(double))))		
		{
		fprintf(stderr, "Error: out of memory\n");		
		return EXIT_FAILURE;		
		}	
		
	if(!(cModelsGlobalUsage = (unsigned *)calloc(nCModels, sizeof(unsigned))))		
		{
		fprintf(stderr, "Error: out of memory\n");		
		return EXIT_FAILURE;		
		}	
		
	if(!(modelsCount = (unsigned int **)calloc(nCModels, sizeof(unsigned int *))))		
		{		
		fprintf(stderr, "Error: out of memory\n");		
		return EXIT_FAILURE;		
		}		
	
		
		
	// ************************************************************************************

	if(fpOut == NULL)
		{
		tmpSize = snprintf(outFileName, FILENAME_MAX, "%s.enc", argv[argc - 1]);
		if(tmpSize >= FILENAME_MAX)
			{
			fprintf(stderr, "Error: can't use string '%s' as a file name. Please use the '-o' flag.\n", outFileName);
			return EXIT_FAILURE;
			}
		if(!(fpOut = fopen(outFileName, "wb")))
			{
			fprintf(stderr, "Error: can't open file '%s' in binary writting mode!\n", outFileName);
			return EXIT_FAILURE;
			}
		}
			
	/* Force re-calculation of the image maximum intensity */
	img->maxIntensity = 0;
	img->maxIntensity = ImageMaxIntensity(img);
	imgPlanes = (int)(log(img->maxIntensity) / M_LN2) + 1;

	if(codingPlanes > imgPlanes)
		codingPlanes = imgPlanes;

	printf("Image has %d rows, %d cols and %d bit-planes\n",
	  img->nRows, img->nCols, imgPlanes);
	printf("Encoding the %d most significant bit-planes\n", codingPlanes);
	printf("Using search area ul=%d,%d lr=%d,%d\n", sa.ulCol, sa.ulRow,
	  sa.lrCol, sa.lrRow);
	printf("Optimization mode: %d\n", optMode);

	printf("Using intra template:\n\n");
	ShowTemplate(cTemplateIntra);
	putchar('\n');
	printf("Using inter template:\n\n");
	ShowTemplate(cTemplateInter);
	putchar('\n');
	printf("Using alternative template:\n\n");
	ShowTemplate(cTemplateSBC);
	putchar('\n');
	
	
	// ************************************************************
	for(cModel=0; cModel < nCModels; cModel++)
		{
		if(cModel == 0)
			{
			// For the 3D Finite Context Model
			cModels[cModel] = CreateCModel(TEMPLATE_MAX_SIZE, N_SYMBOLS, N_CTX_SYMBOLS, deltaNum, deltaDen, maxCount);
			}
		else
			{
			// For the alternative model based on bit-plane modeling based on pixel value estimates
			cModels[cModel] = CreateCModel(cTemplateSBC->size, N_SYMBOLS, N_CTX_SYMBOLS, deltaNum, deltaDen, maxCount);
			}
			
		cModelsProb[cModel] = 1.0 / nCModels;
		cModelsWeight[cModel] = cModelsProb[cModel];
		totalWeight += cModelsWeight[cModel];
		pModels[cModel] = CreatePModel(N_SYMBOLS);	
	
		}
	// 3D Pattern
	cPattern = CreateCPattern(imgPlanes, MAX(cTemplateIntra->size, cTemplateInter->size));				
	
	// The mix model
	mixPModel = CreatePModel(N_SYMBOLS);
	floatPModel = CreateFloatPModel(N_SYMBOLS);
	// ************************************************************

	startoutputtingbits();
	start_encode();

	/* Store the number of rows and cols */
	WriteNBits(img->nRows, STORAGE_BITS_N_ROWS, fpOut);
	WriteNBits(img->nCols, STORAGE_BITS_N_COLS, fpOut);

	/* Store the number of image bit-planes (1..16) */
	WriteNBits(imgPlanes - 1, STORAGE_BITS_N_IMG_PLANES, fpOut);

	/* Store the number of encoded bit-planes (1..16) */
	WriteNBits(codingPlanes - 1, STORAGE_BITS_N_COD_PLANES, fpOut);

	//WriteNBits(cModel->deltaNum - 1, STORAGE_BITS_PMODEL_DELTA_NUM, fpOut);
	//WriteNBits(cModel->deltaDen - 1, STORAGE_BITS_PMODEL_DELTA_DEN, fpOut);
	WriteNBits(maxCount, STORAGE_BITS_PMODEL_MAX_COUNT, fpOut);
	WriteNBits((int)(gamma * GAMMA_K), STORAGE_BITS_GAMMA, fpOut);	

	// ---------------------------------------------------------------------------
	// Create image with zeros
	if(!(expImg = CreateImage(img->nRows, img->nCols, imgPlanes <= 8 ? DATA_TYPE_C : DATA_TYPE_S, 1)))
		return EXIT_FAILURE;
	
	for(row = 0 ; row < expImg->nRows ; row++)
		for(col = 0 ; col < expImg->nCols ; col++)
			{
			// Initialize estimate of a decoded pixel value 2^(N-1) - 1
			PutGrayPixel(expImg, row, col, ((0x1 << (imgPlanes-1)) - 1));
			}
	// ---------------------------------------------------------------------------


	for(plane = imgPlanes - 1 ; plane >= imgPlanes - codingPlanes ; plane--)
		{
		// Used to count the performance of model
		modelsPerformance[0x0] = 0;
		modelsPerformance[0x1] = 0;
		
		// Reset counters
		ResetCModelCounters(cModels[0x0]);
		ResetCModelCounters(cModels[0x1]);
		
		// ---------------------------------------------------------------------------
		// For the 3D Models
		switch(optMode)
			{
			case 0:
				FindBestCPatternMode0(img, imgPlanes, plane, cModels[0x0],
					cTemplateIntra, cTemplateInter, pModels[0x0], cPattern, &sa);
				break;

			case 1:
				FindBestCPatternMode1(img, imgPlanes, plane, cModels[0x0],
					cTemplateIntra, cTemplateInter, pModels[0x0], cPattern, &sa);
				break;

			case 2:
				FindBestCPatternMode2(img, imgPlanes, plane, cModels[0x0],
					cTemplateIntra, cTemplateInter, pModels[0x0], cPattern, &sa);
				break;

			}
		
		// Reset counter of the 3D FCM
		ResetCModelCounters(cModels[0x0]);
		
		// Store the context pattern for this bit-plane 
		for(p = plane ; p < imgPlanes ; p++)
			for(n = 0 ; n < (p == plane ? cTemplateIntra->size :
			  cTemplateInter->size) ; n++)
				  WriteNBits(cPattern->pattern[p][n], 1, fpOut);
		// ---------------------------------------------------------------------------
	
		// ---------------------------------------------------------------------------
		// For the Simple Bitplane coding
	  	// Reset context template size
	  	cTemplateSBC->size = oriSBCTemplateSize;
		
			
	  	// Compute the best context size
	  	switch(ctxMode)
	  		{
	  		case KIKUCHI_CONTEXT_BEST_MODE: ctxSize = GetBestContextSize(img, expImg, plane, 
	  								cModels[0x1], cTemplateSBC, pModels[0x1]);
									break;
				
	  		case KIKUCHI_CONTEXT_GREEDY_MODE: ctxSize = GetGreedyContextSizeV1(img, expImg, plane, 
	  									cModels[0x1], cTemplateSBC, pModels[0x1]);
	  									break;
			/*
	  		case CONTEXT_GREEDY_MODE2:	ctxSize = GetGreedyContextSizeV2(img, expImg, plane, 
	  									cModels[0x1], cTemplateSBC, pModels[0x1]);
	  									break;
										
	  		case CONTEXT_GREEDY_MODE3:	ctxSize = GetGreedyContextSizeV3(img, expImg, plane, 
	  									cModels[0x1], cTemplateSBC, pModels[0x1]);
	  									break;
			*/
	  		}
			
	  	// Write the context size to the output stream	
	  	cTemplateSBC->size = ctxSize;
	  	WriteNBits(ctxSize,STORAGE_BITS_CONTEXT_SIZE,fpOut);
			 
		// ---------------------------------------------------------------------------	  
		// Reset counter of the SBC using pixel values estimates
		ResetCModelCounters(cModels[0x1]);
		cModelsNats[0x0] = 0;
		cModelsNats[0x1] = 0;
		
		
		totalPartialNats = 0.0;				
		for(row = 0 ; row < img->nRows ; row++)
			{
			for(col = 0 ; col < img->nCols ; col++)
				{
				//bestWeightCModel = 0;
				bestWeight = 0;
				
				for(s = 0 ; s < N_SYMBOLS ; s++)
					floatPModel->freqs[s] = 0;
					
				bestCModelNats = DBL_MAX;
				worstCModelNats = 0;
				
				// Get the symbol to encode 0x0 or 0x1
				symbol = (GetGrayPixel(img, row, col) >> plane) & 0x1;
				
				// For each model
				for(cModel = 0 ; cModel < nCModels ; cModel++)
					{
						// 3D template context
						if(cModel == 0x0)
							pModelIdx = GetPModelIdx(img, row, col, plane, imgPlanes,
			 		   			cModels[cModel], cTemplateIntra, cTemplateInter, cPattern);
						// SBC using pixel values estimate
						else
							pModelIdx = GetKikuchiPModelIdx(expImg, row, col, cTemplateSBC);
						
						ComputePModel(cModels[cModel], pModels[cModel], pModelIdx);
						
						nats = PModelSymbolNats(pModels[cModel], symbol);
						cModelsNats[cModel] += nats;
					
						if(nats < bestCModelNats)
						{
							bestCModelNats = nats;
							bestCModel = cModel;
						}
										
						if(cModelsWeight[cModel] > bestWeight)
						{
							bestWeight = cModelsWeight[cModel];
							//bestWeightCModel = cModel;
						}
					
						if(nats > worstCModelNats)
						{
							worstCModelNats = nats;
							//worstCModel = cModel;
						}
					
						for(s = 0 ; s < N_SYMBOLS ; s++)
							{
							floatPModel->freqs[s] += (double) pModels[cModel]->freqs[s] /
							pModels[cModel]->sum * (cModelsWeight[cModel] / totalWeight);		
							}
					}
				
				cModelsGlobalUsage[bestCModel]++;
				cModelsGlobalNats[bestCModel] += bestCModelNats;
				
				// For each bitplane this vector will be reseted to 0
				modelsPerformance[bestCModel]++;
				
				// Transform floating probabilities back to integers.
				// This is needed by the arithmetic encoder. It was left
				// here just for maintaining the parallel with a real
				// encoder.
				
				mixPModel->sum = 0;
				for(s = 0 ; s < N_SYMBOLS ; s++)
					{
					mixPModel->freqs[s] = (unsigned) (floatPModel->freqs[s] * maxCount);
					
					if(!mixPModel->freqs[s])
						mixPModel->freqs[s]++;
					mixPModel->sum += mixPModel->freqs[s];
					}
				
				nats = PModelSymbolNats(mixPModel, symbol);
				totalNats += nats;
				totalPartialNats += nats;
				bestNats += bestCModelNats;
				
				ArithEncodeSymbol(symbol, (int *)mixPModel->freqs, (int)mixPModel->sum, fpOut);
					
				bestTotalNats += bestCModelNats;
				
				// Update models and weights
				totalWeight = 0;
				for(cModel = 0 ; cModel < nCModels ; cModel++)
					{					
					// 3D template context
					if(cModel == 0x0)
						pModelIdx = GetPModelIdx(img, row, col, plane, imgPlanes,
		 		   			cModels[cModel], cTemplateIntra, cTemplateInter, cPattern);
					// SBC using pixel values estimate
					else
						pModelIdx = GetKikuchiPModelIdx(expImg, row, col, cTemplateSBC);
						
					//pModelIdx = GetPModelIdx2(img, row, col, plane, imgPlanes, cTemplate[cModel]);
					cModelsProb[cModel] = Pow(cModelsProb[cModel],gamma) * 
						(double)pModels[cModel]->freqs[symbol]/pModels[cModel]->sum;
					cModelsWeight[cModel] = cModelsProb[cModel];
					totalWeight += cModelsWeight[cModel];
					
					UpdateCModelCounter(cModels[cModel], pModelIdx, symbol);
					
					}
				//lastBestCModel = bestCModel;
				//lastWorstCModel = worstCModel;
				
				
				// Update pixels of the image with the estimate values
				y = GetGrayPixel(expImg, row, col) + symbol * (0x1 << plane);
				if((plane - 1) >= 0) y -= (0x1 << (plane - 1));
				PutGrayPixel(expImg, row, col, y);
				}		
				  
			}

		
			
		if(verbosityLevel > 0)
			{
			printf("Plane %2d: %10.0lf bits ( %5.3f bpp ) : ", plane, totalPartialNats/M_LN2, 
			totalPartialNats/M_LN2/(img->nRows*img->nCols));
			printf("Micro3D %5.2f%% : SBC %5.2f%% ", 
				100.0*((double)modelsPerformance[0x0]/(modelsPerformance[0x0] + modelsPerformance[0x1])), 
				100.0*((double)modelsPerformance[0x1]/(modelsPerformance[0x0] + modelsPerformance[0x1])));
			printf("| SBC Template size: %02d ", cTemplateSBC->size);
			printf("| Pattern %2d: ", cPattern->totalSize);
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
	
	/*
 	 * Send the remaining bit-planes uncoded.
 	 */
	for(plane = imgPlanes - codingPlanes - 1 ; plane >= 0 ; plane--)
		for(row = 0 ; row < img->nRows ; row++)
			for(col = 0 ; col < img->nCols ; col++)
				WriteNBits(GetGrayPixel(img, row, col) >> plane, 1, fpOut);

	finish_encode(fpOut);
	doneoutputtingbits(fpOut);

	fclose(fpOut);
	
	printf("Micro3D %5.2f%% : SBC %5.2f%%\n", 
		100.0*((double)cModelsGlobalUsage[0x0]/(cModelsGlobalUsage[0x0] + cModelsGlobalUsage[0x1])), 
		100.0*((double)cModelsGlobalUsage[0x1]/(cModelsGlobalUsage[0x0] + cModelsGlobalUsage[0x1])));
	
	printf("Number of bytes of the encoded image: %"PRIu64" ( %.3f bpp )\n",
	  (uint64_t)_bytes_output, _bytes_output * 8. / (img->nRows * img->nCols));
	return EXIT_SUCCESS;
	}

