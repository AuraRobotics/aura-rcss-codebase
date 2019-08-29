//
// Created by armanaxh on ۲۰۱۹/۸/۲۹.
//

#ifndef CAFE_2D_BHV_DEFENSIVE_POSITIONING_H
#define CAFE_2D_BHV_DEFENSIVE_POSITIONING_H

#include <rcsc/player/soccer_action.h>

class Bhv_DefensivePositioning
        : public rcsc::SoccerBehavior {
public:
    Bhv_DefensivePositioning() {}

    bool execute(rcsc::PlayerAgent *agent);

private:

};


#endif //CAFE_2D_BHV_DEFENSIVE_POSITIONING_H
