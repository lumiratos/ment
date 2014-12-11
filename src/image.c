/*------------------------------------------------------------------------------

Copyright 2000-2004 Armando J. Pinho (ap@det.ua.pt), All Rights Reserved.

These programs are supplied free of charge for research purposes only,
and may not be sold or incorporated into any commercial product. There is
ABSOLUTELY NO WARRANTY of any sort, nor any undertaking that they are
fit for ANY PURPOSE WHATSOEVER. Use them at your own risk. If you do
happen to find a bug, or have modifications to suggest, please report
the same to Armando J. Pinho, ap@det.ua.pt. The copyright notice above
and this statement of conditions must remain an integral part of each
and every copy made of these files.

Modified:
Thu Nov 25 23:04:08 WET 2004
Fri Oct  8 13:41:00 WEST 2004
Tue Mar 30 14:42:19 WEST 2004
Tue Nov  4 14:08:59 WET 2003

------------------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <string.h>

#include "image.h"
#include "matrix.h"

#define X11_DISPLAY_CMD "xv -"

#define PNM_BUFFER_SIZE	128

#define PBM_ASCII		1
#define PBM_RAW			2
#define PGM_ASCII		3
#define PGM_RAW			4
#define PPM_ASCII		5
#define PPM_RAW			6

/*============================================================================*/

int ReadGrayPixel(int pixDataType, char inpDataType, FILE *fp)

	{
	int pixel;

	if(inpDataType == 'a') /* ASCII */
		if(fscanf(fp, "%d", &pixel) != 1)
			{
			fprintf(stderr, "ReadGrayPixel: while reading file\n");
			return EOF;
			}

		else
			return pixel;

	else /* BINARY */
		if(pixDataType == DATA_TYPE_C)
			return getc(fp);

		else
			{
			int c;

			if((c = getc(fp)) == EOF)
				{
				fprintf(stderr, "ReadGrayPixel: while reading file\n");
				return EOF;
				}

			pixel = c << 8;
			if((c = getc(fp)) == EOF)
				{
				fprintf(stderr, "ReadGrayPixel: while reading file\n");
				return EOF;
				}

			return pixel + c;
			}

	}

/*============================================================================*/

void WriteGrayPixel(int pixel, int pixDataType, char outDataType, FILE *fp)

	{
	if(outDataType == 'a') /* ASCII */
		fprintf(fp, "%d\n", pixel);

	else /* BINARY */
		if(pixDataType == DATA_TYPE_C)
			putc(pixel, fp);

		else
			{
			putc(pixel >> 8, fp);
			putc(pixel & 0x00FF, fp);
			}

	}

/*============================================================================*/

int GetColor(Image *img, int row, int col, int component)

	{
	if(component >= img->nPlanes)
		{
		fprintf(stderr, "Error (GetColor): invalid color plane.\n");
		return(-1);
		}

	if(img->dataType == DATA_TYPE_C)
		return((int)img->data[component].C[row][col]);

	else
		return((int)img->data[component].S[row][col]);

	}

/*============================================================================*/

int CmpRGBColors(RGBColor *rgb1, RGBColor *rgb2)

	{
	return rgb1->plane[0] == rgb2->plane[0] &&
		   rgb1->plane[1] == rgb2->plane[1] &&
		   rgb1->plane[2] == rgb2->plane[2];
	}

/*============================================================================*/

RGBColor GetRGBPixel(Image *img, int row, int col)

	{
	RGBColor color;

	if(img->dataType != DATA_TYPE_C || img->nPlanes != 3)
		{
		fprintf(stderr, "Error (GetRGBPixel): wrong type of image\n");
		exit(1);
		}

	if(row >= 0 && row < img->nRows && col >= 0 && col < img->nCols)
		{
		color.plane[0] = img->data[0].C[row][col];
		color.plane[1] = img->data[1].C[row][col];
		color.plane[2] = img->data[2].C[row][col];
		}

	else
		{
		color.plane[0] = 0;
		color.plane[1] = 0;
		color.plane[2] = 0;
		}

	return color;
	}

/*============================================================================*/

void PutRGBPixel(Image *img, int row, int col, RGBColor *color)

	{
	if(img->dataType != DATA_TYPE_C || img->nPlanes != 3)
		{
		fprintf(stderr, "Error (PutRGBPixel): wrong type of image\n");
		exit(1);
		}

	if(row >= 0 && row < img->nRows && col >= 0 && col < img->nCols)
		{
		img->data[0].C[row][col] = (uChar)color->plane[0];
		img->data[1].C[row][col] = (uChar)color->plane[1];
		img->data[2].C[row][col] = (uChar)color->plane[2];
		}

	}

/*============================================================================*/

int GetGrayPixel(Image *img, int row, int col)

	{
	if(img->dataType == DATA_TYPE_C)
		if(row >= 0 && row < img->nRows && col >= 0 && col < img->nCols)
			return img->data[0].C[row][col];

		else
			return 0;

	else
		if(row >= 0 && row < img->nRows && col >= 0 && col < img->nCols)
			return img->data[0].S[row][col];

		else
			return 0;

	}

/*============================================================================*/

void PutGrayPixel(Image *img, int row, int col, int pixel)

	{
	if(img->dataType == DATA_TYPE_C)
		{
		if(row >= 0 && row < img->nRows && col >= 0 && col < img->nCols)
			img->data[0].C[row][col] = (uChar)pixel;

		}

	else
		{
		if(row >= 0 && row < img->nRows && col >= 0 && col < img->nCols)
			img->data[0].S[row][col] = (uShort)pixel;

		}

	}

/*==============================================================================
	Creates an object of type "Image".
------------------------------------------------------------------------------*/

Image *CreateImage(int nRows, int nCols, int dataType, int nPlanes)

	{
	Image *img;
	int n, row;

	if(nRows <= 0 || nCols <= 0 || (nPlanes != 1 && nPlanes != 3) ||
	  (dataType != DATA_TYPE_C && dataType != DATA_TYPE_S))
		{
		fprintf(stderr, "Error (CreateImage): wrong parameters.\n");
		return(NULL);
		}

	if(!(img = (Image *)calloc(1, sizeof(Image))))
		{
		fprintf(stderr, "Error (CreateImage): unable to allocate memory.\n");
		return(NULL);
		}

	img->nRows = nRows;
	img->nCols = nCols;
	img->nPlanes = nPlanes;
	img->dataType = dataType;
	img->maxIntensity = 0;

	switch(dataType)
		{
		case DATA_TYPE_C:
			for(n = 0 ; n < nPlanes ; n++)
				{
				if(!(img->data[n].C = (uChar **)calloc(nRows, sizeof(uChar *))))
					{
					fprintf(stderr,
					  "Error (CreateImage): unable to allocate memory.\n");
					return(NULL);
					}

				for(row = 0 ; row < nRows ; row++)
					if(!(img->data[n].C[row] = (uChar *)calloc(nCols,
					  sizeof(uChar))))
						{
						fprintf(stderr,
						  "Error (CreateImage): unable to allocate memory.\n");
						return(NULL);
						}

				}

			break;

		case DATA_TYPE_S:
			for(n = 0 ; n < nPlanes ; n++)
				{
				if(!(img->data[n].S = (uShort **)calloc(nRows,
				  sizeof(uShort *))))
					{
					fprintf(stderr,
					  "Error (CreateImage): unable to allocate memory.\n");
					return(NULL);
					}

				for(row = 0 ; row < nRows ; row++)
					if(!(img->data[n].S[row] = (uShort *)calloc(nCols,
					  sizeof(uShort))))
						{
						fprintf(stderr,
						  "Error (CreateImage): unable to allocate memory.\n");
						return(NULL);
						}

				}

			break;

		}

	return(img);
	}

/*==============================================================================
	Destroys an object of type "Image".
------------------------------------------------------------------------------*/

void FreeImage(Image *img)

	{
	int row, n;

	switch(img->dataType)
		{
		case DATA_TYPE_C:
			for(n = 0 ; n < img->nPlanes ; n++)
				{
				for(row = 0 ; row < img->nRows ; row++)
					free((void *)img->data[n].C[row]);

				free((void *)img->data[n].C);
				}

			break;

		case DATA_TYPE_S:
			for(n = 0 ; n < img->nPlanes ; n++)
				{
				for(row = 0 ; row < img->nRows ; row++)
					free((void *)img->data[n].S[row]);

				free((void *)img->data[n].S);
				}

			break;

		}

	free((void *)img);
	}

/*==============================================================================
	Reads the PNM (PBM, PGM or PPM) header from the beginning of a file and
	returns information from the header. The return value is 0 if a PBM, PGM
	or PPM format is not recognized.
------------------------------------------------------------------------------*/

int ReadPNMImageFileHeader(FILE *fp, int *nRows, int *nCols, int *maxIntensity)

	{
	char buffer[PNM_BUFFER_SIZE];
	int i, numFields, c;
	int type = 0;

	/*
	 * Read 3 space-separated fields from the file, ignoring
	 * comments (any line that starts with "#").
	 */
	c = getc(fp);
	i = 0;
	for (numFields = 0 ; numFields < 3 ; numFields++)
		{
		/* Skip comments and white space */
		while(1)
			{
			while(isspace(c))
				c = getc(fp);

			if(c != '#')
				break;

			do
				{
				c = getc(fp);
				}
			while((c != EOF) && (c != '\n'));
			}

		/* Read a field (everything up to the next white space) */
		while((c != EOF) && !isspace(c))
			{
			if(i < (PNM_BUFFER_SIZE - 2))
				buffer[i++]  = c;

			c = getc(fp);
			}

		if(i < (PNM_BUFFER_SIZE - 1))
			buffer[i++] = ' ';

		}

	buffer[i] = 0;

	/* Parse the "id" field */
	if(strncmp(buffer, "P1 ", 3) == 0)
		type = PBM_ASCII;

	if(strncmp(buffer, "P4 ", 3) == 0)
		type = PBM_RAW;

	if(strncmp(buffer, "P2 ", 3) == 0)
		type = PGM_ASCII;

	if(strncmp(buffer, "P5 ", 3) == 0)
		type = PGM_RAW;

	if(strncmp(buffer, "P3 ", 3) == 0)
		type = PPM_ASCII;

	if(strncmp(buffer, "P6 ", 3) == 0)
		type = PPM_RAW;

	if(type == 0)
		return 0;

	/* Parse the "nCols" and "nRows" fields */
	if(sscanf(buffer+3, "%d %d", nCols, nRows) != 2)
		return 0;

	/* Parse the "maxIntensity" field */
	if(type != PBM_ASCII && type != PBM_RAW)
		{
		c = getc(fp);
		i = 0;

		/* Skip comments and white space */
		while(1)
			{
			while(isspace(c))
				c = getc(fp);

			if(c != '#')
				break;

			do
				{
				c = getc(fp);
				}
			while((c != EOF) && (c != '\n'));
			}

		/* Read a field (everything up to the next white space) */
		while((c != EOF) && !isspace(c))
			{
			if(i < (PNM_BUFFER_SIZE - 2))
				buffer[i++]  = c;

			c = getc(fp);
			}

		if(i < (PNM_BUFFER_SIZE - 1))
			buffer[i++] = ' ';

		buffer[i] = 0;

		if(sscanf(buffer, "%d", maxIntensity) != 1)
			return 0;
		}

	else
		*maxIntensity = 1;

	return type;
	}

/*==============================================================================
	Opens a file for reading or writing, depending on "mode" (r or w).
	If the file is or will be compressed via gzip, then an appropriate
	pipe is opened. If the file is opened for writing and the first
	character of "fileName" is "!" then do not ask for overwriting
	permission.
------------------------------------------------------------------------------*/

FILE *OpenImageFile(char *fileName, char *mode, char *fpType)

	{
	FILE *fp;
	char *fName, fNameBuf[256], str[256], overwrite;

	switch(mode[0])
		{
		case 'r':
			if((strlen(fileName) > 3 && !strcmp(fileName +
			  strlen(fileName) - 3, ".gz")))
				{
				sprintf(str, "zcat -f %s", fileName);
				*fpType = 'p';
				return(popen(str, "r"));
				}

			*fpType = 'f';
			return(fopen(fileName, "r"));
			break;

		case 'w':
			strcpy(fNameBuf, fileName);
			while(1)
				{
				if(fNameBuf[0] == '!')
					{
					fName = fNameBuf + 1;
					overwrite = 'y';
					}

				else
					{
					fName = fNameBuf;
					overwrite = 'n';
					}

				if(overwrite == 'n' && (fp = fopen(fName, "r")) != NULL)
					{
					fclose(fp);
					fprintf(stderr,
					  "Warning (OpenImageFile): file %s exists.\n", fName);
					fprintf(stderr,
					  "Type \"y\" to overwrite or an alternative file name: ");
					/* scanf("%s", str); */
					if(scanf("%s", str) != 1)
						{
						fprintf(stderr,"Error obtaining the file name or overwrite option!\n");
						exit(EXIT_FAILURE);
						}
					if(!strcmp("y", str))
						break;

					else
						strcpy(fNameBuf, str);

					}

				else
					break;

				}

			if((strlen(fName) > 3 && !strcmp(fName + strlen(fName) - 3, ".gz")))
				{
				sprintf(str, "gzip - > %s", fName);
				*fpType = 'p';

				fp = popen(str, "w");
				if (fp == NULL)				
					strerror(errno);
				return fp;
				/* return(popen(str, "w")); */
				}

			if(!strcmp(fName, "x11"))
				{
				sprintf(str, "%s", X11_DISPLAY_CMD);
				*fpType = 'p';
				return(popen(str, "w"));
				}

			*fpType = 'f';

			if ((fp=fopen(fName, "w") )== NULL )
			{
				strerror(errno);
			}	
			return fp;
			/* return(fopen(fName, "w")); */
			break;

		}
	
	return(NULL);
	}

/*==============================================================================
	Closes an image file.
------------------------------------------------------------------------------*/

void CloseImageFile(FILE *fp, char fpType)

	{

	switch(fpType)
		{
		case 'p':
			pclose(fp);
			break;

		case 'f':
			fclose(fp);
			break;

		}

	}

/*==============================================================================
	Loads an image from a file.
------------------------------------------------------------------------------*/

Image *ReadImageFile(char *fileName)

	{
	FILE *fp;
	Image *img;
	int imgType, nRows, nCols, maxIntensity, row, col, bytesPerRow,
	  grayLevel, c;
	uChar tmpBuf[3], mask = 0;
	char fpType, *buf;

	if(!(fp = OpenImageFile(fileName, "r", &fpType)))
		{
		fprintf(stderr, "Error (ReadImageFile): unable to open file %s.\n",
		  fileName);
		return(NULL);
		}

	if(!(imgType = ReadPNMImageFileHeader(fp, &nRows, &nCols, &maxIntensity)))
		{
		fprintf(stderr,
		  "Error (ReadImageFile): unsupported image format of file %s.\n",
		  fileName);
		CloseImageFile(fp, fpType);
		return(NULL);
		}

	if(nRows <= 0 || nCols <= 0 || maxIntensity < 0 || maxIntensity >= 65536)
		{
		fprintf(stderr, "Error (ReadImageFile): wrong parameters.\n");
		CloseImageFile(fp, fpType);
		return(NULL);
		}

	switch(imgType)
		{
		/* Bi-level, ascii format */
		case PBM_ASCII:
			fprintf(stderr,
			  "Error (ReadImageFile): PBM ascii not (yet) implemented.\n");
			CloseImageFile(fp, fpType);
			return(NULL);
			break;

		/* Greylevel, ascii format */
		case PGM_ASCII:
			if(maxIntensity < 256)
				{
				if(!(img = CreateImage(nRows, nCols, DATA_TYPE_C, 1)))
					{
					CloseImageFile(fp, fpType);
					return(NULL);
					}

				for(row = 0 ; row < nRows ; row++)
					for(col = 0 ; col < nCols ; col++)
						{
						if(fscanf(fp, "%d", &grayLevel) != 1)
							{
							fprintf(stderr, "Error (ReadImageFile): ");
							fprintf(stderr, "end of file unexpected.\n");
							CloseImageFile(fp, fpType);
							return(NULL);
							}

						img->data[0].C[row][col] = (uChar)grayLevel;
						}

				}

			else
				{
				if(!(img = CreateImage(nRows, nCols, DATA_TYPE_S, 1)))
					{
					CloseImageFile(fp, fpType);
					return(NULL);
					}

				for(row = 0 ; row < nRows ; row++)
					for(col = 0 ; col < nCols ; col++)
						{
						if(fscanf(fp, "%d", &grayLevel) != 1)
							{
							fprintf(stderr, "Error (ReadImageFile): ");
							fprintf(stderr, "end of file unexpected.\n");
							CloseImageFile(fp, fpType);
							return(NULL);
							}

						img->data[0].S[row][col] = (uShort)grayLevel;
						}

				}

			break;

		/* Color, ascii format */
		case PPM_ASCII:
			fprintf(stderr,
			  "Error (ReadImageFile): PPM ascii not (yet) implemented.\n");
			CloseImageFile(fp, fpType);
			return(NULL);
			break;

		/* Bi-level, raw format */
		case PBM_RAW:
			if(!(img = CreateImage(nRows, nCols, DATA_TYPE_C, 1)))
				{
				CloseImageFile(fp, fpType);
				return(NULL);
				}

			bytesPerRow = (nCols + 7) / 8;
			if(!(buf = (char *)malloc(bytesPerRow)))
				{
				fprintf(stderr,
				  "Error (ReadImageFile): cannot allocate memory.\n");
				CloseImageFile(fp, fpType);
				return(NULL);
				}

			for(row = 0 ; row < nRows ; row++)
				{
				if(fread(buf, sizeof(uChar), bytesPerRow, fp) != bytesPerRow)
					{
					fprintf(stderr,
					  "Error (ReadImageFile): end of file unexpected.\n");
					CloseImageFile(fp, fpType);
					return(NULL);
					}

				for(col = 0 ; col < nCols ; col++)
					{
					if(col % 8 == 0)
						mask = 0x80;

					else
						mask >>= 1;

					img->data[0].C[row][col] = buf[col / 8] & mask ? 0 : 1;
					}

				}

			break;

		/* Greylevel, raw format */
		case PGM_RAW:
			if(maxIntensity < 256)
				{
				if(!(img = CreateImage(nRows, nCols, DATA_TYPE_C, 1)))
					{
					CloseImageFile(fp, fpType);
					return(NULL);
					}

				for(row = 0 ; row < nRows ; row++)
					if(fread(img->data[0].C[row], sizeof(uChar),
					  nCols, fp) != nCols)
						{
						fprintf(stderr,
						  "Error (ReadImageFile): end of file unexpected.\n");
						CloseImageFile(fp, fpType);
						return(NULL);
						}

				}

			else
				{
				if(!(img = CreateImage(nRows, nCols, DATA_TYPE_S, 1)))
					{
					CloseImageFile(fp, fpType);
					return(NULL);
					}

				for(row = 0 ; row < nRows ; row++)
					for(col = 0 ; col < nCols ; col++)
						{
						if((c = getc(fp)) == EOF)
							{
							fprintf(stderr, "Error (ReadImageFile): end "
											"of file unexpected.\n");
							CloseImageFile(fp, fpType);
							return(NULL);
							}

						img->data[0].S[row][col] = c << 8;

						if((c = getc(fp)) == EOF)
							{
							fprintf(stderr, "Error (ReadImageFile): end "
											"of file unexpected.\n");
							CloseImageFile(fp, fpType);
							return(NULL);
							}

						img->data[0].S[row][col] += c;
						}

				}

			break;

		/* Color, raw format */
		case PPM_RAW:
			if(maxIntensity >= 256)
				{
				fprintf(stderr, "Error (ReadImageFile): ");
				fprintf(stderr, "PPM image with more than 8 bits per color.\n");
				CloseImageFile(fp, fpType);
				return(NULL);
				}

			if(!(img = CreateImage(nRows, nCols, DATA_TYPE_C, 3)))
				{
				CloseImageFile(fp, fpType);
				return(NULL);
				}

			for(row = 0 ; row < nRows ; row++)
				for(col = 0 ; col < nCols ; col++)
					{
					if(fread(tmpBuf, sizeof(uChar), 3, fp) != 3)
						{
						fprintf(stderr,
						  "Error (ReadImageFile): end of file unexpected.\n");
						CloseImageFile(fp, fpType);
						return(NULL);
						}

					img->data[0].C[row][col] = tmpBuf[0];
					img->data[1].C[row][col] = tmpBuf[1];
					img->data[2].C[row][col] = tmpBuf[2];
					}

			break;

		default:
			return(NULL);

		}

	CloseImageFile(fp, fpType);
	img->maxIntensity = maxIntensity;
	return(img);
	}

/*==============================================================================
	Returns the (real) maximum intensity value of the image, if variable
	img->maxIntensity is zero. If not, returns the value of that variable.
------------------------------------------------------------------------------*/

int ImageMaxIntensity(Image *img)

	{
	int row, col, imgPlane, maxIntensity = 0;

	if(img->maxIntensity != 0)
		return(img->maxIntensity);

	switch(img->dataType)
		{
		case DATA_TYPE_C:
			for(imgPlane = 0 ; imgPlane < img->nPlanes ; imgPlane++)
				for(row = 0 ; row < img->nRows ; row++)
					for(col = 0 ; col < img->nCols ; col++)
						if(img->data[imgPlane].C[row][col] > maxIntensity)
							maxIntensity = img->data[imgPlane].C[row][col];

			break;

		case DATA_TYPE_S:
			for(imgPlane = 0 ; imgPlane < img->nPlanes ; imgPlane++)
				for(row = 0 ; row < img->nRows ; row++)
					for(col = 0 ; col < img->nCols ; col++)
						if(img->data[imgPlane].S[row][col] > maxIntensity)
							maxIntensity = img->data[imgPlane].S[row][col];

			break;

		}

	return(maxIntensity);
	}

/*==============================================================================
	Writes an image to a file.
------------------------------------------------------------------------------*/

Image *WriteImageFile(char *fileName, Image *img)

	{
	FILE *fp;
	int row, col, imgMaxIntensity;
	uChar tmpBuf[3];
	char fpType;

	if(!(fp = OpenImageFile(fileName, "w", &fpType)))
		{
		fprintf(stderr, "Error (WriteImageFile): unable to open file %s.\n",
		  fileName);
		return(NULL);
		}

	imgMaxIntensity = ImageMaxIntensity(img);
	switch(img->nPlanes)
		{
		/* Greylevel raw */
		case 1:
			fprintf(fp, "P5\n%d %d\n%d\n", img->nCols, img->nRows,
			  imgMaxIntensity);

			if(img->dataType == DATA_TYPE_C)
				{
				for(row = 0 ; row < img->nRows ; row++)
					if(fwrite(img->data[0].C[row], sizeof(uChar),
					  img->nCols, fp) != img->nCols)
						{
						fprintf(stderr, "Error (WriteImageFile): ");
						fprintf(stderr, "writing to file.\n");
						CloseImageFile(fp, fpType);
						return(NULL);
						}

				}

			else
				{
				/*
				 * Most significant byte first (BIG ENDIAN)
				 */
				for(row = 0 ; row < img->nRows ; row++)
					for(col = 0 ; col < img->nCols ; col++)
						{
						if(imgMaxIntensity > 255)
							{
							if(putc(img->data[0].S[row][col] >> 8, fp) == EOF)
								{
								fprintf(stderr, "Error (WriteImageFile): "
												"writing to file.\n");
								CloseImageFile(fp, fpType);
								return(NULL);
								}

							}

						if(putc(img->data[0].S[row][col] & 0x00FF, fp) == EOF)
							{
							fprintf(stderr, "Error (WriteImageFile): "
											"writing to file.\n");
							CloseImageFile(fp, fpType);
							return(NULL);
							}

						}

				}

			break;

		/* Color raw */
		case 3:
			if(ImageMaxIntensity(img) >= 256)
				{
				fprintf(stderr, "Error (WriteImageFile): ");
				fprintf(stderr, "PPM image with more than 8 bits per color.\n");
				CloseImageFile(fp, fpType);
				return(NULL);
				}

			fprintf(fp, "P6\n%d %d\n%d\n", img->nCols, img->nRows,
			  ImageMaxIntensity(img));

			if(img->dataType == DATA_TYPE_C)
				{
				for(row = 0 ; row < img->nRows ; row++)
					for(col = 0 ; col < img->nCols ; col++)
						{
						tmpBuf[0] = img->data[0].C[row][col];
						tmpBuf[1] = img->data[1].C[row][col];
						tmpBuf[2] = img->data[2].C[row][col];
						if(fwrite(tmpBuf, sizeof(uChar), 3, fp) != 3)
							{
							fprintf(stderr, "Error (WriteImageFile): ");
							fprintf(stderr, "writing to file.\n");
							CloseImageFile(fp, fpType);
							return(NULL);
							}

						}

				}

			else
				{
				for(row = 0 ; row < img->nRows ; row++)
					for(col = 0 ; col < img->nCols ; col++)
						{
						tmpBuf[0] = (uChar)img->data[0].S[row][col];
						tmpBuf[1] = (uChar)img->data[1].S[row][col];
						tmpBuf[2] = (uChar)img->data[2].S[row][col];
						if(fwrite(tmpBuf, sizeof(uChar), 3, fp) != 3)
							{
							fprintf(stderr, "Error (WriteImageFile): ");
							fprintf(stderr, "writing to file.\n");
							CloseImageFile(fp, fpType);
							return(NULL);
							}

						}

				}

			break;

		}

	CloseImageFile(fp, fpType);
	return(img);
	}

/*==============================================================================
------------------------------------------------------------------------------*/

Image *CopyImage(Image *srcImg, Image *dstImg)

	{
	int row, col, n;

	if(srcImg->nRows != dstImg->nRows || srcImg->nCols != dstImg->nCols)
		{
		fprintf(stderr, "Error (CopyImage): sizes don't match.\n");
		return(NULL);
		}

	if(dstImg->nPlanes < srcImg->nPlanes)
		{
		fprintf(stderr,
		  "Error (CopyImage): dst image has less planes than src image.\n");
		return(NULL);
		}

	switch(srcImg->dataType)
		{
		case DATA_TYPE_C:
			switch(dstImg->dataType)
				{
				case DATA_TYPE_C:
					for(n = 0 ; n < srcImg->nPlanes ; n++)
						for(row = 0 ; row < srcImg->nRows ; row++)
							for(col = 0 ; col < srcImg->nCols ; col++)
								dstImg->data[n].C[row][col] =
								  srcImg->data[n].C[row][col];

					break;

				case DATA_TYPE_S:
					for(n = 0 ; n < srcImg->nPlanes ; n++)
						for(row = 0 ; row < srcImg->nRows ; row++)
							for(col = 0 ; col < srcImg->nCols ; col++)
								dstImg->data[n].S[row][col] = (uShort)
								  srcImg->data[n].C[row][col];

					break;

				}

			break;

		case DATA_TYPE_S:
			switch(dstImg->dataType)
				{
				case DATA_TYPE_C:
					fprintf(stderr, "Warning (CopyImage): ");
					fprintf(stderr, "S -> C, data might be lost.\n");
					for(n = 0 ; n < srcImg->nPlanes ; n++)
						for(row = 0 ; row < srcImg->nRows ; row++)
							for(col = 0 ; col < srcImg->nCols ; col++)
								{
								if(srcImg->data[n].S[row][col] > 255)
									dstImg->data[n].C[row][col] = 255;

								else
									dstImg->data[n].C[row][col] = (uChar)
									  srcImg->data[n].S[row][col];

								}

					break;

				case DATA_TYPE_S:
					for(n = 0 ; n < srcImg->nPlanes ; n++)
						for(row = 0 ; row < srcImg->nRows ; row++)
							for(col = 0 ; col < srcImg->nCols ; col++)
								dstImg->data[n].S[row][col] =
								  srcImg->data[n].S[row][col];

					break;

				}

			break;

		}

	return(srcImg);
	}

/*==============================================================================
	Calculates several error measures between two images.
------------------------------------------------------------------------------*/

Image *CalcImageErr(Image *img1, Image *img2, ImageErr *imgErr)

	{
	int row, col;
	double error, maxValue = 255;

	if(img1->nRows != img2->nRows || img1->nCols != img2->nCols)
		{
		fprintf(stderr, "Error (CalcImageErr): sizes don't match.\n");
		return(NULL);
		}

	if((img1->nPlanes != 1 && img1->nPlanes != 3) ||
	  (img2->nPlanes != 1 && img2->nPlanes != 3))
		{
		fprintf(stderr, "Error (CalcImageErr): ");
		fprintf(stderr, "support only for images having 1 or 3 planes.\n");
		return(NULL);
		}

	if(img1->dataType == DATA_TYPE_S || img2->dataType == DATA_TYPE_S)
		maxValue = 65535;

	imgErr->mabErr[0] = imgErr->rmsErr[0] = imgErr->maxErr[0] = 0;
	imgErr->mabErr[1] = imgErr->rmsErr[1] = imgErr->maxErr[1] = 0;
	imgErr->mabErr[2] = imgErr->rmsErr[2] = imgErr->maxErr[2] = 0;
	imgErr->mabErr[3] = imgErr->rmsErr[3] = imgErr->maxErr[3] = 0;

	for(row = 0 ; row < img1->nRows ; row++)
		for(col = 0 ; col < img1->nCols ; col++)
			{
			error = GetGrayPixel(img1, row, col) - GetGrayPixel(img2, row, col);
			imgErr->mabErr[0] += fabs(error);
			imgErr->mabErr[3] += fabs(error);
			imgErr->rmsErr[0] += error * error;
			imgErr->rmsErr[3] += error * error;
			if(fabs(error) > imgErr->maxErr[0])
				imgErr->maxErr[0] = fabs(error);

			if(fabs(error) > imgErr->maxErr[3])
				imgErr->maxErr[3] = fabs(error);

			}

	imgErr->mabErr[0] = imgErr->mabErr[0] / (img1->nRows * img1->nCols);
	imgErr->rmsErr[0] = sqrt(imgErr->rmsErr[0] / (img1->nRows * img1->nCols));
	if(imgErr->rmsErr[0] != 0)
		imgErr->psnr[0] = 20 * log10(maxValue / imgErr->rmsErr[0]);

	else
		imgErr->psnr[0] = 999;

	if(img1->nPlanes == 3 && img2->nPlanes == 3)
		{
		for(row = 0 ; row < img1->nRows ; row++)
			for(col = 0 ; col < img1->nCols ; col++)
				{
				error = GetColor(img1, row, col, 1) -
				  GetColor(img2, row, col, 1);
				imgErr->mabErr[1] += fabs(error);
				imgErr->mabErr[3] += fabs(error);
				imgErr->rmsErr[1] += error * error;
				imgErr->rmsErr[3] += error * error;
				if(fabs(error) > imgErr->maxErr[1])
					imgErr->maxErr[1] = fabs(error);

				if(fabs(error) > imgErr->maxErr[3])
					imgErr->maxErr[3] = fabs(error);

				}

		imgErr->mabErr[1] = imgErr->mabErr[1] / (img1->nRows * img1->nCols);
		imgErr->rmsErr[1] = sqrt(imgErr->rmsErr[1] / (img1->nRows*img1->nCols));
		if(imgErr->rmsErr[1] != 0)
			imgErr->psnr[1] = 20 * log10(maxValue / imgErr->rmsErr[1]);

		else
			imgErr->psnr[1] = 999;

		for(row = 0 ; row < img1->nRows ; row++)
			for(col = 0 ; col < img1->nCols ; col++)
				{
				error = GetColor(img1, row, col, 2) -
				  GetColor(img2, row, col, 2);
				imgErr->mabErr[2] += fabs(error);
				imgErr->mabErr[3] += fabs(error);
				imgErr->rmsErr[2] += error * error;
				imgErr->rmsErr[3] += error * error;
				if(fabs(error) > imgErr->maxErr[2])
					imgErr->maxErr[2] = fabs(error);

				if(fabs(error) > imgErr->maxErr[3])
					imgErr->maxErr[3] = fabs(error);

				}

		imgErr->mabErr[2] = imgErr->mabErr[2] / (img1->nRows * img1->nCols);
		imgErr->rmsErr[2] = sqrt(imgErr->rmsErr[2] / (img1->nRows*img1->nCols));
		if(imgErr->rmsErr[2] != 0)
			imgErr->psnr[2] = 20 * log10(maxValue / imgErr->rmsErr[2]);

		else
			imgErr->psnr[2] = 999;

		imgErr->mabErr[3] = imgErr->mabErr[3] / (img1->nRows * img1->nCols * 3);
		imgErr->rmsErr[3] = sqrt(imgErr->rmsErr[3] / (img1->nRows *
		  img1->nCols * 3));
		if(imgErr->rmsErr[3] != 0)
			imgErr->psnr[3] = 20 * log10(maxValue / imgErr->rmsErr[3]);

		else
			imgErr->psnr[3] = 999;

		}

	else
		{
		imgErr->psnr[1] = 999;
		imgErr->psnr[2] = 999;
		imgErr->psnr[3] = 999;
		}

	return(img1);
	}

/*==============================================================================
------------------------------------------------------------------------------*/

Image *MatrixToImage(Matrix *src, Image *dst, int imgPlane)

	{
	int row, col;
	Mtype x1, x2;

	if(src->nRows != dst->nRows || src->nCols != dst->nCols)
		{
		fprintf(stderr, "Error (MatrixToImage): sizes don't match\n");
		return(NULL);
		}

	if(imgPlane < 0 || imgPlane > 2)
		{
		fprintf(stderr, "Error (MatrixToImage): invalid image plane\n");
		return(NULL);
		}

	x1.dataType = src->dataType;
	x2.dataType = dst->dataType;
	for(row = 0 ; row < src->nRows ; row++)
		for(col = 0 ; col < src->nCols ; col++)
			{
			x1.data = GetPointerToMatrixElement(src, row, col);
			if(dst->dataType == DATA_TYPE_C)
				x2.data = (void *)(&dst->data[imgPlane].C[row][col]);

			else
				x2.data = (void *)(&dst->data[imgPlane].S[row][col]);

			MtypeToMtype(&x1, &x2);
			}

	return(dst);
	}

/*==============================================================================

	Converts an Image data type to Matrix data type

------------------------------------------------------------------------------*/

Image *ImageToMatrix(Image *src, Matrix *dst, int imgPlane)

	{
	int row, col;
	Mtype x1, x2;

	if(src->nRows != dst->nRows || src->nCols != dst->nCols)
		{
		fprintf(stderr, "Error (ImageToMatrix): sizes don't match\n");
		return(NULL);
		}

	if(imgPlane < 0 || imgPlane > 2)
		{
		fprintf(stderr, "Error (ImageToMatrix): invalid image plane\n");
		return(NULL);
		}

	x1.dataType = src->dataType;
	x2.dataType = dst->dataType;
	for(row = 0 ; row < src->nRows ; row++)
		for(col = 0 ; col < src->nCols ; col++)
			{
			if(src->dataType == DATA_TYPE_C)
				x1.data = (void *)(&src->data[imgPlane].C[row][col]);

			else
				x1.data = (void *)(&src->data[imgPlane].S[row][col]);

			x2.data = GetPointerToMatrixElement(dst, row, col);
			MtypeToMtype(&x1, &x2);
			}

	return(src);
	}

/*==============================================================================

	Computes the variation of an image.

------------------------------------------------------------------------------*/

double ComputeImgVariation(Image *img)

	{
	int row, col;
	double hDiff, vDiff, imgVariation = 0;

	for(row = 0 ; row < img->nRows ; row++)
		for(col = 0 ; col < img->nCols ; col++)
			{
			if(row == 0)
				{
				if(col == 0)
					{
					hDiff = vDiff = GetGrayPixel(img, 0, 0);
					}

				else
					{
					hDiff = GetGrayPixel(img, 0, col) -
					  GetGrayPixel(img, 0, col - 1);
					vDiff = GetGrayPixel(img, 0, col);
					}

				}

			else
				{
				if(col == 0)
					{
					hDiff = GetGrayPixel(img, row, 0);
					vDiff = GetGrayPixel(img, row, 0) -
					  GetGrayPixel(img, row - 1, 0);
					}

				else
					{
					hDiff = GetGrayPixel(img, row, col) -
					  GetGrayPixel(img, row, col - 1);
					vDiff = GetGrayPixel(img, row, col) -
					  GetGrayPixel(img, row - 1, col);
					}

				}

			imgVariation += sqrt((double)(vDiff * vDiff + hDiff * hDiff));
			}

	return(imgVariation / (img->nRows * img->nCols));
	}

/*==============================================================================

	Get a unsigned int value that represents a block with NRSize rows and 
	nCSizes columns.

------------------------------------------------------------------------------*/

unsigned int GetBlockValueFromImage(Image *img, int row, int col, int nRSize, int nCSize, int plane)
{
	unsigned int value = 0;
	int n = 0, r, c, gray;
	/*
	if(row == 0 && col == 0)
	{
		printf("Row = %d\n", row);
		printf("Col = %d\n", col);
		printf("nRSize = %d\n", nRSize);
		printf("nCSize = %d\n", nCSize);
	}
	*/
	for(r = row; r > row-nRSize; r--)
		for(c = col; c < col+nCSize; c++)
		{
			gray = GetGrayPixel(img, r, c);
			value += (((gray >> plane) & 0x1) << n++);
		}
	
	return value;
}

/*==============================================================================

	Merges two images in a single one using 'v' (vertical) or 'h' 
	(horizontal) mode.

------------------------------------------------------------------------------*/

Image *MergeImages(Image *srcImg1, Image *srcImg2, char mode)
{
	int row, col, n;
	Image *img = NULL;

	if(srcImg1 == NULL || srcImg2 == NULL)
	{
		fprintf(stderr, "Error (MergeImages): null image detected!\n");
		return(NULL);		
	}
	if(mode != 'v' && mode != 'V' && mode != 'h' && mode != 'H')
	{
		fprintf(stderr, "Error (MergeImages): invalid mode '%c'!\n", mode);
		return(NULL);		
	}
	
	if(srcImg1->dataType != srcImg2->dataType || srcImg1->nPlanes != srcImg2->nPlanes)
	{
		fprintf(stderr, "Error (MergeImages): wrong type of image!\n");
		fprintf(stderr, "Error (MergeImages): both images must be the same type!\n");
		return(NULL);
	}
	
	if(mode == 'v' || mode == 'V')
	{
		if(srcImg1->nCols != srcImg2->nCols)
		{
			fprintf(stderr, "Error (MergeImages): for vertical mode both image must have the same width!\n");
			fprintf(stderr, "Source image 1 has %dx%d | Source image 2 has %dx%d\n", srcImg1->nRows, srcImg1->nCols, srcImg2->nRows, srcImg2->nCols);
			return(NULL);		
		}
		else
		{
			/* Create new image */
			if(!(img = CreateImage(srcImg1->nRows+srcImg2->nRows, srcImg1->nCols, srcImg1->dataType, srcImg1->nPlanes)))
			{
				fprintf(stderr, "Error (MergeImages): unable to create image with %d x %d pixels\n", srcImg1->nRows+srcImg2->nRows, srcImg1->nCols);
				return(NULL);
			}

			printf("New image has %d x %d pixels\n",img->nRows, img->nCols);

			/* Copy elements of both images to the new image */
			switch(img->dataType)
			{				
				case DATA_TYPE_C:
					for(n = 0 ; n < img->nPlanes ; n++)
						for(row = 0 ; row < srcImg1->nRows ; row++)
						{
							for(col = 0 ; col < srcImg1->nCols ; col++)
								img->data[n].C[row][col] = srcImg1->data[n].C[row][col];
						}

						for(row = 0 ; row < srcImg2->nRows ; row++)
						{
							for(col = 0 ; col < srcImg2->nCols ; col++)
								img->data[n].C[srcImg1->nRows+row][col] = srcImg2->data[n].C[row][col];
						}
						

					break;

				case DATA_TYPE_S:
					for(n = 0 ; n < img->nPlanes ; n++)
					{
						for(row = 0 ; row < srcImg1->nRows ; row++)
						{
							for(col = 0 ; col < srcImg1->nCols ; col++)
								img->data[n].S[row][col] = srcImg1->data[n].S[row][col];
						}	
					}
					for(n = 0 ; n < img->nPlanes ; n++)
					{
						for(row = 0 ; row < srcImg2->nRows ; row++)
						{
							for(col = 0 ; col < srcImg2->nCols ; col++)
							{
								img->data[n].S[srcImg1->nRows+row][col] = srcImg2->data[n].S[row][col];
							}
						}
					}
					break;
			}

		}
	}
		
	if(mode == 'h' || mode == 'H')
	{
		if(srcImg1->nCols != srcImg2->nCols)
		{
			fprintf(stderr, "Error (MergeImages): for horizontal mode both image must have the same heigth!\n");
			fprintf(stderr, "Source image 1 has %dx%d | Source image 2 has %dx%d\n", srcImg1->nRows, srcImg1->nCols, srcImg2->nRows, srcImg2->nCols);
			return(NULL);		
		}
		else
		{
			/* Create new image */
			if(!(img = CreateImage(srcImg1->nRows, srcImg1->nCols+srcImg2->nCols, srcImg1->dataType, srcImg1->nPlanes)))
			{
				fprintf(stderr, "Error (MergeImages): unable to create image with %d x %d pixels\n", srcImg1->nRows, srcImg1->nCols+srcImg2->nCols);
				return(NULL);
			}
			
			/* Copy elements of both images to the new image */
			switch(img->dataType)
			{				
				case DATA_TYPE_C:
					for(n = 0 ; n < img->nPlanes ; n++)
						for(row = 0 ; row < srcImg1->nRows ; row++)
						{
							for(col = 0 ; col < srcImg1->nCols ; col++)
								img->data[n].C[row][col] = srcImg1->data[n].C[row][col];
							for(col = 0 ; col < srcImg2->nCols ; col++)
								img->data[n].C[row][srcImg1->nCols+col] = srcImg2->data[n].C[row][col];
						}
						

					break;

				case DATA_TYPE_S:
					for(n = 0 ; n < img->nPlanes ; n++)
						for(row = 0 ; row < srcImg1->nRows ; row++)
						{
							for(col = 0 ; col < srcImg1->nCols ; col++)
								img->data[n].S[row][col] = srcImg1->data[n].S[row][col];
							for(col = 0 ; col < srcImg2->nCols ; col++)
								img->data[n].S[row][srcImg1->nCols+col] = srcImg2->data[n].S[row][col];
						}
						

					break;
			}

		}
	}	

	return img;
}
