//
// Created by armanaxh on ۲۰۱۹/۸/۲۹.
//


#include <algorithm>
#include <iostream>


#include "bhv_defensive_positioning.h"

#include <rcsc/player/player_agent.h>
#include <rcsc/common/logger.h>
#include <rcsc/common/server_param.h>
#include <rcsc/player/intercept_table.h>
#include "../cafe_model.h"
#include "../strategy.h"
#include "../utils/algo_utils.h"
#include "../utils/geo_utils.h"
#include "../utils/rcsc_utils.h"
#include "bhv_mark_deep.h"
#include "bhv_mark.h"

using namespace rcsc;


bool Bhv_DefensivePositioning::execute(rcsc::PlayerAgent *agent) {

//    const WorldModel & wm = agent->world();
//    const Strategy & stra = Strategy::i();
//    /*--------------------------------------------------------*/
//    // chase ball
//    const int self_min = wm.interceptTable()->selfReachCycle();
//    const int mate_min = wm.interceptTable()->teammateReachCycle();
//    const int opp_min = wm.interceptTable()->opponentReachCycle();
//
//    if(wm.existKickableOpponent() || (self_min > opp_min + 3 && mate_min > opp_min + 3) ){
//        return false;
//    }


    if (defendTheDefendLine(agent)) {
        return true;
    } else if (positioningDengerArea(agent)) {
        return true;
    }

    return false;
}


bool Bhv_DefensivePositioning::defendTheDefendLine(rcsc::PlayerAgent *agent) {

    const WorldModel &wm = agent->world();
    const ServerParam &SP = ServerParam::i();
    const Strategy &stra = Strategy::i();
    const CafeModel &cm = CafeModel::i();


    std::vector<int> defensive_players_unum = stra.getGroupPlayer(Defense);

    if (std::find(defensive_players_unum.begin(), defensive_players_unum.end(), wm.self().unum()) ==
        defensive_players_unum.end()) {
        return false;
    }

    /////danger opponents
    ConstPlayerPtrCont defensive_players = cm.getOurPlayersByUnum(defensive_players_unum);
    PlayerPtrCont denger_opp_on_line = getDengerOpponent(agent);

    if (denger_opp_on_line.empty()) {
        return false;
    }

    const PlayerObject *target_opp = assignOpponent(defensive_players, denger_opp_on_line,
                                                    wm.self().unum());//TODO NULL check
    if (target_opp == NULL) {
        return false;
    }

    dlog.addLine(Logger::TEAM,
                 wm.self().pos(), target_opp->pos(),
                 "#ff0010");

    dlog.addRect(Logger::TEAM,
                 target_opp->pos().x - 2, target_opp->pos().y - 2, 4, 4,
                 "#ff2200");


    if (Bhv_MarkDeep(target_opp).execute(agent)) {
        //TODO check without execute
        return true;
    }
    return false;

}


rcsc::PlayerPtrCont Bhv_DefensivePositioning::getDengerOpponent(rcsc::PlayerAgent *agent) {

    const CafeModel &cm = CafeModel::i();
    const WorldModel &wm = agent->world();
    const Strategy &stra = Strategy::i();

    const Vector2D ball_pos_lord = cm.getBallLord();//wm.ball().pos();

    double defence_line = stra.getDeffanceLine() - 3;//TODO goto getPlayerInRange
    double offside_line = cm.getOurOffsideLine();
    double defence_radus = 13;//TODO 5 cycle

    double x_start = std::min(defence_line, offside_line);
    double x_end = std::max(x_start + defence_radus, offside_line + defence_radus);

    Strategy::BallArea ball_area = stra.get_ball_area(wm);

    if (ball_area == Strategy::BA_Danger || ball_area == Strategy::BA_CrossBlock || ball_pos_lord.x < -31) {
        x_start = -60;
        x_end = -33;
        return PlayerPtrCont();//TODO fix return NULL
    } else {
        x_end = std::min(x_end, ball_pos_lord.x + 3);
    }


    PlayerPtrCont opponents = cm.getPlayerInRangeX(x_start, x_end, false);


    ///////////////////////////////////////////////// DEBUG
    dlog.addLine(Logger::TEAM,
                 Vector2D(x_start, -30), Vector2D(x_start, 30),
                 "#00ffff");
    dlog.addLine(Logger::TEAM,
                 Vector2D(x_end, -30), Vector2D(x_end, 30),
                 "#00ffff");

    dlog.addText(Logger::TEAM,
                 __FILE__": Max dist ball : %.2f",
                 rcscUtils::maxDistBall());


////////////////////////////////////////////////////////////////////
    //TODO for now
    PlayerPtrCont denger_opp;

    const PlayerPtrCont::iterator end_t = opponents.end();
    for (PlayerPtrCont::iterator it = opponents.begin(); it != end_t; it++) {
        if ((*it)->distFromBall() < 4 || (*it)->isKickable()) {
            continue;
        }
        if ((*it)->distFromBall() > rcscUtils::maxDistBall()) {
            continue;
        }


        dlog.addRect(Logger::TEAM,
                     (*it)->pos().x - 2, (*it)->pos().y - 2, 4, 4,
                     "#ebeb34");
        denger_opp.push_back(&(**it));
    }


    return denger_opp;
}

const PlayerObject *
Bhv_DefensivePositioning::assignOpponent(ConstPlayerPtrCont def_ps, PlayerPtrCont opp_ps, int self_unum) {


    const Strategy &stra = Strategy::i();
    Hungarian::Matrix cost_m;

    if (def_ps.empty() || opp_ps.empty()) {
        return NULL;
    }


    int i = 0;
    const ConstPlayerPtrCont::const_iterator end_dps = def_ps.end();
    for (ConstPlayerPtrCont::const_iterator itd = def_ps.begin();
         itd != end_dps; itd++) {
        int j = 0;
        cost_m.push_back(std::vector<int>());
        const PlayerPtrCont::const_iterator opp_ops = opp_ps.end();
        for (PlayerPtrCont::const_iterator ito = opp_ps.begin();
             ito != opp_ops; ito++) {
            const Vector2D opp_pos = (*ito)->pos();
            const Vector2D mate_pos = stra.getPosition((*itd)->unum());//(*itd)->pos();

            double dist2 = opp_pos.dist2(mate_pos);
            cost_m[i].push_back(dist2);
        }
        i++;
    }

    Hungarian::Result result = AlgoUtils::hungarianAssignment(cost_m, Hungarian::MODE_MINIMIZE_COST);

    if (result.success != true) {
        return NULL;
    }

    Hungarian::Matrix solution = result.assignment;
//    /////////////////////////////////TODO DEBUG
//    for (int i = 0; i < solution.size(); i++) {
//        for (int j = 0; j < solution[i].size(); j++) {
//            std::cout << solution[i][j] << " ,";
//        }
//        std::cout << std::endl;
//    }
//    //////////////////////////////////////////

    for (int j = 0; j < opp_ps.size(); j++) {
        for (int i = 0; i < def_ps.size(); i++) {
            if (solution[i][j]) {
                dlog.addLine(Logger::TEAM,
                             def_ps[i]->pos(), opp_ps[j]->pos(),
                             "#4400ff");
                if (def_ps[i]->unum() == self_unum) {
                    return opp_ps[j];
                }
                break;
            }
        }
    }

    return NULL;
}



#include <rcsc/action/basic_actions.h>
#include <rcsc/action/body_go_to_point.h>
#include <rcsc/action/neck_turn_to_ball_or_scan.h>
#include <rcsc/action/neck_turn_to_low_conf_teammate.h>


bool Bhv_DefensivePositioning::positioningDengerArea(rcsc::PlayerAgent *agent) {

    const WorldModel &wm = agent->world();
    const ServerParam &SP = ServerParam::i();
    const Strategy &stra = Strategy::i();
    const CafeModel &cm = CafeModel::i();


    const Vector2D ball_pos_lord = cm.getBallLord();//wm.ball().pos();
    const Strategy::BallArea ball_area = stra.get_ball_area(ball_pos_lord);

    if (ball_area != Strategy::BA_Danger && ball_area != Strategy::BA_CrossBlock && ball_pos_lord.x > -31) {
        return false;
    }


    std::vector<int> defensive_players_unum = stra.getGroupPlayer(Defense);
    std::vector<int> defensive_halfBack_unum = stra.getGroupPlayer(Halfback);
    defensive_players_unum.insert(defensive_players_unum.end(), defensive_halfBack_unum.begin(),
                                  defensive_halfBack_unum.end());

    if (std::find(defensive_players_unum.begin(), defensive_players_unum.end(), wm.self().unum()) ==
        defensive_players_unum.end()) {
        return false;
    }


    /////danger opponents
    ConstPlayerPtrCont defensive_players = cm.getOurPlayersByUnum(defensive_players_unum);
    PlayerPtrCont denger_opp_panalty_area = cm.getPlayerInBallArea(Strategy::BA_Danger, false);

    if (denger_opp_panalty_area.empty()) {
        return false;
    }

    ///////////////////////DEBUG

    const PlayerPtrCont::iterator end_t = denger_opp_panalty_area.end();
    for (PlayerPtrCont::iterator it = denger_opp_panalty_area.begin(); it != end_t; it++) {
        dlog.addRect(Logger::TEAM,
                     (*it)->pos().x - 2, (*it)->pos().y - 2, 4, 4,
                     "#ebeb34");
    }

    ////


    const PlayerObject *target_opp = assignOpponentInPenaltyArea(defensive_players, denger_opp_panalty_area,
                                                    wm.self().unum());//TODO NULL check
    if (target_opp == NULL) {
        return false;
    }

    dlog.addLine(Logger::TEAM,
                 wm.self().pos(), target_opp->pos(),
                 "#ff0010");

    dlog.addRect(Logger::TEAM,
                 target_opp->pos().x - 2, target_opp->pos().y - 2, 4, 4,
                 "#ff2200");


//    if (Bhv_Mark(target_opp).execute(agent)) {
//        //TODO check without execute
//        return true;
//    }


    Vector2D target_point = target_opp->pos() + target_opp->vel() ;//TODO clac exact
    Vector2D target_to_goal =   Vector2D(-52, 0) - target_point;//TODO goal x
    target_to_goal.setLength(1.2);

    target_point += target_to_goal;

    const double dash_power = Strategy::get_normal_dash_power(wm, stra);

    double dist_thr = wm.ball().distFromSelf() * 0.04;
    if (dist_thr < 1.0) dist_thr = 1.0;

    dlog.addText(Logger::TEAM,
                 __FILE__": Bhv_Mark target=(%.1f %.1f) dist_thr=%.2f",
                 target_point.x, target_point.y,
                 dist_thr);


    if (!Body_GoToPoint(target_point, dist_thr, dash_power
    ).execute(agent)) {
        Body_TurnToBall().execute(agent);
    }

    if (wm.existKickableOpponent()
        && wm.ball().distFromSelf() < 18.0) {
        agent->setNeckAction(new Neck_TurnToBall());
    } else {
        agent->setNeckAction(new Neck_TurnToBallOrScan());
    }

    return true;
}


const PlayerObject *
Bhv_DefensivePositioning::assignOpponentInPenaltyArea(ConstPlayerPtrCont def_ps, PlayerPtrCont opp_ps, int self_unum) {


    const Strategy &stra = Strategy::i();
    Hungarian::Matrix cost_m;

    if (def_ps.empty() || opp_ps.empty()) {
        return NULL;
    }


    int i = 0;
    const ConstPlayerPtrCont::const_iterator end_dps = def_ps.end();
    for (ConstPlayerPtrCont::const_iterator itd = def_ps.begin();
         itd != end_dps; itd++) {
        int j = 0;
        cost_m.push_back(std::vector<int>());
        const PlayerPtrCont::const_iterator opp_ops = opp_ps.end();
        for (PlayerPtrCont::const_iterator ito = opp_ps.begin();
             ito != opp_ops; ito++) {
            const Vector2D opp_pos = (*ito)->pos();
            const Vector2D mate_pos = (*itd)->pos();//stra.getPosition((*itd)->unum());//

            double dist2 = opp_pos.dist2(mate_pos);
            cost_m[i].push_back(dist2);
        }
        i++;
    }

    Hungarian::Result result = AlgoUtils::hungarianAssignment(cost_m, Hungarian::MODE_MINIMIZE_COST);

    if (result.success != true) {
        return NULL;
    }

    Hungarian::Matrix solution = result.assignment;
//    /////////////////////////////////TODO DEBUG
//    for (int i = 0; i < solution.size(); i++) {
//        for (int j = 0; j < solution[i].size(); j++) {
//            std::cout << solution[i][j] << " ,";
//        }
//        std::cout << std::endl;
//    }
//    //////////////////////////////////////////

    for (int j = 0; j < opp_ps.size(); j++) {
        for (int i = 0; i < def_ps.size(); i++) {
            if (solution[i][j]) {
                dlog.addLine(Logger::TEAM,
                             def_ps[i]->pos(), opp_ps[j]->pos(),
                             "#4400ff");
                if (def_ps[i]->unum() == self_unum) {
                    return opp_ps[j];
                }
                break;
            }
        }
    }

    return NULL;
}

