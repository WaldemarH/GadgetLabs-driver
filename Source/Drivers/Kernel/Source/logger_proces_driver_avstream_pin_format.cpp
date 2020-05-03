//***************************************************************************
//***************************************************************************
//
//                          PROPRIETARY INFORMATION
//
// This software is supplied under the terms of a license agreement or
// nondisclosure agreement with Waldemar Haszlakiewicz and may not be
// copied or disclosed except in accordance with the terms of that agreement.
//
//               Copyright (C) 2010 Waldemar Haszlakiewicz
//                            All Rights Reserved.
//
//***************************************************************************
//***************************************************************************
//
// PURPOSE:
//		File contains definitions for logger and debugPrint functions.
//
//***************************************************************************
//***************************************************************************
#include "main.h"

#ifdef DBG_AND_LOGGER

void logger_c::Logger_Driver_AVStream_Pin_Format_AudioIntersectHandler( IN char* pBuffer, IN void** ppVariables )
{
	PKSDATARANGE_AUDIO						pCallerDataRange = (PKSDATARANGE_AUDIO)ppVariables[0];
	PKSDATARANGE_AUDIO						pDescriptorDataRange = (PKSDATARANGE_AUDIO)ppVariables[1];
	ULONG									dataBufferSize = *(PULONG)ppVariables[2];
	PKSDATAFORMAT_WAVEFORMATEXTENSIBLE		pDataFormat = (PKSDATAFORMAT_WAVEFORMATEXTENSIBLE)ppVariables[3];
	ULONG									dataSize = *(PULONG)ppVariables[4];

//CallerDataRange...
	pBuffer = Logger_Driver_AVStream_Pin_Format_DataRange( pBuffer, "CallerDataRange", pCallerDataRange );

//DescriptorDataRange...
	pBuffer = Logger_Driver_AVStream_Pin_Format_DataRange( pBuffer, "DescriptorDataRange", pDescriptorDataRange );

//DataBufferSize...
	if ( dataBufferSize == sizeof( KSDATAFORMAT_WAVEFORMATEXTENSIBLE ) )
	{
		pBuffer += sprintf( pBuffer, "  DataBufferSize: sizeof( KSDATAFORMAT_WAVEFORMATEXTENSIBLE )\n" );
	}
	else if ( dataBufferSize == sizeof( KSDATAFORMAT_WAVEFORMATEX ) )
	{
		pBuffer += sprintf( pBuffer, "  DataBufferSize: sizeof( KSDATAFORMAT_WAVEFORMATEX )\n" );
	}
	else if ( dataBufferSize == sizeof( KSDATAFORMAT_DSOUND ) )
	{
		pBuffer += sprintf( pBuffer, "  DataBufferSize: sizeof( KSDATAFORMAT_DSOUND )\n" );
	}
	else
	{
		pBuffer += sprintf( pBuffer, "  DataBufferSize: 0x%X\n", dataBufferSize );
	}

//DataFormat...
	if ( pDataFormat == NULL )
	{
		pBuffer += sprintf( pBuffer, "  pData: NULL\n" );
		pBuffer += sprintf( pBuffer, "  dataSize: (0x%X)\n", dataSize );
	}
	else
	{
	//Data size...
		if ( dataSize == sizeof( KSDATAFORMAT_WAVEFORMATEXTENSIBLE ) )
		{
			pBuffer += sprintf( pBuffer, "  dataSize: sizeof( KSDATAFORMAT_WAVEFORMATEXTENSIBLE )\n" );
		}
		else if ( dataSize == sizeof( KSDATAFORMAT_WAVEFORMATEX ) )
		{
			pBuffer += sprintf( pBuffer, "  dataSize: sizeof( KSDATAFORMAT_WAVEFORMATEX )\n" );
		}
		else if ( dataSize == sizeof( KSDATAFORMAT_DSOUND ) )
		{
			pBuffer += sprintf( pBuffer, "  dataSize: sizeof( KSDATAFORMAT_DSOUND )\n" );
		}
		else
		{
			pBuffer += sprintf( pBuffer, "  dataSize: ERROR (0x%X)\n", dataSize );
		}

	//pDataFormat...
		KSDATAFORMATS			dataFormat;
		GUID*					pGUID;
		unsigned long			typeDriver;

		dataFormat.MME_WDM = (PKSDATAFORMAT_WAVEFORMATEXTENSIBLE)pDataFormat;

		//Get client type ( 0 - MME, WDM; 1 - DX ).
		pGUID = &( pDescriptorDataRange->DataRange.Specifier );
		if ( (BOOL)TRUE == IsEqualGUID( *pGUID, KSDATAFORMAT_SPECIFIER_WAVEFORMATEX ) )
		{
		//MME, WDM
			typeDriver = DRV_MME_WDM;
		}
		else
		{
		//DX
			typeDriver = DRV_DX;
		}

		pBuffer = Logger_Driver_AVStream_Pin_Format_DataFormat( pBuffer, "dataFormat", dataFormat, typeDriver );
	}

	return;
}

void logger_c::Logger_Driver_AVStream_Pin_Format_SetDataFormat( IN char* buffer, IN void** ppVariables )
{
	KSDATAFORMATS				oldDataFormat;
	KSDATAFORMATS				connectionDataFormat;
	PKSDATARANGE_AUDIO			pAudioDataRange;
	GUID*						pGUID;
	unsigned long				typeDriver;

//Initialize.
	oldDataFormat.MME_WDM = (PKSDATAFORMAT_WAVEFORMATEXTENSIBLE)ppVariables[0];
	pAudioDataRange = (PKSDATARANGE_AUDIO)ppVariables[1];
	connectionDataFormat.MME_WDM = (PKSDATAFORMAT_WAVEFORMATEXTENSIBLE)ppVariables[2];

//Get client type ( 0 - MME, WDM; 1 - DX ).
	pGUID = &( pAudioDataRange->DataRange.Specifier );
	if ( (BOOL)TRUE == IsEqualGUID( *pGUID, KSDATAFORMAT_SPECIFIER_WAVEFORMATEX ) )
	{
	//MME, WDM
		typeDriver = DRV_MME_WDM;
	}
	else
	{
	//DX
		typeDriver = DRV_DX;
	}

//pOldFormat
	buffer = Logger_Driver_AVStream_Pin_Format_DataFormat( buffer, "oldFormat", oldDataFormat, typeDriver );

//pAudioDataRange
	buffer = Logger_Driver_AVStream_Pin_Format_DataRange( buffer, "dataRange", pAudioDataRange );

//pConnectionFormat
	buffer = Logger_Driver_AVStream_Pin_Format_DataFormat( buffer, "connectionFormat", connectionDataFormat, typeDriver );
	return;
}

char* logger_c::Logger_Driver_AVStream_Pin_Format_DataRange
(
	char*					pBuffer,
	char*					pDataRangeName,
	PKSDATARANGE_AUDIO		pDataRange
)
{
	unsigned long			formatSize;
	GUID*					pGUID;

//-----------------------------------------
//Create dataRange string.
	pBuffer += sprintf(
	  pBuffer,
	  "  %s ( KSDATARANGE_AUDIO ):\n",
	  pDataRangeName
	);
//DataRange.FormatSize
	formatSize = pDataRange->DataRange.FormatSize;
	if ( formatSize == sizeof( KSDATARANGE_AUDIO ) )
	{
		pBuffer += sprintf(
		  pBuffer,
		  "    DataRange.FormatSize: sizeof( KSDATARANGE_AUDIO )\n"
		);
	}
	else if ( formatSize == sizeof( KSDATARANGE ) )
	{
		pBuffer += sprintf(
		  pBuffer,
		  "    DataRange.FormatSize: sizeof( KSDATARANGE )\n"
		);
	}
	else
	{
		pBuffer += sprintf(
		  pBuffer,
		  "    DataRange.FormatSize: 0x%X -> ERROR\n",
		  formatSize
		);
	}
//DataRange.Flags
	pBuffer += sprintf(
	  pBuffer,
	  "    DataRange.Flags: 0x%X\n",
	  pDataRange->DataRange.Flags
	);
//DataRange.SampleSize
	pBuffer += sprintf(
	  pBuffer,
	  "    DataRange.SampleSize: 0x%X\n",
	  pDataRange->DataRange.SampleSize
	);
//DataRange.Reserved
	pBuffer += sprintf(
	  pBuffer,
	  "    DataRange.Reserved: 0x%X\n",
	  pDataRange->DataRange.Reserved
	);
//DataRange.MajorFormat
	pGUID = &( pDataRange->DataRange.MajorFormat );
	if ( (BOOL)TRUE == IsEqualGUID( *pGUID, KSDATAFORMAT_TYPE_AUDIO ) )
	{
		pBuffer += sprintf(
		  pBuffer,
		  "    DataRange.MajorFormat: KSDATAFORMAT_TYPE_AUDIO\n"
		);
	}
	else if ( (BOOL)TRUE == IsEqualGUID( *pGUID, KSDATAFORMAT_TYPE_WILDCARD ) )
	{
		pBuffer += sprintf(
		  pBuffer,
		  "    DataRange.MajorFormat: KSDATAFORMAT_TYPE_WILDCARD\n"
		);
	}
	else
	{
		pBuffer += sprintf(
		  pBuffer,
		  "    DataRange.MajorFormat: UNKNOWN (%4.4X-%2.2X-%2.2X-%2.2X-%1X%1X%1X%1X%1X%1X)\n",
		  pGUID->Data1,
		  pGUID->Data2,
		  pGUID->Data3,
		  pGUID->Data4[0],
		  pGUID->Data4[2],
		  pGUID->Data4[3],
		  pGUID->Data4[4],
		  pGUID->Data4[5],
		  pGUID->Data4[6],
		  pGUID->Data4[7]
		);
	}
//DataRange.SubFormat
	pGUID = &( pDataRange->DataRange.SubFormat );
	if ( (BOOL)TRUE == IsEqualGUID( *pGUID, KSDATAFORMAT_SUBTYPE_PCM ) )
	{
		pBuffer += sprintf(
		  pBuffer,
		  "    DataRange.SubFormat: KSDATAFORMAT_SUBTYPE_PCM\n"
		);
	}
	else if ( (BOOL)TRUE == IsEqualGUID( *pGUID, KSDATAFORMAT_SUBTYPE_WILDCARD ) )
	{
		pBuffer += sprintf(
		  pBuffer,
		  "    DataRange.SubFormat: KSDATAFORMAT_SUBTYPE_WILDCARD\n"
		);
	}
	else
	{
		pBuffer += sprintf(
		  pBuffer,
		  "    DataRange.SubFormat: UNKNOWN (%4.4X-%2.2X-%2.2X-%2.2X-%1X%1X%1X%1X%1X%1X)\n",
		  pGUID->Data1,
		  pGUID->Data2,
		  pGUID->Data3,
		  pGUID->Data4[0],
		  pGUID->Data4[2],
		  pGUID->Data4[3],
		  pGUID->Data4[4],
		  pGUID->Data4[5],
		  pGUID->Data4[6],
		  pGUID->Data4[7]
		);
	}
//DataRange.Specifier
	pGUID = &( pDataRange->DataRange.Specifier );
	if ( (BOOL)TRUE == IsEqualGUID( *pGUID, KSDATAFORMAT_SPECIFIER_WAVEFORMATEX ) )
	{
		pBuffer += sprintf(
		  pBuffer,
		  "    DataRange.Specifier: KSDATAFORMAT_SPECIFIER_WAVEFORMATEX\n"
		);
	}
	else if ( (BOOL)TRUE == IsEqualGUID( *pGUID, KSDATAFORMAT_SPECIFIER_DSOUND ) )
	{
		pBuffer += sprintf(
		  pBuffer,
		  "    DataRange.Specifier: KSDATAFORMAT_SPECIFIER_DSOUND\n"
		);
	}
	else if ( (BOOL)TRUE == IsEqualGUID( *pGUID, KSDATAFORMAT_SPECIFIER_WILDCARD ) )
	{
		pBuffer += sprintf(
		  pBuffer,
		  "    DataRange.Specifier: KSDATAFORMAT_SPECIFIER_WILDCARD\n"
		);
	}
	else
	{
		pBuffer += sprintf(
		  pBuffer,
		  "    DataRange.Specifier: UNKNOWN (%4.4X-%2.2X-%2.2X-%2.2X-%1X%1X%1X%1X%1X%1X)\n",
		  pGUID->Data1,
		  pGUID->Data2,
		  pGUID->Data3,
		  pGUID->Data4[0],
		  pGUID->Data4[2],
		  pGUID->Data4[3],
		  pGUID->Data4[4],
		  pGUID->Data4[5],
		  pGUID->Data4[6],
		  pGUID->Data4[7]
		);
	}

	if ( formatSize > sizeof( KSDATARANGE ) )
	{
	//MaximumChannels
		pBuffer += sprintf(
		  pBuffer,
		  "    DataRange.MaximumChannels: 0x%X\n",
		  pDataRange->MaximumChannels
		);
	//MinimumBitsPerSample
		pBuffer += sprintf(
		  pBuffer,
		  "    DataRange.MinimumBitsPerSample: %d\n",
		  pDataRange->MinimumBitsPerSample
		);
	//MaximumBitsPerSample
		pBuffer += sprintf(
		  pBuffer,
		  "    DataRange.MaximumBitsPerSample: %d\n",
		  pDataRange->MaximumBitsPerSample
		);
	//MinimumSampleFrequency
		pBuffer += sprintf(
		  pBuffer,
		  "    DataRange.MinimumSampleFrequency: %d\n",
		  pDataRange->MinimumSampleFrequency
		);
	//MaximumSampleFrequency
		pBuffer += sprintf(
		  pBuffer,
		  "    DataRange.MaximumSampleFrequency: %d\n",
		  pDataRange->MaximumSampleFrequency
		);
	}

	return pBuffer;
};

char* logger_c::Logger_Driver_AVStream_Pin_Format_DataFormat
(
	char*			pBuffer,
	char*			pFormatName,
	KSDATAFORMATS	dataFormat,
	unsigned long	typeDriver
)
{
	unsigned long		cbSize;
	unsigned long		dwChannelMask;
	unsigned long		formatSize;
	GUID*				pGUID;
	WORD				wFormatTag;


	if ( dataFormat.MME_WDM == NULL )
	{
		pBuffer += sprintf(
		  pBuffer,
		  "  %s: NULL\n",
		  pFormatName
		);
	}
	else
	{
		if ( typeDriver == DRV_MME_WDM )
		{
		//-----------------------------------------
		//DataRange.FormatSize
			formatSize = dataFormat.MME_WDM->DataFormat.FormatSize;
			if ( formatSize == sizeof( KSDATAFORMAT_WAVEFORMATEXTENSIBLE ) )
			{
			//Create Data Format string.
				pBuffer += sprintf(
				  pBuffer,
				  "  %s ( PKSDATAFORMAT_WAVEFORMATEXTENSIBLE ):\n",
				  pFormatName
				);
			//Format Size
				pBuffer += sprintf(
				  pBuffer,
				  "    dataFormat.FormatSize: sizeof( KSDATAFORMAT_WAVEFORMATEXTENSIBLE )\n"
				);
			}
			else if ( formatSize == sizeof( KSDATAFORMAT_WAVEFORMATEX ) )
			{
			//Create Data Format string.
				pBuffer += sprintf(
				  pBuffer,
				  "  %s ( PKSDATAFORMAT_WAVEFORMATEX ):\n",
				  pFormatName
				);
			//Format Size
				pBuffer += sprintf(
				  pBuffer,
				  "    dataFormat.FormatSize: sizeof( KSDATAFORMAT_WAVEFORMATEX )\n"
				);
			}
			else
			{
				pBuffer += sprintf(
				  pBuffer,
				  "    dataFormat.FormatSize: 0x%X -> ERROR\n",
				  formatSize
				);
			}
		//DataRange.Flags
			pBuffer += sprintf(
			  pBuffer,
			  "    dataFormat.Flags: 0x%X\n",
			  dataFormat.MME_WDM->DataFormat.Flags
			);
		//DataRange.SampleSize
			pBuffer += sprintf(
			  pBuffer,
			  "    dataFormat.SampleSize: 0x%X\n",
			  dataFormat.MME_WDM->DataFormat.SampleSize
			);
		//DataRange.Reserved
			pBuffer += sprintf(
			  pBuffer,
			  "    dataFormat.Reserved: 0x%X\n",
			  dataFormat.MME_WDM->DataFormat.Reserved
			);
		//DataRange.MajorFormat
			pGUID = &( dataFormat.MME_WDM->DataFormat.MajorFormat );
			if ( (BOOL)TRUE == IsEqualGUID( *pGUID, KSDATAFORMAT_TYPE_AUDIO ) )
			{
				pBuffer += sprintf(
				  pBuffer,
				  "    dataFormat.MajorFormat: KSDATAFORMAT_TYPE_AUDIO\n"
				);
			}
			else
			{
				pBuffer += sprintf(
				  pBuffer,
				  "    dataFormat.MajorFormat: UNKNOWN (%4.4X-%2.2X-%2.2X-%2.2X-%1X%1X%1X%1X%1X%1X)\n",
				  pGUID->Data1,
				  pGUID->Data2,
				  pGUID->Data3,
				  pGUID->Data4[0],
				  pGUID->Data4[2],
				  pGUID->Data4[3],
				  pGUID->Data4[4],
				  pGUID->Data4[5],
				  pGUID->Data4[6],
				  pGUID->Data4[7]
				);
			}
		//DataRange.SubFormat
			pGUID = &( dataFormat.MME_WDM->DataFormat.SubFormat );
			if ( (BOOL)TRUE == IsEqualGUID( *pGUID, KSDATAFORMAT_SUBTYPE_PCM ) )
			{
				pBuffer += sprintf(
				  pBuffer,
				  "    dataFormat.SubFormat: KSDATAFORMAT_SUBTYPE_PCM\n"
				);
			}
			else
			{
				pBuffer += sprintf(
				  pBuffer,
				  "    dataFormat.SubFormat: UNKNOWN (%4.4X-%2.2X-%2.2X-%2.2X-%1X%1X%1X%1X%1X%1X)\n",
				  pGUID->Data1,
				  pGUID->Data2,
				  pGUID->Data3,
				  pGUID->Data4[0],
				  pGUID->Data4[2],
				  pGUID->Data4[3],
				  pGUID->Data4[4],
				  pGUID->Data4[5],
				  pGUID->Data4[6],
				  pGUID->Data4[7]
				);
			}
		//DataRange.Specifier
			pGUID = &( dataFormat.MME_WDM->DataFormat.Specifier );
			if ( (BOOL)TRUE == IsEqualGUID( *pGUID, KSDATAFORMAT_SPECIFIER_WAVEFORMATEX ) )
			{
				pBuffer += sprintf(
				  pBuffer,
				  "    dataFormat.Specifier: KSDATAFORMAT_SPECIFIER_WAVEFORMATEX\n"
				);
			}
			else
			{
				pBuffer += sprintf(
				  pBuffer,
				  "    dataFormat.Specifier: UNKNOWN (%4.4X-%2.2X-%2.2X-%2.2X-%1X%1X%1X%1X%1X%1X)\n",
				  pGUID->Data1,
				  pGUID->Data2,
				  pGUID->Data3,
				  pGUID->Data4[0],
				  pGUID->Data4[2],
				  pGUID->Data4[3],
				  pGUID->Data4[4],
				  pGUID->Data4[5],
				  pGUID->Data4[6],
				  pGUID->Data4[7]
				);
			}
		//Format.wFormatTag
			wFormatTag = dataFormat.MME_WDM->WaveFormatEx.Format.wFormatTag;
			if ( wFormatTag == WAVE_FORMAT_EXTENSIBLE )
			{
				pBuffer += sprintf(
				  pBuffer,
				  "    Format.wFormatTag: WAVE_FORMAT_EXTENSIBLE\n"
				);
			}
			else if ( wFormatTag == WAVE_FORMAT_PCM )
			{
				pBuffer += sprintf(
				  pBuffer,
				  "    Format.wFormatTag: WAVE_FORMAT_PCM\n"
				);
			}
			else
			{
				pBuffer += sprintf(
				  pBuffer,
				  "    Format.wFormatTag: ERROR (0x%X)\n",
				  wFormatTag
				);
			}
		//Format.nChannels
			pBuffer += sprintf(
			  pBuffer,
			  "    Format.nChannels: %d\n",
			  dataFormat.MME_WDM->WaveFormatEx.Format.nChannels
			);
		//Format.nSamplesPerSec
			pBuffer += sprintf(
			  pBuffer,
			  "    Format.nSamplesPerSec: %d\n",
			  dataFormat.MME_WDM->WaveFormatEx.Format.nSamplesPerSec
			);
		//Format.nAvgBytesPerSec
			pBuffer += sprintf(
			  pBuffer,
			  "    Format.nAvgBytesPerSec: %d\n",
			  dataFormat.MME_WDM->WaveFormatEx.Format.nAvgBytesPerSec
			);
		//Format.nBlockAlign
			pBuffer += sprintf(
			  pBuffer,
			  "    Format.nBlockAlign: %d\n",
			  dataFormat.MME_WDM->WaveFormatEx.Format.nBlockAlign
			);
		//Format.wBitsPerSample
			pBuffer += sprintf(
			  pBuffer,
			  "    Format.wBitsPerSample: %d\n",
			  dataFormat.MME_WDM->WaveFormatEx.Format.wBitsPerSample
			);
		//Format.cbSize
			cbSize = dataFormat.MME_WDM->WaveFormatEx.Format.cbSize;
			if (
			  cbSize
			  ==
			  ( sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX) )
			)
			{
				pBuffer += sprintf(
				  pBuffer,
				  "    Format.cbSize: sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX)\n"
				);
			}
			else
			{
			//Check if we have KSDATAFORMAT_WAVEFORMATEX.
				if (
				  ( formatSize == sizeof( KSDATAFORMAT_WAVEFORMATEX ) )
				  &&
				  ( cbSize == 0 )
				)
				{
					
					pBuffer += sprintf(
					  pBuffer,
					  "    Format.cbSize: 0\n"
					);
				}
				else
				{
					pBuffer += sprintf(
					  pBuffer,
					  "    Format.cbSize: ERROR (%d)\n",
					  cbSize
					);
				}
			}

			if ( cbSize != 0 )
			{
			//Format.wValidBitesPerSample
				pBuffer += sprintf(
				  pBuffer,
				  "    Format.wValidBitesPerSample: %d\n",
				  dataFormat.MME_WDM->WaveFormatEx.Samples.wValidBitsPerSample
				);
			//Format.dwChannelMask
			//
			//WALDEMAR
			//If support for separate channels will be set then we have to fix this part.
				dwChannelMask = dataFormat.MME_WDM->WaveFormatEx.dwChannelMask;
				switch ( dwChannelMask )
				{
				case KSAUDIO_SPEAKER_STEREO :
					pBuffer += sprintf(
					  pBuffer,
					  "    Format.dwChannelMask: KSAUDIO_SPEAKER_STEREO\n"
					);
					break;

				case KSAUDIO_SPEAKER_QUAD :
					pBuffer += sprintf(
					  pBuffer,
					  "    Format.dwChannelMask: KSAUDIO_SPEAKER_QUAD\n"
					);
					break;

				case KSAUDIO_SPEAKER_5POINT1 :
					pBuffer += sprintf(
					  pBuffer,
					  "    Format.dwChannelMask: KSAUDIO_SPEAKER_5POINT1\n"
					);
					break;

				case KSAUDIO_SPEAKER_7POINT1 :
					pBuffer += sprintf(
					  pBuffer,
					  "    Format.dwChannelMask: KSAUDIO_SPEAKER_7POINT1\n"
					);
					break;

				default:
					pBuffer += sprintf(
					  pBuffer,
					  "    Format.dwChannelMask: ERROR (0x%X)\n",
					  dwChannelMask
					);
					break;
				}

			//Format.SubFormat
				pGUID = &( dataFormat.MME_WDM->WaveFormatEx.SubFormat );
				if ( (BOOL)TRUE == IsEqualGUID( *pGUID, KSDATAFORMAT_SUBTYPE_PCM ) )
				{
					pBuffer += sprintf(
					  pBuffer,
					  "    Format.SubFormat: KSDATAFORMAT_SUBTYPE_PCM\n"
					);
				}
				else
				{
					pBuffer += sprintf(
					  pBuffer,
					  "    Format.SubFormat: UNKNOWN (%4.4X-%2.2X-%2.2X-%2.2X-%1X%1X%1X%1X%1X%1X)\n",
					  pGUID->Data1,
					  pGUID->Data2,
					  pGUID->Data3,
					  pGUID->Data4[0],
					  pGUID->Data4[2],
					  pGUID->Data4[3],
					  pGUID->Data4[4],
					  pGUID->Data4[5],
					  pGUID->Data4[6],
					  pGUID->Data4[7]
					);
				}
			}
		}
		else
		{
		//DX
			pBuffer += sprintf(
			  pBuffer,
			  "  dataFormat ( PKSDATAFORMAT_DSOUND ):\n"
			);
			pBuffer += sprintf(
			  pBuffer,
			  "    Not written yet.\n"
			);
		}
	}

	return pBuffer;
}

#endif //DBG_AND_LOGGER
