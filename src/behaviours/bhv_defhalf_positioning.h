//
// Created by armanaxh on ۲۰۱۹/۹/۵.
//

#ifndef CAFE_2D_BHV_DEFHALF_POSITIONING_H
#define CAFE_2D_BHV_DEFHALF_POSITIONING_H

#include <rcsc/player/soccer_action.h>
#include <rcsc/player/player_object.h>
#include <rcsc/player/world_model.h>
#include "../cafe_model.h"

class Bhv_DefhalfPositioning
        : public rcsc::SoccerBehavior {
public:
    Bhv_DefhalfPositioning() {}

    bool execute(rcsc::PlayerAgent *agent);

private:

    bool defendeTheDengerArea(rcsc::PlayerAgent *agent);

    rcsc::PlayerPtrCont getDengerOpponent(rcsc::PlayerAgent *agent);

    const rcsc::PlayerObject *
    assignOpponent(rcsc::ConstPlayerPtrCont def_ps, rcsc::PlayerPtrCont opp_ps, int self_unum, const rcsc::WorldModel &wm);













    rcsc::PlayerPtrCont defenceLineAssignment(rcsc::PlayerAgent *agent);
    rcsc::PlayerPtrCont getDengerOpponentDefendeLine(rcsc::PlayerAgent *agent);
    const rcsc::PlayerPtrCont assignOpponentToDefenceLine(ConstPlayerPtrCont def_ps, PlayerPtrCont opp_ps, int self_unum);

};


#endif //CAFE_2D_BHV_DEFHALF_POSITIONING_H
