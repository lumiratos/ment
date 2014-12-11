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

/*----------------------------------------------------------------------------*/

BTNode *NewBTNode(void)

	{
	BTNode *node = (BTNode *)malloc(sizeof(BTNode));

	if(!node)
		{
		fprintf(stderr, "Error: no memory\n");
		exit(1);
		}

	node->nGrays = 0;
	node->nPixels = 0;
	node->left = NULL;
	node->right = NULL;
	return node;
	}

/*----------------------------------------------------------------------------*/
/*
static int CompareGrays(const void *x1, const void *x2)

	{
	return (*((Gray **)x1))->value > (*((Gray **)x2))->value ? 1 : -1;
	}
*/
/*----------------------------------------------------------------------------*/

void FindNodeToSplit(BTNode *node, BTNode **splitNode, int *maxError)

	{
	if(node->left == NULL) /* This node is a leaf */
		{
		if(node->error > *maxError)
			{
			*maxError = node->error;
			*splitNode = node;
			}

		}

	else
		{
		FindNodeToSplit(node->left, splitNode, maxError);
		FindNodeToSplit(node->right, splitNode, maxError);
		}

	}

void FindNodeToSplitLowerGray(BTNode *node, BTNode **splitNode, int *gray)
	{
	if(node->left == NULL) /* This node is a leaf */
		{
		if(node->gray < *gray && node->nGrays > 1)
			{
			*gray = node->gray;
			*splitNode = node;
			}
		}

	else
		{
		FindNodeToSplitLowerGray(node->left, splitNode, gray);
		FindNodeToSplitLowerGray(node->right, splitNode, gray);
		}

	}

void FindNodeToSplitHigherGrayCount(BTNode *node, BTNode **splitNode, int *nGrays)
	{
	if(node->left == NULL) /* This node is a leaf */
		{
		if(node->nGrays > *nGrays && node->nGrays > 1)
			{
			*nGrays = node->nGrays;
			*splitNode = node;
			}
		}

	else
		{
		FindNodeToSplitLowerGray(node->left, splitNode, nGrays);
		FindNodeToSplitLowerGray(node->right, splitNode, nGrays);
		}

	}

/*----------------------------------------------------------------------------*/

void AddGrayToBTNode(BTNode *node, Gray *gray)

	{
	if(node->nGrays == 0)
		{
		if(!(node->grays = (Gray **)malloc(sizeof(Gray *))))
			{
			fprintf(stderr, "Error: no memory\n");
			exit(1);
			}

		}

	else
		{
		if(!(node->grays = (Gray **)realloc(node->grays,
		  (node->nGrays + 1) * sizeof(Gray *))))
			{
			fprintf(stderr, "Error: no memory\n");
			exit(1);
			}

		}

	node->grays[node->nGrays++] = gray;
	}

/*----------------------------------------------------------------------------*/

void AddPixelToBTNode(BTNode *node, int row, int col)

	{
	if(node->nPixels == 0)
		{
		if(!(node->pixels = (ImageCoords *)malloc(sizeof(ImageCoords))))
			{
			fprintf(stderr, "Error: no memory\n");
			exit(1);
			}

		}

	else
		{
		if(!(node->pixels = (ImageCoords *)realloc(node->pixels,
		  (node->nPixels + 1) * sizeof(ImageCoords))))
			{
			fprintf(stderr, "Error: no memory\n");
			exit(1);
			}

		}

	node->pixels[node->nPixels].row = row;
	node->pixels[node->nPixels++].col = col;
	}

/*----------------------------------------------------------------------------*/

void ComputeNodeStatistics(BTNode *node)

	{
	node->gray = (node->grays[0]->value +
	  node->grays[node->nGrays-1]->value) / 2;
	node->error = node->grays[node->nGrays-1]->value - node->gray;
	}

/*----------------------------------------------------------------------------*/


void ComputeNodeStatistics2(BTNode *node)

	{
	node->gray = node->grays[0]->value;
	node->error = node->grays[node->nGrays-1]->value - node->gray;
	}

/*----------------------------------------------------------------------------*/

void SplitNode(Image *codImg, BTNode *node, int id)

	{
	int n;

	assert(node->nGrays > 1); /* Just in case... */

	node->left = NewBTNode();
	node->right = NewBTNode();

	/* This node inherits the id of its parent */
	node->left->id = node->id;

	/* This node gets a new id */
	node->right->id = id;

	//qsort(node->grays, node->nGrays, sizeof(Gray *), CompareGrays);

	AddGrayToBTNode(node->left, node->grays[0]);
	for(n = 1 ; n < node->nGrays ; n++)
		{
		if(node->grays[n]->value <= node->gray)
			AddGrayToBTNode(node->left, node->grays[n]);

		else
			AddGrayToBTNode(node->right, node->grays[n]);

		}

	ComputeNodeStatistics(node->left);
	ComputeNodeStatistics(node->right);
	}


/*----------------------------------------------------------------------------*/

void SplitNode2(Image *codImg, BTNode *node, int id)

	{
	int n;

	assert(node->nGrays > 1); /* Just in case... */

	node->left = NewBTNode();
	node->right = NewBTNode();

	/* This node inherits the id of its parent */
	node->left->id = node->id;

	/* This node gets a new id */
	node->right->id = id;

	//qsort(node->grays, node->nGrays, sizeof(Gray *), CompareGrays);

	AddGrayToBTNode(node->left, node->grays[0]);
	for(n = 1 ; n < node->nGrays ; n++)
		{
		if(node->grays[n]->value <= node->gray)
			AddGrayToBTNode(node->left, node->grays[n]);

		else
			AddGrayToBTNode(node->right, node->grays[n]);

		}

	ComputeNodeStatistics2(node->left);
	ComputeNodeStatistics2(node->right);
	}

/*----------------------------------------------------------------------------*/

int FindGrayValue(BTNode *node, int grayValue)

	{
	int n;

	/* The speed of this search could be increased... */
	for(n = 0 ; n < node->nGrays ; n++)
		if(node->grays[n]->value == grayValue)
			return 1;

	return 0;
	}

/*----------------------------------------------------------------------------*/

void WriteRawBits(Image *fullRecImg, Image *recImg, Image *codImg,
  BTNode *node, FILE *fpOut, int bitPlaneShift)

	{
	int n, row, col;

	for(n = 0 ; n < node->nPixels ; n++)
		{
		row = node->pixels[n].row;
		col = node->pixels[n].col;

		if(FindGrayValue(node->left, GetGrayPixel(codImg, row, col)))
			{
			WriteNBits(0, 1, fpOut);
			PutGrayPixel(recImg, row, col, node->left->gray);
			PutGrayPixel(fullRecImg, row, col, node->left->gray <<
			  bitPlaneShift);
			AddPixelToBTNode(node->left, row, col);
			}

		else
			{
			WriteNBits(1, 1, fpOut);
			PutGrayPixel(recImg, row, col, node->right->gray);
			PutGrayPixel(fullRecImg, row, col, node->right->gray <<
			  bitPlaneShift);
			AddPixelToBTNode(node->right, row, col);
			}

		}

	free(node->pixels);
	}

/*----------------------------------------------------------------------------*/

void ReadRawBits(Image *recImg, BTNode *node, FILE *fpInp, int targetBytes)

	{
	int n, row, col;

	for(n = 0 ; n < node->nPixels ; n++)
		{
		row = node->pixels[n].row;
		col = node->pixels[n].col;

		if(ReadNBits(1, fpInp) == 0)
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

