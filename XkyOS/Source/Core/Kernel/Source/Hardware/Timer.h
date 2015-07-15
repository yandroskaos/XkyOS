/******************************************************************************/
/**
* @file		Timer.h
* @brief	XkyOS Hardware Timer Library
* Definitions of timer operations and types
* 
* @date		20/03/2008
* @author	Pablo Bravo
*/
/******************************************************************************/
#ifndef __TIMER_H__
#define __TIMER_H__

	bool	TMR_Init();

	dword	TMR_Ticks();
	//dword	TMR_Count(IN byte _timer);
	//bool	TMR_Setup(IN byte _timer, IN byte _mode, IN word _count);

#endif //__TIMER_H__
