//
// Created by armanaxh on ۲۰۱۹/۸/۲۹.
//

#include "cafe_model.h"

using namespace rcsc;


CafeModel::CafeModel() {

}


CafeModel &CafeModel::instance() {
    static CafeModel s_instance;
    return s_instance;
}


void CafeModel::create(const rcsc::WorldModel &wm) {
    this->wm = &wm;

}


ConstPlayerPtrCont CafeModel::getOurPlayersByUnum(std::vector<int> players_unum) const {
    ConstPlayerPtrCont temp_player;

    for (int i = 0; i < players_unum.size(); i++) {
        int unum = players_unum[i];

        if (unum <= 0 || 11 < unum) continue;
        const PlayerObject *p_temp = static_cast<const PlayerObject *>(wm->ourPlayer(unum));
        if(p_temp == NULL) continue;
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
        if(p_temp == NULL) continue;
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