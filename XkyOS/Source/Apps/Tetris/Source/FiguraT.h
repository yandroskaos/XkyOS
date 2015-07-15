#ifndef __FIGURAT_H__
#define __FIGURAT_H__

//Figura T
void FiguraT_State1(Figura* f, bool activate)
{
	f->grid[0][0] = activate;
	f->grid[1][0] = activate;
	f->grid[1][1] = activate;
	f->grid[2][0] = activate;

}

bool FiguraT_CheckState1(Figura* f, Tablero* t)
{
	dword x = f->position.x;
	dword y = f->position.y;

	if(!Tablero_CheckEmpty(t, x+0, y+0)) return false;
	if(!Tablero_CheckEmpty(t, x+1, y+0)) return false;
	if(!Tablero_CheckEmpty(t, x+1, y+1)) return false;
	if(!Tablero_CheckEmpty(t, x+2, y+0)) return false;
	return true;
}

void FiguraT_State2(Figura* f, bool activate)
{
	f->grid[1][0] = activate;
	f->grid[0][1] = activate;
	f->grid[1][1] = activate;
	f->grid[1][2] = activate;

}

bool FiguraT_CheckState2(Figura* f, Tablero* t)
{
	dword x = f->position.x;
	dword y = f->position.y;

	if(!Tablero_CheckEmpty(t, x+1, y+0)) return false;
	if(!Tablero_CheckEmpty(t, x+0, y+1)) return false;
	if(!Tablero_CheckEmpty(t, x+1, y+1)) return false;
	if(!Tablero_CheckEmpty(t, x+1, y+2)) return false;
	return true;
}

void FiguraT_State3(Figura* f, bool activate)
{
	f->grid[1][0] = activate;
	f->grid[0][1] = activate;
	f->grid[1][1] = activate;
	f->grid[2][1] = activate;
}

bool FiguraT_CheckState3(Figura* f, Tablero* t)
{
	dword x = f->position.x;
	dword y = f->position.y;

	if(!Tablero_CheckEmpty(t, x+1, y+0)) return false;
	if(!Tablero_CheckEmpty(t, x+0, y+1)) return false;
	if(!Tablero_CheckEmpty(t, x+1, y+1)) return false;
	if(!Tablero_CheckEmpty(t, x+2, y+1)) return false;
	return true;
}

void FiguraT_State4(Figura* f, bool activate)
{
	f->grid[0][0] = activate;
	f->grid[0][1] = activate;
	f->grid[0][2] = activate;
	f->grid[1][1] = activate;

}

bool FiguraT_CheckState4(Figura* f, Tablero* t)
{
	dword x = f->position.x;
	dword y = f->position.y;

	if(!Tablero_CheckEmpty(t, x+0, y+0)) return false;
	if(!Tablero_CheckEmpty(t, x+0, y+1)) return false;
	if(!Tablero_CheckEmpty(t, x+0, y+2)) return false;
	if(!Tablero_CheckEmpty(t, x+1, y+1)) return false;
	return true;
}

bool FiguraT_Girar(Figura* f, Tablero* t)
{
	switch(f->state)
	{
	case ST1:
		{
			if(!FiguraT_CheckState2(f, t)) return false;
			FiguraT_State1(f, false);
			f->state = ST2;
			FiguraT_State2(f, true);
			break;
		}
	case ST2:
		{
			if(!FiguraT_CheckState3(f, t)) return false;
			FiguraT_State2(f, false);
			f->state = ST3;
			FiguraT_State3(f, true);
			break;
		}
	case ST3:
		{
			if(!FiguraT_CheckState4(f, t)) return false;
			FiguraT_State3(f, false);
			f->state = ST4;
			FiguraT_State4(f, true);
			break;
		}
	case ST4:
		{
			if(!FiguraT_CheckState1(f, t)) return false;
			FiguraT_State4(f, false);
			f->state = ST1;
			FiguraT_State1(f, true);
			break;
		}
	}
	return true;
}

void FiguraT_Init(Figura* f)
{
	Figura_Init(f);

	f->color = SRGB(0, 255, 0);

	FiguraT_State1(f, true);
	
	f->Girar = FiguraT_Girar;
}


#endif
