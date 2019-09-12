//
// Created by armanaxh on ۲۰۱۹/۹/۱۱.
//

#ifndef CAFE_2D_PLAYER_ESTIMATOR_H
#define CAFE_2D_PLAYER_ESTIMATOR_H

#include <iostream>
#include <rcsc/player/player_object.h>
#include <rcsc/player/world_model.h>
#include <rcsc/geom/vector_2d.h>

class PlayerEstimator {


private:

    static const std::size_t MAX_CYCLE = 50;
    const rcsc::WorldModel &M_world;
    const rcsc::PlayerObject * target_player;
    std::vector< rcsc::Vector2D > M_step_target_cache;
    std::vector< rcsc::Vector2D > M_step_self_cache;


public:
    PlayerEstimator(const rcsc::WorldModel &wm, const rcsc::PlayerObject * player) : M_world(wm), target_player(player) {

    }

    void estimate();

private:
    void createStepTargetCatch();
    void createStepSelfCatch();

};


#endif //CAFE_2D_PLAYER_ESTIMATOR_H
