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

    bool defendTheDefendLine(rcsc::PlayerAgent *agent);

    bool positioningDengerArea(rcsc::PlayerAgent *agent);

    const rcsc::PlayerObject *
    assignOpponent(rcsc::ConstPlayerPtrCont def_ps, rcsc::PlayerPtrCont opp_ps, int self_unum);

    rcsc::Vector2D getDefensivePos(rcsc::PlayerAgent *agent, const rcsc::PlayerObject *opponent, double defense_line_x);

    rcsc::Polygon2D getDengerArea(rcsc::Vector2D ball_pos,
                                  rcsc::Vector2D opp_pos);

    bool checkPosIsValid(rcsc::Vector2D check_point, rcsc::Vector2D self_pos, rcsc::Vector2D opp_pos,
                         rcsc::Vector2D ball_pos);

    double nearToGoalLine(rcsc::Vector2D check_point, rcsc::Vector2D self_pos, rcsc::Vector2D opp_pos,
                          rcsc::Vector2D ball_pos, double max_radius2);

    double nearToPenaltyArea(rcsc::Vector2D check_point, rcsc::Vector2D self_pos, rcsc::Vector2D opp_pos,
                             rcsc::Vector2D ball_pos, double max_radius2);

    double nearToGoal(rcsc::Vector2D check_point, double start_x, double max_radius2);

    double coverDengerPassArea(rcsc::Vector2D check_point, rcsc::Vector2D self_pos, rcsc::Vector2D opp_pos,
                               rcsc::Vector2D ball_pos, rcsc::Polygon2D denger_area,
                               double dist_path);



    void log_table(std::vector<std::vector<double> > table, std::string name);

};


#endif //CAFE_2D_BHV_DEFENSIVE_POSITIONING_H
