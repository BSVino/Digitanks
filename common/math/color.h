#ifndef LW_COLOR_H
#define LW_COLOR_H

#include "maths.h"

class Color
{
public:
	inline			Color();
	inline			Color(class Vector v);
	inline			Color(int _r, int _g, int _b);
	inline			Color(int _r, int _g, int _b, int _a);

	inline void		SetColor(int _r, int _g, int _b, int _a);
	inline void		SetRed(int _r);
	inline void		SetGreen(int _g);
	inline void		SetBlue(int _b);
	inline void		SetAlpha(int _a);
	inline void		SetAlpha(float f);

	int				r() const { return red; };
	int				g() const { return green; };
	int				b() const { return blue; };
	int				a() const { return alpha; };

	operator unsigned char*()
	{
		return(&red);
	}

	operator const unsigned char*() const
	{
		return(&red);
	}

private:
	unsigned char	red;
	unsigned char	green;
	unsigned char	blue;
	unsigned char	alpha;
};

#include "vector.h"

Color::Color()
{
	Color(0, 0, 0, 255);
}

Color::Color(Vector v)
{
	SetColor((int)(v.x*255), (int)(v.y*255), (int)(v.z*255), 255);
}

Color::Color(int _r, int _g, int _b)
{
	SetColor(_r, _g, _b, 255);
}

Color::Color(int _r, int _g, int _b, int _a)
{
	SetColor(_r, _g, _b, _a);
}

void Color::SetColor(int _r, int _g, int _b, int _a)
{
	red = _r;
	green = _g;
	blue = _b;
	alpha = _a;
}

void Color::SetRed(int _r)
{
	red = _r;
}

void Color::SetGreen(int _g)
{
	green = _g;
}

void Color::SetBlue(int _b)
{
	blue = _b;
}

void Color::SetAlpha(int _a)
{
	alpha = _a;
}

void Color::SetAlpha(float f)
{
	alpha = (int)(f*255);
}

#endif
