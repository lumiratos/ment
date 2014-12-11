#include "context95.h"
#include <assert.h>

static ImageCoords ctxJbig[] = {{0, -1}, {-1, 0}, {-1, -1}, {-1, 1},
	{0, -2}, {-2, 0}, {-1, -2}, {-2, -1}, {-2, 1}, {-1, 2}, {0, -3}, {-3, 0}};

static ImageCoords ctx[] = {{0, -1}, {-1, 0}, {-1, -1}, {-1, 1}};

static ImageCoords ctxMsb[] = {{0, 0}, {0, -1}, {-1, 0}, {-1, -1}, {-1, 1},
	{0, -2}, {-2, 0}, {-1, -2}, {-2, -1}, {-2, 1}, {-1, 2}, {0, -3}, {-3, 0}};

static int msbSize[] = { 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6};
static int bp1Size[] = { 0, 0, 0, 0, 0, 0, 0, 5, 6, 6, 6, 6, 6, 6, 0, 0};
static int  bpSize[] = { 2, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 0};

int Connectivity(Image *img, int plane, int r1, int c1, int r2, int c2)
	{
		return 0;
	}

int GetContext2(Image *img, int plane, int nPlanes, int r, int c)
	
	{
	int n = 0, context = 0, p, pixel, i;

	/* MSBP == nPlanes - 1*/
	for(i = (plane == nPlanes - 1 ? 1 : 0) ; i < msbSize[plane]   ; i++)
		{
		pixel = GetGrayPixel(img, r + ctxMsb[i].row, c + ctxMsb[i].col);
		context += ((pixel >> (nPlanes -1)) & 0x01) << n++;
		}

	/* BPk+1 */
	if(plane < nPlanes - 2)
		{
		for(i = 0 ; i < bp1Size[plane] ; i++)
			{
			pixel = GetGrayPixel(img, r + ctxMsb[i].row, c + ctxMsb[i].col);
			context += ((pixel >> (plane + 1)) & 0x01) << n++;
			}

		}

	if(plane <= nPlanes - 4 )
		for(p = plane + (plane <= 3 ? 1 : 2) ; p <= nPlanes - 2 ; p++) 
			context += ((GetGrayPixel(img, r, c) >> p) & 0x01) << n++;

	/* Cintra */
	if(plane < nPlanes -1)
		{
		for(i = 0 ; i < bpSize[plane] ; i++)
			{
			pixel = GetGrayPixel(img, r + ctx[i].row, c + ctx[i].col);
			context += ((pixel >> plane) & 0x01) << n++;
			}

		}

	assert(n <= 22);

	return context;
	}

int GetContext(Image *img, int plane, int nPlanes, int r, int c)
	
	{
	int n = 0, context = 0, p, pixel, i;

	/* MSBP == nPlanes - 1*/
	for(i = (plane == nPlanes - 1 ? 1 : 0) ; i < 6   ; i++)
		{
		pixel = GetGrayPixel(img, r + ctxMsb[i].row, c + ctxMsb[i].col);
		context += ((pixel >> (nPlanes -1)) & 0x01) << n++;
		}

	/* BPk+1 */
	if(plane < nPlanes - 2 && plane >= 8)
		{
		for(i = 0 ; i < 6 ; i++)
			{
			pixel = GetGrayPixel(img, r + ctxMsb[i].row, c + ctxMsb[i].col);
			context += ((pixel >> (plane + 1)) & 0x01) << n++;
			}

		}

	if(plane <= nPlanes - 4 )
		for(p = plane + (plane <= 7 ? 1 : 2) ; p <= nPlanes - 2 ; p++) 
			context += ((GetGrayPixel(img, r, c) >> p) & 0x01) << n++;

	/* Cintra */
	if(plane < nPlanes - 1 && plane >= 8)
		{
		for(i = 0 ; i < 4 ; i++)
			{
			pixel = GetGrayPixel(img, r + ctx[i].row, c + ctx[i].col);
			context += ((pixel >> plane) & 0x01) << n++;
			}

		}

	assert(n <= 22);

	return context;
	}

int GetContextViip(Image *img, int plane, int nPlanes, int r, int c)
	
	{
	int n = 0, context = 0, p;

	/* MSBP == nPlanes - 1*/
	if((nPlanes - plane <= 8) && (plane < nPlanes - 1)) 
		{
		context += ((GetGrayPixel(img,r,c-1) >> (nPlanes-1)) & 0x01) << n++;
		context += ((GetGrayPixel(img,r-1,c-1) >> (nPlanes-1)) & 0x01) << n++;
		context += ((GetGrayPixel(img,r-1,c) >> (nPlanes-1)) & 0x01) << n++;
		context += ((GetGrayPixel(img,r-1,c+1) >>(nPlanes-1)) & 0x01) << n++;
		}

	/* BPk+1 */
	if((nPlanes - plane < 12) && (plane < nPlanes - 2))
		{
		context += ((GetGrayPixel(img,r,c-1) >> (plane+1)) & 0x01) << n++;
		context += ((GetGrayPixel(img,r-1,c-1) >> (plane+1)) & 0x01) << n++;
		context += ((GetGrayPixel(img,r-1,c) >> (plane+1)) & 0x01) << n++;
		context += ((GetGrayPixel(img,r-1,c+1) >> (plane+1)) & 0x01) << n++;
		}

	/* Cinter */
	for(p = plane + 1 ; p < nPlanes ; p++) 
		context += ((GetGrayPixel(img, r, c) >> p) & 0x01) << n++;

	/* Cintra */
	context += ((GetGrayPixel(img, r, c - 1) >> plane) & 0x01) << n++;
	context += ((GetGrayPixel(img, r - 1, c - 1) >> plane) & 0x01) << n++;
	context += ((GetGrayPixel(img, r - 1, c + 1) >> plane) & 0x01) << n++;
	context += ((GetGrayPixel(img, r - 1, c) >> plane) & 0x01);

	return context;
	}

int GetContextIcip(Image *img, int plane, int nPlanes, int r, int c)
	
	{
	int n = 0, context = 0, p;

	/* MSBP == nPlanes - 1*/
	if(plane < nPlanes - 1) 
		{
		context += ((GetGrayPixel(img,r,c+1) >> (nPlanes-1)) & 0x01) << n++;
		context += ((GetGrayPixel(img,r-1,c-1) >> (nPlanes-1)) & 0x01) << n++;
		context += ((GetGrayPixel(img,r-1,c) >> (nPlanes-1)) & 0x01) << n++;
		context += ((GetGrayPixel(img,r-1,c+1) >>(nPlanes-1)) & 0x01) << n++;
		if(plane < 8)
			context += ((GetGrayPixel(img,r,c-1) >> (nPlanes-1)) & 0x01) << n++;

		}

	/* BPk+1 */
	if(plane < nPlanes - 2 && plane >= nPlanes - 8 )
		{
		context += ((GetGrayPixel(img,r,c+1) >> (plane+1)) & 0x01) << n++;
		context += ((GetGrayPixel(img,r-1,c-1) >> (plane+1)) & 0x01) << n++;
		context += ((GetGrayPixel(img,r-1,c) >> (plane+1)) & 0x01) << n++;
		context += ((GetGrayPixel(img,r-1,c+1) >>(plane+1)) & 0x01) << n++;
		}

	if(plane >= nPlanes - 7 )
		for(p = plane + 1 ; p < nPlanes ; p++) 
			{
			context += ((GetGrayPixel(img, r, c) >> p) & 0x01) << n++;
			context += ((GetGrayPixel(img, r, c - 1) >> p) & 0x01) << n++;
			}

	else
		for(p = plane + 1 ; p < nPlanes ; p++) 
			context += ((GetGrayPixel(img, r, c) >> p) & 0x01) << n++;

	/* Cintra */
	if(plane >= nPlanes - 8)
		{
		context += ((GetGrayPixel(img, r, c - 1) >> plane) & 0x01) << n++;
		context += ((GetGrayPixel(img, r - 1, c - 1) >> plane) & 0x01) << n++;
		context += ((GetGrayPixel(img, r - 1, c + 1) >> plane) & 0x01) << n++;
		context += ((GetGrayPixel(img, r - 1, c) >> plane) & 0x01);
		}

	return context;
	}

int GetContextJBIG(Image *img, int plane, int nPlanes, int r, int c)
	
	{
	int n, pixel, context = 0;

	for(n = 0 ; n < 12 ; n++)
        {
		pixel = GetGrayPixel(img, r + ctxJbig[n].row, c + ctxJbig[n].col);
		context += ((pixel >> plane) & 0x01) << n;
        }

	return context;
	}

