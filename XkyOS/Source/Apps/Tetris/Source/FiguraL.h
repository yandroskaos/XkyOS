#ifndef __FIGURAL_H__
#define __FIGURAL_H__

//Figura L
void FiguraL_State1(Figura* f, bool activate)
{
	f->grid[0][0] = activate;
	f->grid[0][1] = activate;
	f->grid[0][2] = activate;
	f->grid[1][2] = activate;

}

bool FiguraL_CheckState1(Figura* f, Tablero* t)
{
	dword x = f->position.x;
	dword y = f->position.y;

	if(!Tablero_CheckEmpty(t, x+0, y+0)) return false;
	if(!Tablero_CheckEmpty(t, x+0, y+1)) return false;
	if(!Tablero_CheckEmpty(t, x+0, y+2)) return false;
	if(!Tablero_CheckEmpty(t, x+1, y+2)) return false;
	return true;
}

void FiguraL_State2(Figura* f, bool activate)
{
	f->grid[0][1] = activate;
	f->grid[1][1] = activate;
	f->grid[2][1] = activate;
	f->grid[2][0] = activate;
}

bool FiguraL_CheckState2(Figura* f, Tablero* t)
{
	dword x = f->position.x;
	dword y = f->position.y;

	if(!Tablero_CheckEmpty(t, x+0, y+1)) return false;
	if(!Tablero_CheckEmpty(t, x+1, y+1)) return false;
	if(!Tablero_CheckEmpty(t, x+2, y+1)) return false;
	if(!Tablero_CheckEmpty(t, x+2, y+0)) return false;
	return true;
}

void FiguraL_State3(Figura* f, bool activate)
{
	f->grid[0][0] = activate;
	f->grid[1][0] = activate;
	f->grid[1][1] = activate;
	f->grid[1][2] = activate;

}

bool FiguraL_CheckState3(Figura* f, Tablero* t)
{
	dword x = f->position.x;
	dword y = f->position.y;

	if(!Tablero_CheckEmpty(t, x+0, y+0)) return false;
	if(!Tablero_CheckEmpty(t, x+1, y+0)) return false;
	if(!Tablero_CheckEmpty(t, x+1, y+1)) return false;
	if(!Tablero_CheckEmpty(t, x+1, y+2)) return false;
	return true;
}

void FiguraL_State4(Figura* f, bool activate)
{
	f->grid[0][0] = activate;
	f->grid[1][0] = activate;
	f->grid[2][0] = activate;
	f->grid[0][1] = activate;
}

bool FiguraL_CheckState4(Figura* f, Tablero* t)
{
	dword x = f->position.x;
	dword y = f->position.y;

	if(!Tablero_CheckEmpty(t, x+0, y+0)) return false;
	if(!Tablero_CheckEmpty(t, x+1, y+0)) return false;
	if(!Tablero_CheckEmpty(t, x+2, y+0)) return false;
	if(!Tablero_CheckEmpty(t, x+0, y+1)) return false;
	return true;
}

bool FiguraL_Girar(Figura* f, Tablero* t)
{
	switch(f->state)
	{
	case ST1:
		{
			if(!FiguraL_CheckState2(f, t)) return false;
			FiguraL_State1(f, false);
			f->state = ST2;
			FiguraL_State2(f, true);
			break;
		}
	case ST2:
		{
			if(!FiguraL_CheckState3(f, t)) return false;
			FiguraL_State2(f, false);
			f->state = ST3;
			FiguraL_State3(f, true);
			break;
		}
	case ST3:
		{
			if(!FiguraL_CheckState4(f, t)) return false;
			FiguraL_State3(f, false);
			f->state = ST4;
			FiguraL_State4(f, true);
			break;
		}
	case ST4:
		{
			if(!FiguraL_CheckState1(f, t)) return false;
			FiguraL_State4(f, false);
			f->state = ST1;
			FiguraL_State1(f, true);
			break;
		}
	}
	return true;
}

void FiguraL_Init(Figura* f)
{
	Figura_Init(f);

	f->color = SRGB(255, 255, 0);

	FiguraL_State1(f, true);

	f->Girar = FiguraL_Girar;
}


#endif
