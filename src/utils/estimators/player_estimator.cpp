//
// Created by armanaxh on ۲۰۱۹/۹/۱۱.
//

#include "player_estimator.h"
#include <rcsc/player/world_model.h>
#include <rcsc/common/server_param.h>
#include <rcsc/common/logger.h>
#include <rcsc/geom/vector_2d.h>
#include <rcsc/timer.h>
#include <iostream>
#include "../utils.cpp"

using namespace rcsc;


void PlayerEstimator::estimate() {
    if(!target_player->posValid() || !target_player->velValid()){
        return;
    }

    M_step_target_cache.clear();
    M_step_self_cache.clear();

    createStepTargetCatch();
    createStepSelfCatch();

    for(int i=0; i<M_step_target_cache.size(); i++){
        dlog.addCircle(Logger::TEAM,
                       M_step_target_cache[i], 0.3, "#ff0000", false);
        dlog.addText(Logger::TEAM,
                     __FILE__": player target cache: %.2f %.2f",
                     M_step_target_cache[i].x,  M_step_target_cache[i].y);
    }

    for(int i=0; i<M_step_self_cache.size(); i++){
        dlog.addCircle(Logger::TEAM,
                       M_step_self_cache[i], 0.3, "#0000ff", false);
        dlog.addText(Logger::TEAM,
                     __FILE__": player self cache: %.2f %.2f",
                     M_step_self_cache[i].x,  M_step_self_cache[i].y);
    }




}


void PlayerEstimator::createStepTargetCatch() {
    const ServerParam &SP = ServerParam::i();
    const double pitch_x_max = (SP.keepawayMode()
                                ? SP.keepawayLength() * 0.5
                                : SP.pitchHalfLength() + 5.0);
    const double pitch_y_max = (SP.keepawayMode()
                                ? SP.keepawayWidth() * 0.5
                                : SP.pitchHalfWidth() + 5.0);

    Vector2D t_pos = target_player->pos();
    Vector2D t_vel = target_player->vel();
    t_vel.scale(1.1);

    M_step_target_cache.push_back(t_pos);


    for (std::size_t i = 1; i <= MAX_CYCLE; ++i) {
        t_pos += t_vel;

        M_step_target_cache.push_back(t_pos);

        if (i >= 5
            && t_vel.r2() < 0.01 * 0.01) {
            // target stopped
            break;
        }

        if (t_pos.absX() > pitch_x_max
            || t_pos.absY() > pitch_y_max) {
            // out of pitch
            break;
        }
    }

}




void PlayerEstimator::createStepSelfCatch() {
    const ServerParam &SP = ServerParam::i();
    const double pitch_x_max = (SP.keepawayMode()
                                ? SP.keepawayLength() * 0.5
                                : SP.pitchHalfLength() + 5.0);
    const double pitch_y_max = (SP.keepawayMode()
                                ? SP.keepawayWidth() * 0.5
                                : SP.pitchHalfWidth() + 5.0);

    Vector2D t_pos = target_player->pos();
    Vector2D t_vel = target_player->vel();
    t_vel.scale(1.1);

    M_step_target_cache.push_back(t_pos);


    for (std::size_t i = 1; i <= MAX_CYCLE; ++i) {
        t_pos += t_vel;

        M_step_target_cache.push_back(t_pos);

        if (i >= 5
            && t_vel.r2() < 0.01 * 0.01) {
            // target stopped
            break;
        }

        if (t_pos.absX() > pitch_x_max
            || t_pos.absY() > pitch_y_max) {
            // out of pitch
            break;
        }
    }

}