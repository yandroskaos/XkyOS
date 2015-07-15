#ifndef __FIGURAI_H__
#define __FIGURAI_H__

//Figura I
void FiguraI_State1(Figura* f, bool activate)
{
	f->grid[0][0] = activate;
	f->grid[1][0] = activate;
	f->grid[2][0] = activate;
	f->grid[3][0] = activate;
}

bool FiguraI_CheckState1(Figura* f, Tablero* t)
{
	dword x = f->position.x;
	dword y = f->position.y;

	if(!Tablero_CheckEmpty(t, x+0, y+0)) return false;
	if(!Tablero_CheckEmpty(t, x+1, y+0)) return false;
	if(!Tablero_CheckEmpty(t, x+2, y+0)) return false;
	if(!Tablero_CheckEmpty(t, x+3, y+0)) return false;
	return true;
}

void FiguraI_State2(Figura* f, bool activate)
{
	f->grid[0][0] = activate;
	f->grid[0][1] = activate;
	f->grid[0][2] = activate;
	f->grid[0][3] = activate;
}

bool FiguraI_CheckState2(Figura* f, Tablero* t)
{
	dword x = f->position.x;
	dword y = f->position.y;

	if(!Tablero_CheckEmpty(t, x+0, y+0)) return false;
	if(!Tablero_CheckEmpty(t, x+0, y+1)) return false;
	if(!Tablero_CheckEmpty(t, x+0, y+2)) return false;
	if(!Tablero_CheckEmpty(t, x+0, y+3)) return false;
	return true;
}

bool FiguraI_Girar(Figura* f, Tablero* t)
{
	switch(f->state)
	{
	case ST1:
		{
			if(!FiguraI_CheckState2(f, t)) return false;
			FiguraI_State1(f, false);
			f->state = ST2;
			FiguraI_State2(f, true);
			break;
		}
	case ST2:
		{
			if(!FiguraI_CheckState1(f, t)) return false;
			FiguraI_State2(f, false);
			f->state = ST1;
			FiguraI_State1(f, true);
			break;
		}
	}
	return true;
}

void FiguraI_Init(Figura* f)
{
	Figura_Init(f);

	f->color = SRGB(255, 0, 0);

	FiguraI_State1(f, true);

	f->Girar = FiguraI_Girar;
}


#endif
