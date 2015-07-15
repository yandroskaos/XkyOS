#ifndef __FIGURAS_H__
#define __FIGURAS_H__

//Figura S
void FiguraS_State1(Figura* f, bool activate)
{
	f->grid[0][1] = activate;
	f->grid[1][0] = activate;
	f->grid[1][1] = activate;
	f->grid[2][0] = activate;
}

bool FiguraS_CheckState1(Figura* f, Tablero* t)
{
	dword x = f->position.x;
	dword y = f->position.y;
	
	if(!Tablero_CheckEmpty(t, x+0, y+1)) return false;
	if(!Tablero_CheckEmpty(t, x+1, y+0)) return false;
	if(!Tablero_CheckEmpty(t, x+1, y+1)) return false;
	if(!Tablero_CheckEmpty(t, x+2, y+0)) return false;
	return true;
}

void FiguraS_State2(Figura* f, bool activate)
{
	f->grid[0][0] = activate;
	f->grid[0][1] = activate;
	f->grid[1][1] = activate;
	f->grid[1][2] = activate;
}

bool FiguraS_CheckState2(Figura* f, Tablero* t)
{
	dword x = f->position.x;
	dword y = f->position.y;

	if(!Tablero_CheckEmpty(t, x+0, y+0)) return false;
	if(!Tablero_CheckEmpty(t, x+0, y+1)) return false;
	if(!Tablero_CheckEmpty(t, x+1, y+1)) return false;
	if(!Tablero_CheckEmpty(t, x+1, y+2)) return false;
	return true;
}

bool FiguraS_Girar(Figura* f, Tablero* t)
{
	switch(f->state)
	{
	case ST1:
		{
			if(!FiguraS_CheckState2(f, t)) return false;
			FiguraS_State1(f, false);
			f->state = ST2;
			FiguraS_State2(f, true);
			break;
		}
	case ST2:
		{
			if(!FiguraS_CheckState1(f, t)) return false;
			FiguraS_State2(f, false);
			f->state = ST1;
			FiguraS_State1(f, true);
			break;
		}
	}
	return true;
}

void FiguraS_Init(Figura* f)
{
	Figura_Init(f);

	f->color = SRGB(0, 0, 255);

	FiguraS_State1(f, true);

	f->Girar = FiguraS_Girar;
}

#endif
