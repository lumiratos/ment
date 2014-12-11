#ifndef CONTEXT_H
#define CONTEXT_H

#include "image.h"

int GetContextViip(Image *img, int plane, int nPlanes, int r, int c);

int GetContextIcip(Image *img, int plane, int nPlanes, int r, int c);

int GetContextJBIG(Image *img, int plane, int nPlanes, int r, int c);

int GetContext(Image *img, int plane, int nPlanes, int r, int c);

int GetContext2(Image *img, int plane, int nPlanes, int r, int c);

#endif
