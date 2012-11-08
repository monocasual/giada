//-------------------------------------------------------------------------------------------------------
// VST Plug-Ins SDK
// Version 2.4		$Date: 2006/02/09 11:05:51 $
//
// Category     : VST 2.x Interfaces
// Filename     : vstfxstore.h
// Created by   : Steinberg Media Technologies
// Description  : Definition of Program (fxp) and Bank (fxb) structures
//
// © 2006, Steinberg Media Technologies, All Rights Reserved
//-------------------------------------------------------------------------------------------------------

#ifndef __vstfxstore__
#define __vstfxstore__

#ifndef __aeffect__
#include "aeffect.h"
#endif

//-------------------------------------------------------------------------------------------------------
/** Root chunk identifier for Programs (fxp) and Banks (fxb). */
#define cMagic				'CcnK'

/** Regular Program (fxp) identifier. */
#define fMagic				'FxCk'

/** Regular Bank (fxb) identifier. */
#define bankMagic			'FxBk'

/** Program (fxp) identifier for opaque chunk data. */
#define chunkPresetMagic	'FPCh'

/** Bank (fxb) identifier for opaque chunk data. */
#define chunkBankMagic		'FBCh'

/* 
	Note: The C data structures below are for illustration only. You can not read/write them directly.
	The byte order on disk of fxp and fxb files is Big Endian. You have to swap integer
	and floating-point values on Little Endian platforms (Windows, MacIntel)!
*/

//-------------------------------------------------------------------------------------------------------
/** Program (fxp) structure. */
//-------------------------------------------------------------------------------------------------------
struct fxProgram
{
//-------------------------------------------------------------------------------------------------------
	VstInt32 chunkMagic;		///< 'CcnK'
	VstInt32 byteSize;			///< size of this chunk, excl. magic + byteSize

	VstInt32 fxMagic;			///< 'FxCk' (regular) or 'FPCh' (opaque chunk)
	VstInt32 version;			///< format version (currently 1)
	VstInt32 fxID;				///< fx unique ID
	VstInt32 fxVersion;			///< fx version

	VstInt32 numParams;			///< number of parameters
	char prgName[28];			///< program name (null-terminated ASCII string)

	union
	{
		float params[1];		///< variable sized array with parameter values
		struct 
		{
			VstInt32 size;		///< size of program data
			char chunk[1];		///< variable sized array with opaque program data
		} data;					///< program chunk data
	} content;					///< program content depending on fxMagic
//-------------------------------------------------------------------------------------------------------
};

//-------------------------------------------------------------------------------------------------------
/** Bank (fxb) structure. */
//-------------------------------------------------------------------------------------------------------
struct fxBank
{
//-------------------------------------------------------------------------------------------------------
	VstInt32 chunkMagic;		///< 'CcnK'
	VstInt32 byteSize;			///< size of this chunk, excl. magic + byteSize

	VstInt32 fxMagic;			///< 'FxBk' (regular) or 'FBCh' (opaque chunk)
	VstInt32 version;			///< format version (1 or 2)
	VstInt32 fxID;				///< fx unique ID
	VstInt32 fxVersion;			///< fx version

	VstInt32 numPrograms;		///< number of programs

#if VST_2_4_EXTENSIONS
	VstInt32 currentProgram;	///< version 2: current program number
	char future[124];			///< reserved, should be zero
#else
	char future[128];			///< reserved, should be zero
#endif

	union
	{
		fxProgram programs[1];	///< variable number of programs
		struct
		{
			VstInt32 size;		///< size of bank data
			char chunk[1];		///< variable sized array with opaque bank data
		} data;					///< bank chunk data
	} content;					///< bank content depending on fxMagic
//-------------------------------------------------------------------------------------------------------
};

#endif // __vstfxstore__
