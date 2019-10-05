//
// Created by armanaxh on ۲۰۱۹/۱۰/۳.
//

#include "deep_pass_generator.h"
#include "../rcsc_utils.h"
//
FastIC *DeepPassGenerator::fic;

void DeepPassGenerator::generate() {


    dlog.addText(Logger::SHOOT,
                 __FILE__": DeepPassGenerator  generate()");


    for (int i = 1; i < 11; i++) {
        const AbstractPlayerObject *sender = wm.ourPlayer(i + 1);
        if (sender == NULL) {
            dlog.addText(Logger::SHOOT,
                         __FILE__"sender is NULL");
        }

        AbstractPlayerCont neighbors = relationships[i];
        const AbstractPlayerCont::const_iterator resiver_end = neighbors.end();
        for (AbstractPlayerCont::const_iterator resiver_it = neighbors.begin();
             resiver_it != resiver_end;
             ++resiver_it) {

            if (!(*resiver_it) || ((*resiver_it)->unum() == -1) || (*resiver_it)->goalie()) {
                continue;
            }

            Vector2D best_area_pass =  generateDeepPass(sender, (*resiver_it));
            if (best_area_pass == Vector2D::INVALIDATED) {
                continue;
            }

            direct_pass[i].push_back(std::make_pair((*resiver_it), best_area_pass));


//            if (i == wm.self().unum() -1 ) {
                //////////////////////////////////
                dlog.addText(Logger::SHOOT,
                             __FILE__": Deep pass ::  %d -> %d  : %.2f %.2f", i + 1, (*resiver_it)->unum(),
                             best_area_pass.x, best_area_pass.y);

                dlog.addLine(Logger::SHOOT,
                             sender->pos(), best_area_pass,
                             "#210440");

                dlog.addCircle(Logger::SHOOT,
                               best_area_pass, 0.5, "#210440", true);


                ///////////////////////
//            }

        }

    }
}

const rcsc::Vector2D DeepPassGenerator::generateDeepPass(const rcsc::AbstractPlayerObject *sender,
                                                         const rcsc::AbstractPlayerObject *resiver) {


    if (sender == NULL || resiver == NULL) {
        return Vector2D::INVALIDATED;
    }

    if(sender->pos().dist(resiver->pos()) < 2){
        return Vector2D::INVALIDATED;
    }

    const int sender_unum = sender->unum();
    const int resiver_unum = resiver->unum();

    const Vector2D sender_pos = sender->pos();
    const Vector2D resiver_pos = resiver->pos();

    int x_offside = wm.offsideLineX(); //TODO change name

    double search_radius = 11;//TODO dynamic

    double max_score = INT_MIN;
    Vector2D best_pos = Vector2D::INVALIDATED;

    const Vector2D goal_pos(  ServerParam::i().pitchHalfLength(), 0.0 );

    Vector2D to_goal_vel = goal_pos - resiver_pos;
    to_goal_vel.setLength(1);
    Vector2D to_end_vel(1,0);


    Vector2D to_goal = resiver_pos+ to_goal_vel + to_goal_vel + to_goal_vel;
    Vector2D to_end = resiver_pos + to_end_vel + to_end_vel + to_end_vel;


    double count_dist = 0;
    while(count_dist < search_radius){


        double pass_dist = resiver_pos.dist(to_goal);
        const double max_receive_ball_speed = 1.24;

        double pass_speed = rcscUtils::first_speed_pass(pass_dist, max_receive_ball_speed);
        const int pass_cycle = rcscUtils::ballCycle(pass_dist, pass_speed);
        Vector2D donor_to_me_vel = to_goal - sender_pos;
        donor_to_me_vel.setLength(pass_speed);

        fic->refresh();
        fic->setBall(sender_pos + donor_to_me_vel, donor_to_me_vel, 0); //TODO donor_to_me_vel
        fic->calculate();


        const AbstractPlayerObject *fastest_player = fic->getFastestPlayer();
        const int fastest_player_cycle = fic->getFastestPlayerReachCycle();
        const int fastest_opp_cycle = fic->getFastestOpponentReachCycle();

        if (fastest_player == NULL) {
            count_dist ++;
            continue;
        }
        if (fastest_player->side() == wm.ourSide() && fastest_player->unum() == resiver_unum &&
            fastest_player_cycle < pass_cycle + 4 && fastest_player_cycle < fastest_opp_cycle - 3) {
            return to_goal;
        }

        to_goal += to_goal_vel;
        to_end += to_end_vel;
        count_dist ++;
    }

    return best_pos;
}