//
// Created by armanaxh on ۲۰۱۹/۹/۲۲.
//

#ifndef CAFE_2D_BHV_PASS_POSITION_H
#define CAFE_2D_BHV_PASS_POSITION_H


#include <rcsc/player/soccer_action.h>
#include <rcsc/player/player_agent.h>
#include <rcsc/geom/vector_2d.h>
#include <rcsc/geom/polygon_2d.h>

class Bhv_PassPosition
        : public rcsc::SoccerBehavior {
    rcsc::PlayerPtrCont receivers;
public:


    Bhv_PassPosition(rcsc::PlayerPtrCont receivers) {
        this->receivers = receivers;
    }

    bool execute(rcsc::PlayerAgent *agent);

private:

    rcsc::Vector2D getPassPos(rcsc::PlayerAgent *agent);

    bool checkPosIsValid(rcsc::Vector2D check_point, rcsc::Vector2D self_pos,
                         rcsc::Vector2D ball_pos, double our_offside_x);

    void log_table(std::vector <std::vector<double> > table, std::string name);
};


#endif //CAFE_2D_BHV_PASS_POSITION_H
