/******************************************************************************/
/**
* @file		Tetris.cpp
* @brief	Tetris game
*
* @date		14/09/2008
* @author	Pablo Bravo García
*/
/******************************************************************************/
#include "Types.h"
#include "Executable.h"

//=================================IMPORTS====================================//
#pragma data_seg(".imports")
//============================================================================//
#include "API.h"

//==================================CODE======================================//
#pragma code_seg(".code")
//============================================================================//
#include "Figura.h"
#include "Tablero.h"
#include "FiguraS.h"
#include "FiguraZ.h"
#include "FiguraL.h"
#include "Figura7.h"
#include "FiguraT.h"
#include "FiguraI.h"
#include "FiguraCuadrado.h"

//==================================DATA======================================//
#pragma data_seg(".data")
//============================================================================//
Tablero tablero;
Figura figura;

dword next = 0;
dword timer = 0;

WINDOW window;

//==================================CODE======================================//
#pragma code_seg(".code")
//============================================================================//
PRIVATE dword Random(IN dword _limit)
{
	next = next * 1103515245 + 12345;
	return((dword)(next/65536) % _limit);
}

PRIVATE void SetSeed(IN dword _seed)
{
	next = _seed;
}

PRIVATE void Figura_New(Figura* f)
{
	dword r = Random(7);
	switch(r)
	{
	case 0: FiguraCuadrado_Init(f);break;
	case 1: FiguraL_Init(f);break;
	case 2: Figura7_Init(f);break;
	case 3: FiguraS_Init(f);break;
	case 4: FiguraZ_Init(f);break;
	case 5: FiguraT_Init(f);break;
	case 6: FiguraI_Init(f);break;
	}
}

void Tablero_QuitarFigura(Tablero* t, Figura* f)
{
	for(dword i=0; i<FIG_COLUMNS; i++)
	{
		for(dword j=0; j<FIG_ROWS; j++)
		{
			if(f->grid[i][j])
			{
				t->grid[f->position.x + i][f->position.y + j] = t->background;
			}
		}
	}
}

void PintaCuadrado(dword x, dword y, ARGB color)
{
#define PIXELS	16
#define START_X 10
#define START_Y	10

	for(dword i=0;i<PIXELS;i++)
	{
		for(dword j=0;j<PIXELS;j++)
		{
			XKY_WINDOW_SetPixel(window, START_X + x*PIXELS + i, START_Y + y*PIXELS + j, color);
		}
	}
}

PRIVATE void Tablero_Pintar(Tablero* t)
{
	for(dword j=0; j<TAB_ROWS; j++)
	{
		for(dword i=0; i<TAB_COLUMNS; i++)
		{
			PintaCuadrado(i, TAB_ROWS - j, t->grid[i][j]);
		}
	}
}

PRIVATE bool TimerCallback(IN XID _xid, IN EXECUTION* _execution)
{
	timer++;
	if(timer==4)
	{
		timer=0;
		if(!figura.Bajar(&figura, &tablero))
		{
			Tablero_AnyadirFigura(&tablero, &figura);
			Tablero_CheckLines(&tablero);
			Figura_New(&figura);
		}

		Tablero_AnyadirFigura(&tablero, &figura);
		Tablero_Pintar(&tablero);
		Tablero_QuitarFigura(&tablero, &figura);
	}

	return false;
}

//KEYBOARD defines
#define VK_ARROW_UP		0x0000001F	//S
#define VK_ARROW_DOWN	0x0000002D	//X
#define VK_ARROW_LEFT	0x0000002C	//Z
#define VK_ARROW_RIGHT	0x0000002E	//C

#define IS_DEPRESSED(X)		((X)&0x00000080)
#define PURE_SCANCODE(X)	((X)&0xFFFFFF7F)

void KeyboardCallback(IN dword _scan_code)
{
	if(_scan_code == PURE_SCANCODE(VK_ARROW_UP))
	{
		if(!IS_DEPRESSED(_scan_code))
		{
			figura.Girar(&figura, &tablero);
		}
	}
	else if(_scan_code == PURE_SCANCODE(VK_ARROW_LEFT))
	{
		if(!IS_DEPRESSED(_scan_code))
		{
			figura.Izquierda(&figura, &tablero);
		}
	}
	else if(_scan_code == PURE_SCANCODE(VK_ARROW_RIGHT))
	{
		if(!IS_DEPRESSED(_scan_code))
		{
			figura.Derecha(&figura, &tablero);
		}
	}
	else if(_scan_code == PURE_SCANCODE(VK_ARROW_DOWN))
	{
		if(!IS_DEPRESSED(_scan_code))
		{
			if(!figura.Bajar(&figura, &tablero))
			{
				Tablero_AnyadirFigura(&tablero, &figura);
				Tablero_CheckLines(&tablero);
				Figura_New(&figura);
			}
		}
	}

	Tablero_AnyadirFigura(&tablero, &figura);
	Tablero_Pintar(&tablero);
	Tablero_QuitarFigura(&tablero, &figura);
}

PUBLIC void Main()
{
	//Alloc window
	window = XKY_WINDOW_Alloc();
	if(!window)	goto _Finish;

	//Set random seed
	SetSeed(XKY_RTC_Seconds() + XKY_TMR_GetTicks());

	//Inciamos el tablero
	Tablero_Init(&tablero);

	//Iniciamos la figura
	Figura_New(&figura);
	
	//Registramos el keyboard
	XKY_WINDOW_RegisterKeyboard(window, XKY_ADDRESS_SPACE_GetCurrent(), KeyboardCallback);

	//Timer
	XKY_CPU_RegisterCallback(XKY_ADDRESS_SPACE_GetCurrent(), TimerCallback);

	//Done
	for(;;);

	//Done
_Finish:
	XKY_OS_Finish();
}

//=================================EXPORTS====================================//
#pragma data_seg(".exports")
//============================================================================//

//=================================MODULE=====================================//
#pragma data_seg(".module")
//============================================================================//
	MODULE(IMAGE_MODE_USER, IMAGE_KIND_MODULE, IMAGE_VERSION(1,0,0,0), 0, Main, 0);
