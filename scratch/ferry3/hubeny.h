/*
 * hubeny.h
 *
 *  Created on: 2021/08/04
 *      Author: matsunaga
 */


#include <iostream>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef HUBENY_H
#define HUBENY_H

namespace ns3{
namespace simple{

class Hubeny
{
public:
    const double L_RAD = 6378137.0;         // WGS84
    const double S_LAD = 6356752.314245;    // WGS84

    inline double toRadian(const double &value) const {
        return value * M_PI / 180.0;
    }

    inline void hubeny_distance(const double &x1, const double &y1, const double &x2, const double &y2, double &d) const {
        double e = pow(sqrt((pow(L_RAD, 2) - pow(S_LAD, 2)) / pow(L_RAD, 2)), 2);
        double dx = toRadian(x2) - toRadian(x1);
        double dy = toRadian(y2) - toRadian(y1);
        double u = (toRadian(y1) + toRadian(y2)) / 2.0;
        double W = sqrt(1.0 - e * pow(sin(u), 2));
        double M = (L_RAD * (1.0 - e)) / pow(W, 3);
        double N = L_RAD / W;
        d = sqrt(pow(dy * M, 2) + pow(dx * N * cos(u), 2));
        return;
    }
};

}
}

#endif // HUBENY_H
