// -*-c++-*-

/*
 *Copyright:

 Copyright (C) Hiroki SHIMORA

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "sample_field_evaluator.h"

#include "field_analyzer.h"
#include "simple_pass_checker.h"

#include <rcsc/player/player_evaluator.h>
#include <rcsc/common/server_param.h>
#include <rcsc/common/logger.h>
#include <rcsc/math_util.h>

#include <iostream>
#include <algorithm>
#include <cmath>
#include <cfloat>

// #define DEBUG_PRINT

using namespace rcsc;

static const int VALID_PLAYER_THRESHOLD = 8;


/*-------------------------------------------------------------------*/
/*!

 */
static double evaluate_state(const PredictState &state, const std::vector <ActionStatePair> &path);

static double areaRate(const PredictState &state, const std::vector <ActionStatePair> &path);

static double playerRoleRate(const PredictState &state, const std::vector <ActionStatePair> &path);

static double freeSpace(const PredictState &state, const std::vector <ActionStatePair> &path);

static double can_shoot_to_goal(const PredictState &state, const std::vector <ActionStatePair> &path);

/*-------------------------------------------------------------------*/
/*!

 */
SampleFieldEvaluator::SampleFieldEvaluator() {

}

/*-------------------------------------------------------------------*/
/*!

 */
SampleFieldEvaluator::~SampleFieldEvaluator() {

}

/*-------------------------------------------------------------------*/
/*!

 */
double
SampleFieldEvaluator::operator()(const PredictState &state,
                                 const std::vector <ActionStatePair> &path) const {
    const double final_state_evaluation = evaluate_state(state, path);

    //
    // ???
    //

    double result = final_state_evaluation;


    /////DEBUG
    const AbstractPlayerObject *holder = state.ballHolder();
    int holder_unum = -1;

    if (holder) {
        holder_unum = holder->unum();
    }
    dlog.addText(Logger::PLAN,
                 __FILE__":  score  %d -----------------> %.2f  - area_rate:  %.2f, player_rate: %.2f, freeSpace: %.2f",
                 holder_unum, result, areaRate(state, path), playerRoleRate(state, path), freeSpace(state, path));

    ////////////////////////

    return result;
}


/*-------------------------------------------------------------------*/
/*!

 */
static
double
evaluate_state(const PredictState &state, const std::vector <ActionStatePair> &path) {
    const ServerParam &SP = ServerParam::i();

    const AbstractPlayerObject *holder = state.ballHolder();

#ifdef DEBUG_PRINT
    dlog.addText( Logger::ACTION_CHAIN,
                  "========= (evaluate_state) ==========" );
#endif

    //
    // if holder is invalid, return bad evaluation
    //
    if (!holder) {
#ifdef DEBUG_PRINT
        dlog.addText( Logger::ACTION_CHAIN,
                      "(eval) XXX null holder" );
#endif
        return -DBL_MAX / 2.0;
    }

    const int holder_unum = holder->unum();


    //
    // ball is in opponent goal
    //
    if (state.ball().pos().x > +(SP.pitchHalfLength() - 0.1)
        && state.ball().pos().absY() < SP.goalHalfWidth() + 2.0) {
#ifdef DEBUG_PRINT
        dlog.addText( Logger::ACTION_CHAIN,
                      "(eval) *** in opponent goal" );
#endif
        return +1.0e+7;
    }

    //
    // ball is in our goal
    //
    if (state.ball().pos().x < -(SP.pitchHalfLength() - 0.1)
        && state.ball().pos().absY() < SP.goalHalfWidth()) {
#ifdef DEBUG_PRINT
        dlog.addText( Logger::ACTION_CHAIN,
                      "(eval) XXX in our goal" );
#endif

        return -1.0e+7;
    }


    //
    // out of pitch
    //
    if (state.ball().pos().absX() > SP.pitchHalfLength()
        || state.ball().pos().absY() > SP.pitchHalfWidth()) {
#ifdef DEBUG_PRINT
        dlog.addText( Logger::ACTION_CHAIN,
                      "(eval) XXX out of pitch" );
#endif

        return -DBL_MAX / 2.0;
    }



    if(state.ball().pos().x > 36) {
        double shoot_count_rate = can_shoot_to_goal(state, path);
        if (shoot_count_rate > 0) {
//            std::cout << " cannn shoootttttttttttttttttttt 0))))))" << std::endl;
            return shoot_count_rate * 1000;
        }
    }





    //
    // set basic evaluation
    //
    double point = state.ball().pos().x;

    point += std::max(0.0,
                      40.0 - ServerParam::i().theirTeamGoalPos().dist(state.ball().pos()));

#ifdef DEBUG_PRINT
    dlog.addText( Logger::ACTION_CHAIN,
                  "(eval) ball pos (%f, %f)",
                  state.ball().pos().x, state.ball().pos().y );

    dlog.addText( Logger::ACTION_CHAIN,
                  "(eval) initial value (%f)", point );
#endif


    double area_rate = areaRate(state, path);
    double player_role_rate = playerRoleRate(state, path) * 25;
    double free_space = freeSpace(state, path) * 10;
//
    point = area_rate;
    point += player_role_rate;
    point += free_space;

    const bool path_is_empty = path.empty();
    if (path_is_empty) {
        double dist_opp = 9999;
        const PlayerObject *nearest_opp = state.getOpponentNearestTo(holder->pos(), 10, &dist_opp);
        rcsc::dlog.addText(rcsc::Logger::ACTION_CHAIN,
                           __FILE__" dist nearset opp( dist : %.2f )  ", (dist_opp));

        if (nearest_opp) {
            int last_observe_count = nearest_opp->posCount();
            dist_opp -= last_observe_count * 2;
            rcsc::dlog.addText(rcsc::Logger::ACTION_CHAIN,
                               __FILE__" dist nearset after observer( dist : %.2f )  %d", (dist_opp),
                               last_observe_count);
        }

        if (dist_opp < 2.5) {
            return -DBL_MAX / 2.0;
        }

        if (dist_opp < 5) {
            return -DBL_MAX / 3.0;
        }

        if (dist_opp < 10) {
            point -= 15 - (dist_opp);
        }


    }

    //
    // add bonus for goal, free situation near offside line
    //
    if (FieldAnalyzer::can_shoot_from
            (holder->unum() == state.self().unum(),
             holder->pos(),
             state.getPlayerCont(new OpponentOrUnknownPlayerPredicate(state.ourSide())),
             VALID_PLAYER_THRESHOLD)) {
        point += 1.0e+6;
#ifdef DEBUG_PRINT
        dlog.addText( Logger::ACTION_CHAIN,
                      "(eval) bonus for goal %f (%f)", 1.0e+6, point );
#endif

        if (holder_unum == state.self().unum()) {
            point += 5.0e+5;
#ifdef DEBUG_PRINT
            dlog.addText( Logger::ACTION_CHAIN,
                          "(eval) bonus for goal self %f (%f)", 5.0e+5, point );
#endif
        }
    }

    return point;
}


#include "strategy.h"

static double areaRate(const PredictState &state, const std::vector <ActionStatePair> &path) {

    Vector2D ball_pos = state.ball().pos();
    Strategy::BallArea ball_area = Strategy::get_ball_area(ball_pos);
    double dist_goal = ServerParam::i().theirTeamGoalPos().dist(state.ball().pos());

    double base = 0;
    double dist_rate = 0;

    switch (ball_area) {
        case Strategy::BA_OffMidField: {
            base = 180;
            double dist_area_min = 18;
            double dist_delta = 36 - (-1);
            double rate_delta = 300 - base;

            dist_rate = (1 - ((dist_goal - dist_area_min) / dist_delta)) * rate_delta;

        }
            break;
        case Strategy::BA_DribbleAttack: {
            base = 180;
            double dist_area_min = 27;
            double dist_delta = 36 - (-1);
            double rate_delta = 220 - base;

            dist_rate = (1 - ((dist_goal - dist_area_min) / dist_delta)) * rate_delta;
        }
            break;
        case Strategy::BA_Cross: {
            base = 220;
            double dist_area_min = 20;
            double dist_delta = 53 - (36);
            double rate_delta = 270 - base;

            dist_rate = (1 - ((dist_goal - dist_area_min) / dist_delta)) * rate_delta;
        }
            break;
        case Strategy::BA_ShootChance: {
            base = 300;
            double dist_area_min = 0;
            double dist_delta = 53 - (36);
            double rate_delta = 500 - base;

            dist_rate = (1 - ((dist_goal - dist_area_min) / dist_delta)) * rate_delta;
        }
            break;
        default: {
            base = 150;
            dist_rate = state.ball().pos().x;
        }
            break;
    }


    return base + dist_rate;
}


static double playerRoleRate(const PredictState &state, const std::vector <ActionStatePair> &path) {
    const Strategy &stra = Strategy::i();

    const int ball_holder_unum = state.ballHolderUnum();
    if (stra.getRoleGroup(ball_holder_unum) == Halfback) {
        return 1;
    }
    return 0;
}

#include "./utils/estimators/HERMES_FastIC.h"
#include "./cafe_model.h"
#include "./chain_action/cooperative_action.h"

static double freeSpace(const PredictState &state, const std::vector <ActionStatePair> &path) {

    if( !(path.empty() || path.back().action().category() == CooperativeAction::Pass) ){
        return 0;
    }



    FastIC *fic = CafeModel::fastIC();
    fic->setByWorldModel();
    fic->setMaxCycleAfterFirstFastestPlayer(20);
    fic->setMaxCycleAfterOppReach(3);

    Vector2D ball_pos = state.ball().pos();
    Vector2D org_ball_pos = state.orgBallPos();
    if(org_ball_pos.x > ball_pos.x){
        return 0;
    }

    const Vector2D goal_pos(ServerParam::i().pitchHalfLength(), 0.0);
    Vector2D to_goal_vel = goal_pos - ball_pos;
    to_goal_vel.setLength(1);
    Vector2D to_end_vel(1, 0);


    fic->refresh();
    fic->setBall(ball_pos, to_goal_vel, 0);
    fic->calculate();

    int min_opp_reach = fic->getFastestOpponentReachCycle();

    fic->refresh();
    fic->setBall(ball_pos, to_end_vel, 0);
    fic->calculate();

    min_opp_reach = std::min(min_opp_reach, fic->getFastestOpponentReachCycle());

    if(min_opp_reach < 5){
        return 0;
    }
    return min_opp_reach;
}


double can_shoot_to_goal(const PredictState &state, const std::vector <ActionStatePair> &path){

    FastIC *fic = CafeModel::fastIC();

    const ServerParam &SP = ServerParam::i();


    fic->reset();
    fic->setShootMode();
    fic->setProbMode();

//    for (int i = 0; i < wm.ourPlayers().size(); i++)
//    {
//        if (isPlayerValid(wm.ourPlayers()[i]))
//        {
//            addPlayer(wm.ourPlayers()[i]);
//
//        }
//    }
    for (int i = 0; i < state.theirPlayers().size(); i++) {
        if (fic->isPlayerValid(state.theirPlayers()[i])) {
            fic->addPlayer(state.theirPlayers()[i]);
        }
    }

    fic->setMaxCycleAfterFirstFastestPlayer(10);
    fic->setMaxCycleAfterOppReach(10);
    fic->setMaxCycles(20);




    Vector2D ball_pos = state.ball().pos();

    double ball_speed = 2.8;//TODO
    Vector2D goal_pos = ServerParam::i().theirTeamGoalPos();

    int shoot_count = 0;
    for(int goal_y = goal_pos.y - 7 ; goal_y < goal_pos.y + 7; goal_y ++ ){

        Vector2D temp_shot_pos(goal_pos.x , goal_y);

        Vector2D to_goal_vel = temp_shot_pos -  ball_pos;
        to_goal_vel.setLength(ball_speed);

        fic->refresh();
        fic->setBall(ball_pos, to_goal_vel, 0);
        fic->calculate();

        const AbstractPlayerObject *faster_opp = fic->getFastestOpponent();
        if(faster_opp == NULL){
            shoot_count ++;
            dlog.addLine(Logger::TEAM,
                         ball_pos, temp_shot_pos,
                         "#ff0f00");
        }

    }
    return shoot_count;
}



