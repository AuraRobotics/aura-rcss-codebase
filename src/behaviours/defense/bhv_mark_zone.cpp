//
// Created by armanaxh on ۲۰۱۹/۹/۵.
//

#include "bhv_mark_zone.h"


#include <rcsc/common/logger.h>
#include <rcsc/common/server_param.h>
#include "../../cafe_model.h"
#include "../../strategy.h"
#include "../../utils/algo_utils.h"
#include "../../utils/geo_utils.h"
#include "../../utils/rcsc_utils.h"

#include <rcsc/player/player_agent.h>
#include <rcsc/action/basic_actions.h>
#include <rcsc/action/body_go_to_point.h>
#include <rcsc/action/neck_turn_to_ball_or_scan.h>
#include <rcsc/action/neck_turn_to_low_conf_teammate.h>
#include <rcsc/geom/vector_2d.h>
#include <rcsc/geom/segment_2d.h>

using namespace rcsc;

bool Bhv_MarkZone::execute(rcsc::PlayerAgent *agent) {

    const WorldModel &wm = agent->world();
    const ServerParam &SP = ServerParam::i();
    const Strategy &stra = Strategy::i();
    const CafeModel &cm = CafeModel::i();


    Vector2D target_point = getDefensivePos(agent);
    if (target_point == Vector2D::INVALIDATED) {
        return false;
    }
    dlog.addLine(Logger::TEAM,
                 target_point, wm.self().pos(),
                 "#f00f00");

    const double dash_power = Strategy::get_normal_dash_power(wm, stra);

    double dist_thr = wm.ball().distFromSelf() * 0.04;
    if (dist_thr < 1.0) dist_thr = 1.0;

    dlog.addText(Logger::TEAM,
                 __FILE__": Bhv_MarkZone target=(%.1f %.1f) dist_thr=%.2f",
                 target_point.x, target_point.y,
                 dist_thr);


    if (!Body_GoToPoint(target_point, dist_thr, dash_power
    ).execute(agent)) {
        Body_TurnToBall().execute(agent);
    }

    if (wm.existKickableOpponent()
        && wm.ball().distFromSelf() < 18.0) {
        agent->setNeckAction(new Neck_TurnToBall());
    } else {
        agent->setNeckAction(new Neck_TurnToBallOrScan());
    }

    return true;
}

rcsc::Vector2D Bhv_MarkZone::getDefensivePos(rcsc::PlayerAgent *agent) {


    const ServerParam &SP = ServerParam::i();
    const WorldModel &wm = agent->world();
    const CafeModel &cm = CafeModel::i();

    Vector2D self_formation_pos = Strategy::i().getPosition(wm.self().unum());
    Vector2D self_pos = wm.self().pos();
    Vector2D ball_next_pos = cm.getBallLordPos();
    const Strategy::BallArea ball_area = Strategy::get_ball_area(wm);
    Vector2D opp_pos = target_opp->pos() + target_opp->vel();

    Segment2D opp_to_ball(opp_pos, ball_next_pos);
    Vector2D foot = geoUtils::findFoot(opp_to_ball, self_pos);

    Vector2D ball_to_opp_arow = ball_next_pos - opp_pos;
    ball_to_opp_arow.setLength(2);
    Vector2D opp_mark_pos = opp_pos + ball_to_opp_arow;

    if (ball_area == Strategy::BA_Danger || ball_area == Strategy::BA_CrossBlock) {
        return opp_mark_pos;
    } else if (foot != Vector2D::INVALIDATED) {
        return foot;
    } else {
        return opp_mark_pos;
    }

    return Vector2D::INVALIDATED;
}