//
// Created by armanaxh on ۲۰۱۹/۹/۵.
//

#include "bhv_block.h"

#include <rcsc/common/server_param.h>
#include <rcsc/common/logger.h>
#include <rcsc/player/world_model.h>
#include <rcsc/player/player_agent.h>
#include <rcsc/common/logger.h>
#include <rcsc/action/body_intercept.h>
#include <rcsc/action/neck_turn_to_ball_or_scan.h>
#include <rcsc/action/neck_turn_to_low_conf_teammate.h>
#include "../actions/neck_offensive_intercept_neck.h"
#include "../strategy.h"
#include "../cafe_model.h"


#include <iostream>


using namespace rcsc;

bool Bhv_Block::execute(rcsc::PlayerAgent *agent) {
    dlog.addText(Logger::TEAM,
                 "execute Bhv_block");

    const ServerParam &SP = ServerParam::i();
    const WorldModel &wm = agent->world();
    const CafeModel &cm = CafeModel::i();
    const Strategy &stra = Strategy::i();


    const Vector2D self_pos = wm.self().pos();
    const Vector2D ball_pos = wm.ball().pos();

    const unsigned self_unum = agent->world().self().unum();
    const RoleGroup role_group = stra.getRoleGroup(self_unum);

    if(role_group == Defense){
        if(self_pos.dist(ball_pos) > 2.5){
            return false;
        }
    }

    const int self_min = wm.interceptTable()->selfReachCycle();
    const int mate_min = wm.interceptTable()->teammateReachCycle();
    const int opp_min = wm.interceptTable()->opponentReachCycle();

    if (wm.existKickableOpponent() || (opp_min < self_min && opp_min < mate_min)) {
        if (self_min < mate_min) {
            dlog.addText(Logger::TEAM,
                         __FILE__": intercept");

            Body_Intercept().execute(agent);
            agent->setNeckAction(new Neck_OffensiveInterceptNeck());
            return true;
        }
    }


    return false;
}