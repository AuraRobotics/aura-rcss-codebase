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
#include "../../actions/neck_offensive_intercept_neck.h"
#include "bhv_basic_tackle.h"

#include "../../strategy.h"
#include "../../cafe_model.h"

bool Bhv_Intercept::execute(rcsc::PlayerAgent *agent) {


    dlog.addText(Logger::TEAM,
                 "execute Bhv_intercept");

    const ServerParam &SP = ServerParam::i();
    const WorldModel &wm = agent->world();
    const CafeModel &cm = CafeModel::i();
    const Strategy &stra = Strategy::i();


    const Vector2D self_pos = wm.self().pos();
    const Vector2D ball_pos = wm.ball().pos();
    const int self_unum = wm.self().unum();

    //-----------------------------------------------
    // tackle
    if (Bhv_BasicTackle(0.8, 80.0).execute(agent)) {
        return true;
    }


    const int self_min = wm.interceptTable()->selfReachCycle();
    const int mate_min = wm.interceptTable()->teammateReachCycle();
    const int opp_min = wm.interceptTable()->opponentReachCycle();

    dlog.addText(Logger::TEAM,
                 __FILE__": ================= self_min : %d , mate_min: %d ,  opp_min: %d", self_min, mate_min,
                 opp_min);
    bool flag_intercept = false;
    if (self_min <= opp_min || (self_min <= opp_min + 3 && opp_min != 0)) {
        if(self_min + 1 <= mate_min){
            flag_intercept = true;
        }


        if (std::abs(self_min -  mate_min) < 1) {

            const PlayerObject * fastest_mate = wm.interceptTable()->fastestTeammate();
            if(fastest_mate != NULL){
                const int unum_mate = fastest_mate->unum();
                if(unum_mate != -1){
                    if(self_unum < unum_mate){
                        flag_intercept = true;
                    }
                }
            }

        }


    }

//    if(!flag_intercept && self_min - 3 <= opp_min && self_min <= mate_min){
//        Bhv_Blo
//    }

    if(flag_intercept && self_min < mate_min && mate_min + 2 < opp_min && std::abs(self_min - mate_min) <= 3 ){

        const PlayerObject * fastest_mate = wm.interceptTable()->fastestTeammate();
        if(fastest_mate != NULL){
            const int unum_mate = fastest_mate->unum();
            if(unum_mate != -1){
                if(self_unum > unum_mate){
                    flag_intercept = false;
                    std::cout << " INTERCEPT FLAG FALSE UNUM mmmmmmmmmmmmmmmmmmmmmmmm_))()()()()()()()()( " << std::endl;
                }
            }
        }

    }



//    const PlayerObject * ball_lord = cm.getBallLord();
//    if(ball_lord != NULL){
//        if(ball_lord->side() == wm.ourSide()){
//            flag_intercept = false;
//        }
//    }

    if(flag_intercept){
        dlog.addText(Logger::TEAM,
                     __FILE__": intercept");

        std::cout << " intercept BALL" << std::endl;

//        HM_Intercept intercept(agent);
//        intercept.calculate();
//        intercept.execute();



        Body_Intercept().execute(agent);
        agent->setNeckAction(new Neck_OffensiveInterceptNeck());
        return true;
    }


    return false;
}