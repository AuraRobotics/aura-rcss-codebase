//
// Created by armanaxh on ۲۰۱۹/۹/۲۲.
//

#include "bhv_offensive_positioning.h"

#include <rcsc/player/player_agent.h>
#include <rcsc/common/logger.h>
#include <rcsc/common/server_param.h>
#include "../../cafe_model.h"
#include "../../strategy.h"
#include "../../utils/allocators/player_relationship.h"
#include "./bhv_pass_position.h"


using namespace rcsc;


bool Bhv_OffensivePositioning::execute(rcsc::PlayerAgent *agent) {

    const WorldModel &wm = agent->world();
    const Strategy &stra = Strategy::i();
    const CafeModel &cm = CafeModel::i();

    dlog.addText(Logger::TEAM,
                 __FILE__": Bhv_OffensivePositioning execute -- ");

    PlayerPtrCont resivers;

    PlayerRelationship player_relationship(wm);

    player_relationship.calc();


    AbstractPlayerCont player_neighber = player_relationship.getNeighbors(wm.self().unum());


    for (AbstractPlayerCont::const_iterator
                 o = player_neighber.begin(),
                 end = player_neighber.end();
         o != end;
         ++o) {
        (*o)->unum();
        dlog.addText(Logger::TEAM,
                     __FILE__": ---->   %.2f %.2f  ) ", (*o)->pos().x, (*o)->pos().y);


        dlog.addCircle(Logger::TEAM,
                       (*o)->pos(), 0.5, "#ff0000", true);
    }


    if (Bhv_PassPosition(resivers).execute(agent)) {
        return true;
    }

    return false;
}
