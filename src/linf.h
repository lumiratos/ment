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

#include "common.h"

#ifndef LINF_H_INCLUDED
#define LINF_H_INCLUDED

/*
typedef struct {
	int value;			// Gray value 
	int usage;			// 0: intensity not present; 1: intensity present 
} Gray;
*/

typedef struct btn {
	int gray;
	int error;
	Gray **grays;		/* List of gray values represented by this node */
	int nGrays;			/* Number of different gray values in this node */
	ImageCoords *pixels;/* List of pixel coordinates under this node */
	int nPixels;		/* Number of pixels in this node */
	struct btn *left;	/* Left ramification of this node */
	struct btn *right;	/* Right ramification of this node */
	int id;
} BTNode;

BTNode *NewBTNode(void);
void FindNodeToSplit(BTNode *node, BTNode **splitNode, int *maxError);
void FindNodeToSplitLowerGray(BTNode *node, BTNode **splitNode, int *gray);
void FindNodeToSplitHigherGrayCount(BTNode *node, BTNode **splitNode, int *nGrays);
void AddGrayToBTNode(BTNode *node, Gray *gray);
void AddPixelToBTNode(BTNode *node, int row, int col);
void ComputeNodeStatistics(BTNode *node);
void ComputeNodeStatistics2(BTNode *node);
void SplitNode(Image *codImg, BTNode *node, int id);
int FindGrayValue(BTNode *node, int grayValue);
void WriteRawBits(Image *fullRecImg, Image *recImg, Image *codImg,
  BTNode *node, FILE *fpOut, int bitPlaneShift);
void ReadRawBits(Image *recImg, BTNode *node, FILE *fpInp, int targetBytes);

#endif
