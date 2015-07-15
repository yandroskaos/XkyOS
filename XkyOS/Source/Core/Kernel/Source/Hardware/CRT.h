/******************************************************************************/
/**
* @file		CRT.h
* @brief	XkyOS Hardware Text Screen Library
* Definitions of text-bases screeen operations and types
* 
* @date		20/03/2008
* @author	Pablo Bravo
*/
/******************************************************************************/
#ifndef __CRT_H__
#define __CRT_H__

	bool	CRT_Init();

	void	CRT_Print	(IN string* s, IN byte color, IN byte background);
	void	CRT_Print32	(IN dword data);
	void	CRT_NewLine	();
	
	void	CRT_SetCursor	(IN dword x, IN dword y);
	dword	CRT_X			();
	dword	CRT_Y			();

#endif //__CRT_H__
