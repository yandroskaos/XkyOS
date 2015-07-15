/******************************************************************************/
/**
* @file		DMA.h
* @brief	XkyOS Hardware DMA Library
* Definitions of DMA operations and types
* 
* @date		20/03/2008
* @author	Pablo Bravo
*/
/******************************************************************************/
#ifndef __DMA_H__
#define __DMA_H__

	typedef dword DMA;

	void EnableController(int controller);
	void DisableController(int controller);
	int OpenChannel(int channel, void *address, int count, int mode);
	int CloseChannel(int channel);

#endif //__DMA_H__
