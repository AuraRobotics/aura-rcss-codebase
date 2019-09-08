//
// Created by armanaxh on ۲۰۱۹/۹/۵.
//

#ifndef CAFE_2D_BHV_MARK_H
#define CAFE_2D_BHV_MARK_H

#include <rcsc/player/soccer_action.h>
#include <rcsc/player/player_agent.h>
#include <rcsc/geom/vector_2d.h>
#include <rcsc/geom/polygon_2d.h>

class Bhv_Mark
        : public rcsc::SoccerBehavior {
    const rcsc::PlayerObject *target_opp;
public:
    Bhv_Mark(const rcsc::PlayerObject *t_opp) {
        this->target_opp = t_opp;
    }

    bool execute(rcsc::PlayerAgent *agent);

private:
    rcsc::Vector2D getDefensivePos(rcsc::PlayerAgent *agent);
};

#endif //CAFE_2D_BHV_MARK_H
