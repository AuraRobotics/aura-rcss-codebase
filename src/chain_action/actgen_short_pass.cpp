//
// Created by armanaxh on ۲۰۱۹/۹/۳۰.
//

#include "actgen_short_pass.h"

#include "action_state_pair.h"
#include "predict_state.h"
#include "../utils/allocators/player_relationship.h"
#include "../cafe_model.h"
#include "pass.h"
#include "../utils/rcsc_utils.h"
#include "field_analyzer.h"
#include <rcsc/player/player_object.h>
#include <rcsc/common/logger.h>
//#define DEBUG_PRINT

using namespace rcsc;

/*-------------------------------------------------------------------*/
/*!

 */
void ActGen_ShortPass::generate(std::vector <ActionStatePair> *result, const PredictState &state,
                                const rcsc::WorldModel &wm, const std::vector <ActionStatePair> &path) const {

    dlog.addText(Logger::Logger::ACTION_CHAIN,
                 __FILE__":   actgen shortPass -----------------");

    const ServerParam &SP = ServerParam::i();
    const Strategy &stra = Strategy::i();
    const CafeModel &cm = CafeModel::i();


    const AbstractPlayerObject *ball_holder = state.ballHolder();

    if (ball_holder == NULL) {

        dlog.addText(Logger::Logger::ACTION_CHAIN,
                     __FILE__":   -----------------   ERROR )))))  ball_holder is NULL !!!");
        return;
    }
    const int ball_holder_unum = ball_holder->unum();

    const PredictBallObject ball_object = state.ball();
    Vector2D ball_pos = ball_object.pos();
    double ball_speed = ball_object.pos().r();
    AngleDeg angle_ball = ball_object.vel().th();

    AbstractPlayerCont short_pass_resiver = cm.playerRel().getShortPass(ball_holder_unum);


    const AbstractPlayerCont::const_iterator resiver_end = short_pass_resiver.end();
    for (AbstractPlayerCont::const_iterator resiver_it = short_pass_resiver.begin();
         resiver_it != resiver_end;
         ++resiver_it) {

        const AbstractPlayerObject * resiver = (*resiver_it);

        if(resiver == NULL || resiver->unum() == -1){
            continue;
        }


        dlog.addText(Logger::Logger::ACTION_CHAIN,
                     __FILE__"pass to %d", resiver->unum());
//        dlog.addLine(Logger::ACTION_CHAIN,
//                     ball_holder->pos(), resiver->pos(),
//                     "#a855232");

        const int receiver_unum = resiver->unum();
        Vector2D receiver_pos = resiver->pos();

        double dist_pass = receiver_pos.dist(ball_pos);

        if(dist_pass < 5){
            continue;
        }

        int kick_count = FieldAnalyzer::predict_kick_count(wm,
                                                           ball_holder,
                                                           dist_pass,
                                                           angle_ball);


        const PlayerType * ptype = resiver->playerTypePtr();
        const double max_receive_ball_speed = 1.45;

        double pass_speed = rcscUtils::first_speed_pass(dist_pass, max_receive_ball_speed);

        CooperativeAction::Ptr pass_temp(new Pass(ball_holder_unum,
                                                  receiver_unum,
                                                  receiver_pos,
                                                  pass_speed, 0, kick_count, 0,
                                                  "description pass"));

        result->push_back(ActionStatePair(pass_temp,
                                          new PredictState(state,
                                                           pass_temp->durationStep(),
                                                           pass_temp->targetPlayerUnum(),
                                                           pass_temp->targetPoint())));


    }

}
