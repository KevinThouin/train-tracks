#include "track.hpp"

void getUnitTangentFromDirection(Direction d, float& x, float& y) {
	switch (d) {
		case EAST:
			x = 1.0;
			y = 0.0;
			break;
		
		case SOUTH:
			x = 0.0;
			y = -1.0;
			break;
		
		case WEST:
			x = -1.0;
			y = 0.0;
			break;
		
		case NORTH:
			x = 0.0;
			y = 1.0;
			break;
	}
}

Bezier TrackBezier::getBezier(float p0x, float p0y, float p1x, float p1y, Direction d0, Direction d1) {
	return Bezier(p0x, p0y, (d0&0x1) ? p0x : 0.5*(p1x+p0x), (d0&0x1) ? 0.5*(p1y+p0y) : p0y,
						   (d1&0x1) ? p1x : 0.5*(p0x+p1x), (d1&0x1) ? 0.5*(p0y+p1y) : p1y , p1x, p1y);
}

Bezier TrackBezier::getBezier(float p0x, float p0y, float p1x, float p1y, float t0x, float t0y, float t1x, float t1y) {
	return Bezier(p0x, p0y, p0x+t0x, p0y+t0y, p1x+t1x, p1y+t1y, p1x, p1y);
}

