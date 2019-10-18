//
// Created by armanaxh on ۲۰۱۹/۱۰/۵.
//

#include "actgen_deep_pass.h"

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
void ActGen_DeepPass::generate(std::vector <ActionStatePair> *result, const PredictState &state,
                               const rcsc::WorldModel &wm, const std::vector <ActionStatePair> &path) const {

    dlog.addText(Logger::Logger::ACTION_CHAIN,
                 __FILE__":   actgen deepPass -----------------");


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

    DeepPassCont deep_passes = cm.playerRel().getDeepPass(ball_holder_unum);


    const DeepPassCont::const_iterator area_pass_end = deep_passes.end();
    for (DeepPassCont::const_iterator area_pass_it = deep_passes.begin();
         area_pass_it != area_pass_end;
         ++area_pass_it) {


        const AbstractPlayerObject *receiver = (*area_pass_it).first;
        const Vector2D pass_pos = (*area_pass_it).second;

        if (receiver == NULL || receiver->unum() == -1) {
            continue;
        }


        dlog.addText(Logger::Logger::ACTION_CHAIN,
                     __FILE__"pass area to %d in pos %.2f %.2f", receiver->unum(), pass_pos.x, pass_pos.y);
//        dlog.addLine(Logger::ACTION_CHAIN,
//                     ball_holder->pos(), receiver->pos(),
//                     "#a855232");
//
        const int receiver_unum = receiver->unum();
        Vector2D receiver_pos = receiver->pos();
//
        double dist_pass = pass_pos.dist(ball_pos);

        if (dist_pass < 5) {
            continue;
        }

        int kick_count = FieldAnalyzer::predict_kick_count(wm,
                                                           ball_holder,
                                                           dist_pass,
                                                           angle_ball);

        const PlayerType *ptype = receiver->playerTypePtr();
        const double max_receive_ball_speed = 1.5;
//
        double pass_speed = rcscUtils::first_speed_pass(dist_pass, max_receive_ball_speed);
//
        FastIC *fic = cm.fastIC();
        fic->setMaxCycleAfterFirstFastestPlayer(8);

        Vector2D donor_to_me_vel = pass_pos - ball_pos;
        Vector2D donor_offset = donor_to_me_vel;

        while (pass_speed > 0.5) {
            donor_to_me_vel.setLength(pass_speed);
            donor_offset.setLength(pass_speed * 2.2);

            fic->refresh();
            fic->setBall(ball_pos + donor_offset, donor_to_me_vel, 0);
            fic->calculate();

            const int pass_cycle = rcscUtils::ballCycle(dist_pass, pass_speed);
            const AbstractPlayerObject *fastest_player = fic->getFastestPlayer();
            const int fastest_player_cycle = fic->getFastestPlayerReachCycle();
            const int fastest_opp_cycle = fic->getFastestOpponentReachCycle();


            if (fastest_player == NULL) {
                pass_speed -= 0.2;
                continue;
            }


            if (fastest_player->side() == wm.ourSide() && fastest_player->unum() == receiver_unum &&
                fastest_player_cycle < pass_cycle + 4 && fastest_player_cycle >= 3 &&
                fastest_opp_cycle > fastest_player_cycle + 3) {
                dlog.addText(Logger::Logger::ACTION_CHAIN,
                             __FILE__"pass area to %d in pos %.2f %.2f,   fast_palyer : %d , fast_cycle, %d, should cycle : %d , speed : %.2f, opp_cycle : %d ",
                             receiver->unum(), pass_pos.x, pass_pos.y, fastest_player->unum(), fastest_player_cycle, pass_cycle, pass_speed, fastest_opp_cycle);

                break;
            }


            pass_speed -= 0.2;
        }
        if (pass_speed < 0.5) {
            continue;
        }

        if (pass_speed > 1.1) {
            pass_speed -= 0.31;
        }


        CooperativeAction::Ptr pass_temp(new Pass(ball_holder_unum,
                                                  receiver_unum,
                                                  pass_pos,
                                                  pass_speed, 0, kick_count, 0,
                                                  "description deep pass"));

        result->push_back(ActionStatePair(pass_temp,
                                          new PredictState(state,
                                                           pass_temp->durationStep(),
                                                           pass_temp->targetPlayerUnum(),
                                                           pass_temp->targetPoint())));


    }

}
