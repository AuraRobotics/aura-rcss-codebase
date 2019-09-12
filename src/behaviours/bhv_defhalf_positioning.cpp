//
// Created by armanaxh on ۲۰۱۹/۹/۵.
//

#include "bhv_defhalf_positioning.h"
#include <rcsc/player/player_agent.h>
#include <rcsc/common/logger.h>
#include <rcsc/common/server_param.h>
#include <rcsc/player/intercept_table.h>
#include <rcsc/geom/vector_2d.h>
#include <rcsc/geom/line_2d.h>
#include <iostream>

#include "../cafe_model.h"
#include "../strategy.h"
#include "../utils/algo_utils.h"
#include "../utils/geo_utils.h"
#include "../utils/rcsc_utils.h"
#include "bhv_mark.h"

using namespace rcsc;

bool Bhv_DefhalfPositioning::execute(rcsc::PlayerAgent *agent) {


//    const WorldModel &wm = agent->world();
//    const Strategy &stra = Strategy::i();
//    /*--------------------------------------------------------*/
//    // chase ball
//    const int self_min = wm.interceptTable()->selfReachCycle();
//    const int mate_min = wm.interceptTable()->teammateReachCycle();
//    const int opp_min = wm.interceptTable()->opponentReachCycle();
//
//    if (wm.existKickableOpponent() || (self_min > opp_min + 3 && mate_min > opp_min + 3)) {
//        return false;
//    }


    if (defendeTheDengerArea(agent)) {
        return true;
    }
    return false;
}

bool Bhv_DefhalfPositioning::defendeTheDengerArea(rcsc::PlayerAgent *agent) {

    const WorldModel &wm = agent->world();
    const ServerParam &SP = ServerParam::i();
    const Strategy &stra = Strategy::i();
    const CafeModel &cm = CafeModel::i();

    Strategy::BallArea ball_area = stra.get_ball_area(wm);
    const Vector2D ball_pos_lord = cm.getBallLord();//wm.ball().pos();


    std::vector<int> defhalf_players_unum = stra.getGroupPlayer(Halfback); // defensivePlayer

    if (ball_area == Strategy::BA_Danger || ball_area == Strategy::BA_CrossBlock || ball_pos_lord.x < -31) {
        std::vector<int> defensive_players_unum = stra.getGroupPlayer(Defense);
        defhalf_players_unum.insert(defhalf_players_unum.end(), defensive_players_unum.begin(),
                                    defensive_players_unum.end());
    }


    if (std::find(defhalf_players_unum.begin(), defhalf_players_unum.end(), wm.self().unum()) ==
        defhalf_players_unum.end()) {
        return false;
    }

    /////danger opponents
    ConstPlayerPtrCont defhalf_players = cm.getOurPlayersByUnum(defhalf_players_unum);
    PlayerPtrCont denger_opp = getDengerOpponent(agent);


    if (denger_opp.empty()) {
        return false;
    }

    const PlayerObject *target_opp = assignOpponent(defhalf_players, denger_opp,
                                                    wm.self().unum(), wm);//TODO NULL check
    if (target_opp == NULL) {
        return false;
    }

    if (Bhv_Mark(target_opp).execute(agent)) {
        return true;
    }

    return false;
}


rcsc::PlayerPtrCont Bhv_DefhalfPositioning::getDengerOpponent(rcsc::PlayerAgent *agent) {


    const CafeModel &cm = CafeModel::i();
    const WorldModel &wm = agent->world();
    const Strategy &stra = Strategy::i();
    const ServerParam &SP = ServerParam::i();

    const Vector2D ball_pos_lord = cm.getBallLord();//wm.ball().pos();

    double defence_line = stra.getDeffanceLine() - 3;//TODO goto getPlayerInRange
    double offside_line = cm.getOurOffsideLine();
    double half_area_black = 0;/////////////6;
    double after_ball = 15;

    double x_start = std::min(defence_line, offside_line) + half_area_black;
    double x_end = ball_pos_lord.x + after_ball;

    Strategy::BallArea ball_area = stra.get_ball_area(wm);

    if (ball_area == Strategy::BA_Danger || ball_area == Strategy::BA_CrossBlock || ball_pos_lord.x < -31) {
        x_start = -60;
        x_end = -25;
//        return PlayerPtrCont();
    }


    PlayerPtrCont opponents = cm.getPlayerInRangeX(x_start, x_end, false);



    ///////////////////////////////////////////////// DEBUG
    dlog.addLine(Logger::TEAM,
                 Vector2D(x_start, -35), Vector2D(x_start, 35),
                 "#ffff00");
    dlog.addLine(Logger::TEAM,
                 Vector2D(x_end, -35), Vector2D(x_end, 35),
                 "#ffff00");

    dlog.addText(Logger::TEAM,
                 __FILE__": Max dist ball : %.2f",
                 rcscUtils::maxDistBall());


////////////////////////////////////////////////////////////////////
    PlayerPtrCont coveredOpponet = defenceLineAssignment(agent);

//////////////////////////////////////////////////////

    FastIC *fast_ic = CafeModel::fastIC();
    fast_ic->setMaxCycleAfterFirstFastestPlayer(10);

    const Vector2D ball_next_pos = cm.getBallLord();
    PlayerPtrCont denger_opp;

    const PlayerPtrCont::iterator end_t = opponents.end();
    for (PlayerPtrCont::iterator it = opponents.begin(); it != end_t; it++) {
        if ((*it)->distFromBall() < 4 || (*it)->isKickable()) {
            continue;
        }
        if ((*it)->distFromBall() > rcscUtils::ballPathDistWithMaxSpeed(15)) {
            dlog.addText(Logger::TEAM,
                         __FILE__": continue opponet ball dist more %.2f", rcscUtils::ballPathDistWithMaxSpeed(10));
            continue;
        }

        Vector2D it_pos = (*it)->pos();
        if (ball_area == Strategy::BA_Danger || ball_area == Strategy::BA_CrossBlock) {
            if (it_pos.x > -33 || it_pos.y > std::abs(20)) {
                continue;
            }
        }


        bool flag_coverd = false;
        const PlayerPtrCont::iterator end_t = coveredOpponet.end();
        for (PlayerPtrCont::iterator it_co = coveredOpponet.begin(); it_co != end_t; it_co++) {
            if( (*it) == (*it_co)){

                dlog.addText(Logger::TEAM,
                             __FILE__": remove %d defence line opp over by %d ",
                        (*it)->unum(), (*it_co)->unum());
                flag_coverd = true;
                break;
            }
        }

        if(flag_coverd){
            continue;
        }




//        //TODO dist BAll
//        {
//            Vector2D ball_vel_temp = it_pos - ball_next_pos;
//            ball_vel_temp.setLength(SP.ballSpeedMax());
//
//            fast_ic->reset();
//            fast_ic->setBall(ball_next_pos, ball_vel_temp, 0);
////            for (int i = 0; i < wm.theirPlayers().size(); i++) {
////                if (wm.theirPlayers()[i]->pos().dist(ball_next_pos) < 6) {
////                    continue;
////                }
////                if (FastIC::isPlayerValid(wm.theirPlayers()[i])) {
////                    int delay = 1, ext = bound(0, wm.theirPlayers()[i]->posCount(), 3);
////                    fast_ic->addPlayer(wm.theirPlayers()[i], 1.0, 1.0, max(0, delay - ext),
////                                       max(0, ext - delay));
////                }
////            }
//            {
//                if (FastIC::isPlayerValid((*it))) {
//                    int delay = 1, ext = bound(0, (*it)->posCount(), 3);
//                    fast_ic->addPlayer((*it), 1.0, 1.0, max(0, delay - ext),
//                                       max(0, ext - delay));
//                }
//            }
//
//            for (int i = 0; i < wm.ourPlayers().size(); i++) {
//                if (stra.getRoleGroup(wm.ourPlayers()[i]->unum()) == Halfback) {
//                    continue;
//                }
//                if (FastIC::isPlayerValid(wm.ourPlayers()[i])) {
//                    int delay = 1, ext = bound(0, wm.ourPlayers()[i]->posCount(), 3);
//                    fast_ic->addPlayer(wm.ourPlayers()[i], 1.0, 1.0, max(0, delay - ext),
//                                       max(0, ext - delay));
//                }
//            }
//            fast_ic->refresh();
//            fast_ic->calculate();
//            int opp_cycle = fast_ic->getFastestOpponentReachCycle();
//            int mate_cycle = fast_ic->getFastestTeammateReachCycle(true);
//
//            dlog.addText(Logger::TEAM,
//                         __FILE__": fast IC in opponet mark  opp_unum: %d))) mate : %d , opp: %d",
//                         (*it)->unum(), mate_cycle, opp_cycle);
//            if (mate_cycle < opp_cycle) {
//
//                continue;
//            }
//        }

        denger_opp.push_back(&(**it));


        dlog.addRect(Logger::TEAM,
                     (*it)->pos().x - 2, (*it)->pos().y - 2, 4, 4,
                     "#ebeb34");
    }


    return denger_opp;
}


const rcsc::PlayerObject *Bhv_DefhalfPositioning::assignOpponent(rcsc::ConstPlayerPtrCont def_ps,
                                                                 rcsc::PlayerPtrCont opp_ps, int self_unum,
                                                                 const rcsc::WorldModel &wm) {

    const Strategy &stra = Strategy::i();
    const Vector2D ball_pos = wm.ball().pos();
    const Strategy::BallArea ball_area = Strategy::get_ball_area(wm);


    Hungarian::Matrix cost_m;

    if (def_ps.empty() || opp_ps.empty()) {
        return NULL;
    }


    int i = 0;
    const ConstPlayerPtrCont::const_iterator end_dpe = def_ps.end();
    for (ConstPlayerPtrCont::const_iterator itd = def_ps.begin();
         itd != end_dpe; itd++) {
        int j = 0;
        cost_m.push_back(std::vector<int>());
        const PlayerPtrCont::const_iterator opp_ope = opp_ps.end();
        for (PlayerPtrCont::const_iterator ito = opp_ps.begin();
             ito != opp_ope; ito++) {
            double dist;

            Vector2D mate_pos = stra.getPosition((*itd)->unum());//(*itd)->pos();
            Vector2D opp_pos = (*ito)->pos();

            Segment2D opp_to_ball(opp_pos, ball_pos);

            Vector2D foot = geoUtils::findFoot(opp_to_ball, mate_pos);
            if (ball_area == Strategy::BA_Danger || ball_area == Strategy::BA_CrossBlock) {
                dist = opp_pos.dist(mate_pos);
            } else if (foot != Vector2D::INVALIDATED) {
                dist = foot.dist(mate_pos);
            } else {
                dist = opp_pos.dist(mate_pos) + 8;
            }

            cost_m[i].push_back(dist);
        }
        i++;
    }


    Hungarian::Result result = AlgoUtils::hungarianAssignment(cost_m, Hungarian::MODE_MINIMIZE_COST);

    if (result.success != true) {
        return NULL;
    }

    Hungarian::Matrix solution = result.assignment;
//    /////////////////////////////////TODO DEBUG
//    if(self_unum == 7) {
//        std::cout << "-------- opp_ps_size: " <<  opp_ps.size() << "----- solution_size: " << solution.size() << std::endl;
//        for (int i = 0; i < solution.size(); i++) {
//            for (int j = 0; j < solution[i].size(); j++) {
//                std::cout << solution[i][j] << " ,";
//            }
//            std::cout << std::endl;
//        }
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







///////////////////////////////////////////////////////////////////////////////////// assign defence Line





rcsc::PlayerPtrCont Bhv_DefhalfPositioning::defenceLineAssignment(rcsc::PlayerAgent *agent) {

    const WorldModel &wm = agent->world();
    const ServerParam &SP = ServerParam::i();
    const Strategy &stra = Strategy::i();
    const CafeModel &cm = CafeModel::i();


    std::vector<int> defensive_players_unum = stra.getGroupPlayer(Defense);

    /////danger opponents
    ConstPlayerPtrCont defensive_players = cm.getOurPlayersByUnum(defensive_players_unum);
    PlayerPtrCont denger_opp_on_line = getDengerOpponentDefendeLine(agent);

    if (denger_opp_on_line.empty()) {
        return PlayerPtrCont();
    }

    const rcsc::PlayerPtrCont target_opp = assignOpponentToDefenceLine(defensive_players, denger_opp_on_line,
                                                                       wm.self().unum());//TODO NULL check
    return target_opp;
}

rcsc::PlayerPtrCont Bhv_DefhalfPositioning::getDengerOpponentDefendeLine(rcsc::PlayerAgent *agent) {

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

        denger_opp.push_back(&(**it));
    }


    return denger_opp;
}

const rcsc::PlayerPtrCont
Bhv_DefhalfPositioning::assignOpponentToDefenceLine(ConstPlayerPtrCont def_ps, PlayerPtrCont opp_ps,
                                                    int self_unum) {


    const Strategy &stra = Strategy::i();
    Hungarian::Matrix cost_m;

    if (def_ps.empty() || opp_ps.empty()) {
        return PlayerPtrCont();
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
        return PlayerPtrCont();
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
    PlayerPtrCont denger_opp;

    for (int j = 0; j < opp_ps.size(); j++) {
        for (int i = 0; i < def_ps.size(); i++) {
            if (solution[i][j]) {

                dlog.addRect(Logger::TEAM,
                             opp_ps[j]->pos().x - 1, opp_ps[j]->pos().y - 1, 2, 2,
                             "#0f0f00");

                denger_opp.push_back(opp_ps[j]);

                break;
            }
        }
    }

    return denger_opp;
}
