//
// Created by armanaxh on ۲۰۱۹/۹/۵.
//

#ifndef CAFE_2D_BHV_BLOCK_H
#define CAFE_2D_BHV_BLOCK_H

#include <rcsc/player/soccer_action.h>

class Bhv_Block
        : public rcsc::SoccerBehavior {
public:
    Bhv_Block()
    { }

    bool execute( rcsc::PlayerAgent * agent );

private:

};
#endif //CAFE_2D_BHV_BLOCK_H
