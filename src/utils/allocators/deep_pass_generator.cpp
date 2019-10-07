//
// Created by armanaxh on ۲۰۱۹/۱۰/۳.
//

#include "deep_pass_generator.h"
#include "../rcsc_utils.h"
#include "../../strategy.h"

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
        const AbstractPlayerCont::const_iterator receiver_end = neighbors.end();
        for (AbstractPlayerCont::const_iterator receiver_it = neighbors.begin();
             receiver_it != receiver_end;
             ++receiver_it) {

            if (!(*receiver_it) || ((*receiver_it)->unum() == -1) || ((*receiver_it)->unum() == 1) ||
                (*receiver_it)->goalie()) {
                continue;
            }
            const int receiver_unum = (*receiver_it)->unum();
            RoleGroup role_group = Strategy::i().getRoleGroup(receiver_unum);
            if (role_group == Defense || role_group == Goalie) {
                continue;
            }

            Vector2D best_area_pass = generateDeepPass(sender, (*receiver_it));
            if (best_area_pass == Vector2D::INVALIDATED) {
                continue;
            }

            direct_pass[i].push_back(std::make_pair((*receiver_it), best_area_pass));


            if (i == wm.self().unum() - 1) {
                //////////////////////////////////
                dlog.addText(Logger::SHOOT,
                             __FILE__": Deep pass ::  %d -> %d  : %.2f %.2f", i + 1, (*receiver_it)->unum(),
                             best_area_pass.x, best_area_pass.y);

                dlog.addLine(Logger::SHOOT,
                             sender->pos(), best_area_pass,
                             "#210440");

                dlog.addCircle(Logger::SHOOT,
                               best_area_pass, 0.5, "#210440", true);


                ///////////////////////
            }

        }

    }
}

const rcsc::Vector2D DeepPassGenerator::generateDeepPass(const rcsc::AbstractPlayerObject *sender,
                                                         const rcsc::AbstractPlayerObject *receiver) {


    if (sender == NULL || receiver == NULL) {
        return Vector2D::INVALIDATED;
    }

    if (sender->pos().dist(receiver->pos()) < 2) {
        return Vector2D::INVALIDATED;
    }

    int x_offside = wm.offsideLineX(); //TODO change name
    if(receiver->pos().x > x_offside){
        return Vector2D::INVALIDATED;
    }

    const int sender_unum = sender->unum();
    const int receiver_unum = receiver->unum();

    const Vector2D sender_pos = sender->pos();
    const Vector2D receiver_pos = receiver->pos();



    double search_radius = 5;//TODO dynamic

    double max_score = INT_MIN;
    Vector2D best_pos = Vector2D::INVALIDATED;

    const Vector2D goal_pos(ServerParam::i().pitchHalfLength(), 0.0);

    Vector2D to_goal_vel = goal_pos - receiver_pos;
    to_goal_vel.setLength(2.5);
    Vector2D to_end_vel(2.5, 0);


    Vector2D to_goal = receiver_pos + to_goal_vel + to_goal_vel;
    Vector2D to_end = receiver_pos + to_end_vel + to_end_vel;


    double count_dist = 0;
    while (count_dist < search_radius) {


        double pass_dist = receiver_pos.dist(to_goal);
        const double max_receive_ball_speed = 1.1;

        double pass_speed = rcscUtils::first_speed_pass(pass_dist, max_receive_ball_speed);
        const int pass_cycle = rcscUtils::ballCycle(pass_dist, pass_speed);
        Vector2D donor_to_me_vel = to_goal - sender_pos;
        donor_to_me_vel.setLength(pass_speed);
        Vector2D donor_offset = donor_to_me_vel;
        donor_offset.setLength(1.1);

        fic->refresh();
        fic->setBall(sender_pos + donor_offset, donor_to_me_vel, 0); //TODO donor_to_me_vel
        fic->calculate();


        const AbstractPlayerObject *fastest_player = fic->getFastestPlayer();
        const int fastest_player_cycle = fic->getFastestPlayerReachCycle();
        const int fastest_opp_cycle = fic->getFastestOpponentReachCycle();

        if (fastest_player == NULL) {
            count_dist++;
            continue;
        }
        if (fastest_player->side() == wm.ourSide() && fastest_player->unum() == receiver_unum &&
            fastest_player_cycle < pass_cycle + 4 && fastest_player_cycle > 3 &&
            fastest_player_cycle < fastest_opp_cycle - 3) {
            return to_goal;
        }

        dlog.addCircle(Logger::SHOOT,
                       to_goal, 0.5, "#210440", false);

        to_goal += to_goal_vel;
        count_dist++;
    }


//

    count_dist = 0;
    while (count_dist < search_radius) {


        double pass_dist = receiver_pos.dist(to_end);
        const double max_receive_ball_speed = 0.6;

        double pass_speed = rcscUtils::first_speed_pass(pass_dist, max_receive_ball_speed);
        const int pass_cycle = rcscUtils::ballCycle(pass_dist, pass_speed);
        Vector2D donor_to_me_vel = to_end - sender_pos;
        donor_to_me_vel.setLength(pass_speed);

        fic->refresh();
        fic->setBall(sender_pos + donor_to_me_vel, donor_to_me_vel, 0); //TODO donor_to_me_vel
        fic->calculate();


        const AbstractPlayerObject *fastest_player = fic->getFastestPlayer();
        const int fastest_player_cycle = fic->getFastestPlayerReachCycle();
        const int fastest_opp_cycle = fic->getFastestOpponentReachCycle();

        if (fastest_player == NULL) {
            count_dist++;
            continue;
        }

        if (fastest_player->side() == wm.ourSide() && fastest_player->unum() == receiver_unum &&
            fastest_player_cycle < pass_cycle + 4 && fastest_player_cycle > 3 &&
            fastest_player_cycle < fastest_opp_cycle - 3) {
            return to_end;
        }


        dlog.addCircle(Logger::SHOOT,
                       to_end, 0.5, "#210440", false);

        to_end += to_end_vel;
        count_dist++;
    }

    return best_pos;
}