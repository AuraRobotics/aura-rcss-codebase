//
// Created by armanaxh on ۲۰۱۹/۸/۲۹.
//

#ifndef CAFE_2D_BHV_DEFENSIVE_POSITIONING_H
#define CAFE_2D_BHV_DEFENSIVE_POSITIONING_H

#include <rcsc/player/soccer_action.h>
#include <rcsc/player/player_object.h>
#include <rcsc/geom/vector_2d.h>
#include <rcsc/geom/polygon_2d.h>
#include "../cafe_model.h"


class Bhv_DefensivePositioning
        : public rcsc::SoccerBehavior {
public:
    Bhv_DefensivePositioning() {}

    bool execute(rcsc::PlayerAgent *agent);

private:


};


#endif //CAFE_2D_BHV_DEFENSIVE_POSITIONING_H
