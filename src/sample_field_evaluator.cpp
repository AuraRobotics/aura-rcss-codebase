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
                 __FILE__":  score  %d -----------------> %.2f",holder_unum,  areaRate(state, path));

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



    point = area_rate;

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