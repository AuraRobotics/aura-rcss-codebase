//
// Created by armanaxh on ۲۰۱۹/۹/۲۲.
//

#include "bhv_offensive_positioning.h"

#include <rcsc/player/player_agent.h>
#include <rcsc/common/logger.h>
#include <rcsc/common/server_param.h>
#include "../../cafe_model.h"
#include "../../strategy.h"



using namespace rcsc;

#include <rcsc/common/server_param.h>
#include <rcsc/action/body_go_to_point.h>

bool Bhv_OffensivePositioning::execute(rcsc::PlayerAgent *agent) {

    dlog.addText(Logger::TEAM,
                 __FILE__": Bhv_Offensive execute -- ");


    if (!Body_GoToPoint(agent->world().self().pos(), 1,ServerParam::i().maxDashPower()
    ).execute(agent)) {
        return true;
    }
    return false;
}
