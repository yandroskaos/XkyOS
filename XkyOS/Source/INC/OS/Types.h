/******************************************************************************/
/**
* @file		Types.h
* @brief	XkyOS basic types and definitions
* This file contains the basic types used in XkyOS an other useful definitions
* 
* @date		20/03/2008
* @author	Pablo Bravo
*/
/******************************************************************************/
#ifndef __TYPES_H__
#define __TYPES_H__

#pragma warning(disable:4200)
#pragma warning(disable:4201)
#pragma warning(disable:4100)

/**
* @brief Basic type 1 byte
*/
typedef unsigned __int8		byte;
/**
* @brief Basic type 2 bytes
*/
typedef unsigned __int16	word;
/**
* @brief Basic type 4 bytes
*/
typedef unsigned __int32	dword;
/**
* @brief Basic type 8 bytes
*/
typedef unsigned __int64	qword;


/**
* @brief Naked functions are funcitons without stack frame
*/
#define NAKED		__declspec(naked)
/**
* @brief For alignment issues in the image
*/
#define ALIGN(X)	__declspec(align(X))


/**
* @brief Visibility scope limited to module
*/
#define PRIVATE static
/**
* @brief Visibility scope unlimited
*/
#define PUBLIC

/**
* @brief Parameter is an input parameter
*/
#define IN
/**
* @brief Parameter is an output parameter
*/
#define OUT

/**
* @brief Basic string definition
*/
struct string
{
	byte size;
	byte text[];
};

/**
* @brief Complete declaration of a string.
*/
#define DECLARE_STRING(X)			string X##_str = {sizeof(#X) - 1, #X}
/**
* @brief Initialization of a string with a size.
*/
#define BOUNDED_STRING(S,X,I)		byte S##_size = (I); byte S##_text[(X)] = {0}; string* S = (string*)&S##_size
/**
* @brief Initialization of a string with a const char*.
*/
#define STRING(X)					{sizeof(X) - 1, X}
/**
* @brief Definition of a binary-compatible string with bound size. 
*/
#define DEFINE_BOUNDED_STRING(F,S)	byte F##_size; byte F##_text[S]

/**
* @brief Definition of a function for an inclusion file. 
*/
//#define FUNCTION(X) extern f##X X
/**
* @brief Definition of a function pointer for an implementation file. 
*/
//#define FUNCTION_POINTER(X) f##X X = 0

#endif //__TYPES_H__
