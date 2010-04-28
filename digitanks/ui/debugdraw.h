#ifndef DEBUGDRAW_H
#define DEBUGDRAW_H

#include <vector.h>
#include <color.h>

void DebugLine(Vector a, Vector b, Color c);
void DebugCircle(Vector vecOrigin, float flRadius, Color c);
void DebugArc(Vector vecOrigin, float flRadius, float flStartDegree, float flEndDegree, Color c);

#endif
