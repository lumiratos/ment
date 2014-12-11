/*------------------------------------------------------------------------------

Copyright 2006-2007 Armando J. Pinho (ap@ua.pt), All Rights Reserved.

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


#ifdef WIN32
typedef unsigned __int64 uint64;
#else
typedef unsigned long long uint64;
#endif


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

	//printf("imgPlanes: %d | %d\n", cPattern->imgPlanes, workingCPattern->imgPlanes);
	//printf("maxTemplateSize: %d | %d\n", cPattern->maxTemplateSize, workingCPattern->maxTemplateSize);
	//printf("totalSize: %d | %d\n", cPattern->totalSize, workingCPattern->totalSize);

	/* Reset cPattern (which always contains the best pattern so far) */
	CopyCPattern(cPattern, workingCPattern);
	//fprintf(stderr, "Nothing?\n");

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
					//fprintf(stderr, "totalSize = %d | %d\n", cPattern->totalSize, workingCPattern->totalSize);
					CopyCPattern(cPattern, workingCPattern);
					//fprintf(stderr, "totalSize = %d | %d\n", cPattern->totalSize, workingCPattern->totalSize);
					newPattern = 1;
					}

				workingCPattern->size[p]--;
				workingCPattern->pattern[p][workingCPattern->size[p]] = 0;
				printf("%d\r", count++);
				fflush(stdout);
				}

			}
		//fprintf(stderr, "workingCPattern->totalSize = %d\n", workingCPattern->totalSize);
		//fprintf(stderr, "cPattern->totalSize = %d\n", cPattern->totalSize);
		//fprintf(stderr, "workingCPattern->maxTemplateSize = %d\n", workingCPattern->maxTemplateSize);
		//fprintf(stderr, "cPattern->maxTemplateSize = %d\n", cPattern->maxTemplateSize);

		CopyCPattern(workingCPattern, cPattern);
		//fprintf(stderr, "-> OK\n");
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

int EncodeBitPlane2(Image *img, int imgPlanes, int plane, CModel *cModel,
  CTemplate *cTemplateIntra, CTemplate *cTemplateInter, PModel *pModel,
  CPattern *cPattern, Image *mask, char BF, FILE *fpOut)

	{
	int row, col, pModelIdx, symbol;
	double nNats = 0;

	ResetCModelCounters(cModel);
	for(row = 0 ; row < img->nRows ; row++)
		for(col = 0 ; col < img->nCols ; col++)
			{
			// Foreground or Background
			if(((GetGrayPixel(mask, row, col) == 0xFF) && (BF == 'F')) || ((GetGrayPixel(mask, row, col) == 0x00) && (BF == 'B')))
				{
				pModelIdx = GetPModelIdx(img, row, col, plane, imgPlanes,
				  cModel, cTemplateIntra, cTemplateInter, cPattern);
				ComputePModel(cModel, pModel, pModelIdx);
				symbol = (GetGrayPixel(img, row, col) >> plane) & 0x1;
				nNats += PModelSymbolNats(pModel, symbol);
				ArithEncodeSymbol(symbol, pModel->freqs, pModel->sum, fpOut);
				UpdateCModelCounter(cModel, pModelIdx, symbol);
				}
			}

	return (int)(nNats / M_LN2 + 0.5);
	}


/*----------------------------------------------------------------------------*/

uint64 getFt (unsigned int te, Image *img)
{
	int r, c;
	unsigned int pixel;
	uint64 pixelsB=0, pixelsF=0, meanB=0, meanF=0, sumDifB=0, sumDifF=0, totalB=0, totalF=0;

	for(r = 0 ; r < img->nRows ; r++)
		{			
		for(c = 0 ; c < img->nCols ; c++)
			{
			pixel = GetGrayPixel(img, r,c);
			// Foreground
			if (pixel >= te) 
				{
				totalF += pixel;	
				pixelsF++;
				}
			// Background				
			else 
				{
				totalB += pixel;
				pixelsB++;
				}
			}
		}
			
	if(pixelsB != 0) meanB = (uint64)(totalB / pixelsB);
	if(pixelsF != 0) meanF = (uint64)(totalF / pixelsF);
			
		for(r = 0 ; r < img->nRows ; r++)
			{			
			for(c = 0 ; c < img->nCols ; c++)
				{
				pixel = GetGrayPixel(img, r,c);
				
				// Foreground
				if (pixel >= te) sumDifF += (uint64)pow(pixel-meanF, 2.0);	 
				// Background				
				else sumDifB += (uint64)pow(pixel-meanB, 2.0);	 
				}
			}	
	return (sumDifB + sumDifF);
}

/*----------------------------------------------------------------------------*/
/* Test threshold value */
/*----------------------------------------------------------------------------*/

unsigned int testThreshold(unsigned int te1, unsigned int te2, unsigned int te3, Image *img)
{
	uint64 ft1=0, ft2=0;
	if (te1 >= te2) return te1;

	ft1 = getFt((te1+te2)/2, img);
	ft2 = getFt((te2+te3)/2, img);

	if (ft1 < ft2)
		return testThreshold(te1,(te1+te2)/2, te2, img);
	return testThreshold(te2,(te2+te3)/2, te3, img);

}

/*----------------------------------------------------------------------------*/

Image *getMask(Image *img, Image *backgroundImg, Image *foregroundImg, unsigned int threshold)
{
	Image *mask = NULL;
	int row, col;
	unsigned int pixel;

	// Create the mask image 
	if(!(mask = CreateImage(img->nRows, img->nCols, DATA_TYPE_C, 1)))
	{
		fprintf(stderr,"Error (getMask): out of memory!\n");
		exit(1);
	}
	
	for(row = 0 ; row < img->nRows ; row++)
		for(col = 0 ; col < img->nCols ; col++)
		{
			pixel = GetGrayPixel(img, row,col);
			
			// Foreground
			if (pixel >= threshold) 
			{
				PutGrayPixel(mask, row, col, 0xFF);
				//PutGrayPixel(foregroundImg, row, col, pixel);
				PutGrayPixel(foregroundImg, row, col, pixel-threshold);
				PutGrayPixel(backgroundImg, row, col, 0x00);
			}

			// Background 
			else
			{
				PutGrayPixel(mask, row, col, 0x00);
				PutGrayPixel(backgroundImg, row, col, pixel);
				//PutGrayPixel(foregroundImg, row, col, threshold);
				PutGrayPixel(foregroundImg, row, col, 0x00);
			}
		}
	
	return mask;
}

void getStatistics(Image *backgroundImg, Image *foregroundImg, Image *mask)
{
	int row, col, n;
	unsigned int backGrays = 0, foreGrays=0, backPixels=0, forePixels=0;
	unsigned int *backCount=NULL, *foreCount=NULL;
	
	if(!(backCount = (unsigned int *)calloc(backgroundImg->maxIntensity + 1, sizeof(unsigned int))))
	{
		fprintf(stderr, "Error (getStatistics): No memory!\n");
		exit(EXIT_FAILURE);
	}
	if(!(foreCount = (unsigned int *)calloc(foregroundImg->maxIntensity + 1, sizeof(unsigned int))))
	{
		fprintf(stderr, "Error (getStatistics): No memory!\n");
		exit(EXIT_FAILURE);
	}
	
	for(row = 0 ; row < backgroundImg->nRows ; row++)
		for(col = 0 ; col < backgroundImg->nCols ; col++)
		{
			if(GetGrayPixel(mask, row, col) == 0x00)
				backCount[GetGrayPixel(backgroundImg, row, col)]++;
			else
				{
				foreCount[GetGrayPixel(foregroundImg, row, col)]++;
				forePixels++;
				}
		}
	
	backPixels = (mask->nRows * mask->nCols) - forePixels;
	for (n = 0; n <= backgroundImg->maxIntensity; n++)
		if (backCount[n]>0) backGrays++;
	for (n = 0; n <= foregroundImg->maxIntensity; n++)
		if (foreCount[n]>0) foreGrays++;
	
	printf("Background image has %u out of %d grays: %.2f%%\n", backGrays, backgroundImg->maxIntensity+1, 100*((double)backGrays/(backgroundImg->maxIntensity+1)));
	printf("Foreground image has %u out of %d grays: %.2f%%\n", foreGrays, foregroundImg->maxIntensity+1, 100*((double)foreGrays/(foregroundImg->maxIntensity+1)));

	printf("Total background pixels %u (%.2f%%)\n", backPixels, 100.0*((double)backPixels/(forePixels+backPixels)));	
	printf("Total foreground pixels %u (%.2f%%)\n", forePixels, 100.0*((double)forePixels/(forePixels+backPixels)));	
}

/*----------------------------------------------------------------------------*/

int main(int argc, char *argv[])

	{
	//int n, p, row, col, codingPlanes1, codingPlanes2, imgPlanes, imgPlanes1, imgPlanes2,
	int n, p, codingPlanes1, codingPlanes2, imgPlanes, imgPlanes1, imgPlanes2,
	  plane, nBits, totalBits = 0, verbosityLevel, optMode;
	Image *img=NULL, *foregroundImg=NULL, *backgroundImg=NULL, *mask=NULL;
	int deltaNum = DEFAULT_PMODEL_DELTA_NUM,
	  deltaDen = DEFAULT_PMODEL_DELTA_DEN,
	  maxCount = DEFAULT_PMODEL_MAX_COUNT;
	FILE *fpOut = NULL;
	//FILE *fpOut = NULL, *fp = NULL;
	CModel *cModel1, *cModel2;
	CTemplate *cTemplateIntra = InitTemplate(TEMPLATE_INTRA);
	CTemplate *cTemplateInter = InitTemplate(TEMPLATE_INTER);
	PModel *pModel;
	CPattern *cPattern1, *cPattern2;
	SearchArea sa1, sa2;
	unsigned int te1 = 0, te2, te3, te = 0;
	uint64_t  previousBytes = 0;	 
	//unsigned long long previousBytes = 0, fileSize=0;	
	//cmd1[200] = "./pbmtojbg -t 1 ";
	//char cmd[] = "~/spl/src/JBIG/jbigkit/pbmtools/./pbmtojbg -t 1 mask.pgm > mask.jbg";
	
	//char cmd[3*FILENAME_MAX] = "~/spl/src/JBIG/jbigkit/pbmtools/./pbmtojbg -t 1";
	//char maskName[FILENAME_MAX] = "", outFileName[FILENAME_MAX]="";
	char outFileName[FILENAME_MAX]="";
	int maskNameIdx = 0;

	codingPlanes1 = 16;
	codingPlanes2 = 16;
	verbosityLevel = 0;
	optMode = 0;
	if(argc == 1)
		{
		fprintf(stderr, "Usage: %12s [ -v (verbose) ]\n", argv[0]);
		//fprintf(stderr, "                      [ -O optMode (def 0) ]\n");
		fprintf(stderr, "                    [ -o outputFile ]\n");
		//fprintf(stderr, "                      [ -delta d/n (def. %d/%d) ]\n",
		  //deltaNum, deltaDen);
		//fprintf(stderr, "                      [ -mc maxCount (def %d) ]\n",
		  //maxCount);
		fprintf(stderr, "                    [ -p codingPlanes (def %d) ]\n",
		  codingPlanes1);
		fprintf(stderr,
		  "                    [ -sa nCxnR (search area, def 256x256) ]\n");
		fprintf(stderr,
		  "                    [ -best (search area, all image) ]\n");
		fprintf(stderr,
		  "                    [ -sc cC,cR (search center, def 0.5,0.5) ]\n");
		fprintf(stderr,
		  "                    [ -te threshold (threshold to slipt background/foreground) ]\n");
		fprintf(stderr,
		  "                    [ -m mask (mask file name) ]\n");
		fprintf(stderr, "                    img\n");
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
				return 1;
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
			codingPlanes1 = atoi(argv[n+1]);
			if(codingPlanes1 < 1)
				codingPlanes1 = 1;

			break;
			}

	for(n = 1 ; n < argc ; n++)
		if(strcmp(argv[n], "-te") == 0)
		{
			te = atoi(argv[n + 1]);
			break;
		}

	for(n = 1 ; n < argc ; n++)
		if(strcmp(argv[n], "-m") == 0)
		{
			maskNameIdx = n + 1;
			break;
		}

	/****************************************************************************************/
	codingPlanes2 = codingPlanes1;
	switch(img->dataType)
	{
		case DATA_TYPE_C: 	te3 = 0xFF; break;
		case DATA_TYPE_S: 	te3 = 0xFFFF; break;
		case DATA_TYPE_I: 	te3 = 0xFFFFFFFF; break;
		case DATA_TYPE_D: 	fprintf(stderr,"Unsupported data type 'DATA_TYPE_D'!\n");
					exit(1);
		case DATA_TYPE_X:	fprintf(stderr,"Unsupported data type 'DATA_TYPE_X'!\n");
					exit(1);
		default: 		fprintf(stderr,"Unknown data type %d!\n", img->dataType);
					exit(1);
	}

	if(te == 0)
	{
		te3 = img->maxIntensity;
		te2 = (te1 + te3)/2;
		te = testThreshold(te1, te2, te3, img);
	}

	if(te > img->maxIntensity)
	{
		fprintf(stderr, "Error: Threshold value is to high!\n");
		fprintf(stderr, "Image maximum intensity: %d\n", img->maxIntensity);
		fprintf(stderr, "Threshold value:         %d\n", te);
		return 1;
	}
	
	if(verbosityLevel > 0)
	{
		printf("Threshold value that will be used: %d\n", te);
		printf("MAX intensity: %d\n", img->maxIntensity);
	}

	// Create background and foreground image
	if(!(backgroundImg = CreateImage(img->nRows, img->nCols, img->dataType, 1)))
	{
		fprintf(stderr,"Error: Unable to create background image!\n");
		return 1;
	}
	if(!(foregroundImg = CreateImage(img->nRows, img->nCols, img->dataType, 1)))
	{
		fprintf(stderr,"Error: Unable to create foreground image!\n");
		return 1;
	}
	
	printf("Threshold used is %u\n", te);
	/* Get mask and split background and foreground image */
	mask = getMask(img, backgroundImg, foregroundImg, te);
	printf("Background image max gray is %d\n", ImageMaxIntensity(backgroundImg));
	printf("Foreground image max gray is %d\n", ImageMaxIntensity(foregroundImg));
	
		
	if(maskNameIdx == 0)
	{
		WriteImageFile("!mask.pgm", mask);
		//strcat(cmd, " mask.pgm");
		//strcat(cmd," > mask.jbg");
		//strcat(maskName, "mask.jbg");
	}	
	else
	{
		WriteImageFile(argv[maskNameIdx], mask);
		/*
		strcat(cmd, " ");
		strcat(cmd, argv[maskNameIdx]);
		strcat(cmd, " > ");
		strcat(cmd, argv[maskNameIdx]);
		strcat(cmd, ".jbg");

		strcat(maskName, argv[maskNameIdx]);
		strcat(maskName, ".jbg");
		*/
	}
	
	//printf("Comand to execute: '%s'\n", cmd);
	
	
	backgroundImg->maxIntensity = 0;
	backgroundImg->maxIntensity = ImageMaxIntensity(backgroundImg);
	if(backgroundImg->maxIntensity == 0)
	{
		fprintf(stderr, "Error: Background image max intensity is 0. Try a different threshold.\n");
		return 1;
	}
	//imgPlanes = (int)(log(backgroundImg->maxIntensity) / M_LN2) + 1;	
	foregroundImg->maxIntensity = 0;
	foregroundImg->maxIntensity = ImageMaxIntensity(foregroundImg);
	if(foregroundImg->maxIntensity == 0)
	{
		fprintf(stderr, "Error: Foreground image max intensity is 0. Try a different threshold.\n");
		return 1;
	}
	//imgPlanes = (int)(log(backgroundImg->maxIntensity) / M_LN2) + 1;	

	getStatistics(backgroundImg, foregroundImg, mask);
	
	/****************************************************************************************/



	//SetSearchArea(img, &sa, argc, argv);
	SetSearchArea(backgroundImg, &sa1, argc, argv);
	SetSearchArea(foregroundImg, &sa2, argc, argv);

	//if(fpOut == NULL)
	//	fpOut = fopen("/dev/null", "w");

	if(fpOut == NULL)
		{
		strcat(outFileName, argv[argc-1]);
		strcat(outFileName, ".enc");
		if(!(fpOut = fopen(outFileName, "wb")) )
			{
			fprintf(stderr, "Error: cannot open file '%s' in write mode!\n", outFileName);
			return 1;
			}
		}
	
	/* Force re-calculation of the image maximum intensity */
	img->maxIntensity = 0;
	img->maxIntensity = ImageMaxIntensity(img);
	imgPlanes = (int)(log(img->maxIntensity) / M_LN2) + 1;

	//backgroundImg->maxIntensity = 0;
	//foregroundImg->maxIntensity = 0;
	//backgroundImg->maxIntensity = ImageMaxIntensity(backgroundImg);
	//foregroundImg->maxIntensity = ImageMaxIntensity(foregroundImg);
	imgPlanes1 = (int)(log(backgroundImg->maxIntensity) / M_LN2) + 1;
	imgPlanes2 = (int)(log(foregroundImg->maxIntensity) / M_LN2) + 1;

	
	if(codingPlanes1 > imgPlanes1)
		codingPlanes1 = imgPlanes1;
	if(codingPlanes2 > imgPlanes2)
		codingPlanes2 = imgPlanes2;


	printf("Image has %d rows, %d cols and %d bit-planes\n", img->nRows, img->nCols, imgPlanes);
	printf("Background image has %d bit-planes\n", imgPlanes1);	
	printf("Foreground image has %d bit-planes\n", imgPlanes2);
	printf("Encoding the %d,%d most significant bit-planes\n", codingPlanes1, codingPlanes2);
	printf("Using search area for background ul=%d,%d lr=%d,%d\n", sa1.ulCol, sa1.ulRow, sa1.lrCol, sa1.lrRow);
	printf("Using search area for foreground ul=%d,%d lr=%d,%d\n", sa2.ulCol, sa2.ulRow, sa2.lrCol, sa2.lrRow);

	printf("Optimization mode: %d\n", optMode);

	printf("Using intra template:\n\n");
	ShowTemplate(cTemplateIntra);
	putchar('\n');
	printf("Using inter template:\n\n");
	ShowTemplate(cTemplateInter);
	putchar('\n');

	cModel1 = CreateCModel(TEMPLATE_MAX_SIZE, N_SYMBOLS, N_CTX_SYMBOLS, deltaNum, deltaDen, maxCount);
	cModel2 = CreateCModel(TEMPLATE_MAX_SIZE, N_SYMBOLS, N_CTX_SYMBOLS, deltaNum, deltaDen, maxCount);
	pModel = CreatePModel(N_SYMBOLS);
	cPattern1 = CreateCPattern(imgPlanes1, MAX(cTemplateIntra->size, cTemplateInter->size));
	cPattern2 = CreateCPattern(imgPlanes2, MAX(cTemplateIntra->size, cTemplateInter->size));	

	startoutputtingbits();
	start_encode();

	/* Store the number of rows and cols */
	WriteNBits(img->nRows, STORAGE_BITS_N_ROWS, fpOut);
	WriteNBits(img->nCols, STORAGE_BITS_N_COLS, fpOut);
	
	/* Store the threshold value */
	WriteNBits(te, STORAGE_BITS_THRESHOLD, fpOut);

	/* Store the number of image bit-planes (1..16) */
	WriteNBits(imgPlanes1 - 1, STORAGE_BITS_N_IMG_PLANES, fpOut);
	WriteNBits(imgPlanes2 - 1, STORAGE_BITS_N_IMG_PLANES, fpOut);

	/* Store the number of encoded bit-planes (1..16) */
	WriteNBits(codingPlanes1 - 1, STORAGE_BITS_N_COD_PLANES, fpOut);
	WriteNBits(codingPlanes2 - 1, STORAGE_BITS_N_COD_PLANES, fpOut);

	WriteNBits(cModel1->deltaNum - 1, STORAGE_BITS_PMODEL_DELTA_NUM, fpOut);
	WriteNBits(cModel1->deltaDen - 1, STORAGE_BITS_PMODEL_DELTA_DEN, fpOut);
	WriteNBits(maxCount, STORAGE_BITS_PMODEL_MAX_COUNT, fpOut);
	
	
	//printf("imgPlanes1 = %d | codingPlanes1 = %d\n", imgPlanes1, codingPlanes1);
	previousBytes = _bytes_output;
	for(plane = imgPlanes1 - 1 ; plane >= imgPlanes1 - codingPlanes1 ; plane--)
		{
		switch(optMode)
			{
			case 0:
				FindBestCPatternMode0(backgroundImg, imgPlanes1, plane, cModel1,
				  cTemplateIntra, cTemplateInter, pModel, cPattern1, &sa1);
				break;

			case 1:
				FindBestCPatternMode1(backgroundImg, imgPlanes1, plane, cModel1,
				  cTemplateIntra, cTemplateInter, pModel, cPattern1, &sa1);
				break;

			case 2:
				FindBestCPatternMode2(backgroundImg, imgPlanes1, plane, cModel1,
				  cTemplateIntra, cTemplateInter, pModel, cPattern1, &sa1);
				break;

			}

		// Store the context pattern for this bit-plane 
		for(p = plane ; p < imgPlanes1 ; p++)
			for(n = 0 ; n < (p == plane ? cTemplateIntra->size : cTemplateInter->size) ; n++)
				WriteNBits(cPattern1->pattern[p][n], 1, fpOut);

		// Encode this bit-plane 
		//nBits = EncodeBitPlane(img, imgPlanes, plane, cModel, cTemplateIntra, cTemplateInter, pModel, cPattern, fpOut);
		nBits = EncodeBitPlane2(backgroundImg, imgPlanes1, plane, cModel1, cTemplateIntra, cTemplateInter, pModel, cPattern1, mask, 'B', fpOut);

		totalBits += nBits;
		if(verbosityLevel > 0)
			{
			printf("Plane %2d: %d bits ( %.3f bpp ) %d: ", plane, nBits,
			  (double)nBits / (backgroundImg->nRows * backgroundImg->nCols), cPattern1->totalSize);
			for(p = imgPlanes1 - 1 ; p >= plane ; p--)
				printf("%d ", cPattern1->size[p]);

			putchar('\n');
			if(verbosityLevel > 1)
				for(p = imgPlanes1 - 1 ; p >= plane ; p--)
					{
					for(n = 0 ; n < (p == plane ? cTemplateIntra->size :
					  cTemplateInter->size) ; n++)
						printf("%d", cPattern1->pattern[p][n]);

					putchar('\n');
					}

			}

		}
	previousBytes = _bytes_output-previousBytes;
	printf("Number of bytes for the background image: %"PRIu64" ( %.3f bpp )\n",
	  (uint64_t)previousBytes, previousBytes * 8. / (img->nRows * img->nCols));	

 	// Send the remaining bit-planes uncoded.
	/*
	for(plane = imgPlanes1 - codingPlanes1 - 1 ; plane >= 0 ; plane--)
		for(row = 0 ; row < img->nRows ; row++)
			for(col = 0 ; col < img->nCols ; col++)
			{
				WriteNBits(GetGrayPixel(img, row, col) >> plane, 1, fpOut);
				fprintf(stderr, "Here: Never!!!\n");
			}
	
	*/
	/****************************************************************************************/
	
	//WriteImageFile("backgroundImg.pgm", backgroundImg);	

	previousBytes = _bytes_output;
	for(plane = imgPlanes2 - 1 ; plane >= imgPlanes2 - codingPlanes2 ; plane--)
		{
		switch(optMode)
			{
			case 0:
				FindBestCPatternMode0(foregroundImg, imgPlanes2, plane, cModel2,
				  cTemplateIntra, cTemplateInter, pModel, cPattern2, &sa2);
				break;

			case 1:
				FindBestCPatternMode1(foregroundImg, imgPlanes2, plane, cModel2,
				  cTemplateIntra, cTemplateInter, pModel, cPattern2, &sa2);
				break;

			case 2:
				FindBestCPatternMode2(foregroundImg, imgPlanes2, plane, cModel2,
				  cTemplateIntra, cTemplateInter, pModel, cPattern2, &sa2);
				break;

			}

		/* Store the context pattern for this bit-plane */
		for(p = plane ; p < imgPlanes2 ; p++)
			for(n = 0 ; n < (p == plane ? cTemplateIntra->size : cTemplateInter->size) ; n++)
				WriteNBits(cPattern2->pattern[p][n], 1, fpOut);

		/* Encode this bit-plane */
		//nBits = EncodeBitPlane(img, imgPlanes, plane, cModel, cTemplateIntra, cTemplateInter, pModel, cPattern, fpOut);
		nBits = EncodeBitPlane2(foregroundImg, imgPlanes2, plane, cModel2, cTemplateIntra, cTemplateInter, pModel, cPattern2, mask, 'F', fpOut);

		totalBits += nBits;
		if(verbosityLevel > 0)
			{
			printf("Plane %2d: %d bits ( %.3f bpp ) %d: ", plane, nBits,
			  (double)nBits / (foregroundImg->nRows * foregroundImg->nCols), cPattern2->totalSize);
			for(p = imgPlanes - 1 ; p >= plane ; p--)
				printf("%d ", cPattern2->size[p]);

			putchar('\n');
			if(verbosityLevel > 1)
				for(p = imgPlanes2 - 1 ; p >= plane ; p--)
					{
					for(n = 0 ; n < (p == plane ? cTemplateIntra->size :
					  cTemplateInter->size) ; n++)
						printf("%d", cPattern2->pattern[p][n]);

					putchar('\n');
					}

			}

		}
	/****************************************************************************************/
	previousBytes = _bytes_output-previousBytes;
	printf("Number of bytes for the foreground image: %"PRIu64" ( %.3f bpp )\n",
	  (uint64_t)previousBytes, previousBytes * 8. / (img->nRows * img->nCols));	
	
	finish_encode(fpOut);
	doneoutputtingbits(fpOut);

	fclose(fpOut);
	/*
	//if(system("~/spl/src/JBIG/jbigkit/pbmtools/./pbmtojbg -t 1 mask.pgm > mask.jbg"))
	if(system(cmd))
	{
		fprintf(stderr,"Error: pbmtojbg conversion failed!\n");
		return 1;
	}	
	
	//if((fp = fopen("mask.jbg", "rb")) == NULL)
	if((fp = fopen(maskName,"rb")) == NULL)
	{	
		fprintf(stderr,"Error: Unable to open binary file mask.jbg with read permission!\n");
		return 1;
	}
	
	fseek(fp, 0, SEEK_END);
	fileSize = ftell(fp);
	rewind(fp);	
	fclose(fp);
	*/
	printf("Number of bytes of the encoded image: %"PRIu64" ( %.3f bpp )\n", (uint64_t)_bytes_output, _bytes_output * 8. / (img->nRows * img->nCols));
	//printf("Number of BYTES of the encoded image: %llu ( %.3f bpp )\n", _bytes_output+fileSize, (_bytes_output+fileSize) * 8. / (img->nRows * img->nCols));
	printf("NOTE: The previous results does not include the binary segmentation mask!\n");	
	return 0;
	}

