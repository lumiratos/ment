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
#include "bitio.h"
#include "arith.h"
#include "arith_aux.h"
#include "common.h"
#include "linf.h"
#include "defs.h"

/*----------------------------------------------------------------------------*/

void UpdateRecImage(Image *recImg, BTNode *node, CModel *cModel,
  CTemplate *cTemplate, PModel *pModel, FILE *fpInp, int targetBytes)

	{
	int n, row, col, symbol, pModelIdx;

	for(n = 0 ; n < node->nPixels ; n++)
		{
		row = node->pixels[n].row;
		col = node->pixels[n].col;

		pModelIdx = GetBinTreePModelIdx(recImg, row, col, cModel, cTemplate,
		  node->left->gray, node->right->gray);
		ComputePModel(cModel, pModel, pModelIdx);
		symbol = ArithDecodeSymbol(cModel->nSymbols, pModel->freqs,
		  pModel->sum, fpInp);
		UpdateCModelCounter(cModel, pModelIdx, symbol);
		if(symbol == 0)
			{
			PutGrayPixel(recImg, row, col, node->left->gray);
			AddPixelToBTNode(node->left, row, col);
			}

		else
			{
			PutGrayPixel(recImg, row, col, node->right->gray);
			AddPixelToBTNode(node->right, row, col);
			}

		if(_bytes_input > targetBytes)
			break;

		}

	free(node->pixels);
	}

/*----------------------------------------------------------------------------*/

int main(int argc, char *argv[])

	{
	int n, row, col, nRows, nCols, nGrays, imgBitPlanes, codingBitPlanes,
	  fileSize, targetBytes, templateId, deltaNum, deltaDen, maxCount,
	  maxLinfError = 0, maxError, maxIntensity,
	  recGrays = 0; /* Stop reconstruction after "recGrays" graylevels */
	double decodedFraction = 1;
	//double decodedFraction = 1, minPsnr = 1000;
	char progressIndicator = 'y', outFilename[FILENAME_MAX]="";
	BTNode *rootNode, *node;
	Gray *imgGrays;
	Image *recImg = NULL;
	//Image *recImg, *origImg = NULL;
	FILE *fpInp;
	CModel *cModel;
	CTemplate *cTemplate;
	PModel *pModel;

	if(argc == 1)
		{
		fprintf(stderr, "Usage: %11s [ -f decodedFraction ]\n", argv[0]);
		fprintf(stderr, "                   [ -n recGrays ]\n");
		fprintf(stderr, "                   [ -o decodedImg ]\n");
		fprintf(stderr, "                   [ -nopi (no progress indicator) ]\n");
		//fprintf(stderr, "                   [ -img origImg ]\n");
		//fprintf(stderr, "                   [ -psnr minPsnr ]\n");
		fprintf(stderr, "                   [ -linf maxLinfError ]\n");
		fprintf(stderr, "                   codeFile\n");
		return 1;
		}

	for(n = 1 ; n < argc ; n++)
		if(!strcmp("-f", argv[n]))
			{
			decodedFraction = atof(argv[n+1]);
			break;
			}

	for(n = 1 ; n < argc ; n++)
		if(!strcmp("-n", argv[n]))
			{
			recGrays = atoi(argv[n+1]);
			break;
			}
	
	for(n = 1 ; n < argc-1 ; n++)
		if(!strcmp("-o", argv[n]))
			{
			strcpy(outFilename, argv[n+1]);
			break;
			}


	for(n = 1 ; n < argc ; n++)
		if(!strcmp("-nopi", argv[n]))
			{
			progressIndicator = 'n';
			break;
			}
	/*
	for(n = 1 ; n < argc ; n++)
		if(!strcmp("-img", argv[n]))
			{
			if(!(origImg = ReadImageFile(argv[n+1])))
				return 1;

			break;
			}

	for(n = 1 ; n < argc ; n++)
		if(!strcmp("-psnr", argv[n]))
			{
			if(origImg == NULL)
				{
				fprintf(stderr, "Option -psnr requires option -img\n");
				return 1;
				}

			minPsnr = atof(argv[n+1]);
			break;
			}
	*/
	for(n = 1 ; n < argc ; n++)
		if(!strcmp("-linf", argv[n]))
			{
			maxLinfError = atoi(argv[n+1]);
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

	/* Get the maximum intensity of the coding image */
	maxIntensity = ReadNBits(STORAGE_BITS_MAX_INTENSITY, fpInp) + 1;

	/* Get the number of image bit-planes (1..16) */
	imgBitPlanes = ReadNBits(STORAGE_BITS_N_IMG_PLANES, fpInp) + 1;

	/* Get the number of encoded bit-planes (1..16) */
	codingBitPlanes = ReadNBits(STORAGE_BITS_N_COD_PLANES, fpInp) + 1;

	/* Get the template id */
	templateId = ReadNBits(STORAGE_BITS_TEMPLATE_ID, fpInp) + 1;

	deltaNum = ReadNBits(STORAGE_BITS_PMODEL_DELTA_NUM, fpInp) + 1;
	deltaDen = ReadNBits(STORAGE_BITS_PMODEL_DELTA_DEN, fpInp) + 1;
	maxCount = ReadNBits(STORAGE_BITS_PMODEL_MAX_COUNT, fpInp);
	if(maxCount == 0 || maxCount > DEFAULT_PMODEL_MAX_COUNT)
		fprintf(stderr, "Warning (maxCount): counters may overflow\n");

	if(!(imgGrays = (Gray *)calloc(maxIntensity + 1, sizeof(Gray))))
		{
		fprintf(stderr, "Error: no memory\n");
		return 1;
		}

	/* Init root node */
	rootNode = NewBTNode();
	rootNode->id = 0;

	nGrays = 0;
	for(n = 0 ; n < maxIntensity + 1 ; n++)
		{
		imgGrays[n].value = n;
		imgGrays[n].usage = ReadNBits(1, fpInp);
		if(imgGrays[n].usage)
			{
			AddGrayToBTNode(rootNode, &imgGrays[n]);
			nGrays++;
			}

		}

	printf("Image has %d rows, %d cols and %d bit-planes\n", nRows,
	  nCols, imgBitPlanes);
	printf("Decoding the %d most significant bit-planes (%d grays)\n",
	  codingBitPlanes, nGrays);
	printf("Using template id %d\n", templateId);
	printf("Delta: %d/%d, MaxCount: %d\n", deltaNum, deltaDen, maxCount);

	if(recGrays == 0 || recGrays > nGrays)
		recGrays = nGrays;

	if(!(recImg = CreateImage(nRows, nCols, imgBitPlanes <= 8 ?
	  DATA_TYPE_C : DATA_TYPE_S, 1)))
		return 1;

	cTemplate = InitTemplate(templateId);
	cModel = CreateCModel(cTemplate->size, N_SYMBOLS, N_CTX_SYMBOLS,
	  deltaNum, deltaDen, maxCount);

	pModel = CreatePModel(N_SYMBOLS);

	ComputeNodeStatistics(rootNode);

	/* Init the reconstructed image */
	for(row = 0 ; row < recImg->nRows ; row++)
		for(col = 0 ; col < recImg->nCols ; col++)
			{
			PutGrayPixel(recImg, row, col, rootNode->gray);
			AddPixelToBTNode(rootNode, row, col);
			}

	for(n = 1 ; n < recGrays ; n++)
		{
		node = NULL;
		maxError = 0;
		FindNodeToSplit(rootNode, &node, &maxError);
		if(maxError << (imgBitPlanes - codingBitPlanes) <= maxLinfError)
			{
			targetBytes = 0;
			break;
			}

		SplitNode(recImg, node, n);

		if(ReadNBits(1, fpInp) == CODE_MODE)
			{
			cModel->ctxSize = ReadNBits(STORAGE_BITS_CONTEXT_SIZE, fpInp) + 1;
			ResetCModelCounters(cModel);
			UpdateRecImage(recImg, node, cModel, cTemplate, pModel,
			  fpInp, targetBytes);
			}

		else
			{
			ReadRawBits(recImg, node, fpInp, targetBytes);
			}

		if(_bytes_input > targetBytes)
			break;

		if(progressIndicator == 'y')
			fprintf(stderr, "%3d\r", n + 1);

		}

	for(n = imgBitPlanes - codingBitPlanes - 1 ; n >= 0 ; n--)
		{
		if(1 << n <= maxLinfError)
			targetBytes = 0;

		for(row = 0 ; row < recImg->nRows ; row++)
			for(col = 0 ; col < recImg->nCols ; col++)
				if(_bytes_input > targetBytes)
					PutGrayPixel(recImg, row, col, (GetGrayPixel(recImg,
					  row, col) << 1));

				else
					PutGrayPixel(recImg, row, col, (GetGrayPixel(recImg,
					  row, col) << 1) | ReadNBits(1, fpInp));

		}

	printf("Decoded %"PRIu64" bytes ( %.3f bpp )\n", (uint64_t)_bytes_input,
	  _bytes_input * 8. / (recImg->nRows * recImg->nCols));
	finish_decode();
	doneinputtingbits();

	fclose(fpInp);

	if(strcmp(outFilename, "") == 0)
		{
		strcat(outFilename, argv[argc-1]);
		strcat(outFilename, ".dec");
		}
	WriteImageFile(outFilename, recImg);
	//WriteImageFile("!grayImage.pgm.gz", recImg);
	putchar('\n');
	return 0;
	}

