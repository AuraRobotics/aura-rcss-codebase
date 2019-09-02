//
// Created by armanaxh on ۲۰۱۹/۸/۳۱.
//

#include "algo_utils.h"
#include "algorithms/hungarian.h"
#include <iostream>

Hungarian::Result AlgoUtils::hungarianAssignment(const Hungarian::Matrix &cost, Hungarian::MODE mode) {
    return Hungarian::Solve(cost, mode);
}