//
// Created by armanaxh on ۲۰۱۹/۹/۵.
//

#ifndef CAFE_2D_BHV_BLOCK_H
#define CAFE_2D_BHV_BLOCK_H

#include <rcsc/player/soccer_action.h>
#include <rcsc/player/player_agent.h>

class Bhv_Block
        : public rcsc::SoccerBehavior {
    const rcsc::PlayerObject *target_opp;

public:
    Bhv_Block(const rcsc::PlayerObject *t_opp) {
        this->target_opp = t_opp;
    }

    bool execute( rcsc::PlayerAgent * agent );

private:

};
#endif //CAFE_2D_BHV_BLOCK_H
