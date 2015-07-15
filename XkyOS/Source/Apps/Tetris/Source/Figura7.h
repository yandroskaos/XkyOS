#ifndef __FIGURA7_H__
#define __FIGURA7_H__

//Figura 7
void Figura7_State1(Figura* f, bool activate)
{
	f->grid[1][0] = activate;
	f->grid[1][1] = activate;
	f->grid[1][2] = activate;
	f->grid[0][2] = activate;
}

bool Figura7_CheckState1(Figura* f, Tablero* t)
{
	dword x = f->position.x;
	dword y = f->position.y;

	if(!Tablero_CheckEmpty(t, x+1, y+0)) return false;
	if(!Tablero_CheckEmpty(t, x+1, y+1)) return false;
	if(!Tablero_CheckEmpty(t, x+1, y+2)) return false;
	if(!Tablero_CheckEmpty(t, x+0, y+2)) return false;
	return true;
}

void Figura7_State2(Figura* f, bool activate)
{
	f->grid[0][1] = activate;
	f->grid[1][1] = activate;
	f->grid[2][1] = activate;
	f->grid[0][0] = activate;
}

bool Figura7_CheckState2(Figura* f, Tablero* t)
{
	dword x = f->position.x;
	dword y = f->position.y;

	if(!Tablero_CheckEmpty(t, x+0, y+1)) return false;
	if(!Tablero_CheckEmpty(t, x+1, y+1)) return false;
	if(!Tablero_CheckEmpty(t, x+2, y+1)) return false;
	if(!Tablero_CheckEmpty(t, x+0, y+0)) return false;
	return true;
}

void Figura7_State3(Figura* f, bool activate)
{
	f->grid[1][0] = activate;
	f->grid[0][0] = activate;
	f->grid[0][1] = activate;
	f->grid[0][2] = activate;
}

bool Figura7_CheckState3(Figura* f, Tablero* t)
{
	dword x = f->position.x;
	dword y = f->position.y;

	if(!Tablero_CheckEmpty(t, x+1, y+0)) return false;
	if(!Tablero_CheckEmpty(t, x+0, y+0)) return false;
	if(!Tablero_CheckEmpty(t, x+0, y+1)) return false;
	if(!Tablero_CheckEmpty(t, x+0, y+2)) return false;
	return true;
}

void Figura7_State4(Figura* f, bool activate)
{
	f->grid[0][0] = activate;
	f->grid[1][0] = activate;
	f->grid[2][0] = activate;
	f->grid[2][1] = activate;
}

bool Figura7_CheckState4(Figura* f, Tablero* t)
{
	dword x = f->position.x;
	dword y = f->position.y;

	if(!Tablero_CheckEmpty(t, x+0, y+0)) return false;
	if(!Tablero_CheckEmpty(t, x+1, y+0)) return false;
	if(!Tablero_CheckEmpty(t, x+2, y+0)) return false;
	if(!Tablero_CheckEmpty(t, x+2, y+1)) return false;
	return true;
}

bool Figura7_Girar(Figura* f, Tablero* t)
{
	switch(f->state)
	{
	case ST1:
		{
			if(!Figura7_CheckState2(f, t)) return false;
			Figura7_State1(f, false);
			f->state = ST2;
			Figura7_State2(f, true);
			break;
		}
	case ST2:
		{
			if(!Figura7_CheckState3(f, t)) return false;
			Figura7_State2(f, false);
			f->state = ST3;
			Figura7_State3(f, true);
			break;
		}
	case ST3:
		{
			if(!Figura7_CheckState4(f, t)) return false;
			Figura7_State3(f, false);
			f->state = ST4;
			Figura7_State4(f, true);
			break;
		}
	case ST4:
		{
			if(!Figura7_CheckState1(f, t)) return false;
			Figura7_State4(f, false);
			f->state = ST1;
			Figura7_State1(f, true);
			break;
		}
	}
	return true;
}

void Figura7_Init(Figura* f)
{
	Figura_Init(f);

	f->color = SRGB(125, 1, 221);

	Figura7_State1(f, true);

	f->Girar = Figura7_Girar;
}


#endif
