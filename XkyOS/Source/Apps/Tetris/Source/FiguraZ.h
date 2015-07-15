#ifndef __FIGURAZ_H__
#define __FIGURAZ_H__

//Figura Z
void FiguraZ_State1(Figura* f, bool activate)
{
	f->grid[0][0] = activate;
	f->grid[1][0] = activate;
	f->grid[1][1] = activate;
	f->grid[2][1] = activate;
}

bool FiguraZ_CheckState1(Figura* f, Tablero* t)
{
	dword x = f->position.x;
	dword y = f->position.y;

	if(!Tablero_CheckEmpty(t, x+0, y+0)) return false;
	if(!Tablero_CheckEmpty(t, x+1, y+0)) return false;
	if(!Tablero_CheckEmpty(t, x+1, y+1)) return false;
	if(!Tablero_CheckEmpty(t, x+2, y+1)) return false;
	return true;
}

void FiguraZ_State2(Figura* f, bool activate)
{
	f->grid[1][0] = activate;
	f->grid[1][1] = activate;
	f->grid[0][1] = activate;
	f->grid[0][2] = activate;
}

bool FiguraZ_CheckState2(Figura* f, Tablero* t)
{
	dword x = f->position.x;
	dword y = f->position.y;

	if(!Tablero_CheckEmpty(t, x+1, y+0)) return false;
	if(!Tablero_CheckEmpty(t, x+1, y+1)) return false;
	if(!Tablero_CheckEmpty(t, x+0, y+1)) return false;
	if(!Tablero_CheckEmpty(t, x+0, y+2)) return false;
	return true;
}


bool FiguraZ_Girar(Figura* f, Tablero* t)
{
	switch(f->state)
	{
	case ST1:
		{
			if(!FiguraZ_CheckState2(f, t)) return false;
			FiguraZ_State1(f, false);
			f->state = ST2;
			FiguraZ_State2(f, true);
			break;
		}
	case ST2:
		{
			if(!FiguraZ_CheckState1(f, t)) return false;
			FiguraZ_State2(f, false);
			f->state = ST1;
			FiguraZ_State1(f, true);
			break;
		}
	}
	return true;
}

void FiguraZ_Init(Figura* f)
{
	Figura_Init(f);

	f->color = SRGB(255, 100, 0);

	FiguraZ_State1(f, true);

	f->Girar = FiguraZ_Girar;
}

#endif
