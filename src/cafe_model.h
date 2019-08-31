//
// Created by armanaxh on ۲۰۱۹/۸/۲۹.
//

#ifndef CAFE_2D_CAFE_MODEL_H
#define CAFE_2D_CAFE_MODEL_H

#include "strategy.h"
#include <rcsc/player/world_model.h>
#include <rcsc/player/player_object.h>

class CafeModel {

    typedef const boost::shared_ptr <rcsc::WorldModel> WorldModelPtr;
private:

    const rcsc::WorldModel *wm;


    CafeModel();

    const CafeModel &operator=(const CafeModel &);

public:
    static CafeModel &instance();

    static
    const
    CafeModel &i() {
        return instance();
    }

    void create(const rcsc::WorldModel &wm);


    rcsc::PlayerPtrCont getPlayerInRangeX(int x1, int x2, bool ourTeam) const;

    rcsc::PlayerPtrCont getPlayerInRangeX(rcsc::PlayerPtrCont player, int x1, int x2) const;

private:

};


#endif //CAFE_2D_CAFE_MODEL_H
