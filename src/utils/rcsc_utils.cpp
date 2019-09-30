//
// Created by armanaxh on ۲۰۱۹/۹/۵.
//

#include "rcsc_utils.h"
#include <rcsc/common/server_param.h>

using namespace rcsc;

double rcscUtils::ballPathDistWithMaxSpeed(int cycle) {
    const static ServerParam &SP = ServerParam::i();
    double max_speed = SP.ballSpeedMax();
    double dencay = SP.ballDecay();

    if (dencay == 1) {
        std::cerr << "dancay is 1 ..." << std::endl;
        std::exit(EXIT_FAILURE);
    }
    return max_speed * (1 - std::pow(dencay, cycle)) / (1 - dencay);
}


double rcscUtils::maxDistBall(double initial_speed) {
    const static ServerParam &SP = ServerParam::i();
    double dencay = SP.ballDecay();
    return initial_speed / (1 - dencay);
}

double rcscUtils::maxDistBall() {
    const static ServerParam &SP = ServerParam::i();
    double max_speed = SP.ballSpeedMax();
    double dencay = SP.ballDecay();

    return max_speed / (1 - dencay);
}

int rcscUtils::ballCycle(double dist, double speed ) {
    const static ServerParam &SP = ServerParam::i();
    double dencay = SP.ballDecay();
    double max_speed = SP.ballSpeedMax();
    if(speed == -1){
        speed = max_speed;
    }
    int cycle = 0;
    while(dist > 0){
        dist -= speed;
        speed *= dencay;
        cycle ++;
        if(cycle >= 50) break;
    }

    return cycle;
}
