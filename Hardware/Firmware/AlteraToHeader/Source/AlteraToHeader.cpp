
#include <windows.h>
#include <iostream>
#include <tchar.h>

#define SIZEOF_RBF_FILE		32385

//Conversion procedure.
void ConvertToText( BYTE* pRawFile, char* pHeaderFile, char* pName )
{
	unsigned long	n = 0;
	char*			pHeaderFile2 = pHeaderFile;

//Copyright part.
	pHeaderFile += sprintf( pHeaderFile, "//***************************************************************************\n" );
	pHeaderFile += sprintf( pHeaderFile, "//***************************************************************************\n" );
	pHeaderFile += sprintf( pHeaderFile, "//\n" );
	pHeaderFile += sprintf( pHeaderFile, "//                          PROPRIETARY INFORMATION\n" );
	pHeaderFile += sprintf( pHeaderFile, "//\n" );
	pHeaderFile += sprintf( pHeaderFile, "// This software is supplied under the terms of a license agreement or\n" );
	pHeaderFile += sprintf( pHeaderFile, "// nondisclosure agreement with Waldemar Haszlakiewicz and may not be\n" );
	pHeaderFile += sprintf( pHeaderFile, "// copied or disclosed except in accordance with the terms of that agreement.\n" );
	pHeaderFile += sprintf( pHeaderFile, "//\n" );
	pHeaderFile += sprintf( pHeaderFile, "//                 Copyright (C) 2006 Waldemar Haszlakiewicz\n" );
	pHeaderFile += sprintf( pHeaderFile, "//                           All Rights Reserved.\n" );
	pHeaderFile += sprintf( pHeaderFile, "//\n" );
	pHeaderFile += sprintf( pHeaderFile, "//***************************************************************************\n" );
	pHeaderFile += sprintf( pHeaderFile, "//***************************************************************************\n\n" );

//If/Def
	pHeaderFile += sprintf( pHeaderFile, "#pragma once\n\n" );

//Data
	pHeaderFile += sprintf( pHeaderFile, "BYTE %s[] = {\n", pName );
	int i = 0;
	while ( i < ( SIZEOF_RBF_FILE - 15 ) )
	{
		pHeaderFile += sprintf(
		  pHeaderFile,
		  "	0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X,\n",
		  pRawFile[i],
		  pRawFile[i+1],
		  pRawFile[i+2],
		  pRawFile[i+3],
		  pRawFile[i+4],
		  pRawFile[i+5],
		  pRawFile[i+6],
		  pRawFile[i+7],
		  pRawFile[i+8],
		  pRawFile[i+9],
		  pRawFile[i+10],
		  pRawFile[i+11],
		  pRawFile[i+12],
		  pRawFile[i+13],
		  pRawFile[i+14]
		);

	//Update.
		i += 15;
	}

//Last line -> without ',' at the end.
	pHeaderFile += sprintf(
	  pHeaderFile,
	  "	0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X\n",
	  pRawFile[i],
	  pRawFile[i+1],
	  pRawFile[i+2],
	  pRawFile[i+3],
	  pRawFile[i+4],
	  pRawFile[i+5],
	  pRawFile[i+6],
	  pRawFile[i+7],
	  pRawFile[i+8],
	  pRawFile[i+9],
	  pRawFile[i+10],
	  pRawFile[i+11],
	  pRawFile[i+12],
	  pRawFile[i+13],
	  pRawFile[i+14]
	);

	pHeaderFile += sprintf( pHeaderFile, "};\n\n" );

	return;
}


//parameter    explanation
//1.           input path -> path to rbf file
//2.           output path -> path to header file that will get written
//3.           name of the array variable
int _tmain(int argc, _TCHAR* argv[])
{
//	_TCHAR*		pName;
	FILE*		hInputFile = NULL;
	FILE*		hOutputFile = NULL;
	BOOL		showHelp = FALSE;

//Get parameters.
	if ( argc == 4 )
	{
	//Test arguments -> try to open files.
		hInputFile = fopen( argv[1], "rb" );
		hOutputFile = fopen( argv[2], "w+b" );

	//Check if input file is 32385 bytes long.
		if ( hInputFile != NULL )
		{
			fseek( hInputFile, 0, SEEK_END );
			if ( SIZEOF_RBF_FILE != ftell( hInputFile ) )
			{
				fclose( hInputFile );
				hInputFile = NULL;
			}
			else
			{
			//Size is correct -> move file pointer back to start.
				fseek( hInputFile, 0, SEEK_SET );
			}
		}

	//All ok?
		if ( hInputFile == NULL )
		{
			printf( "ERROR: can't open input file or file size is not correct\n\n" );
			showHelp = TRUE;
		}
		if ( hOutputFile == NULL )
		{
			printf( "ERROR: can't open output file\n\n" );
			showHelp = TRUE;
		}
	}
	else
	{
		showHelp = TRUE;
	}

//Show help?
	if ( showHelp == TRUE )
	{
		printf( "Altera Flex6000 Rbf To Header file converter\n\n" );
		printf( "command line: rbf2h.exe pathToRbfFile pathToHeaderFile nameOfArrayVariable\n\n" );
		printf( "pathToRbfFile - path to Altera software generated rbf file\n" );
		printf( "                Notice:\n" );
		printf( "                This file must be exactly 32385 bytes long\n" );
		printf( "pathToHeaderFile - path to file that will receive generated header file\n" );
		printf( "nameOfArrayVariable - name of the array variable in header file\n\n" );
	}

//Convert...
	if ( showHelp != TRUE )
	{
	//Allocate buffers.
		BYTE*	pInBuffer = new BYTE[SIZEOF_RBF_FILE];
		char*	pOutBuffer = new char[0x50000];

	//Did we get memory?
		if ( ( pInBuffer != NULL ) && ( pOutBuffer != NULL ) )
		{
		//Read input file.
			if ( SIZEOF_RBF_FILE == fread( pInBuffer, sizeof( BYTE ), SIZEOF_RBF_FILE, hInputFile ) )
			{
			//Zero out memory.
				ZeroMemory( pOutBuffer, 0x50000 );

			//Convert binary to text.
				ConvertToText( pInBuffer, pOutBuffer, argv[3] );

			//Save text file.
				fwrite( pOutBuffer, sizeof( BYTE ), strlen( pOutBuffer ), hOutputFile );
			}
		}

	//Release resources.
		if ( pInBuffer != NULL )
		{
			delete pInBuffer;
		}
		if ( pOutBuffer != NULL )
		{
			delete pOutBuffer;
		}
	}
	
//Free resources.
	if ( hInputFile != NULL )
	{
		fclose( hInputFile );
	}
	if ( hOutputFile != NULL )
	{
		fclose( hOutputFile );
	}

	return 0;
}

