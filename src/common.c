/*------------------------------------------------------------------------------

Copyright 2006 Armando J. Pinho (ap@ua.pt), All Rights Reserved.

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
#include <math.h>
#include <string.h>
#include <limits.h>
#include "common.h"

/*
 *     5  2  6
 *     1  0  3
 *     8  4  7
 */
static ImageCoords templateInter[] = {{0, 0}, {0, -1}, {-1, 0}, {0, 1}, {1, 0},
	{-1, -1}, {-1, 1}, {1, 1}, {1, -1}};

/*
 * From cod:martins:98a, 2-norm.
 *
 *        10  6  9
 *      8  4  2  3  7
 *      5  1  X
 */
static ImageCoords templateIntra[] = {{0, -1}, {-1, 0}, {-1, 1}, {-1, -1},
	{0, -2}, {-2, 0}, {-1, 2}, {-1, -2}, {-2, 1}, {-2, -1}};


/*
 * From Kikuchi-2009
 *
 *            9
 *         5  2  6  8
 *      7  1  X  3
 *            4
 */
//static ImageCoords templateK9[] = {{0, -1}, {-1, 0}, {0, 1}, {1, 0},
//      {-1, -1}, {-1, 1}, {0, -2}, {-1, 2}, {-2, 0}};


/*
 *
 *         13  9 12
 *      14  5  2  6  8
 *   15  7  1  X  3
 *         10  4 11
 */
static ImageCoords templateK9[] = {{0, -1}, {-1, 0}, {0, 1}, {1, 0},
    {-1, -1}, {-1, 1}, {0, -2}, {-1, 2}, {-2, 0}, {1, -1}, {1, 1},
        {-2, 1}, {-2, -1}, {-1, -2}, {0, -3}};




/*
 *      14 10 15
 *   13  5  2  6 16
 *    9  1  X  3 11
 *       8  4  7
 *         12
 */
static ImageCoords templateA[] = {{0, -1}, {-1, 0}, {0, 1}, {1, 0},
        {-1, -1}, {-1, 1}, {1, 1}, {1, -1}, {0, -2}, {-2, 0}, {0, 2}, {2, 0},
        {-1, -2}, {-2, -1}, {-2, 1}, {-1, 2}};

/*
 * From cod:martins:98a, 2-norm.
 *
 *             14
 *       12 10  6  9 11
 *    16  8  4  2  3  7 15
 *    13  5  1  X
 */
static ImageCoords templateB[] = {{0, -1}, {-1, 0}, {-1, 1}, {-1, -1},
        {0, -2}, {-2, 0}, {-1, 2}, {-1, -2}, {-2, 1}, {-2, -1}, {-2, 2},
        {-2, -2}, {0, -3}, {-3, 0}, {-1, 3}, {-1, -3}};

/*
 * From cod:martins:98a, 1-norm.
 *
 *             12
 *          11  6 10 16
 *    15  9  5  2  4  8 14
 * 13  7  3  1  X
 */
static ImageCoords templateC[] = {{0, -1}, {-1, 0}, {0, -2}, {-1, 1},
        {-1, -1}, {-2, 0}, {0, -3}, {-1, 2}, {-1, -2}, {-2, 1}, {-2, -1},
        {-3, 0}, {0, -4}, {-1, 3}, {-1, -3}, {-2, 2}};

/*
 *      14 10 15
 *   13  5  2  6
 *    9  1  X  3 11
 *       8  4  7
 *         12
 */
static ImageCoords templateD[] = {{0, 0}, {0, -1}, {-1, 0}, {0, 1}, {1, 0},
        {-1, -1}, {-1, 1}, {1, 1}, {1, -1}, {0, -2}, {-2, 0}, {0, 2}, {2, 0},
        {-1, -2}, {-2, -1}, {-2, 1}};


/*----------------------------------------------------------------------------*/

int GetPModelIdx(Image *img, int row, int col, int plane, int imgPlanes,
  CModel *cModel, CTemplate *cTemplateIntra, CTemplate *cTemplateInter,
  CPattern *cPattern)

    {
    int i, n = 0, ctxPlane, gray, idx = 0, ctxSize;

	/* INTRA */
	ctxSize = 0;
	for(i = 0 ; i < cTemplateIntra->size &&
	  ctxSize < cPattern->size[plane] ; i++)
		{
		if(cPattern->pattern[plane][i])
			{
			gray = GetGrayPixel(img, row + cTemplateIntra->position[i].row,
			  col + cTemplateIntra->position[i].col);
			idx += (((gray >> plane) & 0x1) << n++);
			ctxSize++;
			}

		}

	/* INTER */
	for(ctxPlane = plane + 1 ; ctxPlane < imgPlanes ; ctxPlane++)
		{
		ctxSize = 0;
		for(i = 0 ; i < cTemplateInter->size &&
		  ctxSize < cPattern->size[ctxPlane] ; i++)
			if(cPattern->pattern[ctxPlane][i])
				{
				gray = GetGrayPixel(img, row + cTemplateInter->position[i].row,
				  col + cTemplateInter->position[i].col);
				idx += (((gray >> ctxPlane) & 0x1) << n++);
				ctxSize++;
				}

		}

	return idx;
    }

/*----------------------------------------------------------------------------*/

int GetKikuchiPModelIdx(Image *tmpImg, int row, int col, CTemplate *kikuchiTemplate)
        {
                int i, y, yi, qi, n=0, idx = 0x0;

                for(i = 0 ; i < kikuchiTemplate->size; i++)
                {
                        // Non-causal pixel are not considered in the most significant plane
                        //if( ((plane == imgPlanes) && (IsItCasual(row, col, kikuchiTemplate, i) == 0x1)) || (plane != imgPlanes) )
                        //{

                        // Latest decoded estimated Y
                        y = GetGrayPixel(tmpImg, row, col);

                        // Latest decoded estimated Y(i)
                    yi = GetGrayPixel(tmpImg, row + kikuchiTemplate->position[i].row,
                        col + kikuchiTemplate->position[i].col);

                        // Context bit
                        qi = ((yi > y) ? 0x1 : 0x0);

                        // Index
                        idx += qi << n++;

                        //}
                }
                return idx;
        }


/*----------------------------------------------------------------------------*/

int GetBinTreePModelIdx(Image *recImg, int row, int col, CModel *cModel,
  CTemplate *cTemplate, int leftGray, int rightGray)

    {
    int n, gray, idx = 0;

        /*
         * Note that the real size used for context calculation may be smaller
         * than the template size. By changing the value of "cModel->ctxSize"
         * this can be dynamically controlled.
         */
        for(n = 0 ; n < cModel->ctxSize ; n++)
                {
                gray = GetGrayPixel(recImg, row + cTemplate->position[n].row,
                  col + cTemplate->position[n].col);
                if(abs(gray - leftGray) > abs(gray - rightGray))
            idx += (1 << n);

        }

        return idx;
    }


/*----------------------------------------------------------------------------*/

CTemplate *InitTemplate(templateId)

	{
	CTemplate *cTemplate;

	if((cTemplate = malloc(sizeof(CTemplate))) == NULL)
		{
		fprintf(stderr, "Error: out of memory\n");
		exit(1);
		}

	switch(templateId)
		{
		case TEMPLATE_INTRA:
			cTemplate->position = templateIntra;
			cTemplate->size = sizeof(templateIntra) / sizeof(templateIntra[0]);
			break;

		case TEMPLATE_INTER:
			cTemplate->position = templateInter;
			cTemplate->size = sizeof(templateInter) / sizeof(templateInter[0]);
			break;

		case TEMPLATE_KIKUCHI:
                        cTemplate->position = templateK9;
                        cTemplate->size = sizeof(templateK9) / sizeof(templateK9[0]);
                        break;
		
		case TEMPLATE_BIN_TREE_A:
			cTemplate->position = templateA;
			cTemplate->size = sizeof(templateA) / sizeof(templateA[0]);
			break;

		case TEMPLATE_BIN_TREE_B:
			cTemplate->position = templateB;
			cTemplate->size = sizeof(templateB) / sizeof(templateB[0]);
			break;

		case TEMPLATE_BIN_TREE_C:	
			cTemplate->position = templateC;
			cTemplate->size = sizeof(templateC) / sizeof(templateC[0]);
			break;

		case TEMPLATE_BIN_TREE_D:	
			cTemplate->position = templateD;
			cTemplate->size = sizeof(templateD) / sizeof(templateD[0]);
			break;


		default:
			fprintf(stderr, "Error: invalid template id\n");
			exit(1);

		}

	return cTemplate;
	}

/*----------------------------------------------------------------------------*/

void ShowTemplate(CTemplate *cTemplate)

	{
	int minRow, maxRow, minCol, maxCol, n, row, col;
	int **templateMatrix;

	minRow = maxRow = cTemplate->position[0].row;
	minCol = maxCol = cTemplate->position[0].col;
	for(n = 1 ; n < cTemplate->size ; n++)
		{
		if(cTemplate->position[n].row > maxRow)
			maxRow = cTemplate->position[n].row;

		if(cTemplate->position[n].row < minRow)
			minRow = cTemplate->position[n].row;

		if(cTemplate->position[n].col > maxCol)
			maxCol = cTemplate->position[n].col;

		if(cTemplate->position[n].col < minCol)
			minCol = cTemplate->position[n].col;

		}

	if((templateMatrix = (int **)calloc(maxRow - minRow + 1,
	  sizeof(int *))) == NULL)
		{
		fprintf(stderr, "Error: out of memory\n");
		exit(1);
		}

	for(row = 0 ; row < maxRow - minRow + 1 ; row++)
		if((templateMatrix[row] = (int *)calloc(maxCol - minCol + 1,
		  sizeof(int))) == NULL)
			{
			fprintf(stderr, "Error: out of memory\n");
			exit(1);
			}

	for(n = 0 ; n < cTemplate->size ; n++)
		templateMatrix[cTemplate->position[n].row - minRow]
		  [cTemplate->position[n].col - minCol] = n + 1;

	templateMatrix[-minRow][-minCol] = -1;

	for(row = 0 ; row < maxRow - minRow + 1 ; row++)
		{
		for(col = 0 ; col < maxCol - minCol + 1 ; col++)
			if(templateMatrix[row][col])
				{
				if(templateMatrix[row][col] == -1)
					printf("  X");

				else
					printf("%3d", templateMatrix[row][col]);

				}

			else
				printf("   ");

		putchar('\n');
		}

	for(row = 0 ; row < maxRow - minRow + 1 ; row++)
		free(templateMatrix[row]);
	
	free(templateMatrix);
	}

/*----------------------------------------------------------------------------*/

CPattern *CreateCPattern(int imgPlanes, int maxTemplateSize)

	{
	int plane;
	CPattern *cPattern;

	if((cPattern = (CPattern *)malloc(sizeof(CPattern))) == NULL)
		{
		fprintf(stderr, "Error: out of memory\n");
		exit(1);
		}

	if((cPattern->size = (int *)calloc(imgPlanes, sizeof(int))) == NULL)
		{
		fprintf(stderr, "Error: out of memory\n");
		exit(1);
		}

	if((cPattern->pattern = (int **)calloc(imgPlanes, sizeof(int *))) == NULL)
		{
		fprintf(stderr, "Error: out of memory\n");
		exit(1);
		}

	for(plane = 0 ; plane < imgPlanes ; plane++)
		if((cPattern->pattern[plane] = (int *)calloc(maxTemplateSize,
		  sizeof(int))) == NULL)
			{
			fprintf(stderr, "Error: out of memory\n");
			exit(1);
			}

	cPattern->totalSize = 0;
	cPattern->imgPlanes = imgPlanes;
	cPattern->maxTemplateSize = maxTemplateSize;
	return cPattern;
	}

/*----------------------------------------------------------------------------*/

void FreeCPattern(CPattern *cPattern)

	{
	int plane;

	for(plane = 0 ; plane < cPattern->imgPlanes ; plane++)
		free(cPattern->pattern[plane]);

	free(cPattern->pattern);
	free(cPattern->size);
	free(cPattern);
	}

/*----------------------------------------------------------------------------*/

void CopyCPattern(CPattern *cPatternDest, CPattern *cPatternOrig)

	{
	int plane;

	for(plane = 0 ; plane < cPatternDest->imgPlanes ; plane++)
		memcpy(cPatternDest->pattern[plane], cPatternOrig->pattern[plane],
		  cPatternDest->maxTemplateSize * sizeof(int));

	memcpy(cPatternDest->size, cPatternOrig->size,
	  cPatternDest->imgPlanes * sizeof(int));
	cPatternDest->totalSize = cPatternOrig->totalSize;
	}

/*----------------------------------------------------------------------------*/

int FindGrayFromCodeWord(Gray *imgGrays, int nGrays, int codeword)

        {
        int i;
        for(i = 0; i < nGrays; i++)
        	{
                if( (imgGrays[i].codeword == codeword) && (imgGrays[i].usage == 1) )
                	return imgGrays[i].value;
                        //return i;
                }

        fprintf(stderr, "Error(FindGrayFromCodeWord): codeword %d not found!\n", codeword);
        exit(EXIT_FAILURE);
        }

/*----------------------------------------------------------------------------*/

double Pow(double a, double b)

        {
        /*
        int tmp = (*(1 + (int *)&a));
        int tmp2 = (int)(b * (tmp - 1072632447) + 1072632447);
        double p = 0.0;
        *(1 + (int * )&p) = tmp2;
        return p;
        */
        return pow(a,b);
        }

/*----------------------------------------------------------------------------*/
/*
int IsItCasual(int row, int col, CTemplate *cTemplate, int templateInd)

        {
                int tRow, tCol;
                tRow = cTemplate->position[templateInd].row + row;
                tCol = cTemplate->position[templateInd].row + col;

                if((tRow > row) || (tRow >= row && tCol >= col)) return 0x0; // false is non-casual
                return 0x1; // true it is casual
        }
*/
