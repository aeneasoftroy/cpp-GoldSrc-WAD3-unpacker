
#include "stdafx.h"
#include "PCX.h"
#include "WAD3.h"

LPVOID MakeFile( LPCTSTR szFileName, DWORD dwFileSize)
{
	HANDLE hFile = NULL;
	HANDLE hFileMapping = NULL;
	LPVOID lpView = NULL;
	DWORD dwError = 0;

	hFile = CreateFile( 
		szFileName,							// Name of file
		GENERIC_READ | GENERIC_WRITE,		// Desired access
		0,									// Share mode
		NULL,								// Security attributes
		CREATE_ALWAYS,						// Creation disposition
		FILE_ATTRIBUTE_NORMAL,				// Attributes and flags
		NULL);								// Template file	

	if (hFile == INVALID_HANDLE_VALUE)
	{
		// CreateFile() does not return NULL on error
		throw CWADException();
	}

	hFileMapping = CreateFileMapping( 
		hFile,						// Handle to file
		NULL,						// Security attributes
		PAGE_READWRITE,				// Protection
		0,							// Max size high
		dwFileSize,					// Max size low
		NULL);						// Name of mapping object	
	
	if (hFileMapping == NULL)
	{
		dwError = ::GetLastError();
		CloseHandle( hFile);
		throw CWADException(dwError);
	}

	// We don't need this anymore
	CloseHandle( hFile);

	// Map to the entire file
	lpView = MapViewOfFile(
		hFileMapping,				// Handle to the mapping
		FILE_MAP_WRITE,				// Desired access
		0,							// Offset high
		0,							// Offset low
		dwFileSize);				// Number of bytes

	if (lpView == NULL)
	{
		dwError = ::GetLastError();
		CloseHandle( hFileMapping);
		throw CWADException( dwError);
	}

	// We don't need this anymore
	CloseHandle( hFileMapping);

	return lpView;
}

void EncodePCX( LPPCX_ENCODE lpEncode)
{
	LPBYTE		lpPack = NULL;	
	LPBYTE		lpPackedData = NULL;
	LPBYTE		lpImageData = lpEncode->lpImageData;
	BYTE		byThisByte = 0;
	BYTE		byLastByte = 0;
	BYTE		byRunLength	= 0;
		
	DWORD		dwDataSize = 0;	
	DWORD		dwPackedLength = 0;
	DWORD		dwOffset = 0;
	DWORD		dwWidth = lpEncode->dwWidth;
	DWORD		dwHeight = lpEncode->dwHeight;
	DWORD		dwPaletteSize = lpEncode->dwPaletteSize;

	LPVOID			lpView;
	LPPCX_HEADER	pcxHeader;
	
	// Give some room for worst-case scenario
	dwDataSize = (dwWidth * dwHeight * 2) + 1000;
	
	lpPackedData = new BYTE[dwDataSize];
	if (!lpPackedData)
	{		
		throw CWADException( "Failed to allocate memory for the PCX image.");
	}

	ZeroMemory( lpPackedData, dwDataSize);
	
	pcxHeader = (LPPCX_HEADER)lpPackedData;
	
	pcxHeader->byManufacturer      = 0x0A;	// ZSoft PCX file id
	pcxHeader->byVersion           = 5;		// 256 color (with palette)
 	pcxHeader->byEncoding          = 1;		// standard run-length encoding
	pcxHeader->byBitsPerPixelPlane = 8;		// 256 color
	pcxHeader->wXmin = 0;
	pcxHeader->wYmin = 0;
	pcxHeader->wXmax = (short )(dwWidth - 1);
	pcxHeader->wYmax = (short )(dwHeight - 1);
	pcxHeader->wHdpi = 92;					// just use screen res
	pcxHeader->wVdpi = 92;
	pcxHeader->wBytesPerLine = (short)dwWidth;
	pcxHeader->wPalInfo = 1;				// not a grey scale image
	pcxHeader->byNumPlanes = 1;
		
	// pack the image
	lpPack = &pcxHeader->byData;
	
	for (DWORD i = 0; i < dwHeight; i++)
	{		
		byLastByte = *(lpImageData);
		byRunLength = 1;

		for (DWORD j = 1; j < dwWidth; j++)
		{
			if (j < dwWidth)
			{
				byThisByte = *(++lpImageData);
			}
			// else, just repeat previous pixel (for WORD padding)
			
			if (byThisByte == byLastByte)
			{
				byRunLength++;
			
				if (byRunLength == 63)
				{
					*lpPack++ = 0xC0 | byRunLength;
					*lpPack++ = byLastByte;
					byRunLength = 0;					
				}				
			}
			else
			{
				if (byRunLength)
				{				
					// If there's only one byte, and it's less than 192, just move it in
					// as no coded runlength
					if ((byRunLength == 1) && ((byLastByte & 0xC0) != 0xC0))
					{
						*lpPack++ = byLastByte;
					}
					else
					{				
						*lpPack++ = 0xC0 | byRunLength;
						*lpPack++ = byLastByte;
					}
				}
				
				byLastByte = byThisByte;
				byRunLength = 1;
			}
		
		}
		if (byRunLength)
		{				
			// If there's only one byte, and it's less than 192, just move it in
			// as no coded runlength

			if ((byRunLength == 1) && ((byLastByte & 0xc0) != 0xc0))
			{
				*lpPack++ = byLastByte;
			}
			else
			{				
				*lpPack++ = 0xc0 | byRunLength;
				*lpPack++ = byLastByte;						
			}	
		}	
		lpImageData++;
	}

	// write the palette
	*lpPack++ = 0x0C;	// palette ID byte
	
	memcpy( lpPack, lpEncode->lpPalette, dwPaletteSize * 3);
	lpPack += dwPaletteSize * 3;

	if (dwPaletteSize != 256)
	{
		// Set the rest to black
		memset( lpPack, 0, 768 - (dwPaletteSize * 3));
		lpPack += 768 - (dwPaletteSize * 3);
	}
			
	// write output file 
	dwPackedLength = lpPack - lpPackedData;

	lpView = MakeFile( lpEncode->szFileName, dwPackedLength);

	memcpy( lpView, lpPackedData, dwPackedLength);
	UnmapViewOfFile( lpView);
	
	if (lpPackedData)
	{
		delete []lpPackedData;
		lpPackedData = NULL;
	}
}
