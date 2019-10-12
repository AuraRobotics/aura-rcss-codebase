//
// Created by armanaxh on ۲۰۱۹/۱۰/۱۰.
//

#ifndef CAFE_2D_BHV_SHOOT_H
#define CAFE_2D_BHV_SHOOT_H

#include <rcsc/player/soccer_action.h>
#include <rcsc/player/player_object.h>
#include "../../utils/estimators/HERMES_FastIC.h"

class Bhv_Shoot
        : public rcsc::SoccerBehavior {
public:
    Bhv_Shoot() {}

    bool execute(rcsc::PlayerAgent *agent);

private:

    bool doTurnNeckToShootPoint(rcsc::PlayerAgent *agent, const rcsc::Vector2D &shoot_point);

    rcsc::Vector2D doShoot(rcsc::PlayerAgent *agent);

    void fastICConfig(FastIC *fastIC, rcsc::PlayerAgent *agent);


};


#endif //CAFE_2D_BHV_SHOOT_H
