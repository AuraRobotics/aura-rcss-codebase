//
// Created by armanaxh on ۲۰۱۹/۱۰/۱.
//

#include "area_pass_generator.h"
#include "../utils/rcsc_utils.h"
#include <limits.h>
#include <rcsc/geom/vector_2d.h>
#include <rcsc/player/player_object.h>
#include <rcsc/common/logger.h>
#include <rcsc/player/abstract_player_object.h>

using namespace rcsc;

FastIC *AreaPassGenerator::fic;

void AreaPassGenerator::generate() {


    dlog.addText(Logger::CROSS,
                 __FILE__": AreaPassGenerator  generate()");

    if(!wm.self().isKickable()){
        return;
    }

//    for (int i = 1; i < 11; i++) {
        int i = wm.self().unum() - 1;
        const AbstractPlayerObject *sender = wm.ourPlayer(i + 1);
        if (sender == NULL) {
            dlog.addText(Logger::CROSS,
                         __FILE__"sender is NULL");
        }

        AbstractPlayerCont neighbors = relationships[i];
        const AbstractPlayerCont::const_iterator receiver_end = neighbors.end();
        for (AbstractPlayerCont::const_iterator receiver_it = neighbors.begin();
             receiver_it != receiver_end;
             ++receiver_it) {

            if (!(*receiver_it) || ((*receiver_it)->unum() == -1) || (*receiver_it)->goalie()) {
                continue;
            }

            if((*receiver_it)->pos().dist(sender->pos()) < 5){
                continue;
            }

            Vector2D best_area_pass = generateAreaPass(sender, (*receiver_it));
            if (best_area_pass == Vector2D::INVALIDATED) {
                continue;
            }

            area_pass[i].push_back(std::make_pair((*receiver_it), best_area_pass));


            if (i == wm.self().unum() - 1) {
                //////////////////////////////////
                dlog.addText(Logger::CROSS,
                             __FILE__": Area pass ::  %d -> %d  : %.2f %.2f", i + 1, (*receiver_it)->unum(),
                             best_area_pass.x, best_area_pass.y);

                dlog.addLine(Logger::CROSS,
                             sender->pos(), best_area_pass,
                             "#5534eb");

                dlog.addCircle(Logger::CROSS,
                               best_area_pass, 0.5, "#000000", true);


                ///////////////////////
            }

        }

//    }

}

//
//static std::vector <std::vector<double> > table_near_goal;
//static std::vector <std::vector<double> > table_near_receiver;
//static std::vector <std::vector<double> > table_final_score;

const Vector2D AreaPassGenerator::generateAreaPass(const rcsc::AbstractPlayerObject *sender,
                                                   const rcsc::AbstractPlayerObject *receiver) {

    if (sender == NULL || receiver == NULL) {
        return Vector2D::INVALIDATED;
    }

    const int sender_unum = sender->unum();
    const int receiver_unum = receiver->unum();

    const Vector2D sender_pos = sender->pos();
    const Vector2D receiver_pos = receiver->pos();

    int x_offside = wm.offsideLineX(); //TODO change name

    if(receiver_pos.x > x_offside){
        return Vector2D::INVALIDATED;
    }
    double search_radius = 4;//TODO dynamic
    std::pair<double, double> check_line_y(receiver_pos.y - search_radius, receiver_pos.y + search_radius);
    std::pair<double, double> check_line_x(receiver_pos.x - search_radius, receiver_pos.x + search_radius);

    double max_score = INT_MIN;
    Vector2D best_pos = Vector2D::INVALIDATED;


//    ////DEBUG
//
//    table_near_goal.clear();
//    table_near_receiver.clear();
//    table_final_score.clear();
    ///////////////////////////////////////

    for (int i = check_line_x.first; i <= check_line_x.second; i += 2) {
        //////DEBUGG
//        table_near_goal.push_back(std::vector<double>());
//        table_near_receiver.push_back(std::vector<double>());
//        table_final_score.push_back(std::vector<double>());
        /////DEBUG
        for (int j = check_line_y.first; j <= check_line_y.second; j += 2) {
            Vector2D check_point(i, j);


            if (!checkPosIsValid(check_point, sender_pos, receiver_pos, x_offside, receiver_unum)) {
                continue;
            }

            double temp_score = 0;

            double near_receiver = nearreceiver(check_point, sender_pos, receiver_pos, search_radius * 2) * 0.7;
            double near_goal =
                    nearGoal(check_point, sender_pos, receiver_pos, search_radius * 2, check_line_x.first) * 0.3;


            temp_score += near_receiver;
            temp_score += near_goal;

            ///////////
//            table_near_goal.back().push_back(near_goal);
//            table_near_receiver.back().push_back(near_receiver);
            ///////////////////////////////////////////////////



            if (max_score < temp_score) {
                max_score = temp_score;
                best_pos = check_point;
            }

            dlog.addCircle(Logger::CROSS,
                           check_point, 0.5, "#0fff00", false);

//            table_final_score.back().push_back(temp_score);
        }

    }

//
//    log_table(table_near_goal, "table_near_goal ");
//    log_table(table_near_receiver, "table_near_receiver  ");
//    log_table(table_final_score, "final score ");

//
//    dlog.addCircle(Logger::CROSS,
//                   best_pos, 0.5, "#000000", true);


    return best_pos;
}


bool AreaPassGenerator::checkPosIsValid(const rcsc::Vector2D check_point, const rcsc::Vector2D sender_pos,
                                        const rcsc::Vector2D receiver_pos, double x_offside, const int receiver_unum) {

    if (std::abs(check_point.y) > 31) {
        return false;
    }

    double pass_dist = sender_pos.dist(check_point);
    const double max_receive_ball_speed = 1.19;

    double pass_speed = rcscUtils::first_speed_pass(pass_dist, max_receive_ball_speed);

    const int pass_cycle = rcscUtils::ballCycle(pass_dist, pass_speed);
    Vector2D donor_to_me_vel = check_point - sender_pos;
    donor_to_me_vel.setLength(pass_speed);
    Vector2D donor_offset = donor_to_me_vel;
    donor_offset.setLength(4);

    fic->refresh();
    fic->setBall(sender_pos + donor_offset, donor_to_me_vel, 0); //TODO donor_to_me_vel
    fic->calculate();

    const AbstractPlayerObject *fastest_player = fic->getFastestPlayer();
    const int fastest_player_cycle = fic->getFastestPlayerReachCycle();
    const int fastest_opp_cycle = fic->getFastestOpponentReachCycle();

    if (fastest_player == NULL) {
        return false;
    }
    if (fastest_player->side() == wm.ourSide() && fastest_player->unum() == receiver_unum &&
        fastest_player_cycle < pass_cycle + 3 && fastest_player_cycle < fastest_opp_cycle - 3) {
        return true;
    }

    return false;
}


double AreaPassGenerator::nearGoal(rcsc::Vector2D check_point, rcsc::Vector2D sender_pos, rcsc::Vector2D receiver_pos,
                                   double max_radius2, double start_x) {

    return (check_point.x - start_x) / max_radius2;

}


double AreaPassGenerator::nearreceiver(rcsc::Vector2D check_point, rcsc::Vector2D sender_pos, rcsc::Vector2D receiver_pos,
                                      double max_radius2) {

    double dist_from_receiver = receiver_pos.dist(check_point);
    return (max_radius2 - dist_from_receiver) / max_radius2;
}


#include "../utils/utils.cpp"

void AreaPassGenerator::log_table(std::vector <std::vector<double> > table, std::string name) {
    dlog.addText(Logger::CROSS,
                 __FILE__":   %s ----------------- %d", name.c_str(), table.size());


    int max_d = table.size();
    int arr[max_d][max_d] = {0};

    for (int i = 0; i < table.size(); i++) {
        std::string temp;
        for (int j = 0; j < table[i].size(); j++) {
            arr[i][j] = int(table[i][j] * 100);
        }
    }


    int arr2[max_d][max_d];
    for (int i = 0; i < max_d; ++i) {
        for (int j = 0; j < max_d; ++j) {
            arr2[i][j] = arr[j][i];
        }
    }


    std::string temp;
    for (int i = 0; i < max_d; i++) {
        temp += "    ";
        for (int j = 0; j < max_d; j++) {
            temp += patch::to_string(arr2[i][j]) + "\t";
        }
        rcsc::dlog.addText(rcsc::Logger::CROSS,
                           __FILE__" %s ", temp.c_str());
        temp = "";
    }

}