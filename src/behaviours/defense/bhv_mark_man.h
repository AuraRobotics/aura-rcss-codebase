//
// Created by armanaxh on ۲۰۱۹/۹/۱۶.
//

#ifndef CAFE_2D_BHV_MARK_MAN_H
#define CAFE_2D_BHV_MARK_MAN_H

#include <rcsc/player/soccer_action.h>
#include <rcsc/player/player_agent.h>
#include <rcsc/geom/vector_2d.h>


#include <rcsc/player/soccer_action.h>
#include <rcsc/player/player_agent.h>
#include <rcsc/geom/vector_2d.h>
#include <rcsc/geom/polygon_2d.h>

class Bhv_MarkMan
        : public rcsc::SoccerBehavior {
    const rcsc::PlayerObject *target_opp;

public:


    Bhv_MarkMan(const rcsc::PlayerObject *t_opp) {
        this->target_opp = t_opp;
    }

    bool execute(rcsc::PlayerAgent *agent);

private:

    rcsc::Vector2D getDefensivePos(rcsc::PlayerAgent *agent);

    rcsc::Polygon2D getDengerArea(rcsc::Vector2D ball_pos,
                                  rcsc::Vector2D opp_pos);

    bool checkPosIsValid(rcsc::Vector2D check_point, rcsc::Vector2D self_pos, rcsc::Vector2D opp_pos,
                         rcsc::Vector2D ball_pos, double our_offside_x);

    double nearToTarget(rcsc::Vector2D check_point, rcsc::Vector2D self_pos, rcsc::Vector2D opp_pos,
                        rcsc::Vector2D ball_pos, double max_radius2);

    double nearToGoalLine(rcsc::Vector2D check_point, rcsc::Vector2D self_pos, rcsc::Vector2D opp_pos,
                          rcsc::Vector2D ball_pos, double max_radius2);

    double nearToBodyDir(rcsc::Vector2D check_point, rcsc::Vector2D self_pos, rcsc::Vector2D opp_pos,
                         rcsc::Vector2D ball_pos, double max_radius2, rcsc::PlayerAgent *agent);

    double nearToPenaltyArea(rcsc::Vector2D check_point, rcsc::Vector2D self_pos, rcsc::Vector2D opp_pos,
                             rcsc::Vector2D ball_pos, double max_radius2);

    double betweenGoalAngle(rcsc::Vector2D check_point, rcsc::Vector2D self_pos, rcsc::Vector2D opp_pos,
                            rcsc::Vector2D ball_pos, double max_radius2);

    double nearToPassLine(rcsc::Vector2D check_point, rcsc::Vector2D self_pos, rcsc::Vector2D opp_pos,
                          rcsc::Vector2D ball_pos, double max_radius2);

    double nearToGoal(rcsc::Vector2D check_point, double start_x, double max_radius2);

    double coverDengerPassArea(rcsc::Vector2D check_point, rcsc::Vector2D self_pos, rcsc::Vector2D opp_pos,
                               rcsc::Vector2D ball_pos, rcsc::Polygon2D denger_area,
                               double dist_path);


    void log_table(std::vector <std::vector<double> >  table, std::string name);
};

#endif //CAFE_2D_BHV_MARK_MAN_H
