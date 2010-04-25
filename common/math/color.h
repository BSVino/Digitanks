#ifndef CF_COLOR_H
#define CF_COLOR_H

class Color
{
public:
	inline			Color();
	inline			Color(int _r, int _g, int _b);
	inline			Color(int _r, int _g, int _b, int _a);

	inline void		SetColor(int _r, int _g, int _b, int _a);
	inline void		SetRed(int _r);
	inline void		SetGreen(int _g);
	inline void		SetBlue(int _b);
	inline void		SetAlpha(int _a);

	int				r() { return red; };
	int				g() { return green; };
	int				b() { return blue; };
	int				a() { return alpha; };

	operator unsigned char*()
	{
		return(&red);
	}

private:
	unsigned char	red;
	unsigned char	green;
	unsigned char	blue;
	unsigned char	alpha;
};

Color::Color()
{
	Color(0, 0, 0, 255);
}

Color::Color(int _r, int _g, int _b)
{
	Color(_r, _g, _b, 255);
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

#endif