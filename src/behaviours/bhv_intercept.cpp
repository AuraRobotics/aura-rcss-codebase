//
// Created by armanaxh on ۲۰۱۹/۹/۸.
//

#include "bhv_intercept.h"

#include <rcsc/common/server_param.h>
#include <rcsc/common/logger.h>
#include <rcsc/player/world_model.h>
#include <rcsc/player/player_agent.h>
#include <rcsc/common/logger.h>
#include <rcsc/action/body_intercept.h>
#include <rcsc/action/neck_turn_to_ball_or_scan.h>
#include <rcsc/action/neck_turn_to_low_conf_teammate.h>
#include "../actions/neck_offensive_intercept_neck.h"
#include "bhv_basic_tackle.h"

#include "../strategy.h"
#include "../cafe_model.h"

bool Bhv_Intercept::execute(rcsc::PlayerAgent *agent) {


    dlog.addText(Logger::TEAM,
                 "execute Bhv_intercept");

    const ServerParam &SP = ServerParam::i();
    const WorldModel &wm = agent->world();
    const CafeModel &cm = CafeModel::i();
    const Strategy &stra = Strategy::i();


    const Vector2D self_pos = wm.self().pos();
    const Vector2D ball_pos = wm.ball().pos();


    //-----------------------------------------------
    // tackle
    if ( Bhv_BasicTackle( 0.8, 80.0 ).execute( agent ) )
    {
        return true;
    }




    const int self_min = wm.interceptTable()->selfReachCycle();
    const int mate_min = wm.interceptTable()->teammateReachCycle();
    const int opp_min = wm.interceptTable()->opponentReachCycle();

    dlog.addText(Logger::TEAM,
                 __FILE__": ================= self_min : %d , mate_min: %d ,  opp_min: %d", self_min, mate_min,
                 opp_min);

    if (self_min <= opp_min && self_min <= mate_min) {

            dlog.addText(Logger::TEAM,
                         __FILE__": intercept");

            std::cout << " intercept BALL" << std::endl;

            Body_Intercept().execute(agent);
            agent->setNeckAction(new Neck_OffensiveInterceptNeck());
            return true;

    }


    return false;
}