//
// Created by armanaxh on ۲۰۱۹/۸/۳۱.
//

#ifndef CAFE_2D_ALGOUTILS_H
#define CAFE_2D_ALGOUTILS_H

#include "algorithms/hungarian.h"

class AlgoUtils {
    AlgoUtils() {}

public:
    static Hungarian::Result hungarianAssignment(const Hungarian::Matrix &cost, Hungarian::MODE mode);

};


#endif //CAFE_2D_ALGOUTILS_H
