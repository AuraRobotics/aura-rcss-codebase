//
// Created by armanaxh on ۲۰۱۹/۸/۲۹.
//

#ifndef CAFE_2D_CAFE_MODEL_H
#define CAFE_2D_CAFE_MODEL_H

#include "strategy.h"
#include <rcsc/player/world_model.h>
#include <rcsc/common/server_param.h>
#include <rcsc/player/player_object.h>

namespace rcsc{
    typedef std::vector<const rcsc::PlayerObject *> ConstPlayerPtrCont;
}

class CafeModel {

    typedef const boost::shared_ptr <rcsc::WorldModel> WorldModelPtr;

private:


    const rcsc::WorldModel *wm;


    CafeModel();

    CafeModel(const CafeModel &);

    const CafeModel &operator=(const CafeModel &);

public:
    static CafeModel &instance();

    static
    const
    CafeModel &i() {
        return instance();
    }

    void create(const rcsc::WorldModel &wm);

    rcsc::ConstPlayerPtrCont getOurPlayersByUnum(std::vector<int> players_unum) const;

    rcsc::ConstPlayerPtrCont getTheirPlayersByUnum(std::vector<int> players_unum) const;

    rcsc::PlayerPtrCont getPlayerInRangeX(double x1, double x2, bool ourTeam) const;

    rcsc::PlayerPtrCont getPlayerInRangeX(rcsc::PlayerPtrCont player, double x1, double x2) const;

    rcsc::PlayerPtrCont getPlayerInRangeGoal(double dist, bool ourTeam) const;

    rcsc::PlayerPtrCont getPlayerInRangeGoal(rcsc::PlayerPtrCont player, double dist) const;


private:

};


#endif //CAFE_2D_CAFE_MODEL_H
