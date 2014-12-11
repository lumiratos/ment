/*------------------------------------------------------------------------------

Copyright 2006-2007 Armando J. Pinho (ap@ua.pt), All Rights Reserved.
	  2009-2014 Luis M. O. Matos (luismatos@ua.pt)

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

/*----------------------------------------------------------------------------*/



double EvaluateContextSize(Image *img, Image *tmpImg, int plane,
	CModel *cModel, CTemplate *kikuchiTemplate, PModel *pModel)
		
	{
		int row, col, pModelIdx=0, symbol, y;
		double nNats = 0;
		ResetCModelCounters(cModel);
		
		for(row = 0 ; row < img->nRows ; row++)
			{
			for(col = 0 ; col < img->nCols ; col++)
				{
				pModelIdx = GetKikuchiPModelIdx(tmpImg, row, col, kikuchiTemplate);
				ComputePModel(cModel, pModel, pModelIdx);
				symbol = (GetGrayPixel(img, row, col) >> plane) & 0x1;
				//symbol = (GetGrayPixel(img, row, col) >> (plane - 1)) & 0x1;
								
				nNats += PModelSymbolNats(pModel, symbol);
				//ArithEncodeSymbol(symbol, pModel->freqs, pModel->sum, fpOut);
				UpdateCModelCounter(cModel, pModelIdx, symbol);
				
				//y = GetGrayPixel(tmpImg, row, col) + symbol * (0x1 << (plane - 1));
				//if((plane - 2) >= 0) y -= (0x1 << (plane - 2));
				
				y = GetGrayPixel(tmpImg, row, col) + symbol * (0x1 << plane);
				if((plane - 1) >= 0) y -= (0x1 << (plane - 1));
				PutGrayPixel(tmpImg, row, col, y);
				}
			}
		//return (int)(nNats / M_LN2 + 0.5);
		return (nNats / M_LN2 + 0.5);
	}

/*----------------------------------------------------------------------------*/
	
int EncodeBitPlane(Image *img, Image *tmpImg, int plane,
	CModel *cModel, CTemplate *kikuchiTemplate, PModel *pModel, FILE *fpOut)
	{
		int row, col, pModelIdx=0, symbol, y;
		double nNats = 0;
		ResetCModelCounters(cModel);
				
		for(row = 0 ; row < img->nRows ; row++)
			{
			for(col = 0 ; col < img->nCols ; col++)
				{
				pModelIdx = GetKikuchiPModelIdx(tmpImg, row, col, kikuchiTemplate);
				ComputePModel(cModel, pModel, pModelIdx);
				symbol = (GetGrayPixel(img, row, col) >> plane) & 0x1;
				//symbol = (GetGrayPixel(img, row, col) >> (plane - 1)) & 0x1;
								
				nNats += PModelSymbolNats(pModel, symbol);
				ArithEncodeSymbol(symbol, pModel->freqs, pModel->sum, fpOut);
				UpdateCModelCounter(cModel, pModelIdx, symbol);
				
				//y = GetGrayPixel(tmpImg, row, col) + symbol * (0x1 << (plane - 1));
				//if((plane - 2) >= 0) y -= (0x1 << (plane - 2));
				y = GetGrayPixel(tmpImg, row, col) + symbol * (0x1 << plane);
				if((plane - 1) >= 0) y -= (0x1 << (plane - 1));
				PutGrayPixel(tmpImg, row, col, y);
				}
			}
		return (int)(nNats / M_LN2 + 0.5);
	}


/*----------------------------------------------------------------------------*/

int GetBestContextSize(Image *img, Image *tmpImg, int plane,
	CModel *cModel, CTemplate *kikuchiTemplate, PModel *pModel)
		
	{
	Image *cloneTmpImg;
	int ctxSize = 0, bestCtxSize = kikuchiTemplate->size;
	double bestBits = 0, nBits = 0;
		
	cloneTmpImg = CreateImage(tmpImg->nRows, tmpImg->nCols, tmpImg->dataType, tmpImg->nPlanes);
	if(cloneTmpImg == NULL)
		{
		fprintf(stderr, "Error: memory allocation!\n");
		exit(EXIT_FAILURE);
		}
		
	CopyImage(tmpImg, cloneTmpImg);
	bestBits = EvaluateContextSize(img, cloneTmpImg, plane, cModel, kikuchiTemplate, pModel);
		
	for(ctxSize = kikuchiTemplate->size - 1; ctxSize > 0; ctxSize--)
		{
		// Update context size
		kikuchiTemplate->size = ctxSize;
			
		// Copy image with the estimated pixel values
		CopyImage(tmpImg, cloneTmpImg);
			
		// Evaluate context size
		nBits = EvaluateContextSize(img, cloneTmpImg, plane, cModel, kikuchiTemplate, pModel);
		if(nBits < bestBits)
			{
			bestCtxSize = ctxSize;
			bestBits = nBits;
			}			
		}
	return bestCtxSize;
	}

/*----------------------------------------------------------------------------*/
int GetGreedyContextSizeV1(Image *img, Image *tmpImg, int plane, 
	CModel *cModel, CTemplate *kikuchiTemplate, PModel *pModel)
		
	{
	Image *cloneTmpImg;
	//int bestBits = 0, ctxSize = 0, nBits = 0, bestCtxSize = 10;
	int ctxSize = 0, bestCtxSize = kikuchiTemplate->size;
	double bestBits = 0, nBits = 0;
		
	cloneTmpImg = CreateImage(tmpImg->nRows, tmpImg->nCols, tmpImg->dataType, tmpImg->nPlanes);
	if(cloneTmpImg == NULL)
		{
		fprintf(stderr, "Error: memory allocation!\n");
		exit(EXIT_FAILURE);
		}
		
	CopyImage(tmpImg, cloneTmpImg);
	bestBits = EvaluateContextSize(img, cloneTmpImg, plane, cModel, kikuchiTemplate, pModel);
	
	for(ctxSize = kikuchiTemplate->size - 1; ctxSize > 0; ctxSize--)
		{
		// Update context size
		kikuchiTemplate->size = ctxSize;
		
		// Copy image with the estimated pixel values
		CopyImage(tmpImg, cloneTmpImg);
		
		// Evaluate context size
		nBits = EvaluateContextSize(img, cloneTmpImg, plane, cModel, kikuchiTemplate, pModel);
		if(nBits < bestBits)
			{
			bestCtxSize = ctxSize;
			bestBits = nBits;
			}
		// The obtained number of bits is worse than the previous best, stop the searching
		// procedure
		else
			{
			break;
			}
		}
	
		return bestCtxSize;
	}
	
/*----------------------------------------------------------------------------*/
int GetGreedyContextSizeV2(Image *img, Image *tmpImg, int plane,
	CModel *cModel, CTemplate *kikuchiTemplate, PModel *pModel)
		
	{
	Image *cloneTmpImg;
	int ctxSize = 0, maxContextSize = kikuchiTemplate->size;
	int middleCtxSize = maxContextSize/2;
	double bestBitsL, bestBitsR, nBits;
	int bestCtxSizeL, bestCtxSizeR;
	
	cloneTmpImg = CreateImage(tmpImg->nRows, tmpImg->nCols, tmpImg->dataType, tmpImg->nPlanes);
	if(cloneTmpImg == NULL)
		{
		fprintf(stderr, "Error: memory allocation!\n");
		exit(EXIT_FAILURE);
		}
		
	CopyImage(tmpImg, cloneTmpImg);
	
	// Starting from middleCtxSize to the left
	kikuchiTemplate->size = middleCtxSize;
	bestCtxSizeL = bestCtxSizeR = middleCtxSize;
	bestBitsL = bestBitsR = EvaluateContextSize(img, cloneTmpImg, plane, cModel, kikuchiTemplate, pModel);
		
	for(ctxSize = middleCtxSize-1; ctxSize > 0; ctxSize--)
		{
		// Update context size
		kikuchiTemplate->size = ctxSize;
		
		// Copy image with the estimated pixel values
		CopyImage(tmpImg, cloneTmpImg);
		
		// Evaluate context size
		nBits = EvaluateContextSize(img, cloneTmpImg, plane, cModel, kikuchiTemplate, pModel);	
				
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
	
	CopyImage(tmpImg, cloneTmpImg);
	
	// Starting from middleCtxSize to the right
	for(ctxSize = middleCtxSize+1; ctxSize <= maxContextSize; ctxSize++)
		{
		
		// Update context size
		kikuchiTemplate->size = ctxSize;
		
		// Copy image with the estimated pixel values
		CopyImage(tmpImg, cloneTmpImg);
		
		// Evaluate context size
		nBits = EvaluateContextSize(img, cloneTmpImg, plane, cModel, kikuchiTemplate, pModel);	
		
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
int GetGreedyContextSizeV3(Image *img, Image *tmpImg, int plane,
	CModel *cModel, CTemplate *kikuchiTemplate, PModel *pModel)
		
	{
	Image *cloneTmpImg;
	int ctxSize = 0, maxContextSize = kikuchiTemplate->size;
	int middleCtxSize = maxContextSize/2;
	double bestBitsL, bestBitsR, nBits;
	int bestCtxSizeL, bestCtxSizeR;
	
	cloneTmpImg = CreateImage(tmpImg->nRows, tmpImg->nCols, tmpImg->dataType, tmpImg->nPlanes);
	if(cloneTmpImg == NULL)
		{
		fprintf(stderr, "Error: memory allocation!\n");
		exit(EXIT_FAILURE);
		}
		
	CopyImage(tmpImg, cloneTmpImg);
	
	// Starting from 1 to middleCtxSize
	kikuchiTemplate->size = 1;
	bestCtxSizeL = 1;
	bestBitsL = EvaluateContextSize(img, cloneTmpImg, plane, cModel, kikuchiTemplate, pModel);
	
	for(ctxSize = 2; ctxSize <= middleCtxSize; ctxSize++)
		{
		// Update context size
		kikuchiTemplate->size = ctxSize;
		
		// Copy image with the estimated pixel values
		CopyImage(tmpImg, cloneTmpImg);
		
		// Evaluate context size
		nBits = EvaluateContextSize(img, cloneTmpImg, plane, cModel, kikuchiTemplate, pModel);	
		
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
	
	CopyImage(tmpImg, cloneTmpImg);
		
	// Starting from maxContextSize to middleCtxSize
	kikuchiTemplate->size = maxContextSize;
	bestCtxSizeR = maxContextSize;
	bestBitsR = EvaluateContextSize(img, cloneTmpImg, plane, cModel, kikuchiTemplate, pModel);
		
	// Starting from maxContextSize to middleCtxSize
	for(ctxSize = maxContextSize-1; ctxSize > middleCtxSize; ctxSize--)
		{			
		// Update context size
		kikuchiTemplate->size = ctxSize;
		
		// Copy image with the estimated pixel values
		CopyImage(tmpImg, cloneTmpImg);
		
		// Evaluate context size
		nBits = EvaluateContextSize(img, cloneTmpImg, plane, cModel, kikuchiTemplate, pModel);	
		
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
	/*
int GetGreedyContextSizeV2(Image *img, Image *tmpImg, int plane, 
	CModel *cModel, CTemplate *kikuchiTemplate, PModel *pModel)
		
	{
	Image *cloneTmpImg;
	//int bestBits = 0, ctxSize = 0, nBits = 0, bestCtxSize = 10;
	int ctxSize = 0, maxContextSize = kikuchiTemplate->size;
	int middleCtxSize = maxContextSize/2;
	//int bestCtxSizeL1=1, bestCtxSizeL2=middleCtxSize, bestCtxSizeR1 = middleCtxSize, bestCtxSizeR2=maxContextSize;
	int bestCtxSizeL1, bestCtxSizeL2, bestCtxSizeR1, bestCtxSizeR2;
	double bestBitsL1 = 0.0, bestBitsL2 = 0.0, nBits = 0.0;
	double bestBitsR1 = 0.0, bestBitsR2 = 0.0;
	
	//printf("maxContextSize = %d | middleCtxSize = %d |\n", maxContextSize, middleCtxSize);
		
	cloneTmpImg = CreateImage(tmpImg->nRows, tmpImg->nCols, tmpImg->dataType, tmpImg->nPlanes);
	if(cloneTmpImg == NULL)
		{
		fprintf(stderr, "Error: memory allocation!\n");
		exit(EXIT_FAILURE);
		}
		
	CopyImage(tmpImg, cloneTmpImg);
	
	// Set size to 1
	kikuchiTemplate->size = 1;
	bestCtxSizeL1 = 1;
	bestBitsL1 = EvaluateContextSize(img, cloneTmpImg, plane, cModel, kikuchiTemplate, pModel);

	//if(plane == 6) printf("L1 | size = %02d | nBits = %05.0lf\n", bestCtxSizeL1, bestBitsL1);
		
	// [ 1 ... middleCtxSize [
	for(ctxSize = 2; ctxSize < middleCtxSize; ctxSize++)
		{
		// Update context size
		kikuchiTemplate->size = ctxSize;
		
		// Copy image with the estimated pixel values
		CopyImage(tmpImg, cloneTmpImg);
		
		// Evaluate context size
		nBits = EvaluateContextSize(img, cloneTmpImg, plane, cModel, kikuchiTemplate, pModel);	
		
		if(nBits < bestBitsL1)
			{
			bestCtxSizeL1 = ctxSize;
			bestBitsL1 = nBits;
			
			//if(plane == 6) printf("L1 | size = %02d | nBits = %05.0lf\n", bestCtxSizeL1, bestBitsL1);
			
			}
		// The obtained number of bits is worse than the previous best, stop the searching
		// procedure
		else
			{
			//if(plane == 6) printf("L1 | size = %02d | nBits = %05.0lf\n", ctxSize, nBits);
			break;
			}
		
		}
	
	CopyImage(tmpImg, cloneTmpImg);
	// Set size to middleCtxSize
	kikuchiTemplate->size = middleCtxSize;
	bestCtxSizeL2 = middleCtxSize;
	bestBitsL2 = EvaluateContextSize(img, cloneTmpImg, plane, cModel, kikuchiTemplate, pModel);
	//if(plane == 6) printf("L2 | size = %02d | nBits = %05.0lf\n", bestCtxSizeL2, bestBitsL2);
	// [ middleCtxSize ... 1 ]	
	for(ctxSize = middleCtxSize-1; ctxSize > 2; ctxSize--)
		{
		// Update context size
		kikuchiTemplate->size = ctxSize;
		
		// Copy image with the estimated pixel values
		CopyImage(tmpImg, cloneTmpImg);
		
		// Evaluate context size
		nBits = EvaluateContextSize(img, cloneTmpImg, plane, cModel, kikuchiTemplate, pModel);	
		
		if(nBits < bestBitsL2)
			{
			bestCtxSizeL2 = ctxSize;
			bestBitsL2 = nBits;
			
			//if(plane == 6) printf("L2 | size = %02d | nBits = %05.0lf\n", bestCtxSizeL2, bestBitsL2);
			}
		// The obtained number of bits is worse than the previous best, stop the searching
		// procedure
		else
			{
			//if(plane == 6) printf("L2 | size = %02d | nBits = %05.0lf\n", ctxSize, nBits);
			break;
			}
		
		}
	
	CopyImage(tmpImg, cloneTmpImg);
	// Set size to middleCtxSize+1;
	kikuchiTemplate->size = middleCtxSize+1;
	bestCtxSizeR1 = middleCtxSize+1;
	bestBitsR1 = EvaluateContextSize(img, cloneTmpImg, plane, cModel, kikuchiTemplate, pModel);
	//if(plane == 6) printf("R1 | size = %02d | nBits = %05.0lf\n", bestCtxSizeR1, bestBitsR1);
	// [ middleCtxSize+1 ... maxContextSize ]	
	for(ctxSize = middleCtxSize+2; ctxSize <= maxContextSize; ctxSize++)
		{
		// Update context size
		kikuchiTemplate->size = ctxSize;
		
		// Copy image with the estimated pixel values
		CopyImage(tmpImg, cloneTmpImg);
		
		// Evaluate context size
		nBits = EvaluateContextSize(img, cloneTmpImg, plane, cModel, kikuchiTemplate, pModel);	
		
		if(nBits < bestBitsR1)
			{
			bestCtxSizeR1 = ctxSize;
			bestBitsR1 = nBits;
			
			//if(plane == 6) printf("R1 | size = %02d | nBits = %05.0lf\n", bestCtxSizeR1, bestBitsR1);
			}
		// The obtained number of bits is worse than the previous best, stop the searching
		// procedure
		else
			{
			//if(plane == 6) printf("R1 | size = %02d | nBits = %05.0lf\n", ctxSize, nBits);
			break;
			}
		
		}
	
	CopyImage(tmpImg, cloneTmpImg);
	// Set size to middleCtxSize
	kikuchiTemplate->size = maxContextSize;
	bestCtxSizeR2 = maxContextSize;
	bestBitsR2 = EvaluateContextSize(img, cloneTmpImg, plane, cModel, kikuchiTemplate, pModel);
	//if(plane == 16) printf("R2 | size = %02d | nBits = %05.0lf\n", bestCtxSizeR2, bestBitsR2);
	// [ maxContextSize ... middleCtxSize]	
	for(ctxSize = maxContextSize-1; ctxSize > middleCtxSize+2; ctxSize--)
		{
		// Update context size
		kikuchiTemplate->size = ctxSize;
		
		// Copy image with the estimated pixel values
		CopyImage(tmpImg, cloneTmpImg);
		
		// Evaluate context size
		nBits = EvaluateContextSize(img, cloneTmpImg, plane, cModel, kikuchiTemplate, pModel);	
		
		if(nBits < bestBitsR2)
			{
			bestCtxSizeR2 = ctxSize;
			bestBitsR2 = nBits;
			
			//if(plane == 6) printf("R2 | size = %02d | nBits = %05.0lf\n", bestCtxSizeR2, bestBitsR2);
			}
		// The obtained number of bits is worse than the previous best, stop the searching
		// procedure
		else
			{
			//if(plane == 6) printf("R2 | size = %02d | nBits = %05.0lf\n", ctxSize, nBits);
			break;
			}
		
		}
		//  bestCtxSizeL1
	if(bestBitsL1 <= bestBitsL2 && bestBitsL1 <= bestBitsR1 && bestBitsL1 <= bestBitsR2)
		return bestCtxSizeL1;
	
	//  bestCtxSizeL2
	if(bestBitsL2 <= bestBitsL1 && bestBitsL2 <= bestBitsR1 && bestBitsL2 <= bestBitsR2)
		return bestCtxSizeL2;

	//  bestCtxSizeR1
	if(bestBitsR1 <= bestBitsL1 && bestBitsR1 <= bestBitsL2 && bestBitsR1 <= bestBitsR2)
		return bestCtxSizeR1;
	
	//  bestCtxSizeR2
	//if(bestBitsR2 < bestBitsL1 && bestBitsR2 < bestBitsL2 && bestBitsR2 < bestBitsR1)
	//	return bestCtxSizeR2;
	return bestCtxSizeR2;
	
	}	
*/			
/*----------------------------------------------------------------------------*/
				
int main(int argc, char *argv[])

	{
	int n, row, col, codingPlanes, imgPlanes, plane, nBits,
	  totalBits = 0, verbosityLevel, tmpSize=0, ctxSize = 0;
	Image *img, *tmpImg;
	int deltaNum = DEFAULT_PMODEL_DELTA_NUM,
	  deltaDen = DEFAULT_PMODEL_DELTA_DEN,
	  maxCount = DEFAULT_PMODEL_MAX_COUNT; 
	char outFileName[FILENAME_MAX] = "";
	FILE *fpOut = NULL;
	CModel *cModel;
	CTemplate *kikuchiTemplate = InitTemplate(TEMPLATE_KIKUCHI);
	int oriCtxSize = kikuchiTemplate->size, ctxMode = KIKUCHI_CONTEXT_GREEDY_MODE;
	PModel *pModel;

	codingPlanes = 16;
	verbosityLevel = 0;
	//optMode = 0;
	if(argc == 1)
		{
		//fprintf(stderr, "Usage:	SimpleBitplaneEnc [ -v (verbose) ]\n", argv[0]);
		fprintf(stderr, "\nUsage: %12s [ -v (verbose) ]\n", argv[0]);
		fprintf(stderr, "                    [ -o outputFile ]\n");
		fprintf(stderr, "                    [ -p codingPlanes (def %d) ]\n", codingPlanes);
		//fprintf(stderr, "                        [ -ctxMode best/greedy ]\n");
		//fprintf(stderr, "                         [ -ctxMode best/greedy1/greedy2/greedy3 ]\n");
		fprintf(stderr, "                    [ -ctxMode best/greedy (def greedy) ]\n");
		fprintf(stderr, "                    img\n");
		
		fprintf(stderr, "\n\nThis compressor is based on the work of Kikuchi et al.\n");
		fprintf(stderr, "Please consult the following papers for more information.\n\n");
		fprintf(stderr, "%s\n\n", KIKUCHI);
		return EXIT_FAILURE;
		}

	if(!(img = ReadImageFile(argv[argc - 1])))
		return 1;

	for(n = 1 ; n < argc ; n++)
		if(strcmp("-v", argv[n]) == 0)
			verbosityLevel++;

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

	printf("Image has %d rows, %d cols and %d bit-planes\n", img->nRows, img->nCols, imgPlanes);
	printf("Encoding the %d most significant bit-planes\n", codingPlanes);

	printf("Using template:\n\n");
	ShowTemplate(kikuchiTemplate);
	putchar('\n');
	
	//cModel = CreateCModel(TEMPLATE_MAX_SIZE, N_SYMBOLS, N_CTX_SYMBOLS, deltaNum, deltaDen, maxCount);
	cModel = CreateCModel(kikuchiTemplate->size, N_SYMBOLS, N_CTX_SYMBOLS, deltaNum, deltaDen, maxCount);
	pModel = CreatePModel(N_SYMBOLS);

	startoutputtingbits();
	start_encode();

	/* Store the number of rows and cols */
	WriteNBits(img->nRows, STORAGE_BITS_N_ROWS, fpOut);
	WriteNBits(img->nCols, STORAGE_BITS_N_COLS, fpOut);

	/* Store the number of image bit-planes (1..16) */
	WriteNBits(imgPlanes - 1, STORAGE_BITS_N_IMG_PLANES, fpOut);

	/* Store the number of encoded bit-planes (1..16) */
	WriteNBits(codingPlanes - 1, STORAGE_BITS_N_COD_PLANES, fpOut);

	WriteNBits(cModel->deltaNum - 1, STORAGE_BITS_PMODEL_DELTA_NUM, fpOut);
	WriteNBits(cModel->deltaDen - 1, STORAGE_BITS_PMODEL_DELTA_DEN, fpOut);
	WriteNBits(maxCount, STORAGE_BITS_PMODEL_MAX_COUNT, fpOut);
	
	// Create image with zeros
	if(!(tmpImg = CreateImage(img->nRows, img->nCols, imgPlanes <= 8 ? DATA_TYPE_C : DATA_TYPE_S, 1)))
		return EXIT_FAILURE;
	
	for(row = 0 ; row < tmpImg->nRows ; row++)
		for(col = 0 ; col < tmpImg->nCols ; col++)
			{
				// Initialize estimate of a decoded pixel value 2^(N-1) - 1
				PutGrayPixel(tmpImg, row, col, ((0x1 << (imgPlanes-1)) - 1));
			}
	
	
	//for(plane = imgPlanes ; plane > imgPlanes - codingPlanes ; plane--)
	for(plane = imgPlanes - 1; plane >= imgPlanes - codingPlanes ; plane--)		
		{
			
		// Reset context template size
		kikuchiTemplate->size = oriCtxSize;
			
		// Compute the best context size
		switch(ctxMode)
		{
			case KIKUCHI_CONTEXT_BEST_MODE: ctxSize = GetBestContextSize(img, tmpImg, plane, 
										cModel, kikuchiTemplate, pModel);
				break;
				
			case KIKUCHI_CONTEXT_GREEDY_MODE: ctxSize = GetGreedyContextSizeV1(img, tmpImg, plane, 
										cModel, kikuchiTemplate, pModel);
				break;
			
			/*
			case CONTEXT_GREEDY_MODE2: ctxSize = GetGreedyContextSizeV2(img, tmpImg, plane, 
										cModel, kikuchiTemplate, pModel);
				break;
			case CONTEXT_GREEDY_MODE3: ctxSize = GetGreedyContextSizeV3(img, tmpImg, plane, 
										cModel, kikuchiTemplate, pModel);
				break;
			*/
			
		}
			
		// Write the context size to the output stream	
		kikuchiTemplate->size = ctxSize;
		WriteNBits(ctxSize,4,fpOut);
		
		nBits = EncodeBitPlane(img, tmpImg, plane, cModel, kikuchiTemplate,
			pModel, fpOut);
			 
	 	totalBits += nBits;
	 		
		if(verbosityLevel > 0)
	 		{
	 		printf("Plane %02d: %8d bits ( %5.3f bpp ) | Template size: %02d\n", 
				plane, nBits, (double)nBits / (img->nRows * img->nCols), kikuchiTemplate->size);
			}
		}
		
	 // Send the remaining bit-planes uncoded.
	//for(plane = imgPlanes - codingPlanes ; plane > 0 ; plane--)
	for(plane = imgPlanes - codingPlanes - 1 ; plane >= 0 ; plane--)
		for(row = 0 ; row < img->nRows ; row++)
			for(col = 0 ; col < img->nCols ; col++)
				WriteNBits(GetGrayPixel(img, row, col) >> plane, 1, fpOut);

	finish_encode(fpOut);
	doneoutputtingbits(fpOut);
		
	fclose(fpOut);
	printf("Number of bytes of the encoded image: %"PRIu64" ( %.3f bpp )\n",
	  (uint64_t)_bytes_output, _bytes_output * 8. / (img->nRows * img->nCols));
	return EXIT_SUCCESS;
	}

