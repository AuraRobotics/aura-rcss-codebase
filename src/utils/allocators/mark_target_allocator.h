//
// Created by armanaxh on ۲۰۱۹/۹/۱۲.
//

#ifndef CAFE_2D_MARKTARGETALLOCATOR_H
#define CAFE_2D_MARKTARGETALLOCATOR_H

#include <rcsc/player/player_object.h>

#include <rcsc/player/world_model.h>
#include <rcsc/common/logger.h>
#include <rcsc/common/server_param.h>
#include "../cafe_model.h"
#include "../strategy.h"
#include "../algorithms/hungarian.h"
#include <iostream>

using namespace rcsc;

class MarkTargetAllocator {

    typedef std::map<unsigned, PlayerObject *> assignmentTable;
    typedef std::map<const PlayerObject *, PlayerObject *> assignmentObjectTable;

    const WorldModel &wm;
    const ServerParam &SP;
    const Strategy &stra;
    const CafeModel &cm;

    PlayerObject *target;
    assignmentTable assignments_table;
    assignmentObjectTable assignments_object_table;


public:
    MarkTargetAllocator(const WorldModel &wm) :
            wm(wm),
            SP(ServerParam::i()),
            stra(Strategy::i()),
            cm(CafeModel::i()) {}

    bool calculate();

    PlayerObject *getMarkTarget();


private:
    void recordResults(const Hungarian::Result &result, const Hungarian::Matrix &cost_matrix,
                       const ConstPlayerPtrCont defensive_player,
                       const PlayerPtrCont denger_opp);

    const ConstPlayerPtrCont getDefensivePlayers();

    const PlayerPtrCont getDengerOpponents();

    Hungarian::Matrix &createCostMatrix(Hungarian::Matrix &cost_matrix, const ConstPlayerPtrCont defensive_player,
                                        const PlayerPtrCont denger_opp);

    int calcStateCost(const PlayerObject *our_p, const PlayerObject *opp_p, Vector2D mate_form_pos,
                      double max_cover_dist);


    void log_table(const Hungarian::Matrix table, std::string name, const ConstPlayerPtrCont defensive_player,
                   const PlayerPtrCont denger_opp);

    void log_draw(const ConstPlayerPtrCont &defensive_player,
                  const PlayerPtrCont &denger_opp);

};


#endif //CAFE_2D_MARKTARGETALLOCATOR_H
