//
// Created by armanaxh on ۲۰۱۹/۹/۲۲.
//

#ifndef CAFE_2D_BHV_OFFENSIVE_POSITIONING_H
#define CAFE_2D_BHV_OFFENSIVE_POSITIONING_H

#include <rcsc/player/soccer_action.h>
#include <rcsc/player/player_object.h>


class Bhv_OffensivePositioning
        : public rcsc::SoccerBehavior {
public:
    Bhv_OffensivePositioning() {}

    bool execute(rcsc::PlayerAgent *agent);

private:


};

#endif //CAFE_2D_BHV_OFFENSIVE_POSITIONING_H
