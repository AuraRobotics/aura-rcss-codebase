//
// Created by armanaxh on ۲۰۱۹/۱۰/۱۰.
//

#include "bhv_shoot.h"


#include <rcsc/player/player_agent.h>
#include <rcsc/common/logger.h>
#include <rcsc/common/server_param.h>
#include "../../cafe_model.h"
#include "../../strategy.h"
#include "../../utils/estimators/HERMES_FastIC.h"
#include "../../chain_action/bhv_strict_check_shoot.h"
#include <rcsc/action/body_smart_kick.h>
#include <rcsc/action/kick_table.h>
#include <rcsc/action/neck_turn_to_point.h>
#include <rcsc/action/neck_turn_to_goalie_or_scan.h>
#include "../../utils/rcsc_utils.h"

using namespace rcsc;


bool Bhv_Shoot::execute(rcsc::PlayerAgent *agent) {

    dlog.addText(Logger::SHOOT,
                 __FILE__": shoot execute");
    std::cout << ": shoot execute" << std::endl;

    const static ServerParam &SP = ServerParam::i();
    const static CafeModel &cm = CafeModel::i();
    const static WorldModel &wm = agent->world();

    if(wm.self().pos().x < 35){
        return false;
    }

    Vector2D best_shoot = doShoot(agent);
    if (best_shoot == Vector2D::INVALIDATED) {
        return false;
    }

    Vector2D one_step_vel
            = KickTable::calc_max_velocity((best_shoot - wm.ball().pos()).th(),
                                           wm.self().kickRate(),
                                           wm.ball().vel());


    double one_step_speed = one_step_vel.r();

//    dlog.addText( Logger::SHOOT,
//                  __FILE__": shoot[%d] target=(%.2f, %.2f) first_speed=%f one_kick_max_speed=%f",
//                  best_shoot->index_,
//                  best_shoot->target_point_.x,
//                  best_shoot->target_point_.y,
//                  best_shoot->first_ball_speed_,
//                  one_step_speed );

    if (one_step_speed > SP.ballSpeedMax() - 0.2) {
        if (Body_SmartKick(best_shoot,
                           one_step_speed,
                           one_step_speed * 0.99 - 0.0001,
                           1).execute(agent)) {
            agent->setNeckAction(new Neck_TurnToGoalieOrScan(-1));
            agent->debugClient().addMessage("Force1Step");
            return true;
        }
    }

    if (Body_SmartKick(best_shoot,
                       SP.ballSpeedMax() - 0.2,
                       SP.ballSpeedMax() - 0.2 * 0.99,
                       3).execute(agent)) {
        if (!doTurnNeckToShootPoint(agent, best_shoot)) {
            agent->setNeckAction(new Neck_TurnToGoalieOrScan(-1));
        }
        return true;
    }

    dlog.addText(Logger::SHOOT,
                 __FILE__": failed shoot");
    return false;

}
/*-------------------------------------------------------------------*/
/*!

 */
bool
Bhv_Shoot::doTurnNeckToShootPoint(PlayerAgent *agent,
                                  const Vector2D &shoot_point) {
    const double angle_buf = 10.0; // Magic Number

    if (!agent->effector().queuedNextCanSeeWithTurnNeck(shoot_point, angle_buf)) {
        dlog.addText(Logger::TEAM,
                     __FILE__": cannot look the shoot point(%.2f %.2f)",
                     shoot_point.x,
                     shoot_point.y);
        return false;
    }

#if 0
    const WorldModel & wm = agent->world();
    if ( wm.seeTime() == wm.time() )
    {
        double current_width = wm.self().viewWidth().width();
        AngleDeg target_angle = ( shoot_point - wm.self().pos() ).th();
        double angle_diff = ( target_angle - wm.self().face() ).abs();

        if ( angle_diff < current_width*0.5 - angle_buf )
        {
            dlog.addText( Logger::TEAM,
                          __FILE__": already seen. width=%.1f, diff=%.1f. shoot point(%.2f %.2f)",
                          current_width,
                          angle_diff,
                          shoot_point.x,
                          shoot_point.y );
            return false;
        }
    }
#endif

    dlog.addText(Logger::TEAM,
                 __FILE__": turn_neck to the shoot point(%.2f %.2f)",
                 shoot_point.x,
                 shoot_point.y);
    agent->debugClient().addMessage("Shoot:NeckToTarget");

    agent->setNeckAction(new Neck_TurnToPoint(shoot_point));

    return true;
}


rcsc::Vector2D Bhv_Shoot::doShoot(rcsc::PlayerAgent *agent) {
    const static ServerParam &SP = ServerParam::i();
    const static CafeModel &cm = CafeModel::i();
    const static WorldModel &wm = agent->world();
    static FastIC *fastIC = cm.fastIC();
    fastICConfig(fastIC, agent);


    double max_speed = SP.ballSpeedMax() - 0.2;
    double ball_speed = max_speed;
    Vector2D goal_pos = ServerParam::i().theirTeamGoalPos();
    Vector2D ball_pos = wm.ball().pos();

    bool shoot_flag = false;
    double dist_shoot_from_goal = 999;
    Vector2D best_shoot(Vector2D::INVALIDATED);

    for (double goal_y = goal_pos.y - 7; goal_y <= goal_pos.y + 7.25; goal_y += 0.4) {

        if (goal_y > 7) {
            goal_y = 6.87;
        }

        goal_y = goal_y == -7 ? goal_y + 0.24 : goal_y;

        Vector2D temp_shot_pos(goal_pos.x, goal_y);

        Vector2D to_goal_vel = temp_shot_pos - ball_pos;
        to_goal_vel.setLength(ball_speed);

        fastIC->refresh();
        fastIC->setBall(ball_pos, to_goal_vel, 0);
        fastIC->calculate();
//        dlog.addLine(Logger::SHOOT,
//                     ball_pos, temp_shot_pos,
//                     "#ff0000");
        const AbstractPlayerObject *fastest_opp = fastIC->getFastestOpponent();

        double temp_dist_to_goal = temp_shot_pos.dist(ball_pos);
        const int to_goal_cycle = rcscUtils::ballCycle(temp_dist_to_goal, ball_speed);

        if (fastIC->getFastestOpponentReachCycle() > to_goal_cycle) {
            dlog.addLine(Logger::SHOOT,
                         ball_pos, ball_pos + to_goal_vel * 10,
                         "#0000ff");
            dlog.addText(Logger::SHOOT,
                         __FILE__": goal speed: %.2f  cycle Opp: %d  , cycle goal:   %d ", ball_speed,
                         fastIC->getFastestOpponentReachCycle(), to_goal_cycle);

            if (temp_dist_to_goal < dist_shoot_from_goal) {
                dist_shoot_from_goal = temp_dist_to_goal;
                best_shoot = temp_shot_pos;
                std::cout << " shoooooooooooooooooooooooooooooooooootttttttttttttttttt " << std::endl;
                dlog.addText(Logger::SHOOT,
                             __FILE__": shoooooooooooooooooooooooooooooooooootttttttttttttttttt");
            }
        } else {
            dlog.addText(Logger::SHOOT,
                         __FILE__": oppnent intercept : %d speed: %.2f  cycle : %d", fastest_opp->unum(), ball_speed,
                         fastIC->getFastestOpponentReachCycle());
        }

    }

    return best_shoot;
}


void Bhv_Shoot::fastICConfig(FastIC *fastIC, rcsc::PlayerAgent *agent) {


    const ServerParam &SP = ServerParam::i();
    const WorldModel &wm = agent->world();

    fastIC->reset();
    fastIC->setShootMode();
    fastIC->setProbMode();

//    for (int i = 0; i < wm.ourPlayers().size(); i++)
//    {
//        if (isPlayerValid(wm.ourPlayers()[i]))
//        {
//            addPlayer(wm.ourPlayers()[i]);
//
//        }
//    }
    for (int i = 0; i < wm.theirPlayers().size(); i++) {
        if (fastIC->isPlayerValid(wm.theirPlayers()[i])) {
            fastIC->addPlayer(wm.theirPlayers()[i]);
        }
    }

    fastIC->setMaxCycleAfterFirstFastestPlayer(10);
    fastIC->setMaxCycleAfterOppReach(10);
    fastIC->setMaxCycles(20);
}