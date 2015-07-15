#ifndef __FIGURA_CUADRADO_H__
#define __FIGURA_CUADRADO_H__

bool FiguraCuadrado_Girar(Figura* f, Tablero* t)
{
	//Do nothing
	return true;
}

void FiguraCuadrado_Init(Figura* f)
{
	Figura_Init(f);

	f->color = SRGB(254, 0, 227);

	f->grid[0][0] = true;
	f->grid[0][1] = true;
	f->grid[1][0] = true;
	f->grid[1][1] = true;

	f->Girar = FiguraCuadrado_Girar;
}

#endif
