//
// Created by armanaxh on ۲۰۱۹/۹/۵.
//

#include "bhv_block.h"

#include <rcsc/geom/vector_2d.h>
#include <rcsc/geom/segment_2d.h>
#include <rcsc/common/server_param.h>
#include <rcsc/common/logger.h>
#include <rcsc/player/world_model.h>
#include <rcsc/player/player_agent.h>
#include <rcsc/common/logger.h>
#include <rcsc/action/body_intercept.h>
#include <rcsc/action/basic_actions.h>
#include <rcsc/action/body_go_to_point.h>
#include <rcsc/action/neck_turn_to_ball_or_scan.h>
#include <rcsc/action/neck_turn_to_low_conf_teammate.h>
#include "../../actions/neck_offensive_intercept_neck.h"
#include "../../utils/geo_utils.h"
#include "../../strategy.h"
#include "../../cafe_model.h"


#include <iostream>


using namespace rcsc;

bool Bhv_Block::execute(rcsc::PlayerAgent *agent) {
    dlog.addText(Logger::TEAM,
                 "execute Bhv_block");

    const ServerParam &SP = ServerParam::i();
    const WorldModel &wm = agent->world();
    const CafeModel &cm = CafeModel::i();
    const Strategy &stra = Strategy::i();

    const unsigned self_unum = wm.self().unum();
    const Vector2D self_pos = wm.self().pos();

    const Vector2D &our_goal = SP.ourTeamGoalPos();
    const Vector2D ball_pos = wm.ball().pos();

    const Vector2D target_pos = target_opp->pos();

    double dash_power = SP.maxDashPower(); //stra.get_normal_dash_power(wm, stra);

    if (stra.getRoleGroup(self_unum) == Defense && stra.get_ball_area(wm) != Strategy::BA_Danger && (self_pos.dist(ball_pos) > 4 || ( self_pos.x + 0.5 > target_pos.x ) )   ) {

        double goal_y = target_pos.y > 0 ? 7 : -7;
        Vector2D goal_pos(our_goal.x, goal_y);
        Segment2D target_to_goal(target_pos, our_goal);
        Vector2D foot = geoUtils::findFootNearThan(target_to_goal, self_pos, target_pos);
        dlog.addText(Logger::TEAM,
                     __FILE__": find foot : %f %f ", foot.x , foot.y);
        if (foot == Vector2D::INVALIDATED) {
            foot = goal_pos;
        }
        double dist_form_foot = self_pos.dist(foot);
        if (Body_GoToPoint(foot, 0.5, dash_power).execute(agent)) {
            agent->setNeckAction(new Neck_TurnToBall());
        }
        dlog.addCircle(Logger::TEAM,
                       foot, 0.5, "#0000ff", true);

    } else {
        if (Body_GoToPoint(target_pos, 0.5, dash_power).execute(agent)) {
            agent->setNeckAction(new Neck_TurnToBall());
        }

        dlog.addCircle(Logger::TEAM,
                       target_pos, 0.5, "#3275a8", true);

    }
////////////////////////////////////////////////////////////////////////////
    dlog.addText(Logger::TEAM,
                 __FILE__": intercept");
    const int self_min = wm.interceptTable()->selfReachCycle();
    const int mate_min = wm.interceptTable()->teammateReachCycle();
    const int opp_min = wm.interceptTable()->opponentReachCycle();

    dlog.addText(Logger::TEAM,
                 __FILE__": ================= self_min : %d , mate_min: %d ,  opp_min: %d", self_min, mate_min,
                 opp_min);
    /////////////////////////////////////////////////////////////////////////
    return true;
}