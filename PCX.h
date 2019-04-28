#ifndef _PCX_H_
#define _PCX_H_

#define PCXFILLER        (128-74)		// 54

typedef struct
{
	BYTE  byManufacturer;		// keyword (10 decimal)
	BYTE  byVersion;			// hardware version (5, old files with no palette are 3)
	BYTE  byEncoding;			// run length encoded (1)
	BYTE  byBitsPerPixelPlane;	// bits per pixel per plane
	WORD  wXmin;				// picture dimensions in pixels
	WORD  wYmin;
	WORD  wXmax;
	WORD  wYmax;
	WORD  wHdpi;				// horizontal image dpi
	WORD  wVdpi;				// vertical image dpi
	BYTE  byPal[48];			// palette
	BYTE  byVMode;				// monitor (ignored)
	BYTE  byNumPlanes;			// number of planes in image 
	WORD  wBytesPerLine;		// bytes per line per plane in picture 
	WORD  wPalInfo;				// grayscale/color flag (obsolete)
	WORD  wHScreenSize;			// horizontal screen size in pixels
	WORD  wVScreenSize;			// vertical screen size in pixels
								
	// The rest of the 128 byte header is not used
	BYTE byFiller[PCXFILLER];	// (reserved, should be zero)
    BYTE byData;				// start of image data (unbounded)
} PCX_HEADER, *LPPCX_HEADER;

typedef struct
{
	LPCTSTR	szFileName;
	LPBYTE	lpImageData;
	DWORD	dwWidth;
	DWORD	dwHeight;
	LPBYTE	lpPalette;
	DWORD	dwPaletteSize;	
} PCX_ENCODE, *LPPCX_ENCODE;

LPVOID MakeFile( LPCTSTR szFileName, DWORD dwFileSize);
void EncodePCX( LPPCX_ENCODE lpEncode);


#endif		// #ifndef _PCX_H_