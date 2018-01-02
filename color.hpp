#ifndef __COLOR_HPP__
#define __COLOR_HPP__


struct RGB {
	float r;
	float g;
	float b;
	
	RGB(float red, float green, float blue) : r(red), g(green), b(blue) {}
};

extern RGB backgroundColor;
extern RGB handleColor;
extern RGB borderColor;
extern RGB trackColor;
extern RGB arrowColor;

#endif // __COLOR_HPP__
