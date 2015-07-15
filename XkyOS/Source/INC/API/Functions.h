/******************************************************************************/
/**
* @file		Functions.h
* @brief	XkyOS Services Index
* Indexes of the services (functions) offered by XkyOS to user and kernel-mode programs.
* 
* @date		20/03/2008
* @author	Pablo Bravo
*/
/******************************************************************************/
#ifndef __FUNCTIONS_H__
#define __FUNCTIONS_H__

/**
* @brief System Service Interrupt
*/
#define OS_API_SERVICES 0x80

//Memory
#define IDX_XKY_MEMORY_START	0x10
#define IDX_XKY_ADDRESS_SPACE_Alloc			(IDX_XKY_MEMORY_START + 1) /**< XKY_ADDRESS_SPACE_Alloc Index*/
#define IDX_XKY_ADDRESS_SPACE_Free			(IDX_XKY_MEMORY_START + 2) /**< XKY_ADDRESS_SPACE_Free Index*/
#define IDX_XKY_ADDRESS_SPACE_GetCurrent	(IDX_XKY_MEMORY_START + 3) /**< XKY_ADDRESS_SPACE_GetCurrent Index*/
#define IDX_XKY_ADDRESS_SPACE_SwitchTo		(IDX_XKY_MEMORY_START + 4) /**< XKY_ADDRESS_SPACE_SwitchTo Index*/
#define IDX_XKY_PAGE_Alloc					(IDX_XKY_MEMORY_START + 5) /**< XKY_PAGE_Alloc Index*/
#define IDX_XKY_PAGE_Free					(IDX_XKY_MEMORY_START + 6) /**< XKY_PAGE_Free Index*/
#define IDX_XKY_PAGE_Share					(IDX_XKY_MEMORY_START + 7) /**< XKY_PAGE_Share Index*/

//Disk
#define IDX_XKY_DISK_START	0x20
#define IDX_XKY_DISK_Alloc	(IDX_XKY_DISK_START + 1) /**< XKY_DISK_Alloc Index*/
#define IDX_XKY_DISK_Free	(IDX_XKY_DISK_START + 2) /**< XKY_DISK_Free Index*/
#define IDX_XKY_DISK_Read	(IDX_XKY_DISK_START + 3) /**< XKY_DISK_Read Index*/
#define IDX_XKY_DISK_Write	(IDX_XKY_DISK_START + 4) /**< XKY_DISK_Write Index*/

//Windows
#define IDX_XKY_WINDOW_START	0x30
#define IDX_XKY_WINDOW_Alloc			(IDX_XKY_WINDOW_START + 1) /**< XKY_WINDOW_Alloc Index*/
#define IDX_XKY_WINDOW_Free				(IDX_XKY_WINDOW_START + 2) /**< XKY_WINDOW_Free Index*/
#define IDX_XKY_WINDOW_GetHeight		(IDX_XKY_WINDOW_START + 3) /**< XKY_WINDOW_GetHeight Index*/
#define IDX_XKY_WINDOW_GetWidth			(IDX_XKY_WINDOW_START + 4) /**< XKY_WINDOW_GetWidth Index*/
#define IDX_XKY_WINDOW_GetPixel			(IDX_XKY_WINDOW_START + 5) /**< XKY_WINDOW_GetPixel Index*/
#define IDX_XKY_WINDOW_SetPixel			(IDX_XKY_WINDOW_START + 6) /**< XKY_WINDOW_SetPixel Index*/
#define IDX_XKY_WINDOW_PrintText		(IDX_XKY_WINDOW_START + 7) /**< XKY_WINDOW_PrintText Index*/
#define IDX_XKY_WINDOW_RegisterKeyboard	(IDX_XKY_WINDOW_START + 8) /**< XKY_WINDOW_RegisterKeyboard Index*/
#define IDX_XKY_WINDOW_RegisterMouse	(IDX_XKY_WINDOW_START + 9) /**< XKY_WINDOW_RegisterMouse Index*/
#define IDX_XKY_POINTER_GetArrow		(IDX_XKY_WINDOW_START + 10) /**< XKY_POINTER_GetArrow Index*/
#define IDX_XKY_POINTER_GetClock		(IDX_XKY_WINDOW_START + 11) /**< XKY_POINTER_GetClock Index*/
#define IDX_XKY_WINDOW_SetPointer		(IDX_XKY_WINDOW_START + 12) /**< XKY_WINDOW_SetPointer Index*/
#define IDX_XKY_WINDOW_SetPointerColor	(IDX_XKY_WINDOW_START + 13) /**< XKY_WINDOW_SetPointerColor Index*/

//PCI
#define IDX_XKY_PCI_START	0x40
#define IDX_XKY_PCI_Alloc						(IDX_XKY_PCI_START + 1) /**< XKY_PCI_Alloc Index*/
#define IDX_XKY_PCI_Free						(IDX_XKY_PCI_START + 2) /**< XKY_PCI_Free Index*/
#define IDX_XKY_PCI_GetNumberOfDevices			(IDX_XKY_PCI_START + 3) /**< XKY_PCI_GetNumberOfDevices Index*/
#define IDX_XKY_PCI_GetDevice					(IDX_XKY_PCI_START + 4) /**< XKY_PCI_GetDevice Index*/
#define IDX_XKY_PCI_GetBusFromDirection			(IDX_XKY_PCI_START + 5) /**< XKY_PCI_GetBusFromDirection Index*/
#define IDX_XKY_PCI_GetDeviceFromDirection		(IDX_XKY_PCI_START + 6) /**< XKY_PCI_GetDeviceFromDirection Index*/
#define IDX_XKY_PCI_GetFunctionFromDirection	(IDX_XKY_PCI_START + 7) /**< XKY_PCI_GetFunctionFromDirection Index*/
#define IDX_XKY_PCI_ReadDword					(IDX_XKY_PCI_START + 8) /**< XKY_PCI_ReadDword Index*/
#define IDX_XKY_PCI_WriteDword					(IDX_XKY_PCI_START + 9) /**< XKY_PCI_WriteDword Index*/

//CPU
#define IDX_XKY_CPU_START	0x50
#define IDX_XKY_CPU_FillExecution		(IDX_XKY_CPU_START + 1) /**< XKY_CPU_FillExecution Index*/
#define IDX_XKY_CPU_Alloc				(IDX_XKY_CPU_START + 2) /**< XKY_CPU_Alloc Index*/
#define IDX_XKY_CPU_AllocCode			(IDX_XKY_CPU_START + 3) /**< XKY_CPU_AllocCode Index*/
#define IDX_XKY_CPU_Free				(IDX_XKY_CPU_START + 4) /**< XKY_CPU_Free Index*/
#define IDX_XKY_CPU_RegisterCallback	(IDX_XKY_CPU_START + 5) /**< XKY_CPU_RegisterCallback Index*/
#define IDX_XKY_CPU_GetCurrent			(IDX_XKY_CPU_START + 6) /**< XKY_CPU_GetCurrent Index*/

//RTC
#define IDX_XKY_RTC_START	0x60
#define IDX_XKY_RTC_Seconds		(IDX_XKY_RTC_START + 1) /**< XKY_RTC_Seconds Index*/
#define IDX_XKY_RTC_Minutes		(IDX_XKY_RTC_START + 2) /**< XKY_RTC_Minutes Index*/
#define IDX_XKY_RTC_Hour		(IDX_XKY_RTC_START + 3) /**< XKY_RTC_Hour Index*/
#define IDX_XKY_RTC_DayOfWeek	(IDX_XKY_RTC_START + 4) /**< XKY_RTC_DayOfWeek Index*/
#define IDX_XKY_RTC_DayOfMonth	(IDX_XKY_RTC_START + 5) /**< XKY_RTC_DayOfMonth Index*/
#define IDX_XKY_RTC_Month		(IDX_XKY_RTC_START + 6) /**< XKY_RTC_Month Index*/
#define IDX_XKY_RTC_Year		(IDX_XKY_RTC_START + 7) /**< XKY_RTC_Year Index*/

//TMR
#define IDX_XKY_TMR_START	0x70
#define IDX_XKY_TMR_GetTicks		(IDX_XKY_TMR_START + 1) /**< XKY_TMR_GetTicks Index*/

//LDR
#define IDX_XKY_LDR_START	0x80
#define IDX_XKY_LDR_GetProcedureAddress	(IDX_XKY_LDR_START + 1) /**< XKY_LDR_GetProcedureAddress Index*/
#define IDX_XKY_LDR_LoadUserModule		(IDX_XKY_LDR_START + 2) /**< XKY_LDR_LoadUserModule Index*/
#define IDX_XKY_LDR_LoadKernelModule	(IDX_XKY_LDR_START + 3) /**< XKY_LDR_LoadKernelModule Index*/
#define IDX_XKY_LDR_LoadFile			(IDX_XKY_LDR_START + 4) /**< XKY_LDR_LoadFile Index*/
#define IDX_XKY_LDR_FileSize			(IDX_XKY_LDR_START + 5) /**< XKY_LDR_FileSize Index*/

//OS
#define IDX_XKY_OS_START	0x90
#define IDX_XKY_OS_Start		(IDX_XKY_OS_START + 1) /**< XKY_OS_Start Index*/
#define IDX_XKY_OS_Finish		(IDX_XKY_OS_START + 2) /**< XKY_OS_Finish Index*/

//EXCEPTION
#define IDX_XKY_EXCEPTION_START	0xA0
#define IDX_XKY_EXCEPTION_SetHandler		(IDX_XKY_EXCEPTION_START + 1) /**< XKY_EXCEPTION_SetHandler Index*/
#define IDX_XKY_EXCEPTION_UnsetHandler		(IDX_XKY_EXCEPTION_START + 2) /**< XKY_EXCEPTION_UnsetHandler Index*/
#define IDX_XKY_INTERRUPT_SetHandler		(IDX_XKY_EXCEPTION_START + 3) /**< XKY_INTERRUPT_SetHandler Index*/
#define IDX_XKY_INTERRUPT_UnsetHandler		(IDX_XKY_EXCEPTION_START + 4) /**< XKY_INTERRUPT_UnsetHandler Index*/

//DEBUG
#define IDX_XKY_DEBUG_START	0xB0
#define IDX_XKY_DEBUG_Message	(IDX_XKY_DEBUG_START + 1) /**< XKY_DEBUG_Message Index*/
#define IDX_XKY_DEBUG_Data		(IDX_XKY_DEBUG_START + 2) /**< XKY_DEBUG_Data Index*/

#endif //__FUNCTIONS_H__
