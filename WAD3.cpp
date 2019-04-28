//////////////////////////////////////////////////////////////////////////////////////////
//	Wally dudes:  ty@wwa.com, neal_white_iii@hotmail.com
//
//	01/08/2001
//
//	Restrictions:  None... do whatever you want with this!
//
//	This is a basic WAD3 reader.  It converts all of the images in a WAD to PCX files.
//	Assumes the current working directory for output, but the function supports any old
//	directory.  I didn't want to bother with checking for the silly backslashes and such,
//	so that's your fun part :)
//
//	This code is completely unsupported, and was written purely for other people to
//	learn the WAD3 structure.  That said, it *does* work and is a useful tool all by
//	itself.  Of course the really cool stuff we do inside Wally, but that wouldn't be
//	so easy to duplicate here.  I will answer any reasonable questions about the 
//	format, or even anything else in this code, but I make no warranties about using
//	this as the base for another program.  IE, test the crap out of what you're 
//	coding and really make sure it's working the way it should be.  
//
//	I put this together in a short amount of time, so there may be bugs or such.  
//	It's also not very optimized, but it runs fast enough to not bother me too 
//	much :)  I am doing a *lot* of bounds checking, so make sure you keep that stuff 
//	in there.  It is always better to be safe than sorry, and you never know when 
//	something might be corrupt.
//
//	Since this code only reads data and doesn't actually write a WAD file (that's 
//	for you to play with <g>) it doesn't bother looking at some important items.  
//	Specifically, the WORD padding at the very end of a mip's data block.  This WORD 
//	padding isn't used for anything, but it HAS to be there.  Check out the pseudocode 
//	structure in the header file for more details there.  If you come up with a WAD3
//	creation utility, just make sure Wally and WorldCraft can load your WAD, and you'll
//	be good to go.
//
//	- Ty
//
//////////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "WAD3.h"
#include "PCX.h"
#include "stdio.h"
#include "iostream"

using namespace std;

void MapFile( LPCTSTR szFileName, LPVOID *pView, LPDWORD pdwFileSize)
{
	HANDLE hFile = NULL;
	HANDLE hFileMapping = NULL;
	LPVOID lpView = NULL;
	DWORD dwFileSize = 0;
	DWORD dwError = 0;

	hFile = CreateFile( 
		szFileName,					// Name of file
		GENERIC_READ,				// Desired access
		FILE_SHARE_READ,			// Share mode
		NULL,						// Security attributes
		OPEN_EXISTING,				// Creation disposition
		FILE_FLAG_SEQUENTIAL_SCAN,	// Attributes and flags
		NULL);						// Template file	

	if (hFile == INVALID_HANDLE_VALUE)
	{
		// CreateFile() does not return NULL on error
		throw CWADException();
	}

	// Store this away for now...
	dwFileSize = GetFileSize( hFile, NULL);

	if (dwFileSize == -1)
	{
		dwError = ::GetLastError();
		CloseHandle( hFile);
		throw CWADException( dwError);
	}
	
	hFileMapping = CreateFileMapping( 
		hFile,						// Handle to file
		NULL,						// Security attributes
		PAGE_READONLY,				// Protection
		0,							// Max size high
		0,							// Max size low
		NULL);						// Name of mapping object	
	
	if (hFileMapping == NULL)
	{
		dwError = ::GetLastError();
		CloseHandle( hFile);
		throw CWADException();
	}

	// We don't need this anymore
	CloseHandle( hFile);

	// Map to the entire file
	lpView = MapViewOfFile(
		hFileMapping,				// Handle to the mapping
		FILE_MAP_READ,				// Desired access
		0,							// Offset high
		0,							// Offset low
		0);							// Number of bytes

	if (lpView == NULL)
	{
		dwError = ::GetLastError();
		CloseHandle( hFileMapping);
		throw CWADException( dwError);
	}

	// We don't need this anymore
	CloseHandle( hFileMapping);

	if (pView)
	{
		*pView = lpView;
	}
	
	if (pdwFileSize)
	{
		*pdwFileSize = dwFileSize;
	}
}

int CorruptWAD3( LPCTSTR szErrorMessage, LPVOID lpView)
{
	UnmapViewOfFile( lpView);
	throw CWADException( szErrorMessage);

	// We never actually get here, but oh well
	return 1;
}

int EnumerateImages( LPCTSTR szFileName, LPCTSTR szOutputFolder)
{
	LPVOID			lpView = NULL;
	LPWAD3_HEADER	lpHeader = NULL;
	LPWAD3_LUMP		lpLump = NULL;
	LPWAD3_MIP		lpMip = NULL;
	PCX_ENCODE		pcxEncode;

	DWORD			dwNumLumps = 0;
	DWORD			dwTableOffset = 0;
	DWORD			dwFileSize = 0;
	DWORD			dwFilePos = 0;
	DWORD			dwPaletteOffset = 0;
	WORD			wPaletteSize = 0;
	DWORD			dwWidth = 0;
	DWORD			dwHeight = 0;
	char			szFilePath[_MAX_PATH];

	// Go map into the file
	MapFile( szFileName, &lpView, &dwFileSize);

	// Make sure it's at least big enough to manipulate the header
	if (dwFileSize < sizeof(WAD3_HEADER))
	{		
		return CorruptWAD3( "WAD3 file is malformed.", lpView);
	}

	lpHeader = (LPWAD3_HEADER)lpView;

	if (lpHeader->identification != WAD3_ID)
	{
		return CorruptWAD3( "Invalid WAD3 header id.", lpView);
	}

	dwNumLumps = lpHeader->numlumps;
	dwTableOffset = lpHeader->infotableofs;

	// Make sure our table is really there
	if ( ((dwNumLumps * sizeof(WAD3_LUMP)) + dwTableOffset) > dwFileSize)
	{
		return CorruptWAD3( "WAD3 file is malformed.", lpView);
	}

	// Point at the first table entry
	lpLump = (LPWAD3_LUMP)((LPBYTE)lpView + dwTableOffset);

	for (DWORD j = 0; j < dwNumLumps; j++, lpLump++)
	{		
		if (lpLump->type == WAD3_TYPE_MIP)
		{
			// Find out where the MIP actually is
			dwFilePos = lpLump->filepos;
			
			// Make sure it's in bounds
			if ( dwFilePos >= dwFileSize)
			{
				return CorruptWAD3( "Invalid lump entry; filepos is malformed.", lpView);
			}

			// Point at the mip
			lpMip = (LPWAD3_MIP)((LPBYTE)lpView + dwFilePos);

			dwWidth = lpMip->width;
			dwHeight = lpMip->height;

			// Set these
			pcxEncode.dwWidth = dwWidth;
			pcxEncode.dwHeight = dwHeight;

			sprintf( szFilePath, "%s%s.pcx", szOutputFolder, lpMip->name);
			pcxEncode.szFileName = szFilePath;

			// First find the size of the palette (should be 256, but you never know.)  The size of the palette is a WORD entry, located 
			// directly after the very last mip (or is *supposed* to be there.)  Immediately following the palette size entry is the palette
			// itself.  After the palette is another WORD entry that contains no useful data... it is used just to pad the mip
			dwPaletteOffset = GET_MIP_DATA_SIZE( dwWidth, dwHeight);
			wPaletteSize = *(WORD *)((LPBYTE)lpMip + dwPaletteOffset);
			dwPaletteOffset += sizeof( WORD);
			
			pcxEncode.dwPaletteSize = wPaletteSize;
			pcxEncode.lpPalette = ((LPBYTE)lpMip + dwPaletteOffset);
			
			// Now for the mip data.  The first mip (the largest one, width * height in size) should always be the first offset, followed
			// in order by the other three mips, descending in size.  Each successive mip is 1/2 the width and 1/2 the height of the previous
			// one, or 1/4 the total size.  Since I'm only working with the largest one here, it doesn't really matter.  But if you're trying
			// to look at all 4 mips, you'll need to point at each offset and handle them.  The first mip should always start immediately 
			// after the mip header, hence the sizeof(WAD3_MIP).  Here's a sample image:
			//
			// width = 128  height = 128
			// datasize[0] = width * height = 16384
			// datasize[1] = width * height / 4 = 4096
			// datasize[2] = width * height / 16 = 1024
			// datasize[3] = width * height / 64 = 256
			// offset[0] = sizeof(WAD3_MIP)
			// offset[1] = sizeof(WAD3_MIP) + datasize[0]
			// offset[2] = sizeof(WAD3_MIP) + datasize[0] + datasize[1]
			// offset[3] = sizeof(WAD3_MIP) + datasize[0] + datasize[1] + datasize[2]

			// Make sure we've got the data there (bad things happen otherwise!)
			if ( (dwFilePos + lpMip->offsets[0] + dwWidth * dwHeight) > dwFileSize)
			{
				return CorruptWAD3( "Invalid mip entry; offsets[0] is malformed.", lpView);
			}

			pcxEncode.lpImageData = ((LPBYTE)lpMip + lpMip->offsets[0]);

			cout << lpMip->name << "[ " << dwWidth << " x " << dwHeight << " ]  ====> " << szFilePath << " ...";
			EncodePCX( &pcxEncode);
			cout << "done." << endl;			
		}
	}

	UnmapViewOfFile( lpView);

	return 0;
}

int main(int argc, char* argv[])
{
	int iReturnCode = 0;

	if (argc != 2)
	{
		cout << "USAGE:  WAD3 filename.wad" << endl;
		iReturnCode = 1;
	}
	else
	{
		try
		{
			EnumerateImages( argv[1], "./");			
		}		
		catch( CWADException we)
		{
			we.PrintError();
		}
	}
	
	return iReturnCode;
}
