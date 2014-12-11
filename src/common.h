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

#include "image.h"
#include "context.h"

#ifndef _COMMON
#define _COMMON

#define N_SYMBOLS			2
#define N_CTX_SYMBOLS			2

#define DEFAULT_PMODEL_DELTA_NUM	1
#define DEFAULT_PMODEL_DELTA_DEN	1
#define DEFAULT_PMODEL_MAX_COUNT	((1 << (sizeof(CCounter) * 8)) - 1)
/* 0: don't re-scale counters */

#define STORAGE_BITS_N_ROWS		16
#define STORAGE_BITS_N_COLS		16
#define STORAGE_BITS_N_IMG_PLANES	4
#define STORAGE_BITS_N_COD_PLANES	4
#define STORAGE_BITS_CONTEXT_SIZE       4
#define STORAGE_BITS_TEMPLATE_ID	3
#define STORAGE_BITS_PMODEL_DELTA_NUM	4
#define STORAGE_BITS_PMODEL_DELTA_DEN	4
#define STORAGE_BITS_PMODEL_MAX_COUNT	30
#define STORAGE_BITS_MAX_INTENSITY	16 
#define STORAGE_BITS_THRESHOLD		16
#define STORAGE_BITS_GAMMA              16

#define TEMPLATE_INTRA			0
#define TEMPLATE_INTER			1
#define TEMPLATE_KIKUCHI                2
#define TEMPLATE_BIN_TREE_A		3
#define TEMPLATE_BIN_TREE_B		4
#define TEMPLATE_BIN_TREE_C		5
#define TEMPLATE_BIN_TREE_D		6

#define TEMPLATE_MAX_SIZE		20

#define KIKUCHI_CONTEXT_BEST_MODE       0
#define KIKUCHI_CONTEXT_GREEDY_MODE     1

#define BIN_TREE_CONTEXT_GUESS_CHEN     0
#define BIN_TREE_CONTEXT_GUESS_FIT      1
#define BIN_TREE_CONTEXT_GUESS_PIXS     2

#define BIN_TREE_CONTEXT_MODE_GUESS     0
#define BIN_TREE_CONTEXT_MODE_GREEDY    1
#define BIN_TREE_CONTEXT_MODE_VAR       2
#define BIN_TREE_CONTEXT_MODE_BEST      3

#define CODE_MODE                       0
#define RAW_MODE                        1

#define GAMMA_K                         65536

#define MAX(a,b) ((a) > (b) ? (a) : (b))

#define KIKUCHI \
"[1] Hisakazu Kikuchi, Kunio Funahashi, and Shogo Muramatsu, \n\
\"Simple bit-plane coding for lossless image compression and extended functionalities\",\n\
Picture Coding Symposium, PCS 2009, pp. 1-4, 6-8 May 2009.\n\n\
[2] Hisakazu Kikuchi, Ryosuke Abe, and Shogo Muramatsu, \n\
\"Simple bitplane coding and its application to multi-functional image compression\",\n\
IEICE Transactions on Fundamentals of Electronics, Communications and Computer Sciences\n\
vol. E95.A (2012), no. 5, pp. 938-951, 2012. \n\n\
[3] Hisakazu Kikuchi, Taiyo Deguchi, and Masahiro Okuda, \n\
\"Lossless compression of LogLuv32 HDR images by simple bitplane coding\", \n\
Picture Coding Symposium, PCS 2013, pp. 265-268, 8-11 December 2013."



typedef struct
	{
	int value;
	int usage;
	unsigned int count;
	int codeword;
	int codewordSize;
	}
Gray;
typedef struct
	{
	int size;
	ImageCoords *position;
	}
CTemplate;

typedef struct
	{
	int imgPlanes;
	int maxTemplateSize;
	int *size;
	int **pattern;
	int totalSize;
	}
CPattern;

int GetPModelIdx(Image *img, int row, int col, int plane, int imgPlanes,
  CModel *cModel, CTemplate *cTemplateIntra, CTemplate *cTemplateInter,
  CPattern *cPattern);
int GetKikuchiPModelIdx(Image *tmpImg, int row, int col, CTemplate *kikuchiTemplate);
int GetBinTreePModelIdx(Image *recImg, int row, int col, CModel *cModel,
  CTemplate *cTemplate, int leftGray, int rightGray);


CTemplate *InitTemplate(int templateId);
void ShowTemplate(CTemplate *cTemplate);
CPattern *CreateCPattern(int imgPlanes, int maxTemplateSize);
void FreeCPattern(CPattern *cPattern);
void CopyCPattern(CPattern *cPatternDest, CPattern *cPatternOrig);

int FindGrayFromCodeWord(Gray *imgGrays, int nGrays, int codeword);
double Pow(double a, double b);

#endif

