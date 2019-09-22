//
// Created by armanaxh on ۲۰۱۹/۹/۸.
//

#ifndef CAFE_2D_BHV_INTERCEPT_H
#define CAFE_2D_BHV_INTERCEPT_H


class bhv_intercept {

};
#include <rcsc/player/soccer_action.h>

class Bhv_Intercept
        : public rcsc::SoccerBehavior {
public:
    Bhv_Intercept()
    { }

    bool execute( rcsc::PlayerAgent * agent );

private:
};

#endif //CAFE_2D_BHV_INTERCEPT_H
