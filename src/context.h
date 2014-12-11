/*------------------------------------------------------------------------------
typedef unsigned short CCounter;

Copyright 2005-2006 Armando J. Pinho (ap@ua.pt), All Rights Reserved.

These programs are supplied free of charge for research purposes only,
and may not be sold or incorporated into any commercial product. There is
ABSOLUTELY NO WARRANTY of any sort, nor any undertaking that they are
fit for ANY PURPOSE WHATSOEVER. Use them at your own risk. If you do
happen to find a bug, or have modifications to suggest, please report
the same to Armando J. Pinho, ap@ua.pt. The copyright notice above
and this statement of conditions must remain an integral part of each
and every copy made of these files.

------------------------------------------------------------------------------*/

/*
 * Data structures for handling finite-context models.
 */

#ifndef CONTEXT_H_INCLUDED
#define CONTEXT_H_INCLUDED

typedef unsigned short CCounter;

#define MAX_ARRAY_MEMORY 1024 /* DNA: size 12 = 128 MB */

/* Hashing stuff */

#define max_entries  21      /* 5 (128), 21 (512), 85 (2k), 341 (8k) */

typedef struct node
	{
	struct node *next;           	/* pointer to the next node */
	unsigned int n_entries;        	/* number of used entries in this node */
	unsigned long long keys[max_entries];  /* the keys stored in this node */
	CCounter counters[max_entries][4];     /* their associated counters */
	}
Node;


//#define hash_size  104729           /* should be a prime number  */
//#define hash_size  804729           /* this one is NOT a prime number! */
#define hash_size  799999           /* this one is a prime number  */
//#define hash_size  1047           /* NOT a prime number...  */

typedef struct
	{
	Node *hash_table[hash_size]; /* the heads of the hash table linked lists */
	Node *free_nodes;            /* list of free nodes */
	unsigned int n_used_nodes;   /* optional */
	unsigned int n_used_keys;    /* optional */
	unsigned long long contextLen;
	}
HashTable;

/* End hashing stuff */

typedef struct
	{
	int			*freqs;
	int			sum;
	}
PModel;

typedef struct
	{
	double			*freqs;
	}
FloatPModel;


typedef struct
	{
	int			maxCtxSize;	/* Maximum depth of context template */
	int			ctxSize;	/* Current depth of context template */
	int			nSymbols;	/* Number of coding symbols */
	int			nCtxSymbols;/* Number of symbols used for context computation */
	unsigned long long	nPModels;	/* Number of probability models */
	unsigned long long	arrayMemory;/* Memory used using arrays... */
	int			deltaNum;	/* Numerator of delta */
	int			deltaDen;	/* Denominator of delta */
	int			maxCount;	/* Counters /= 2 if one counter >= maxCount */
	CCounter	*counters;
	HashTable	table;
	}
CModel;

PModel *CreatePModel(int nSymbols);
FloatPModel *CreateFloatPModel(int nSymbols);
void UpdateCModelCounter(CModel *cModel, unsigned long long pModelIdx,
  int symbol);
CModel *CreateCModel(int maxCtxSize, int nSymbols, int nCtxSymbols,
  int deltaNum, int deltaDen, int maxCount);
void ResetCModelCounters(CModel *cModel);
double FractionOfPModelsUsed(CModel *cModel);
double FractionOfPModelsUsedOnce(CModel *cModel);
void ComputePModel(CModel *cModel, PModel *pModel,
  unsigned long long pModelIdx);
double PModelSymbolNats(PModel *pModel, int symbol);
void HashingStats(CModel *cModel);

#endif /* CONTEXT_H_INCLUDED */

