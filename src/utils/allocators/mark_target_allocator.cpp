//
// Created by armanaxh on ۲۰۱۹/۹/۱۲.
//

#include "mark_target_allocator.h"
#include "../../strategy.h"
#include "../algo_utils.h"


bool MarkTargetAllocator::calculate() {

    target = NULL;
    assignments_table.clear();

    const Vector2D &ball_pos = wm.ball().pos();

    const ConstPlayerPtrCont defensive_players = getDefensivePlayers();
    const PlayerPtrCont denger_opp = getDengerOpponents();


    if (defensive_players.size() == 0 || denger_opp.size() == 0) {
        return false;
    }

    Hungarian::Matrix cost_matrix;
    createCostMatrix(cost_matrix, defensive_players, denger_opp);

    log_table(cost_matrix, "final Cost Matrix", defensive_players, denger_opp);//// log

    Hungarian::Result result = AlgoUtils::hungarianAssignment(cost_matrix, Hungarian::MODE_MINIMIZE_COST);


    if (result.success != true) {
        dlog.addText(Logger::MARK,
                     __FILE__": Hungarian NOT Success!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
        );
        return false;
    }

    ////////////////////////////////////////////////////////////
    Hungarian::Matrix solution = result.assignment;
    Hungarian::Matrix cost = result.cost;

    log_table(solution, "result solution", defensive_players, denger_opp);//// log
    log_table(cost, "result Cost Matrix", defensive_players, denger_opp);//// log
    /////////////////////////////////////////////////////////

    recordResults(result, cost_matrix, defensive_players, denger_opp);


    /////////////////////////////////////////////////
    log_draw(defensive_players, denger_opp);
    ///////////////////////////////////////////////

    return true;
}
//***************************************************************************

const ConstPlayerPtrCont MarkTargetAllocator::getDefensivePlayers() {
    std::vector<int> defence_players_unum = stra.getGroupPlayer(Defense);
    std::vector<int> halfback_players_unum = stra.getGroupPlayer(Halfback); // defensivePlayer
    std::vector<int> offence_players_unum = stra.getGroupPlayer(Offensive);


    std::vector<int> defensive_players_unum;
    defensive_players_unum.insert(defensive_players_unum.end(), halfback_players_unum.begin(),
                                  halfback_players_unum.end());
    defensive_players_unum.insert(defensive_players_unum.end(), defence_players_unum.begin(),
                                  defence_players_unum.end());
    defensive_players_unum.insert(defensive_players_unum.end(), offence_players_unum.begin(),
                                  offence_players_unum.end());

    ConstPlayerPtrCont defensive_players = cm.getOurPlayersByUnum(defensive_players_unum);

    return defensive_players;
}
//***************************************************************************

const PlayerPtrCont MarkTargetAllocator::getDengerOpponents() {
    const Vector2D &ball_pos = wm.ball().pos();


    double ball_radius = 6;
    if (ball_pos.x < 0) {
        ball_radius += 4;
    }

    double x_start = -60;
    double x_end = ball_pos.x + ball_radius;
    if (Strategy::defense_mode == Dangerous) {
        x_end = -25;
    }
    PlayerPtrCont opp_in_range = cm.getPlayerInRangeX(x_start, x_end, false);
    PlayerPtrCont denger_opps;

    const PlayerPtrCont::const_iterator end_dp = opp_in_range.end();
    for (PlayerPtrCont::const_iterator it = opp_in_range.begin(); it != end_dp; it++) {
        if ((*it)->unum() != -1 && (*it)->posValid()) {
//            if (Strategy::defense_mode == Dangerous) {
//                Vector2D pos = (*it)->pos();
//                if (pos.y < -20 || pos.y > 20) {
//                    continue;
//                }
//            }
            denger_opps.push_back((*it));
        }

    }
    return denger_opps;
}


//***************************************************************************
Hungarian::Matrix &MarkTargetAllocator::createCostMatrix(Hungarian::Matrix &cost_matrix,
                                                         const ConstPlayerPtrCont defensive_player,
                                                         const PlayerPtrCont denger_opp) {

    int i = 0;
    const ConstPlayerPtrCont::const_iterator end_dp = defensive_player.end();
    for (ConstPlayerPtrCont::const_iterator it_dp = defensive_player.begin(); it_dp != end_dp; it_dp++) {
        int j = 0;


        unsigned mate_unum = (*it_dp)->unum();
        Vector2D mate_form_pos = stra.getPosition(mate_unum);
        double max_cover_dist = stra.getPlayerZoneRadius(mate_unum);
        cost_matrix.push_back(std::vector<int>());
        const PlayerPtrCont::const_iterator end_op = denger_opp.end();
        for (PlayerPtrCont::const_iterator it_op = denger_opp.begin();
             it_op != end_op; it_op++) {
            double cost = calcStateCost(*it_dp, *it_op, mate_form_pos, max_cover_dist);
            cost_matrix[i].push_back(cost);
        }
        i++;
    }


    return cost_matrix;
}

#define MAX_COST 1000
#define MAX_DIST_COVER 17

//***************************************************************************
int MarkTargetAllocator::calcStateCost(const PlayerObject *our_p, const PlayerObject *opp_p, Vector2D mate_form_pos,
                                       double max_cover_dist) {

    const Vector2D &opp_pos = opp_p->pos();
    const Vector2D &our_pos = our_p->pos();

    double mate_to_opp_dist = our_pos.dist(opp_pos);
    double mate_form_to_opp_dist = mate_form_pos.dist(opp_pos);

    if (mate_form_to_opp_dist > max_cover_dist) {
        return MAX_COST;
    }

    if (mate_to_opp_dist > MAX_DIST_COVER) {
        return MAX_COST;
    }

    if(stra.getRoleGroup(our_p->unum()) == Defense){
        mate_to_opp_dist -= 0;
    }

    if(wm.self().unum() == our_p->unum()){
        mate_to_opp_dist *= 0.9;
    }

    if(our_pos.x  > opp_pos.x ){
        mate_to_opp_dist += our_pos.x - opp_pos.x;
    }

    return mate_to_opp_dist;
}

//***************************************************************************
PlayerObject *MarkTargetAllocator::getMarkTarget() {
    return target;
}

//***************************************************************************


void MarkTargetAllocator::recordResults(const Hungarian::Result &result, const Hungarian::Matrix &cost_matrix,
                                        const ConstPlayerPtrCont defensive_player,
                                        const PlayerPtrCont denger_opp) {

    Hungarian::Matrix solution = result.assignment;
    unsigned self_unum = wm.self().unum();

    for (int i = 0; i < defensive_player.size(); i++) {
        for (int j = 0; j < denger_opp.size(); j++) {
            if (solution[i][j]) {
                if (cost_matrix[i][j] >= MAX_COST) {
                    continue;
                }
                this->assignments_table[defensive_player[i]->unum()] = denger_opp[j];
                this->assignments_object_table[defensive_player[i]] = denger_opp[j];
                if (defensive_player[i]->unum() == self_unum) {
                    this->target = denger_opp[j];
                }
            }
        }
    }


}





/////////////////////////////////////////////////////////////////////////////////////////////
////DEBUG TOOLS


#include "../utils/utils.cpp"

void MarkTargetAllocator::log_table(const Hungarian::Matrix table, std::string name,
                                    const ConstPlayerPtrCont defensive_player,
                                    const PlayerPtrCont denger_opp) {

    if (table.size() == 0) {
        dlog.addText(Logger::MARK,
                     __FILE__" target mark assign table is NULLLLLLLL ");
    }

    dlog.addText(Logger::MARK,
                 __FILE__":   %s ----------------- %d %d", name.c_str(), table.size(), table[0].size());

    std::string temp;

    temp = patch::to_string(0) + "    ";
    for (int i = 0; i < denger_opp.size(); i++) {
        temp += patch::to_string(denger_opp[i]->unum()) + "\t";
    }
    dlog.addText(Logger::MARK,
                 __FILE__" %s ", temp.c_str());
    temp = "";
    for (int i = 0; i < defensive_player.size(); i++) {
        temp += patch::to_string(defensive_player[i]->unum()) + "    ";
        for (int j = 0; j < denger_opp.size(); j++) {
            temp += patch::to_string(table[i][j]) + "\t";
        }
        dlog.addText(Logger::MARK,
                     __FILE__" %s ", temp.c_str());
        temp = "";
    }
}


void MarkTargetAllocator::log_draw(const ConstPlayerPtrCont &defensive_player,
                                   const PlayerPtrCont &denger_opp) {


    const ConstPlayerPtrCont::const_iterator end_dp = defensive_player.end();
    for (ConstPlayerPtrCont::const_iterator it = defensive_player.begin(); it != end_dp; it++) {
        dlog.addRect(Logger::ROLE,
                     (*it)->pos().x - 2, (*it)->pos().y - 2, 4, 4,
                     "#0000ff");
        unsigned mate_unum = (*it)->unum();
        double max_cover_dist = stra.getPlayerZoneRadius(mate_unum);

        dlog.addText(Logger::MARK,
                     __FILE__": max_cover_dist---- %d :-:> %.2f",
                     mate_unum, max_cover_dist);

        Vector2D pos = stra.getPosition(mate_unum);
        dlog.addCircle(Logger::MARK,
                       pos, max_cover_dist, "#ed93e3", false);

    }

    const PlayerPtrCont::const_iterator end_op = denger_opp.end();
    for (PlayerPtrCont::const_iterator it = denger_opp.begin(); it != end_op; it++) {
        dlog.addRect(Logger::ROLE,
                     (*it)->pos().x - 2, (*it)->pos().y - 2, 4, 4,
                     "#ff0000");
    }


    assignmentObjectTable::const_iterator end_t = assignments_object_table.end();
    for (assignmentObjectTable::const_iterator it = assignments_object_table.begin(); it != end_t; it++) {
        const PlayerObject *mate_p = it->first;
        const PlayerObject *opp_p = it->second;

        dlog.addLine(Logger::ROLE,
                     mate_p->pos(), opp_p->pos(),
                     "#a009db");
    }
}