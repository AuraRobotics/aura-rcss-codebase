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



    const PlayerObject * ball_lord = cm.getBallLord();
    if(ball_lord == NULL){
        return false;
    }



    const int ball_lord_unum = ball_lord->unum();
    AbstractPlayerCont pass_path = cm.playerRel().getPassPath(ball_lord_unum, wm.self().unum());

    ////DEBUG
    Vector2D temp_r = Vector2D::INVALIDATED;
    for (AbstractPlayerCont::const_iterator
                 o = pass_path.begin(),
                 end = pass_path.end();
         o != end;
         ++o) {
        (*o)->unum();

        if (temp_r != Vector2D::INVALIDATED) {
            dlog.addLine(Logger::TEAM,
                         (*o)->pos(), temp_r,
                         "#fc03f4");
        }
        temp_r = (*o)->pos();

    }
    dlog.addText(Logger::TEAM,
                 __FILE__": ----> size pass path :: --> %d  ::::::     ) ", pass_path.size());

    //////////////////////////////////////
    if(pass_path.size() <= 1){
        return false;
    }


    const AbstractPlayerObject *donor_player = pass_path[1];
    if (Bhv_PassPosition(donor_player).execute(agent)) {
        return true;
    }

    return false;
}
