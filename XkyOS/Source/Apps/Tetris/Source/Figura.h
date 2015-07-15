#ifndef __FIGURA_H__
#define __FIGURA_H__

struct Figura;
struct Tablero;

typedef bool (*fFiguraMethod)(Figura* f, Tablero* t);

#define FIG_ROWS	4
#define FIG_COLUMNS	4

enum TurnState {ST1, ST2, ST3, ST4};

struct Point
{
	dword x;
	dword y;
};

bool Figura_Girar(Figura* f, Tablero* t)
{
	//Do nothing
	return true;
}

struct Figura
{
	fFiguraMethod Bajar;
	fFiguraMethod Izquierda;
	fFiguraMethod Derecha;
	fFiguraMethod Girar;

	ARGB color;
	Point position;
	TurnState state;
	bool grid[FIG_COLUMNS][FIG_ROWS];
};

#define VALID_VALUE(X) ((X)!=0xFFFFFFFF)

dword Figura_AnchuraDerecha(Figura* f, int row)
{
	for(dword i = FIG_COLUMNS; i > 0; i--)
		if(f->grid[i-1][row])
			return i-1;
	return 0xFFFFFFFF;
}

dword Figura_AnchuraIzquierda(Figura* f, int row)
{
	for(dword i = 0; i < FIG_COLUMNS; i++)
		if(f->grid[i][row])
			return i;
	return 0xFFFFFFFF;
}

dword Figura_Altura(Figura* f, int column)
{
	for(dword j = FIG_ROWS; j > 0; j--)
		if(f->grid[column][j-1])
			return j-1;
	return 0xFFFFFFFF;
}



#endif
