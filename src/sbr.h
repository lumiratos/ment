/*
 ***********************************************************************
 * Copyright 2011 Luis M. O. Matos (luismatos@ua.pt), All Rights
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

#include "image.h"
//#include "common.h"

#ifndef _SBR
#define _SBR

#include "common.h"

/*
typedef struct {
	int value;			// Gray value 
	int usage;			// 0: intensity not present; 1: intensity present 
	unsigned int count;		// Number of occurrences 
	int codeword;			// Codeword representation 
	int codewordSize;		// Codeword size 
} Gray;
*/

typedef struct sbrn {
	int codeword;		/* Codeword */
	int codewordSize;	/* Size of the codeword (bits) */
	int MinMax;		/* (MIN + MAX) / 2 */
	int *grays;		/* List of gray values represented by this node */
	int nGrays;		/* Number of different gray values in this node */
	struct sbrn *left;	/* Left ramification of this node */
	struct sbrn *right;	/* Right ramification of this node */
	int id;
} SBRNode;

SBRNode *NewSBRNode(void);
void AddGrayToSBRTNode(SBRNode *node, int gray);
void SplitSBRNode(SBRNode *node);
int BuidSBRT(SBRNode *node, Gray *imgGrays, int imgPlanes);
Image *GetSBRImage(Image *img, Gray *imgGrays);
SBRNode *CreateSBRT(Gray *imgGrays, int nGrays, int imgPlanes);
int GetGrayValueFromCodeword(Gray *imgGrays, int nGrays, int codeword);
Gray *GetMappingGrayVector(Gray *imgGrays, int nGrays);

#endif
