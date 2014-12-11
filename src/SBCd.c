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
void DecodeBitPlane(Image *recImg, Image *tmpImg, int plane, CModel *cModel, 
	CTemplate *kikuchiTemplate, PModel *pModel, FILE *fpInp, int targetBytes)

	{
	int row, col, pModelIdx, symbol, y;

	ResetCModelCounters(cModel);
	
	for(row = 0 ; row < recImg->nRows ; row++)
		for(col = 0 ; col < recImg->nCols ; col++)
			{
			
			pModelIdx = GetKikuchiPModelIdx(tmpImg, row, col, kikuchiTemplate);
			ComputePModel(cModel, pModel, pModelIdx);
			symbol = ArithDecodeSymbol(cModel->nSymbols, pModel->freqs,
			  pModel->sum, fpInp);
								
			//PutGrayPixel(recImg, row, col, GetGrayPixel(recImg, row, col) |
			//  symbol << (plane-1));
			PutGrayPixel(recImg, row, col, GetGrayPixel(recImg, row, col) |
				symbol << plane);
			UpdateCModelCounter(cModel, pModelIdx, symbol);

			
			//y = GetGrayPixel(tmpImg, row, col) + symbol * (0x1 << (plane - 1));
			//if((plane - 2) >= 0) y -= (0x1 << (plane - 2));
			y = GetGrayPixel(tmpImg, row, col) + symbol * (0x1 << plane);
			if((plane - 1) >= 0) y -= (0x1 << (plane - 1));
			PutGrayPixel(tmpImg, row, col, y);

			if(_bytes_input > targetBytes)
				break;

			}

	}

/*----------------------------------------------------------------------------*/

int main(int argc, char *argv[])

	{
	int n, row, col, nRows, nCols, imgPlanes, codingPlanes, fileSize,
	  targetBytes, deltaNum, deltaDen, maxCount, plane, tmpSize;
	double decodedFraction = 1;
	char verbose = 'n', outFileName[FILENAME_MAX]="";
	unsigned int outFile = 0;
	Image *recImg, *tmpImg;
	FILE *fpInp;
	CModel *cModel;
	CTemplate *kikuchiTemplate = InitTemplate(TEMPLATE_KIKUCHI);
	PModel *pModel;

	if(argc == 1)
		{
		fprintf(stderr,"\nUsage: %12s [ -f decodedFraction ]\n", argv[0]);
		//fprintf(stderr,"Usage: SimpleBitplaneDec [ -f decodedFraction ]\n");
		fprintf(stderr,"                    [ -v (verbose) ]\n");
		fprintf(stderr,"                    [ -o outputFile ]\n");
		fprintf(stderr,"                    codeFile\n");
		
		fprintf(stderr, "\n\nThis compressor is based on the work of Kikuchi et al.\n");
		fprintf(stderr, "Please consult the following papers for more information.\n\n");
		fprintf(stderr, "%s\n\n", KIKUCHI);
		
		return EXIT_FAILURE;
		}

	for(n = 1 ; n < argc ; n++)
		if(!strcmp("-v", argv[n]))
			{
			verbose = 'y';
			break;
			}
			
	for(n = 1 ; n < argc ; n++)
		if(!strcmp("-f", argv[n]))
			{
			decodedFraction = atof(argv[n+1]);
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

	deltaNum = ReadNBits(STORAGE_BITS_PMODEL_DELTA_NUM, fpInp) + 1;
	deltaDen = ReadNBits(STORAGE_BITS_PMODEL_DELTA_DEN, fpInp) + 1;
	maxCount = ReadNBits(STORAGE_BITS_PMODEL_MAX_COUNT, fpInp);
	if(maxCount == 0 || maxCount > DEFAULT_PMODEL_MAX_COUNT)
		fprintf(stderr, "Warning (maxCount): counters may overflow\n");

	printf("Image has %d rows, %d cols and %d bit-planes\n", nRows, nCols, imgPlanes);
	printf("Decoding the %d most significant bit-planes)\n", codingPlanes);
	printf("Delta: %d/%d, MaxCount: %d\n", deltaNum, deltaDen, maxCount);

	if(!(recImg = CreateImage(nRows, nCols, imgPlanes <= 8 ? DATA_TYPE_C : DATA_TYPE_S, 1)))
		return EXIT_FAILURE;
	if(!(tmpImg = CreateImage(nRows, nCols, imgPlanes <= 8 ? DATA_TYPE_C : DATA_TYPE_S, 1)))
		return EXIT_FAILURE;
	
	for(row = 0 ; row < tmpImg->nRows ; row++)
		for(col = 0 ; col < tmpImg->nCols ; col++)
			{
			// Initialize estimate of a decoded pixel value 2^(N-1) - 1
			PutGrayPixel(tmpImg, row, col, ((0x1 << (imgPlanes-1)) - 1));
			}
	
	if(verbose == 'y')
		{
		printf("Using template:\n\n");
		ShowTemplate(kikuchiTemplate);
		putchar('\n');
		}

	cModel = CreateCModel(kikuchiTemplate->size, N_SYMBOLS, N_CTX_SYMBOLS, deltaNum, deltaDen, maxCount);
	pModel = CreatePModel(N_SYMBOLS);

	
	//for(plane = imgPlanes ; plane > imgPlanes - codingPlanes ; plane--)
	for(plane = imgPlanes-1 ; plane >= imgPlanes - codingPlanes ; plane--)
		{
		// Read the context size
		kikuchiTemplate->size = ReadNBits(4, fpInp);	
		// Decode the bitplane
		DecodeBitPlane(recImg,	tmpImg, plane, cModel, kikuchiTemplate, pModel, fpInp, targetBytes);

		if(verbose == 'y')
			{
				fprintf(stderr, "Plane %02d decoded | Template size: %02d\n", plane, kikuchiTemplate->size);
			}
		if(_bytes_input > targetBytes)
			break;
		
	}


		
	//for(plane = imgPlanes - codingPlanes ; plane > 0 ; plane--)
	for(plane = imgPlanes - codingPlanes-1 ; plane >= 0 ; plane--)
		for(row = 0 ; row < recImg->nRows ; row++)
			for(col = 0 ; col < recImg->nCols ; col++)
				if(_bytes_input <= targetBytes)
					PutGrayPixel(recImg, row, col, GetGrayPixel(recImg, row, 
						col) | (ReadNBits(1, fpInp) << plane));

	printf("Decoded %"PRIu64" bytes\n", (uint64_t)_bytes_input);
	finish_decode();
	doneinputtingbits();

	fclose(fpInp);

	tmpSize = snprintf(outFileName, FILENAME_MAX, "%s.dec", argv[argc - 1]);
	if(tmpSize >= FILENAME_MAX)
		{
			fprintf(stderr, "Error: can't use string '%s' as a file name. Please use the '-o' flag.\n", outFileName);
			return EXIT_FAILURE;
		}

	if ((outFile > 0) && (outFile < argc))
		WriteImageFile(argv[outFile], recImg);
	else
		WriteImageFile(outFileName, recImg);

	putchar('\n');

	return EXIT_SUCCESS;
	}

