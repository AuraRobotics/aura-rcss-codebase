//
// Created by armanaxh on ۲۰۱۹/۸/۲۹.
//

#include <rcsc/common/logger.h>
#include "cafe_model.h"
#include <rcsc/player/intercept_table.h>

using namespace rcsc;


CafeModel::CafeModel() {

}

double CafeModel::our_offside_line = 0;
FastIC *CafeModel::fic;


CafeModel &CafeModel::instance() {
    static CafeModel s_instance;
    return s_instance;
}

void CafeModel::update(PlayerAgent *agent) {
    fic = new FastIC(agent);
    fic->setByWorldModel();


    calcOurOffsideLine();
    player_rel->calc(agent, fic);
}

FastIC *CafeModel::fastIC() {
    return CafeModel::fic;
}

void CafeModel::create(const rcsc::WorldModel &wm, PlayerAgent *agent) {
    this->wm = &wm;

    player_rel = new PlayerRelationship(wm);
}


ConstPlayerPtrCont CafeModel::getOurPlayersByUnum(std::vector<int> players_unum) const {
    ConstPlayerPtrCont temp_player;

    for (int i = 0; i < players_unum.size(); i++) {
        int unum = players_unum[i];

        if (unum <= 0 || 11 < unum) continue;
        const PlayerObject *p_temp = static_cast<const PlayerObject *>(wm->ourPlayer(unum));
        if (p_temp == NULL) continue;
        temp_player.push_back(p_temp);

    }

    return temp_player;
}


rcsc::ConstPlayerPtrCont CafeModel::getTheirPlayersByUnum(std::vector<int> players_unum) const {
    ConstPlayerPtrCont temp_player;

    for (int i = 0; i < players_unum.size(); i++) {
        int unum = players_unum[i];

        if (unum <= 0 || 11 < unum) continue;
        const PlayerObject *p_temp = static_cast<const PlayerObject *>(wm->theirPlayer(unum));
        if (p_temp == NULL) continue;
        temp_player.push_back(p_temp);

    }

    return temp_player;
}

rcsc::PlayerPtrCont CafeModel::getPlayerInRangeX(double x1, double x2, bool ourTeam) const {
    if (ourTeam == true) {
        return getPlayerInRangeX(wm->teammatesFromSelf(), x1, x2);
    }
    return getPlayerInRangeX(wm->opponentsFromSelf(), x1, x2);
}

rcsc::PlayerPtrCont CafeModel::getPlayerInRangeX(const rcsc::PlayerPtrCont player, double x1, double x2) const {
    PlayerPtrCont temp_player;
    const PlayerPtrCont::const_iterator p_end = player.end();
    for (PlayerPtrCont::const_iterator it = player.begin();
         it != p_end;
         ++it) {
        if ((*it)->pos().x > x1 && (*it)->pos().x < x2) {
            temp_player.push_back(*it);
        }
    }
    return temp_player;
}

rcsc::PlayerPtrCont CafeModel::getPlayerInRect(rcsc::PlayerPtrCont player, double x1, double x2, double y1,
                                               double y2) const {
    PlayerPtrCont temp_player;
    const PlayerPtrCont::const_iterator p_end = player.end();
    for (PlayerPtrCont::const_iterator it = player.begin();
         it != p_end;
         ++it) {
        Vector2D p_pos = (*it)->pos();
        if (p_pos.x > x1 && p_pos.x < x2 && p_pos.y > y1 && p_pos.y < y2) {
            temp_player.push_back(*it);
        }
    }
    return temp_player;
}


rcsc::PlayerPtrCont CafeModel::getPlayerInRangeGoal(double dist, bool ourTeam) const {
    if (ourTeam == true) {
        return getPlayerInRangeGoal(wm->teammatesFromSelf(), dist);
    }
    return getPlayerInRangeGoal(wm->opponentsFromSelf(), dist);
}

rcsc::PlayerPtrCont CafeModel::getPlayerInRangeGoal(const rcsc::PlayerPtrCont player, double dist) const {
    PlayerPtrCont temp_player;

    const PlayerPtrCont::const_iterator p_end = player.end();
    for (PlayerPtrCont::const_iterator it = player.begin();
         it != p_end;
         ++it) {
        if (ServerParam::i().ourTeamGoalPos().dist((*it)->pos()) < dist) {
            temp_player.push_back(*it);
        }
    }

    return temp_player;
}

rcsc::PlayerPtrCont CafeModel::getPlayerInBallArea(Strategy::BallArea ball_area, bool ourTeam) const {
    if (ourTeam == true) {
        return getPlayerInBallArea(wm->teammatesFromSelf(), ball_area);
    }
    return getPlayerInBallArea(wm->opponentsFromSelf(), ball_area);
}

rcsc::PlayerPtrCont CafeModel::getPlayerInBallArea(rcsc::PlayerPtrCont player, Strategy::BallArea ball_area) const {
    PlayerPtrCont temp_player;
    const PlayerPtrCont::const_iterator p_end = player.end();
    for (PlayerPtrCont::const_iterator it = player.begin();
         it != p_end;
         ++it) {
        if (Strategy::get_ball_area((*it)->pos()) == ball_area) {
            temp_player.push_back(*it);
        }
    }
    return temp_player;
}


double CafeModel::calcOurOffsideLine() const {
    const PlayerCont teammates = wm->teammates();
    const int goalie_unum = wm->ourGoalieUnum();


    double offside_line_x = 0;
    const PlayerCont::const_iterator end_t = teammates.end();
    for (PlayerCont::const_iterator it = teammates.begin(); it != end_t; it++) {

        int unum = it->unum();
        if (goalie_unum == unum) {
            continue;
        }
        if (unum == Unum_Unknown) { //TODO not true
            continue;
        }
        if (goalie_unum == Unum_Unknown && it->pos().x < -45) {
            continue;
        } // TODO brodcast Goalie position

        double pos_x = it->pos().x;
        if (pos_x < offside_line_x) {
            offside_line_x = pos_x;
        }

    }


    if (wm->ball().pos().x < offside_line_x) {
        offside_line_x = wm->ball().pos().x;
    }
    if (wm->self().pos().x < offside_line_x) {
        offside_line_x = wm->self().pos().x;
    }

    this->our_offside_line = offside_line_x;
    return offside_line_x;
}

double CafeModel::getOurOffsideLine() const {
    return our_offside_line;
}

rcsc::Vector2D CafeModel::getBallLordPos() const {
    const PlayerObject *ball_lord = getBallLord();
    if (ball_lord) { return ball_lord->pos(); }
    return Vector2D::INVALIDATED;
}

const PlayerObject *CafeModel::getBallLord() const {
    const PlayerObject *lord;
    if (wm->ball().pos() == Vector2D::INVALIDATED) {
        return lord;
    }
    const InterceptTable *interceptTable = wm->interceptTable();


    const int mate_min = interceptTable->teammateReachCycle();
    const int opp_min = interceptTable->opponentReachCycle();

    if (mate_min < opp_min) {
        if (interceptTable->fastestTeammate())
            return interceptTable->fastestTeammate();
    } else {
        if (interceptTable->fastestOpponent())
            return interceptTable->fastestOpponent();
    }

    return lord;
}


rcsc::Vector2D CafeModel::getOptimizedPosition(const rcsc::Vector2D &form_pos) const {

    if (Strategy::defense_mode == Dangerous) {
        return form_pos;
    }

    double form_pos_x = std::max(form_pos.x, our_offside_line);
    form_pos_x = form_pos.x;
    return Vector2D((form_pos_x + form_pos.x) / 2, form_pos.y);
}