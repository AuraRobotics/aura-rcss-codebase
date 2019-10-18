//
// Created by armanaxh on ۲۰۱۹/۱۰/۱۶.
//

#include "actgen_deg_pass.h"


#include "pass.h"
#include "simple_pass_checker.h"


#include "action_state_pair.h"
#include "predict_state.h"
#include "../cafe_model.h"
#include "pass.h"
#include "../utils/rcsc_utils.h"
#include "field_analyzer.h"
#include <rcsc/player/player_object.h>
#include <rcsc/common/logger.h>
//#define DEBUG_PRINT

using namespace rcsc;

void ActGen_DegPass::generate(std::vector <ActionStatePair> *result, const PredictState &state,
                              const rcsc::WorldModel &wm, const std::vector <ActionStatePair> &path) const {


    dlog.addText(Logger::Logger::ACTION_CHAIN,
                 __FILE__":   actgen degPass -----------------");

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
    if (ball_holder_unum != wm.self().unum()) {
        return;
    }
    if (!wm.self().isKickable()) {
        return;
    }

    if (wm.ball().posCount() > 0
        && wm.ball().velCount() > 0) {
        return;
    }


    getDegPass(wm, state, result);
}


void ActGen_DegPass::getDegPass(const rcsc::WorldModel &wm, const PredictState &state,
                                std::vector <ActionStatePair> *result) const {

    const ServerParam &SP = ServerParam::i();
    const Strategy &stra = Strategy::i();
    const CafeModel &cm = CafeModel::i();


    FastIC *fic = cm.fastIC();
    configFastIC(fic, wm);

    const PredictBallObject ball_object = state.ball();
    Vector2D ball_pos = ball_object.pos();
    double ball_speed = ball_object.pos().r();
    AngleDeg angle_ball = ball_object.vel().th();

    dlog.addText(Logger::Logger::ACTION_CHAIN,
                 __FILE__":   -------------------------- start    ");
    dlog.flush();

    int count_run = 0;
    double pass_speed_step = 0.25;
    for (double pass_speed = 1.25; pass_speed <= 3; pass_speed += pass_speed_step) {


        Vector2D temp_dir(pass_speed, 0);
        double step_deg = 5;

        for (double i = 0; i < 360; i += step_deg) {
            //////////////////////////////////////
            dlog.addText(Logger::Logger::ACTION_CHAIN,
                         __FILE__":   -------------------------- mid 1  i: %.2f , pass_speed: %.2f", i, pass_speed);
            dlog.flush();
            ////////////////////////////////
            temp_dir = temp_dir.rotate(step_deg);

            fic->refresh();
            fic->setBall(ball_pos, temp_dir, 0);
            fic->calculate();

            count_run++;
            const int mate_reach = fic->getFastestTeammateReachCycle();
            const int opp_reach = fic->getFastestOpponentReachCycle();

            if (mate_reach < opp_reach && mate_reach < 20) {


                const AbstractPlayerObject *ball_holder = state.ballHolder();
                if (ball_holder == NULL) {
                    return;
                }
                const int ball_holder_unum = ball_holder->unum();
                const AbstractPlayerObject *fastest_mate = fic->getFastestTeammate(false);
                if (fastest_mate == NULL) {
                    continue;
                }


                const int receiver_unum = fastest_mate->unum();
                if(receiver_unum == -1){
                    continue;
                }


                Vector2D receiver_pos = fastest_mate->pos();
                if (receiver_pos == Vector2D::INVALIDATED) {
                    continue;
                }

                //////////////////////////////////////
                dlog.addText(Logger::Logger::ACTION_CHAIN,
                             __FILE__":   -------------------------- mid 2   i: %.2f , pass_speed: %.2f", i,
                             pass_speed);
                dlog.flush();
                ////////////////////////////////
                Vector2D resive_pass_pos = fic->getFastestTeammateReachPoint(false);
                if (resive_pass_pos == Vector2D::INVALIDATED) {
                    continue;
                }
                double dist_pass = rcscUtils::ballPathDist(mate_reach, pass_speed);

                Vector2D pass_vel = temp_dir;
                pass_vel.setLength(dist_pass);
                Vector2D pass_pos = resive_pass_pos;//ball_pos + pass_vel;
                if (dist_pass < 5) {
                    continue;
                }

                if (std::abs(pass_pos.x) > 50 || std::abs(pass_pos.y) > 31) {
                    continue;
                }

                if (pass_pos == Vector2D::INVALIDATED) {
                    continue;
                }

                /////////////////////////////////////////////////

                dlog.addLine(Logger::KICK,
                             ball_pos, pass_pos,
                             "#a832a8");

                ////////////////////////////////////


                int kick_count = FieldAnalyzer::predict_kick_count(wm,
                                                                   ball_holder,
                                                                   dist_pass,
                                                                   angle_ball);

//////////////////////////////////////

                dlog.addText(Logger::Logger::ACTION_CHAIN,
                             __FILE__":   -------------------------- mid 3    i: %.2f  , pass_speed: %.2f", i,
                             pass_speed);
                dlog.flush();
                ////////////////////////////////

                CooperativeAction::Ptr pass_temp(new Pass(ball_holder_unum,
                                                          receiver_unum,
                                                          pass_pos,
                                                          pass_speed, mate_reach, kick_count, 0,
                                                          "description deg pass"));

//////////////////////////////////////
                dlog.addText(Logger::Logger::ACTION_CHAIN,
                             __FILE__":   -------------------------- mid 4.0    i: %.2f , pass_speed: %.2f", i,
                             pass_speed);
                dlog.flush();
                ////////////////////////////////
//////////////////////////////////////
                dlog.addText(Logger::Logger::ACTION_CHAIN,
                             __FILE__":  %d , %d , %.2f %.2f ", mate_reach, receiver_unum, pass_pos.x, pass_pos.y);
                dlog.flush();
                ////////////////////////////////


                PredictState temp_test(state,
                             mate_reach,
                             receiver_unum,
                             pass_pos);
                //////////////////////////////////////
                dlog.addText(Logger::Logger::ACTION_CHAIN,
                             __FILE__":   -------------------------- mid 4.1    i: %.2f , pass_speed: %.2f", i,
                             pass_speed);
                dlog.flush();
                ////////////////////////////////
                PredictState *temp_predict_state (new PredictState(state,
                                                                   mate_reach,
                                                                   receiver_unum,
                                                                   pass_pos));
                //////////////////////////////////////
                dlog.addText(Logger::Logger::ACTION_CHAIN,
                             __FILE__":   -------------------------- mid 5    i: %.2f , pass_speed: %.2f", i,
                             pass_speed);
                dlog.flush();
                ////////////////////////////////
                ActionStatePair temp_action_state_pair(pass_temp,
                                                       temp_predict_state);
                //////////////////////////////////////
                dlog.addText(Logger::Logger::ACTION_CHAIN,
                             __FILE__":   -------------------------- mid 6    i: %.2f , pass_speed: %.2f", i,
                             pass_speed);
                dlog.flush();
                ////////////////////////////////

                result->push_back(temp_action_state_pair);
                //////////////////////////////////////
                dlog.addText(Logger::Logger::ACTION_CHAIN,
                             __FILE__":   -------------------------- mid 7    i: %.2f , pass_speed: %.2f", i,
                             pass_speed);
                dlog.flush();
                ////////////////////////////////




            }

        }
    }

    dlog.addText(Logger::Logger::ACTION_CHAIN,
                 __FILE__":   -------------------------- end    ");
    dlog.flush();
}


void ActGen_DegPass::configFastIC(FastIC *fastIC, const rcsc::WorldModel &wm) const {

    const ServerParam &SP = ServerParam::i();

    fastIC->reset();
    fastIC->setShootMode();
    fastIC->setProbMode();

    double kickable_area_s = 0.5;
    if(wm.ball().pos().x > 33){
        kickable_area_s = 1;
    }

    for (int i = 0; i < wm.ourPlayers().size(); i++) {
        if (fastIC->isPlayerValid(wm.ourPlayers()[i])) {
            if (wm.ourPlayers()[i]->unum() != wm.self().unum()) {
                fastIC->addPlayer(wm.ourPlayers()[i], kickable_area_s, 1.0, 2.0);
            }
        }
    }
    for (int i = 0; i < wm.theirPlayers().size(); i++) {
        if (fastIC->isPlayerValid(wm.theirPlayers()[i])) {
            fastIC->addPlayer(wm.theirPlayers()[i]);
        }
    }

    fastIC->setMaxCycleAfterFirstFastestPlayer(10);
    fastIC->setMaxCycleAfterOppReach(10);
    fastIC->setMaxCycles(20);


}