/*------------------------------------------------------------------------------

Copyright 2004-2008 Armando J. Pinho (ap@ua.pt), All Rights Reserved.

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
#include "linf.h"
#include "defs.h"

/*----------------------------------------------------------------------------*/

int TestUpdateRecImage(Image *recImg, Image *codImg, BTNode *node,
  CModel *cModel, CTemplate *cTemplate, PModel *pModel)

	{
	int n, row, col, pModelIdx;
	double nNats = 0;

	for(n = 0 ; n < node->nPixels ; n++)
		{
		row = node->pixels[n].row;
		col = node->pixels[n].col;

		pModelIdx = GetBinTreePModelIdx(recImg, row, col, cModel, cTemplate,
		  node->left->gray, node->right->gray);
		ComputePModel(cModel, pModel, pModelIdx);
		if(FindGrayValue(node->left, GetGrayPixel(codImg, row, col)))
			{
			nNats += PModelSymbolNats(pModel, 0);
			UpdateCModelCounter(cModel, pModelIdx, 0);
			PutGrayPixel(recImg, row, col, node->left->gray);
			}

		else
			{
			nNats += PModelSymbolNats(pModel, 1);
			UpdateCModelCounter(cModel, pModelIdx, 1);
			PutGrayPixel(recImg, row, col, node->right->gray);
			}

		}

	for(n = 0 ; n < node->nPixels ; n++)
		{
		row = node->pixels[n].row;
		col = node->pixels[n].col;
		PutGrayPixel(recImg, row, col, node->gray);
		}

	return (int)(nNats / M_LN2 + 0.5);
	}

/*----------------------------------------------------------------------------*/

int FindBestContextSizeGreedy(Image *recImg, Image *codImg, BTNode *node,
  CModel *cModel, CTemplate *cTemplate, PModel *pModel)

	{
	int nBits, bestNBitsL = INT_MAX, bestNBitsR = INT_MAX, bestSizeL = 0,
	  bestSizeR = 0, guessedCtxSize, ctxVar;

	guessedCtxSize = cModel->ctxSize;

	ResetCModelCounters(cModel);
	bestNBitsL = bestNBitsR = TestUpdateRecImage(recImg, codImg, node, cModel,
	  cTemplate, pModel);
	bestSizeL = bestSizeR = cModel->ctxSize;

	for(ctxVar = 1 ; ctxVar < cTemplate->size ; ctxVar++)
		{
		cModel->ctxSize = guessedCtxSize - ctxVar;
		if(cModel->ctxSize <= 0)
			break;

		ResetCModelCounters(cModel);
		nBits = TestUpdateRecImage(recImg, codImg, node, cModel,
		  cTemplate, pModel);

		if(nBits < bestNBitsL)
			{
			bestNBitsL = nBits;
			bestSizeL = cModel->ctxSize;
			}

		else
			break;

		}

	for(ctxVar = 1 ; ctxVar < cTemplate->size ; ctxVar++)
		{
		cModel->ctxSize = guessedCtxSize + ctxVar;
		if(cModel->ctxSize > cTemplate->size)
			break;

		ResetCModelCounters(cModel);
		nBits = TestUpdateRecImage(recImg, codImg, node, cModel,
		  cTemplate, pModel);

		if(nBits < bestNBitsR)
			{
			bestNBitsR = nBits;
			bestSizeR = cModel->ctxSize;
			}

		else
			break;

		}

	if(bestNBitsL < bestNBitsR)
		{
		cModel->ctxSize = bestSizeL;
		return bestNBitsL;
		}

	else
		{
		//cModel->ctxSize = bestSizeL;
		//return bestNBitsL;
		cModel->ctxSize = bestSizeR;
		return bestNBitsR;
		}

	}

/*----------------------------------------------------------------------------*/

int FindBestContextSize(Image *recImg, Image *codImg, BTNode *node,
  CModel *cModel, CTemplate *cTemplate, PModel *pModel, int ctxVar)

	{
	int nBits, bestNBits = INT_MAX, bestSize = 0, guessedCtxSize;

	guessedCtxSize = cModel->ctxSize;
	for(cModel->ctxSize = guessedCtxSize - ctxVar ;
	  cModel->ctxSize <= guessedCtxSize + ctxVar ; cModel->ctxSize++)
		{
		if(cModel->ctxSize <= 0 || cModel->ctxSize > cTemplate->size)
			continue;

		ResetCModelCounters(cModel);
		nBits = TestUpdateRecImage(recImg, codImg, node, cModel,
		  cTemplate, pModel);
		if(nBits < bestNBits)
			{
			bestNBits = nBits;
			bestSize = cModel->ctxSize;
			}

		}

	cModel->ctxSize = bestSize;
	return bestNBits;
	}

/*----------------------------------------------------------------------------*/

void UpdateRecImages(Image *fullRecImg, Image *recImg, Image *codImg,
  BTNode *node, CModel *cModel, CTemplate *cTemplate, PModel *pModel,
  FILE *fpOut, int bitPlaneShift)

	{
	int n, row, col, pModelIdx;

	for(n = 0 ; n < node->nPixels ; n++)
		{
		row = node->pixels[n].row;
		col = node->pixels[n].col;

		pModelIdx = GetBinTreePModelIdx(recImg, row, col, cModel, cTemplate,
		  node->left->gray, node->right->gray);
		ComputePModel(cModel, pModel, pModelIdx);
		if(FindGrayValue(node->left, GetGrayPixel(codImg, row, col)))
			{
			ArithEncodeSymbol(0, pModel->freqs, pModel->sum, fpOut);
			UpdateCModelCounter(cModel, pModelIdx, 0);
			PutGrayPixel(recImg, row, col, node->left->gray);
			PutGrayPixel(fullRecImg, row, col, node->left->gray <<
			  bitPlaneShift);
			AddPixelToBTNode(node->left, row, col);
			}

		else
			{
			ArithEncodeSymbol(1, pModel->freqs, pModel->sum, fpOut);
			UpdateCModelCounter(cModel, pModelIdx, 1);
			PutGrayPixel(recImg, row, col, node->right->gray);
			PutGrayPixel(fullRecImg, row, col, node->right->gray <<
			  bitPlaneShift);
			AddPixelToBTNode(node->right, row, col);
			}

		}

	free(node->pixels);
	}

/*----------------------------------------------------------------------------*/

int main(int argc, char *argv[])

	{
	int n, row, col, nGrays, codingBitPlanes, imgBitPlanes, bit, ctxMode,
	  ctxGuess, maxError, templateId, lastBytes = 0, nBits = 0, ctxVar;
	Image *codImg, *origImg, *recImg, *fullRecImg;
	ImageErr imgErr;
	int deltaNum = DEFAULT_PMODEL_DELTA_NUM,
	  deltaDen = DEFAULT_PMODEL_DELTA_DEN,
	  maxCount = DEFAULT_PMODEL_MAX_COUNT;
	char verbose = 'n';
	BTNode *rootNode, *node;
	Gray *imgGrays;
	FILE *fpOut = NULL;
	double alpha, b, m;
	CModel *cModel;
	CTemplate *cTemplate;
	PModel *pModel;
	unsigned long long prevBytes=0;

	alpha = 9; /* parameter for the chen guess mode */
	b = 0; m = 1; /* parameters for the pixs guess mode */
	codingBitPlanes = 16;
	templateId = TEMPLATE_BIN_TREE_A;
	ctxGuess = BIN_TREE_CONTEXT_GUESS_FIT;
	ctxMode = BIN_TREE_CONTEXT_MODE_GUESS;
	ctxVar = 2;
	if(argc == 1)
		{
		fprintf(stderr, "Usage: %11s [ -v (verbose) ]\n", argv[0]);
		fprintf(stderr, "                   [ -o outFile ]\n");
		fprintf(stderr, "                   [ -t templateId (def %d) ]\n",
		  templateId);
		fprintf(stderr, "                   [ -delta n/d (def %d/%d) ]\n",
		  deltaNum, deltaDen);
		fprintf(stderr, "                   [ -mc maxCount (def %d) ]\n",
		  maxCount);
		fprintf(stderr, "                   [ -p codingPlanes (def %d) ]\n",
		  codingBitPlanes);
		fprintf(stderr,
			"                   [ -ctxGuess chen/fit/pixs (def fit) ]\n");
		fprintf(stderr,
			"                   [ -ctxMode guess/greedy/var/best (def guess) ]\n");
        fprintf(stderr, "                   [ -alpha alpha (def %g) ]\n",alpha);
        fprintf(stderr, "                   [ -b b (def %g) ]\n", b);
        fprintf(stderr, "                   [ -m m (def %g) ]\n", m);
		fprintf(stderr, "                   [ -var ctxVar (def %d) ]\n",ctxVar);
		fprintf(stderr, "                   img\n");
		return 1;
		}

	for(n = 1 ; n < argc ; n++)
		if(strcmp("-v", argv[n]) == 0)
			{
			verbose = 'y';
			break;
			}

	for(n = 1 ; n < argc ; n++)
		if(strcmp("-ctxGuess", argv[n]) == 0)
			{
			if(strcmp("fit", argv[n+1]) == 0)
				ctxGuess = BIN_TREE_CONTEXT_GUESS_FIT;

			if(strcmp("chen", argv[n+1]) == 0)
				ctxGuess = BIN_TREE_CONTEXT_GUESS_CHEN;

			if(strcmp("pixs", argv[n+1]) == 0)
				ctxGuess = BIN_TREE_CONTEXT_GUESS_PIXS;

			break;
			}

	for(n = 1 ; n < argc ; n++)
		if(strcmp("-ctxMode", argv[n]) == 0)
			{
			if(strcmp("guess", argv[n+1]) == 0)
				ctxMode = BIN_TREE_CONTEXT_MODE_GUESS;

			if(strcmp("greedy", argv[n+1]) == 0)
				ctxMode = BIN_TREE_CONTEXT_MODE_GREEDY;

			if(strcmp("var", argv[n+1]) == 0)
				ctxMode = BIN_TREE_CONTEXT_MODE_VAR;

			if(strcmp("best", argv[n+1]) == 0)
				ctxMode = BIN_TREE_CONTEXT_MODE_BEST;

			break;
			}

	for(n = 1 ; n < argc ; n++)
		if(strcmp("-o", argv[n]) == 0)
			{
			if(!(fpOut = fopen(argv[n+1], "w")))
				{
				fprintf(stderr, "Error: can't open output file\n");
				exit(1);
				}

			break;
			}

	for(n = 1 ; n < argc ; n++)
		if(!strcmp("-t", argv[n]))
			{
			//templateId = atoi(argv[n+1]);
			switch(atoi(argv[n+1]))
				{
				case 1: templateId = TEMPLATE_BIN_TREE_A;
					break;
			
				case 2: templateId = TEMPLATE_BIN_TREE_B;
					break;	
				case 3: templateId = TEMPLATE_BIN_TREE_C;
					break;
				case 4: templateId = TEMPLATE_BIN_TREE_D;
					break;
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
			codingBitPlanes = atoi(argv[n+1]);
			if(codingBitPlanes < 1)
				codingBitPlanes = 1;

			break;
			}

	for(n = 1 ; n < argc ; n++)
		if(!strcmp("-alpha", argv[n]))
			{
			alpha = atof(argv[n+1]);
			break;
			}

	for(n = 1 ; n < argc ; n++)
		if(!strcmp("-b", argv[n]))
			{
			b = atof(argv[n+1]);
			break;
			}

	for(n = 1 ; n < argc ; n++)
		if(!strcmp("-m", argv[n]))
			{
			m = atof(argv[n+1]);
			break;
			}

	for(n = 1 ; n < argc ; n++)
		if(!strcmp("-var", argv[n]))
			{
			ctxVar = atoi(argv[n+1]);
			break;
			}

	pModel = CreatePModel(N_SYMBOLS);

	if(fpOut == NULL)
		fpOut = fopen("/dev/null", "w");

	if(!(origImg = ReadImageFile(argv[argc - 1])))
		return 1;

	/* Force re-calculation of the image maximum intensity */
	origImg->maxIntensity = 0;
	origImg->maxIntensity = ImageMaxIntensity(origImg);
	imgBitPlanes = (int)(log(origImg->maxIntensity) / M_LN2) + 1;

	/* Create the full reconstructed image */
	if(!(fullRecImg = CreateImage(origImg->nRows, origImg->nCols,
	  origImg->dataType, 1)))
		return 1;

	if(codingBitPlanes > imgBitPlanes)
		codingBitPlanes = imgBitPlanes;

	/* Create the image with only the coding bit-planes */
	if(!(codImg = CreateImage(origImg->nRows, origImg->nCols,
	  origImg->dataType, 1)))
		return 1;

	for(row = 0 ; row < origImg->nRows ; row++)
		for(col = 0 ; col < origImg->nCols ; col++)
			PutGrayPixel(codImg, row, col, GetGrayPixel(origImg, row, col) >>
			  (imgBitPlanes - codingBitPlanes));

	/* Force re-calculation of the image maximum intensity */
	codImg->maxIntensity = 0;
	codImg->maxIntensity = ImageMaxIntensity(codImg);

	if(!(imgGrays = (Gray *)calloc(codImg->maxIntensity + 1, sizeof(Gray))))
		{
		fprintf(stderr, "Error: no memory\n");
		return 1;
		}

	/* Init root node */
	rootNode = NewBTNode();
	rootNode->id = 0;
	for(row = 0 ; row < codImg->nRows ; row++)
		for(col = 0 ; col < codImg->nCols ; col++)
			{
			imgGrays[GetGrayPixel(codImg, row, col)].usage = 1;
			AddPixelToBTNode(rootNode, row, col);
			}

	/*
	 * Count the number of different pixel values, and initialize the
	 * list of the root node with them.
	 */
	nGrays = 0;
	for(n = 0 ; n < codImg->maxIntensity + 1 ; n++)
		{
		imgGrays[n].value = n;
		if(imgGrays[n].usage)
			{
			AddGrayToBTNode(rootNode, &imgGrays[n]);
			nGrays++;
			}

		}

	printf("Image has %d rows, %d cols and %d bit-planes\n",
	  codImg->nRows, codImg->nCols, imgBitPlanes);
	printf("Encoding the %d most significant bit-planes (%d grays)\n",
	  codingBitPlanes, nGrays);

	cTemplate = InitTemplate(templateId);

	printf("Using template:\n\n");
	ShowTemplate(cTemplate);
	putchar('\n');

	cModel = CreateCModel(cTemplate->size, N_SYMBOLS, N_CTX_SYMBOLS,
	  deltaNum, deltaDen, maxCount);

	startoutputtingbits();
	start_encode();

	/* Store the number of rows and cols */
	WriteNBits(codImg->nRows, STORAGE_BITS_N_ROWS, fpOut);
	WriteNBits(codImg->nCols, STORAGE_BITS_N_COLS, fpOut);

	/* Store the maximum intensity of the coding image */
	WriteNBits(codImg->maxIntensity - 1, STORAGE_BITS_MAX_INTENSITY, fpOut);

	/* Store the number of image bit-planes (1..16) */
	WriteNBits(imgBitPlanes - 1, STORAGE_BITS_N_IMG_PLANES, fpOut);

	/* Store the number of encoded bit-planes (1..16) */
	WriteNBits(codingBitPlanes - 1, STORAGE_BITS_N_COD_PLANES, fpOut);

	/* Store the template id */
	WriteNBits(templateId - 1, STORAGE_BITS_TEMPLATE_ID, fpOut);

	WriteNBits(cModel->deltaNum - 1, STORAGE_BITS_PMODEL_DELTA_NUM, fpOut);
	WriteNBits(cModel->deltaDen - 1, STORAGE_BITS_PMODEL_DELTA_DEN, fpOut);
	WriteNBits(maxCount, STORAGE_BITS_PMODEL_MAX_COUNT, fpOut);
	
	printf("Header Info: %"PRIu64" bytes\n", (uint64_t)_bytes_output);
	prevBytes=_bytes_output;

	/* Store an indication of which intensities are present in the image */
	for(n = 0 ; n < codImg->maxIntensity + 1 ; n++)
		if(imgGrays[n].usage)
			WriteNBits(1, 1, fpOut);

		else
			WriteNBits(0, 1, fpOut);

	printf("Gray side info: %"PRIu64" bytes\n", (uint64_t)(_bytes_output-prevBytes));
	/* Create the image for incremental reconstruction */
	if(!(recImg = CreateImage(codImg->nRows, codImg->nCols,
	  codImg->dataType, 1)))
		return 1;

	ComputeNodeStatistics(rootNode);

	/* Init the reconstructed images */
	for(row = 0 ; row < codImg->nRows ; row++)
		for(col = 0 ; col < codImg->nCols ; col++)
			{
			PutGrayPixel(recImg, row, col, rootNode->gray);
			PutGrayPixel(fullRecImg, row, col, rootNode->gray <<
			  (imgBitPlanes - codingBitPlanes));
			}

	for(n = 1 ; n < nGrays ; n++)
		{
		if(verbose == 'y')
			{
			CalcImageErr(origImg, fullRecImg, &imgErr);
			printf("%d : Ctx: %d Bytes: %"PRIu64" ( %"PRIu64" ) Bpp: %.3f "
			  "PSNR: %.1f Linf: %d (%.2f) %.2f\n", n, cModel->ctxSize,
			  (uint64_t)_bytes_output,(uint64_t)_bytes_output - lastBytes,
			  _bytes_output * 8. / (recImg->nRows * recImg->nCols),
			  imgErr.psnr[0], (int)imgErr.maxErr[0], n == 1 ? 0 :
			  (_bytes_output - lastBytes) * 8.0 / node->nPixels, n == 1 ? 0 :
			  (double)(node->nPixels) / n);
			lastBytes = _bytes_output;
			}
		/*
		else
			{
			printf("%3d\r", n);
			fflush(stdout);
			}
		*/
		node = NULL;
		maxError = 0;
		FindNodeToSplit(rootNode, &node, &maxError);
		SplitNode(codImg, node, n);

		switch(ctxGuess)
			{
			case BIN_TREE_CONTEXT_GUESS_CHEN:
				cModel->ctxSize = (int)ceil(alpha - log(n + 1) / M_LN2);
				break;

			case BIN_TREE_CONTEXT_GUESS_FIT:
				cModel->ctxSize = (int)ceil(0.671 * log(codImg->nRows *
				  codImg->nCols) / M_LN2 - 0.859 - log(n + 1) / M_LN2);
				break;

			case BIN_TREE_CONTEXT_GUESS_PIXS:
				cModel->ctxSize = (int)(m * log(node->nPixels) / M_LN2 + b -
				  log(n + 1) / M_LN2);
				break;
			}

		if(cModel->ctxSize < 1)
			cModel->ctxSize = 1;

		if(cModel->ctxSize > cTemplate->size)
			cModel->ctxSize = cTemplate->size;

		switch(ctxMode)
			{
			case BIN_TREE_CONTEXT_MODE_GUESS:
				nBits = FindBestContextSize(recImg, codImg, node, cModel,
				  cTemplate, pModel, 0);
				break;

			case BIN_TREE_CONTEXT_MODE_GREEDY:
				nBits = FindBestContextSizeGreedy(recImg, codImg, node,
				  cModel, cTemplate, pModel);
				break;

			case BIN_TREE_CONTEXT_MODE_VAR:
				nBits = FindBestContextSize(recImg, codImg, node, cModel,
				  cTemplate, pModel, ctxVar);
				break;

			case BIN_TREE_CONTEXT_MODE_BEST:
				nBits = FindBestContextSize(recImg, codImg, node, cModel,
				  cTemplate, pModel, cTemplate->size);
				break;
			}

		if(nBits + STORAGE_BITS_CONTEXT_SIZE < node->nPixels)
			{
			WriteNBits(CODE_MODE, 1, fpOut);
			WriteNBits(cModel->ctxSize - 1, STORAGE_BITS_CONTEXT_SIZE, fpOut);
			ResetCModelCounters(cModel);
			UpdateRecImages(fullRecImg, recImg, codImg, node, cModel,
			  cTemplate, pModel, fpOut, imgBitPlanes - codingBitPlanes);
			}

		else
			{
			WriteNBits(RAW_MODE, 1, fpOut);
			WriteRawBits(fullRecImg, recImg, codImg, node, fpOut,
			  imgBitPlanes - codingBitPlanes);
			if(verbose == 'y')
				putchar('*');

			}

		}

	if(verbose == 'y')
		{
		CalcImageErr(origImg, fullRecImg, &imgErr);
		printf("%d : Ctx: %d Bytes: %"PRIu64" ( %"PRIu64" ) Bpp: %.3f "
		  "PSNR: %.1f Linf: %d (%.2f) %.2f\n", n, cModel->ctxSize,
		  (uint64_t)_bytes_output, (uint64_t)_bytes_output - lastBytes,
		  _bytes_output * 8. / (recImg->nRows * recImg->nCols),
		  imgErr.psnr[0], (int)imgErr.maxErr[0], n == 1 ? 0 :
		  (_bytes_output - lastBytes) * 8.0 / node->nPixels, n == 1 ? 0 :
		  (double)(node->nPixels) / n);
		lastBytes = _bytes_output;
		}

	for(n = imgBitPlanes - codingBitPlanes - 1 ; n >= 0 ; n--)
		{
		if(verbose == 'y')
			{
			CalcImageErr(origImg, fullRecImg, &imgErr);
			printf("Bitplane: %d Bytes: %"PRIu64" ( %"PRIu64" ) Bpp: %.3f "
			  "PSNR: %.1f Linf: %d (%.2f)\n", n + 1,
			  (uint64_t)_bytes_output, (uint64_t)_bytes_output - lastBytes,
			  _bytes_output * 8. / (recImg->nRows * recImg->nCols),
			  imgErr.psnr[0], (int)imgErr.maxErr[0],
			  (_bytes_output - lastBytes) * 8.0 / node->nPixels);
			lastBytes = _bytes_output;
			}
		/*
		else
			{
			printf("Bitplane %3d\r", n + 1);
			fflush(stdout);
			}
		*/
		for(row = 0 ; row < origImg->nRows ; row++)
			for(col = 0 ; col < origImg->nCols ; col++)
				{
				bit = GetGrayPixel(origImg, row, col) >> n & 0x1;
				WriteNBits(bit, 1, fpOut);
				PutGrayPixel(fullRecImg, row, col,
				  GetGrayPixel(fullRecImg, row, col) | bit << n);
				}

		}

	finish_encode(fpOut);
	doneoutputtingbits(fpOut);

	if(imgBitPlanes != codingBitPlanes)
		{
		if(verbose == 'y')
			{
			CalcImageErr(origImg, fullRecImg, &imgErr);
			printf("Bitplane: 0 Bytes: %"PRIu64" ( %"PRIu64" ) Bpp: %.3f "
			  "PSNR: %.1f Linf: %d (%.2f)\n",
			  (uint64_t)_bytes_output, (uint64_t)_bytes_output - lastBytes,
			  _bytes_output * 8. / (recImg->nRows * recImg->nCols),
			  imgErr.psnr[0], (int)imgErr.maxErr[0],
			  (_bytes_output - lastBytes) * 8.0 / node->nPixels);
			lastBytes = _bytes_output;
			}
		/*
		else
			{
			printf("Bitplane 0\r");
			fflush(stdout);
			}
		*/
		}

	fclose(fpOut);
	printf("Number of bytes of the encoded image: %"PRIu64" ( %.3f bpp )\n",
	  (uint64_t)_bytes_output, _bytes_output * 8. / (recImg->nRows * recImg->nCols));
	return 0;
	}

