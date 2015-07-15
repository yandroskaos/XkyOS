#ifndef __TABLERO_H__
#define __TABLERO_H__

#define TAB_ROWS	20
#define TAB_COLUMNS	12

struct Tablero
{
	ARGB grid[TAB_COLUMNS][TAB_ROWS];
	ARGB background;
};

void Tablero_Init(Tablero* t)
{
	t->background = SRGB(0, 0, 0);
	for(dword i=0; i<TAB_COLUMNS; i++)
		for(dword j=0; j<TAB_ROWS; j++)
			t->grid[i][j] = t->background;
}

bool Tablero_CheckEmpty(Tablero* t, dword x, dword y)
{
	if(x >= TAB_COLUMNS)
		return false;

	if(y >= TAB_ROWS)
		return false;

	return (t->grid[x][y] == t->background);
}

dword Tablero_CheckLines(Tablero* t)
{
	dword lines = 0;

	for(dword j=0; j<TAB_ROWS; j++)
	{
		dword i=0;
		for(; i<TAB_COLUMNS; i++)
		{
			if(t->grid[i][j] == t->background)
			{
				break;
			}
		}
		if(i==TAB_COLUMNS)
		{
			lines++;
			for(dword j2=j; j2>0; j2--)
			{
				for(dword i=0; i<TAB_COLUMNS; i++)
				{
					t->grid[i][j2] = t->grid[i][j2-1];
				}
				for(dword i=0; i<TAB_COLUMNS; i++)
				{
					t->grid[i][0] = t->background;
				}
			}
		}
	}

	return lines;
}

void Tablero_AnyadirFigura(Tablero* t, Figura* f)
{
	for(dword i=0; i<FIG_COLUMNS; i++)
	{
		for(dword j=0; j<FIG_ROWS; j++)
		{
			if(f->grid[i][j])
			{
				t->grid[f->position.x + i][f->position.y + j] = f->color;
			}
		}
	}
}

bool Figura_Bajar(Figura* f, Tablero* t)
{
	dword x = f->position.x;
	dword y = f->position.y;

	dword a0 = Figura_Altura(f, 0);
	if(VALID_VALUE(a0) && !Tablero_CheckEmpty(t, x+0, y+a0+1))
		return false;

	dword a1 = Figura_Altura(f, 1);
	if(VALID_VALUE(a1) && !Tablero_CheckEmpty(t, x+1, y+a1+1))
		return false;

	dword a2 = Figura_Altura(f, 2);
	if(VALID_VALUE(a2) && !Tablero_CheckEmpty(t, x+2, y+a2+1))
		return false;

	dword a3 = Figura_Altura(f, 3);
	if(VALID_VALUE(a3) && !Tablero_CheckEmpty(t, x+3, y+a3+1))
		return false;

	f->position.y++;
	return true;
}

bool Figura_Izquierda(Figura* f, Tablero* t)
{
	if(f->position.x > 0)
	{
		dword x = f->position.x;
		dword y = f->position.y;

		dword ai0 = Figura_AnchuraIzquierda(f, 0);
		if(VALID_VALUE(ai0) && !Tablero_CheckEmpty(t, x+ai0-1, y+0))
			return false;

		dword ai1 = Figura_AnchuraIzquierda(f, 1);
		if(VALID_VALUE(ai1) && !Tablero_CheckEmpty(t, x+ai1-1, y+1))
			return false;

		dword ai2 = Figura_AnchuraIzquierda(f, 2);
		if(VALID_VALUE(ai2) && !Tablero_CheckEmpty(t, x+ai2-1, y+2))
			return false;

		dword ai3 = Figura_AnchuraIzquierda(f, 3);
		if(VALID_VALUE(ai3) && !Tablero_CheckEmpty(t, x+ai3-1, y+3))
			return false;

		f->position.x--;
		return true;
	}
	return false;
}

bool Figura_Derecha(Figura* f, Tablero* t)
{
	dword x = f->position.x;
	dword y = f->position.y;

	dword ad0 = Figura_AnchuraDerecha(f, 0);
	if(VALID_VALUE(ad0) && !Tablero_CheckEmpty(t, x+ad0+1, y+0))
		return false;

	dword ad1 = Figura_AnchuraDerecha(f, 1);
	if(VALID_VALUE(ad1) && !Tablero_CheckEmpty(t, x+ad1+1, y+1))
		return false;

	dword ad2 = Figura_AnchuraDerecha(f, 2);
	if(VALID_VALUE(ad2) && !Tablero_CheckEmpty(t, x+ad2+1, y+2))
		return false;

	dword ad3 = Figura_AnchuraDerecha(f, 3);
	if(VALID_VALUE(ad3) && !Tablero_CheckEmpty(t, x+ad3+1, y+3))
		return false;

	f->position.x++;
	return true;
}

void Figura_Init(Figura* f)
{
	for(dword i=0; i<FIG_COLUMNS; i++)
		for(dword j=0; j<FIG_ROWS; j++)
			f->grid[i][j] = false;

	f->state = ST1;

	f->color = 0;

	f->position.x = 0;
	f->position.y = 0;

	f->Bajar = Figura_Bajar;
	f->Izquierda = Figura_Izquierda;
	f->Derecha = Figura_Derecha;
	f->Girar = Figura_Girar;
}


#endif
