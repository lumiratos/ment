/*------------------------------------------------------------------------------

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "context.h"

void InitHashTable(HashTable table)

	{ 
	int i;

	for(i = 0 ; i < hash_size ; i++)
		table.hash_table[i] = NULL;

	table.free_nodes = NULL;
	table.n_used_nodes = 0;
	table.n_used_keys = 0;
	}

Node* NewNode(HashTable *table)

	{
	Node *n;

	n = (Node*)malloc(sizeof(Node));
	table->n_used_nodes++;

	return n;
	}


CCounter *FindCounters(HashTable *table, unsigned long long key)

	{
	Node *n;
	int i,j;

	i = (unsigned int)(key % (unsigned long long)hash_size); // the hash code

	for(n = table->hash_table[i] ; n != NULL ; n = n->next)
		for(j = 0 ; j < n->n_entries ; j++)
			if(n->keys[j] == key)
			return &n->counters[j][0];

	// key not found, insert it
	n = table->hash_table[i];
	if(n == NULL || n->n_entries == max_entries)
		{
		n = NewNode(table);
		n->next = table->hash_table[i];
		n->n_entries = 0;
		table->hash_table[i] = n;
		}

	j = n->n_entries++;
	n->keys[j] = key;
	n->counters[j][0] = 0;
	n->counters[j][1] = 0;
	n->counters[j][2] = 0;
	n->counters[j][3] = 0;
	table->n_used_keys++;

	return &n->counters[j][0];
	}

/*----------------------------------------------------------------------------*/

PModel *CreatePModel(int nSymbols)
	{
	PModel *pModel;

	if(!(pModel = (PModel *)malloc(sizeof(PModel))))
		{
		fprintf(stderr, "Error: in memory allocation\n");
		exit(1);
		}

	if(!(pModel->freqs = (int *)malloc(nSymbols * sizeof(int))))
		{
		fprintf(stderr, "Error: in memory allocation\n");
		exit(1);
		}

	return pModel;
	}

/*----------------------------------------------------------------------------*/

FloatPModel *CreateFloatPModel(int nSymbols)
	{
	FloatPModel *floatPModel;

	if(!(floatPModel = (FloatPModel *)malloc(sizeof(FloatPModel))))
		{
		fprintf(stderr, "Error: in memory allocation\n");
		exit(1);
		}

	if(!(floatPModel->freqs = (double *)malloc(nSymbols * sizeof(double))))
		{
		fprintf(stderr, "Error: in memory allocation\n");
		exit(1);
		}

	return floatPModel;
	}


/*----------------------------------------------------------------------------*/

void UpdateCModelCounter(CModel *cModel, unsigned long long pModelIdx,
int symbol)

	{
	int n;
	CCounter *counters;

	if(cModel->arrayMemory > MAX_ARRAY_MEMORY)
		counters = FindCounters(&(cModel->table), pModelIdx);
	else
		counters = &(cModel->counters[pModelIdx * cModel->nSymbols]);

	counters[symbol]++;

	if(cModel->maxCount != 0 && counters[symbol] == cModel->maxCount)
		for(n = 0 ; n < cModel->nSymbols ; n++)
			counters[n] >>= 1;

	}

/*----------------------------------------------------------------------------*/

CModel *CreateCModel(int maxCtxSize, int nSymbols, int nCtxSymbols,
  int deltaNum, int deltaDen, int maxCount)

	{
	CModel *cModel;

	if(!(cModel = (CModel *)calloc(1, sizeof(CModel))))
		{
		fprintf(stderr, "Error: in memory allocation\n");
		exit(1);
		}

	if(maxCtxSize > 30)
		{
		fprintf(stderr, "Error: context size greater than 30 is not allowed\n");
		exit(1);
		}
	
	cModel->nPModels = (unsigned long long)pow(nCtxSymbols, maxCtxSize);
	cModel->maxCtxSize = maxCtxSize;
	cModel->ctxSize = maxCtxSize;
	cModel->nSymbols = nSymbols;
	cModel->nCtxSymbols = nCtxSymbols;
	cModel->deltaNum = deltaNum;
	cModel->deltaDen = deltaDen;
	cModel->maxCount = maxCount;
	cModel->arrayMemory = cModel->nPModels * nSymbols *
	  sizeof(CCounter) / 1024 / 1024 ;

	if(cModel->arrayMemory > MAX_ARRAY_MEMORY)
		{
		InitHashTable(cModel->table);
		}

	else
		{
		if(!(cModel->counters = (CCounter *)calloc(cModel->nPModels *
		  nSymbols, sizeof(CCounter))))
			{
			fprintf(stderr, "Error: in memory allocation\n");
			exit(1);
			}

		}

	return cModel;
	}

/*----------------------------------------------------------------------------*/

double FractionOfPModelsUsed(CModel *cModel)

	{
	int pModel, symbol, sum, counter = 0;
	CCounter *counters;


	for(pModel = 0 ; pModel < cModel->nPModels ; pModel++)
		{
		if(cModel->arrayMemory > MAX_ARRAY_MEMORY)
			counters = FindCounters(&(cModel->table), pModel);
		else
			counters = &(cModel->counters[pModel * cModel->nSymbols]);

		sum = 0;
		for(symbol = 0 ; symbol < cModel->nSymbols ; symbol++)
			sum += counters[symbol];

		if(sum != 0)
			counter++;

		}

	return (double)counter / cModel->nPModels;
	}

/*----------------------------------------------------------------------------*/

double FractionOfPModelsUsedOnce(CModel *cModel)

	{
	unsigned long long pModelIdx;
	int symbol, sum, counter = 0;
	CCounter *counters;

	for(pModelIdx = 0 ; pModelIdx < cModel->nPModels ; pModelIdx++)
		{
		if(cModel->arrayMemory > MAX_ARRAY_MEMORY)
			counters = FindCounters(&(cModel->table), pModelIdx);
		else
			counters = &(cModel->counters[pModelIdx * cModel->nSymbols]);

		sum = 0;
		for(symbol = 0 ; symbol < cModel->nSymbols ; symbol++)
			sum += counters[symbol];

		if(sum == 1)
			counter++;

		}

	return (double)counter / cModel->nPModels;
	}

/*----------------------------------------------------------------------------*/

void ResetCModelCounters(CModel *cModel)

	{
	unsigned long long pModelIdx;
	CCounter *counters;

	
	for(pModelIdx = 0 ; pModelIdx < cModel->nPModels ; pModelIdx++)
		{
		if(cModel->arrayMemory > MAX_ARRAY_MEMORY)
			counters = FindCounters(&(cModel->table), pModelIdx);
		else
			counters = &(cModel->counters[pModelIdx * cModel->nSymbols]);
		memset(counters, 0, cModel->nSymbols * sizeof(CCounter));
		}
	}

/*----------------------------------------------------------------------------*/

void ComputePModel(CModel *cModel, PModel *pModel, unsigned long long pModelIdx)

	{
	int symbol;
	CCounter *counters;

	if(cModel->arrayMemory > MAX_ARRAY_MEMORY)
		counters = FindCounters(&(cModel->table), pModelIdx);
	else
		counters = &(cModel->counters[pModelIdx * cModel->nSymbols]);

	//printf("pModelIdx = %llu\n", pModelIdx);
	pModel->sum = 0;
	for(symbol = 0 ; symbol < cModel->nSymbols ; symbol++)
		{
		pModel->freqs[symbol] = cModel->deltaNum + cModel->deltaDen *
		  counters[symbol];
		pModel->sum += pModel->freqs[symbol];
		}

	}

/*----------------------------------------------------------------------------*/

double PModelSymbolNats(PModel *pModel, int symbol)

	{
	return log((double)pModel->sum / pModel->freqs[symbol]);
	}

void HashingStats(CModel *cModel)

	{
	unsigned int i, k, j, n_min, n_max,empty_entries = 0, ones = 0, sum = 0;
	unsigned int zeros = 0, moreThanOnce = 0;
	unsigned long long possible_keys;
	double n1,n2;
	long double p;
	Node *n;

	n_min = cModel->table.n_used_keys;
	n_max = 0;
	n1 = n2 = 0.0;
	for(i = 0 ; i < hash_size ; i++)
		{
		if(cModel->table.hash_table[i] == NULL)
			empty_entries++;

		j = 0;
		for(n = cModel->table.hash_table[i] ; n != NULL ; n = n->next)
			{
			for(j = 0 ; j < n->n_entries ; j++)
				{
				sum = 0;
				for(k = 0 ; k < 4 ; k++)
					sum += n->counters[j][k];

					if(sum == 1)
						ones++;

					if(sum == 0)
						zeros++;

					if(sum > 1)
						moreThanOnce++;
				}

			j += n->n_entries;
			}

		if(j < n_min)
		  n_min = j;

		if(j > n_max)
		  n_max = j;

		n1 += (double)j;
		n2 += (double)j * (double)j;
		}

	n1 /= (double)hash_size;
	n2 /= (double)hash_size;
	possible_keys = powl((double)cModel->nSymbols, (double)cModel->maxCtxSize);
	p = 100.0 * (double)cModel->table.n_used_keys / possible_keys;

	printf("hash_size .... %u\n",hash_size);
	printf("keys ......... %u [%.0Lf %% of %lld]\n",
	  cModel->table.n_used_keys, p, possible_keys);
	printf("used once .... %u\n", ones);
	printf("never used ... %u\n", zeros);
	printf("used several . %u\n", moreThanOnce);
	printf("nodes ........ %u\n",cModel->table.n_used_nodes);
	// i - minimum number of nodes needed to store all keys
	//i = (cModel->table.n_used_keys + max_entries - 1) / max_entries;
	//printf("efficiency ... %.2f%%\n",
	  //100.0 * (double)cModel->table.n_used_nodes / (double)i);
	printf("empty ........ %.2f%%\n",100.0 * (double)empty_entries /
	  (double)hash_size);
	printf("min .......... %u\n",n_min);
	printf("max .......... %u\n",n_max);
	printf("avg .......... %.2f\n",n1);
	// not perfect, because there is one less degree of freedom...
	printf("std .......... %.2f\n",sqrt(n2 - n1 * n1));
	printf("=====================\n");
	}
