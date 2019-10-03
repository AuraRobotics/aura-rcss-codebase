//
// Created by armanaxh on ۲۰۱۹/۸/۲۹.
//

#ifndef CAFE_2D_CAFE_MODEL_H
#define CAFE_2D_CAFE_MODEL_H

#include "strategy.h"
#include <rcsc/player/world_model.h>
#include <rcsc/common/server_param.h>
#include <rcsc/player/player_object.h>
#include "utils/estimators/HERMES_FastIC.h";
#include "utils/allocators/player_relationship.h"

namespace rcsc {
    typedef std::vector<const rcsc::PlayerObject *> ConstPlayerPtrCont;
}

class CafeModel {

    typedef const boost::shared_ptr <rcsc::WorldModel> WorldModelPtr;

private:

    const rcsc::WorldModel *wm;
    static FastIC *fic;
    static double our_offside_line;

    PlayerRelationship *player_rel;

    CafeModel();

    CafeModel(const CafeModel &);

    const CafeModel &operator=(const CafeModel &);

public:
    static CafeModel &instance();


    static FastIC *fastIC();

    static
    const
    CafeModel &i() {
        return instance();
    }

    void update(PlayerAgent *agent);

    void create(const rcsc::WorldModel &wm, PlayerAgent *agent);

    rcsc::ConstPlayerPtrCont getOurPlayersByUnum(std::vector<int> players_unum) const;

    rcsc::ConstPlayerPtrCont getTheirPlayersByUnum(std::vector<int> players_unum) const;

    rcsc::PlayerPtrCont getPlayerInRangeX(double x1, double x2, bool ourTeam) const;

    rcsc::PlayerPtrCont getPlayerInRangeX(rcsc::PlayerPtrCont player, double x1, double x2) const;

    rcsc::PlayerPtrCont getPlayerInRect(rcsc::PlayerPtrCont player, double x1, double x2, double y1, double y2) const;

    rcsc::PlayerPtrCont getPlayerInBallArea(Strategy::BallArea ball_area, bool ourTeam) const;

    rcsc::PlayerPtrCont getPlayerInBallArea(rcsc::PlayerPtrCont player, Strategy::BallArea ball_area) const;

    rcsc::PlayerPtrCont getPlayerInRangeGoal(double dist, bool ourTeam) const;

    rcsc::PlayerPtrCont getPlayerInRangeGoal(rcsc::PlayerPtrCont player, double dist) const;

    double getOurOffsideLine() const;

    double calcOurOffsideLine() const;

    rcsc::Vector2D getBallLordPos() const;

    const rcsc::PlayerObject *getBallLord() const;

    rcsc::Vector2D getOptimizedPosition(const rcsc::Vector2D &form_pos) const;

    const PlayerRelationship &playerRel() const{
        return (*player_rel);
    }


    const rcsc::PlayerObject * nearsetOpp(const int unum);
private:

};


#endif //CAFE_2D_CAFE_MODEL_H
