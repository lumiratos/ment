/*
 ***********************************************************************
 * Copyright 2011 Lui M. O. Matos (luismatos@ua.pt), All Rights
 * Reserved. These programs are supplied free of charge for research 
 * purposes only, and may not be sold or incorporated into any commercial 
 * product. There is ABSOLUTELY NO WARRANTY of any sort, nor any 
 * undertaking that they are fit for ANY PURPOSE WHATSOEVER. Use them at 
 * your own risk. If you do happen to find a bug, or have modifications 
 * to suggest, please report the same to Luís M. O. Matos, luismatos@ua.pt. 
 * The copyright notice above and this statement of conditions must 
 * remain an integral part of each and every copy made of these files.
 ***********************************************************************
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <limits.h>
#include <assert.h>
#include "image.h"
#include "sbr.h"


/*************************************************************/

SBRNode *NewSBRNode(void)
{
	SBRNode *node = (SBRNode *)malloc(sizeof(SBRNode));

	if(!node)
	{
		fprintf(stderr, "Error (NewSBRNode): no memory\n");
		return NULL;
	}

	node->codeword = -1;
	node->codewordSize = 0;
	node->MinMax = 0;
	node->grays = NULL;
	node->nGrays = 0;
	node->left = NULL;
	node->right = NULL;
	return node;
}

/*************************************************************/

void AddGrayToSBRTNode(SBRNode *node, int gray)
{
	if(node->nGrays == 0)
	{
		if(!(node->grays = (int *)malloc(sizeof(int))))
		{
			fprintf(stderr, "Error (AddGrayToSBRTNode): no memory!\n");
			exit(EXIT_FAILURE);
		}

	}
	else
	{
		if(!(node->grays = (int *)realloc(node->grays, (node->nGrays + 1) * sizeof(int))))
		{
			fprintf(stderr, "Error (AddGrayToSBRTNode): no memory!\n");
			exit(EXIT_FAILURE);
		}
	}
	
	node->grays[node->nGrays++] = gray;
}

/*************************************************************/

void SplitSBRNode(SBRNode *node)
{
	
	int n;

	assert(node->nGrays > 1); /* Just in case... */

	node->left = NewSBRNode();
	node->right = NewSBRNode();
	node->MinMax = (node->grays[node->nGrays - 1] + node->grays[0]) / 2;
	
	for(n = 0 ; n < node->nGrays ; n++)
	{
			if(node->grays[n] <= node->MinMax)
				AddGrayToSBRTNode(node->left, node->grays[n]);
			else
				AddGrayToSBRTNode(node->right, node->grays[n]);
	}
	
	/* Root node */
	if(node->codewordSize == 0)
	{
		node->left->codewordSize = 1;
		node->right->codewordSize = 1;
		node->left->codeword =  0x0;
		node->right->codeword = 0x1;
	}
	
	else
	{
		node->left->codewordSize = node->codewordSize+1;
		node->right->codewordSize = node->codewordSize+1;
		node->left->codeword = (node->codeword << 0x1);
		node->right->codeword = (node->codeword << 0x1) | 0x1;
	}
}

/*************************************************************/
int BuidSBRT(SBRNode *node, Gray *imgGrays, int imgPlanes)
{
	int l, r;
	if(node == NULL || node->nGrays < 2)
		return 0;
	
	SplitSBRNode(node);
	
	if(node->left->nGrays > 1)
	{
		l = BuidSBRT(node->left, imgGrays, imgPlanes);
	}
	else
	{
		//printf("\tGray %3d =>L %02X (%d)\n", node->left->grays[0], node->left->codeword << (imgPlanes - node->left->codewordSize), node->left->codewordSize);
		imgGrays[node->left->grays[0]].codeword = (node->left->codeword << (imgPlanes - node->left->codewordSize));
		imgGrays[node->left->grays[0]].codewordSize = node->left->codewordSize;
		l = imgGrays[node->left->grays[0]].codewordSize;
	}
	
	if(node->right->nGrays > 1)
	{
		r = BuidSBRT(node->right, imgGrays, imgPlanes);
	}
	else
	{
		//printf("\tGray %3d =>R %02X (%d)\n", node->right->grays[0], node->right->codeword << (imgPlanes - node->right->codewordSize), node->right->codewordSize);
		imgGrays[node->right->grays[0]].codeword = (node->right->codeword << (imgPlanes - node->right->codewordSize));
		imgGrays[node->right->grays[0]].codewordSize = node->right->codewordSize;
		r = imgGrays[node->right->grays[0]].codewordSize;
	}
	
	if(r > l) return r;
	return l;
}

/*************************************************************/

Image *GetSBRImage(Image *img, Gray *imgGrays)
{
	int i, nGrays = 0, r, c;
	SBRNode *rootNode;
	Image *SBRImage;
	int imgPlanes = (int)(log(img->maxIntensity) / M_LN2) + 1;
	
	//printf("N PLANES = %d\n", imgPlanes);
	
	rootNode = NewSBRNode();
	for(i=0; i < img->maxIntensity + 1; i++)
	{
		imgGrays[i].codeword = -1;
		imgGrays[i].codewordSize = 0;
		imgGrays[i].value = i;
		if(imgGrays[i].usage)
		{
			AddGrayToSBRTNode(rootNode, imgGrays[i].value);
			nGrays++;
		}
	}
		
	/* Build the tree */
	BuidSBRT(rootNode, imgGrays, imgPlanes);
	//printf("LEVELS = %d\n", *levels);
	
	/* Create the new image */
	if(!(SBRImage = CreateImage(img->nRows, img->nCols, img->dataType, 1)))
		return NULL;
		
	/* Map the original bitplanes into the Scalable Bitplane Reduction representation */
	for(r = 0 ; r < SBRImage->nRows ; r++)
		for(c = 0 ; c < SBRImage->nCols ; c++)
			//PutGrayPixel(SBRImage, r, c, imgGrays[GetGrayPixel(img, r, c)].codeWord << (imgPlanes - imgGrays[GetGrayPixel(img, r, c)].codeWordSize));
			PutGrayPixel(SBRImage, r, c, imgGrays[GetGrayPixel(img, r, c)].codeword);
	return SBRImage;
}
/*************************************************************/
SBRNode *CreateSBRT(Gray *imgGrays, int nGrays, int imgPlanes)
{
	SBRNode *rootNode;
	int levels, i;
	
	rootNode = NewSBRNode();
	for(i=0; i < nGrays; i++)
	{
		imgGrays[i].codeword = -1;
		imgGrays[i].codewordSize = 0;
		imgGrays[i].value = i;
		if(imgGrays[i].usage)
		{
			AddGrayToSBRTNode(rootNode, imgGrays[i].value);
		}
	}
	
	/* Build the tree */
	levels = BuidSBRT(rootNode, imgGrays, imgPlanes);
	printf("N Levels = %d\n", levels);
	/* Return the rootnode */
	return rootNode;
	
}

/*************************************************************/

int GetGrayValueFromCodeword(Gray *imgGrays, int nGrays, int codeword)
{
	int i;
	for(i=0; i < nGrays; i++)
	{
		if( (imgGrays[i].codeword == codeword) && (imgGrays[i].usage == 1) )
		{
			return imgGrays[i].value;
		}
	}
	return -1;
	//fprintf(stderr, "Error(GetGrayValueFromCodeword): codeword '%d' not found!\n", codeword);
	//exit(EXIT_FAILURE);
}

/*************************************************************/

Gray *GetMappingGrayVector(Gray *imgGrays, int nGrays)
{
	int i, maxCodeWord = 0;
	Gray *codewordVector;
	
	for(i=0; i < nGrays; i++)
	{
		if(imgGrays[i].usage)
		{
			if(imgGrays[i].codeword > maxCodeWord)
				maxCodeWord = imgGrays[i].codeword;
		}
	}
	
	// Allocate memory for the codeword vector
	if(!(codewordVector = (Gray *)calloc(maxCodeWord + 1, sizeof(Gray))))
	{
		fprintf(stderr, "Error(GetMappingGrayVector): no memory!\n");
		exit(EXIT_FAILURE);
	}
	
	printf("The maximum codeword is %d\n", maxCodeWord);
	
	for(i=0; i < nGrays; i++)
	{
		codewordVector[imgGrays[i].codeword].usage = imgGrays[i].usage;
		
		// The codeword in this case is the gray value
		codewordVector[imgGrays[i].codeword].codeword = imgGrays[i].value;
		
		codewordVector[imgGrays[i].codeword].codewordSize = imgGrays[i].codewordSize;
	}
	return codewordVector;
}
