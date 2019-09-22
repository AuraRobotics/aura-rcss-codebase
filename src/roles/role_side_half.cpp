// -*-c++-*-

/*
 *Copyright:

 Copyright (C) Hidehisa AKIYAMA

 This code is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3, or (at your option)
 any later version.

 This code is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this code; see the file COPYING.  If not, write to
 the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

 *EndCopyright:
 */

/////////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "role_side_half.h"

#include "bhv_chain_action.h"
#include "../strategy.h"
#include "../behaviours/bhv_basic_offensive_kick.h"
#include "../behaviours/bhv_basic_move.h"
#include "../behaviours/defense/bhv_block.h"
#include "../behaviours/defense/bhv_intercept.h"
#include "../behaviours/defense/bhv_defensive_positioning.h"
#include "../behaviours/offense/bhv_offensive_positioning.h"
#include <rcsc/player/player_agent.h>
#include <rcsc/player/debug_client.h>

#include <rcsc/common/logger.h>

using namespace rcsc;

const std::string RoleSideHalf::NAME("SideHalf");

/*-------------------------------------------------------------------*/
/*!

 */
namespace {
    rcss::RegHolder role = SoccerRole::creators().autoReg(&RoleSideHalf::create,
                                                          RoleSideHalf::NAME);
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
RoleSideHalf::execute(PlayerAgent *agent) {
    bool kickable = agent->world().self().isKickable();
    if (agent->world().existKickableTeammate()
        && agent->world().teammatesFromBall().front()->distFromBall()
           < agent->world().ball().distFromSelf()) {
        kickable = false;
    }

    if (kickable) {
        doKick(agent);
    } else {
        doMove(agent);
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
void
RoleSideHalf::doKick(PlayerAgent *agent) {
    if (Bhv_ChainAction().execute(agent)) {
        dlog.addText(Logger::TEAM,
                     __FILE__": (execute) do chain action");
        agent->debugClient().addMessage("ChainAction");
        return;
    }

    Bhv_BasicOffensiveKick().execute(agent);
}

/*-------------------------------------------------------------------*/
/*!

 */
void
RoleSideHalf::doMove(PlayerAgent *agent) {
    const Strategy &stra = Strategy::i();

    if (Bhv_Intercept().execute(agent)) {
        return;
    }

    switch (stra.getSituation()){
        case Defense_Situation:
            if (Bhv_DefensivePositioning().execute(agent)) {
                return;
            }
            break;
        case Offense_Situation:
            if (Bhv_OffensivePositioning().execute(agent)) {
                return;
            }
            break;
    }

    Bhv_BasicMove().execute(agent);
}
