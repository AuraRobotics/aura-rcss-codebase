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


rcsc::PlayerPtrCont CafeModel::getPlayerInRangeX(int x1, int x2, bool ourTeam) const {
    if (ourTeam == true) {
        return getPlayerInRangeX(wm->teammatesFromSelf(), x1, x2);
    }
    return getPlayerInRangeX(wm->opponentsFromSelf(), x1, x2);
}

rcsc::PlayerPtrCont CafeModel::getPlayerInRangeX(const rcsc::PlayerPtrCont player, int x1, int x2) const {
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
