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
#include "bitio.h"
#include "arith.h"
#include "arith_aux.h"
#include "common.h"
#include "defs.h"

/*----------------------------------------------------------------------------*/
/*
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
	*/
/*----------------------------------------------------------------------------*/

int main(int argc, char *argv[])

	{
	int n, p, row, col, nRows, nCols, imgPlanes, codingPlanes, fileSize,
	  targetBytes, plane;
  	
  	int deltaNum = DEFAULT_PMODEL_DELTA_NUM,
  	  deltaDen = DEFAULT_PMODEL_DELTA_DEN,
  	  maxCount = DEFAULT_PMODEL_MAX_COUNT;
	
	int nChrs1 = (int)strlen(argv[0]), nChrs2;
  	nChrs2 = nChrs1 + (int)strlen("Usage: ");
	
	double decodedFraction = 1, gamma;
	char progressIndicator = 'y', verbose = 'n';
	unsigned int outFile = 0;
	Image *recImg, *expImg;
	FILE *fpInp;
	//CModel *cModel;
	CTemplate *cTemplateIntra = InitTemplate(TEMPLATE_INTRA);
	CTemplate *cTemplateInter = InitTemplate(TEMPLATE_INTER);
	CTemplate *cTemplateSBC = InitTemplate(TEMPLATE_KIKUCHI);
	//PModel *pModel;
	CPattern *cPattern;

	//----------------------------------------------------------------------
	int nCModels = 2, cModel, ctxSize, s, pModelIdx, y, symbol;
	CModel **cModels;
	PModel **pModels, *mixPModel;
	FloatPModel *floatPModel;
	double *cModelsNats, *cModelsWeight, *cModelsProb, totalWeight = 0;
	//double bestCModelNats, totalWeight = 0;
		
	//----------------------------------------------------------------------

	if(argc == 1)
		{
		fprintf(stderr, "\n");
		fprintf(stderr, "Usage: %*s [ -f decodedFraction ]\n", nChrs1, argv[0]);
		fprintf(stderr, "%*s [ -v (verbose)  ]\n", nChrs2, " ");
		fprintf(stderr, "%*s [ -o outputFile ]\n", nChrs2, " ");
		fprintf(stderr, "%*s codeFile\n", nChrs2, " ");
		return EXIT_FAILURE;
		}

	for(n = 1 ; n < argc ; n++)
		if(!strcmp("-f", argv[n]))
			{
			decodedFraction = atof(argv[n+1]);
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
			outFile = n+1;
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

	//deltaNum = ReadNBits(STORAGE_BITS_PMODEL_DELTA_NUM, fpInp) + 1;
	//deltaDen = ReadNBits(STORAGE_BITS_PMODEL_DELTA_DEN, fpInp) + 1;
	maxCount = ReadNBits(STORAGE_BITS_PMODEL_MAX_COUNT, fpInp);
	if(maxCount == 0 || maxCount > DEFAULT_PMODEL_MAX_COUNT)
		fprintf(stderr, "Warning (maxCount): counters may overflow\n");
	gamma = ReadNBits(STORAGE_BITS_GAMMA, fpInp) / (double)GAMMA_K;
	
	printf("Image has %d rows, %d cols and %d bit-planes\n", nRows,
	  nCols, imgPlanes);
	printf("Decoding the %d most significant bit-planes)\n", codingPlanes);
	printf("Delta: %d/%d, MaxCount: %d\n", deltaNum, deltaDen, maxCount);

	if(!(recImg = CreateImage(nRows, nCols, imgPlanes <= 8 ?
	  DATA_TYPE_C : DATA_TYPE_S, 1)))
		return EXIT_FAILURE;
	
	// ---------------------------------------------------------------------------
	// Create image with zeros
	if(!(expImg = CreateImage(nRows, nCols, imgPlanes <= 8 ? DATA_TYPE_C : DATA_TYPE_S, 1)))
		return EXIT_FAILURE;
	
	for(row = 0 ; row < expImg->nRows ; row++)
		for(col = 0 ; col < expImg->nCols ; col++)
			{
			// Initialize estimate of a decoded pixel value 2^(N-1) - 1
			PutGrayPixel(expImg, row, col, ((0x1 << (imgPlanes-1)) - 1));
			}
	// ---------------------------------------------------------------------------
	
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
	
	
	
	if(verbose == 'y')
		{
		printf("Using intra template:\n\n");
		ShowTemplate(cTemplateIntra);
		putchar('\n');
		printf("Using inter template:\n\n");
		ShowTemplate(cTemplateInter);
		putchar('\n');
		printf("Using alternative template:\n\n");
		ShowTemplate(cTemplateSBC);
		putchar('\n');
		}

	//cModel = CreateCModel(TEMPLATE_MAX_SIZE, N_SYMBOLS, N_CTX_SYMBOLS, deltaNum, deltaDen, maxCount);
	//pModel = CreatePModel(N_SYMBOLS);
	//cPattern = CreateCPattern(imgPlanes, MAX(cTemplateIntra->size, cTemplateInter->size));

	// ************************************************************
	for(cModel=0; cModel < nCModels; cModel++)
		{
		// For the 3D Finite Context Model
		if(cModel == 0)
			{
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

	for(plane = imgPlanes - 1 ; plane >= imgPlanes - codingPlanes ; plane--)
		{
		//printf("plane %d | imgPlanes - codingPlanes = %d\n", plane, imgPlanes - codingPlanes);
		/* Get the context pattern for this bit-plane */
		for(p = plane ; p < imgPlanes ; p++)
			{
			cPattern->size[p] = 0;
			for(n = 0 ; n < (p == plane ? cTemplateIntra->size :
			  cTemplateInter->size) ; n++)
				if((cPattern->pattern[p][n] = ReadNBits(1, fpInp)))
					cPattern->size[p]++;

			}

		// Get the context size to use in the Simple Bitplane Coding using the
		// pixels values estimates
		ctxSize = ReadNBits(STORAGE_BITS_CONTEXT_SIZE, fpInp);
		cTemplateSBC->size = ctxSize;
		
		if(verbose == 'y')
			{
			fprintf(stderr, "Plane %d: ", plane);
			for(n = plane ; n < imgPlanes ; n++)
				fprintf(stderr, "%d ", cPattern->size[n]);

			fprintf(stderr, "\n");
			}
		
		// Reset counters
		ResetCModelCounters(cModels[0x0]);
		ResetCModelCounters(cModels[0x1]);
		//cModelsNats[0x0] = 0;
		//cModelsNats[0x1] = 0;
		
		for(row = 0 ; row < recImg->nRows ; row++)
			{
			for(col = 0 ; col < recImg->nCols ; col++)
				{
				
				//printf("Row %d | Col %d \n", row, col);
				
				for(s = 0 ; s < N_SYMBOLS ; s++)
					floatPModel->freqs[s] = 0;
					
				//bestCModelNats = DBL_MAX;
				
				// For each model
				for(cModel = 0 ; cModel < nCModels ; cModel++)
					{
					// 3D template context
					if(cModel == 0x0)
						pModelIdx = GetPModelIdx(recImg, row, col, plane, imgPlanes,
			 		   		cModels[cModel], cTemplateIntra, cTemplateInter, cPattern);
					// SBC using pixel values estimate
					else
						pModelIdx = GetKikuchiPModelIdx(expImg, row, col, cTemplateSBC);
						
					ComputePModel(cModels[cModel], pModels[cModel], pModelIdx);	
															
					for(s = 0 ; s < N_SYMBOLS ; s++)
						{
						floatPModel->freqs[s] += (double) pModels[cModel]->freqs[s] /
						pModels[cModel]->sum * (cModelsWeight[cModel] / totalWeight);		
						}
					
					}
					
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
					
				// Decode the pixel bit value
				symbol = ArithDecodeSymbol(N_SYMBOLS, (int *)mixPModel->freqs, (int)mixPModel->sum, fpInp);
				
				PutGrayPixel(recImg, row, col, GetGrayPixel(recImg, row, col) | symbol << plane);	
				
				// Update models counts and their weights	
				totalWeight = 0;
				for(cModel = 0 ; cModel < nCModels ; cModel++)
					{					
					// 3D template context
					if(cModel == 0x0)
						pModelIdx = GetPModelIdx(recImg, row, col, plane, imgPlanes,
		 		   			cModels[cModel], cTemplateIntra, cTemplateInter, cPattern);
					// SBC using pixel values estimate
					else
						pModelIdx = GetKikuchiPModelIdx(expImg, row, col, cTemplateSBC);
						
					cModelsProb[cModel] = Pow(cModelsProb[cModel],gamma) * 
						(double)pModels[cModel]->freqs[symbol]/pModels[cModel]->sum;
					cModelsWeight[cModel] = cModelsProb[cModel];
					totalWeight += cModelsWeight[cModel];
					
					UpdateCModelCounter(cModels[cModel], pModelIdx, symbol);
					
					}
					
				// Update pixels of the image with the estimate values
				y = GetGrayPixel(expImg, row, col) + symbol * (0x1 << plane);
				if((plane - 1) >= 0) y -= (0x1 << (plane - 1));
				PutGrayPixel(expImg, row, col, y);
				
				}
			}

		if(_bytes_input > targetBytes)
			break;

		if(progressIndicator == 'y')
			fprintf(stderr, "Plane %02d decoded\n", plane);

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

	if ((outFile > 0) && (outFile < argc))
		WriteImageFile(argv[outFile], recImg);
	else
		WriteImageFile("!decMicroImg09.pgm", recImg);

	putchar('\n');

	return EXIT_SUCCESS;
	}

