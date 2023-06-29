#pragma once
#include "curve.h"

class Bezier :
    public Curve
{
public:
    Bezier();
    void generateCurve(int pointsPerSegment);
};
