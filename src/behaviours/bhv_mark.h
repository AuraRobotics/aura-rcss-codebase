//
// Created by armanaxh on ۲۰۱۹/۹/۵.
//

#ifndef CAFE_2D_BHV_MARK_H
#define CAFE_2D_BHV_MARK_H

#include <rcsc/player/soccer_action.h>

class Bhv_Mark
        : public rcsc::SoccerBehavior {
public:
    Bhv_Mark()
    { }

    bool execute( rcsc::PlayerAgent * agent);

private:

};

#endif //CAFE_2D_BHV_MARK_H
