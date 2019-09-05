//
// Created by armanaxh on ۲۰۱۹/۹/۵.
//

#ifndef CAFE_2D_BHV_DEFHALF_POSITIONING_H
#define CAFE_2D_BHV_DEFHALF_POSITIONING_H

#include <rcsc/player/soccer_action.h>

class Bhv_DefhalfPositioning
        : public rcsc::SoccerBehavior {
public:
    Bhv_DefhalfPositioning()
    { }

    bool execute( rcsc::PlayerAgent * agent );

private:

};


#endif //CAFE_2D_BHV_DEFHALF_POSITIONING_H
