//
// Created by armanaxh on ۲۰۱۹/۹/۵.
//

#include "bhv_defhalf_positioning.h"
#include <rcsc/player/player_agent.h>
#include <rcsc/common/logger.h>
#include <rcsc/common/server_param.h>
#include <rcsc/player/intercept_table.h>
#include <rcsc/geom/vector_2d.h>
#include <rcsc/geom/line_2d.h>
#include <iostream>

#include "../cafe_model.h"
#include "../strategy.h"
#include "../utils/algo_utils.h"
#include "../utils/geo_utils.h"
#include "../utils/rcsc_utils.h"
#include "../utils/allocators/mark_target_allocator.h"

#include "bhv_mark_zone.h"
#include "bhv_mark_man.h"
#include "bhv_block.h"

using namespace rcsc;

bool Bhv_DefhalfPositioning::execute(rcsc::PlayerAgent *agent) {


    const WorldModel &wm = agent->world();
    const Strategy &stra = Strategy::i();
    const CafeModel &cm = CafeModel::i();

    MarkTargetAllocator targetAllocator(wm);

    bool success_allocate_target = targetAllocator.calculate();
    if (!success_allocate_target) {
        return false;
    }

    PlayerObject *target_opp = targetAllocator.getMarkTarget();
    if (!target_opp) {
        return false;
    }


    /////////////////////////////////////////////////////
    dlog.addText(Logger::TEAM,
                 __FILE__": ------------------- target for mark %p: %d     %.2f %.2f",
                 target_opp, target_opp->unum(), target_opp->pos().x, target_opp->pos().y);
    /////////////////////////////////////////////////////////////

    const PlayerObject *ball_lord = cm.getBallLord();
    if(ball_lord->unum() == target_opp->unum()){
        if (Bhv_Block(target_opp).execute(agent)) {
            return true;
        }
    }

    if (Strategy::defense_mode == Normal) {
        if (Bhv_MarkZone(target_opp).execute(agent)) {
            return true;
        }
    } else if (Strategy::defense_mode == Dangerous) {
        if (Bhv_MarkMan(target_opp).execute(agent)) {
            return true;
        }
    }


    return false;
}

