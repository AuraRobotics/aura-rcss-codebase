//
// Created by armanaxh on ۲۰۱۹/۹/۲۲.
//

#include "bhv_pass_position.h"


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
#include <rcsc/geom/angle_deg.h>


#include <iostream>

using namespace rcsc;

bool Bhv_PassPosition::execute(rcsc::PlayerAgent *agent) {


    const WorldModel &wm = agent->world();
    const ServerParam &SP = ServerParam::i();
    const Strategy &stra = Strategy::i();
    const CafeModel &cm = CafeModel::i();


    Vector2D target_point = getPassPos(agent);
    if (target_point == Vector2D::INVALIDATED) {
        dlog.addText(Logger::TEAM,
                     __FILE__": Bhv_PassPosition invalid target_point !!!");
        return false;
    }

    const double dash_power = Strategy::get_normal_dash_power(wm, stra);

    double dist_thr = wm.ball().distFromSelf() * 0.04;
    if (dist_thr < 1.0) dist_thr = 1.0;

    dlog.addText(Logger::TEAM,
                 __FILE__": Bhv_PassPosition target=(%.1f %.1f) dist_thr=%.2f",
                 target_point.x, target_point.y,
                 dist_thr);


    if (!Body_GoToPoint(target_point, dist_thr, dash_power
    ).execute(agent)) {
        Body_TurnToBall().execute(agent);
    }

    if (wm.ball().distFromSelf() < 18.0) {
        agent->setNeckAction(new Neck_TurnToBall());
    } else {
        agent->setNeckAction(new Neck_TurnToBallOrScan());
    }

    return true;
}


///////////////////////DEBUG
static Polygon2D max_covered_denger_poly;
static double max_area_cover = INT_MIN;


static std::vector <std::vector<double> > table_near_to_goal;
static std::vector <std::vector<double> > table_body_dir;
static std::vector <std::vector<double> > table_final_score;

///////////////////////////////

Vector2D Bhv_PassPosition::getPassPos(rcsc::PlayerAgent *agent) {


    const ServerParam &SP = ServerParam::i();
    const WorldModel &wm = agent->world();
    const CafeModel &cm = CafeModel::i();

    const SideID our_side = wm.ourSide();

    Vector2D donor_pos = donor->pos();
    Vector2D ball_next_pos = cm.getBallLordPos();
    dlog.addRect(Logger::PASS,
                 donor_pos.x - 1, donor_pos.y - 1,
                 2, 2,
                 "#0000ff");
    dlog.addText(Logger::TEAM,
                 __FILE__":   donor unum :   %d, pos: %.2f %.2f  ", donor->unum(), donor_pos.x, donor_pos.y);

    Vector2D self_form_pos = Strategy::i().getPosition(wm.self().unum()); // TODO check self_pos is ok
    Vector2D self_pos = wm.self().pos();
    Vector2D ball_pos = cm.getBallLordPos();//wm.ball().pos();

    int x_offside = wm.offsideLineX(); //TODO change name

    double path_dist = rcscUtils::ballPathDistWithMaxSpeed(20);//TODO why 20?


    double search_radius = 5;//TODO dynamic
    std::pair<double, double> check_line_y(self_form_pos.y - search_radius, self_form_pos.y + search_radius);
    std::pair<double, double> check_line_x(self_form_pos.x - search_radius, self_form_pos.x + search_radius);


    double max_score = 0;
    Vector2D best_pos = Vector2D::INVALIDATED;


    Vector2D ball_lord_pos = cm.getBallLordPos();


    ////DEBUG

    table_near_to_goal.clear();
    table_body_dir.clear();
    table_final_score.clear();
    ///////////////////////////////////////

    FastIC *fastIC = CafeModel::fastIC();

    fastICConfig(fastIC, agent);


    for (int i = check_line_x.first; i <= check_line_x.second; i++) {
        //////DEBUGG

        table_near_to_goal.push_back(std::vector<double>());
        table_body_dir.push_back(std::vector<double>());
        table_final_score.push_back(std::vector<double>());
        /////DEBUG
        for (int j = check_line_y.first; j <= check_line_y.second; j++) {
            Vector2D check_point(i, j);


            if (!checkPosIsValid(check_point, self_pos, ball_lord_pos, x_offside)) {
                ///////////
                table_near_to_goal.back().push_back(0);
                table_body_dir.back().push_back(0);
                table_final_score.back().push_back(0);
                ///////////////////////////////////////////////////
                continue;
            }

            double ball_speed = 3;//TODO

            Vector2D donor_to_me_vel = check_point - donor_pos;
            donor_to_me_vel.setLength(ball_speed);

            fastIC->refresh();
            fastIC->setBall(donor_pos + donor_to_me_vel, donor_to_me_vel, 0);
            fastIC->calculate();

            const AbstractPlayerObject *fastestPlayer = fastIC->getFastestPlayer();

            double dist_pass = check_point.dist(donor_pos);
            int cycle_pass_me = rcscUtils::ballCycle(dist_pass, ball_speed);
            int cycle_opp_intercept = fastIC->getFastestPlayerReachCycle();

            /////
            if (fastestPlayer != NULL)
                dlog.addText(Logger::PASS,
                             __FILE__":   pass fastest Player  :   %d, pos: %.2f %.2f  cycle opp : %d  cycle me : %d",
                             fastestPlayer->unum(),
                             check_point.x, check_point.y, cycle_opp_intercept, cycle_pass_me);
            ///////////////////////////////////////////////////

            if (fastestPlayer != NULL && cycle_pass_me > cycle_opp_intercept) {
                ///////////
                table_near_to_goal.back().push_back(-0.01);
                table_body_dir.back().push_back(-0.01);
                table_final_score.back().push_back(-0.01);
                ///////////////////////////////////////////////////
                continue;
            }

            double temp_score = 0;

            double near_to_body_dir =
                    nearToBodyDir(check_point, self_pos, ball_next_pos, 2 * search_radius, agent) * 1;
            double near_to_goal = nearToGoal(check_point, 2 * search_radius) * 1;

            temp_score += near_to_body_dir;
            temp_score += near_to_goal;


            ///////////
            table_near_to_goal.back().push_back(near_to_goal);
            table_body_dir.back().push_back(near_to_body_dir);
            ///////////////////////////////////////////////////


            if (max_score < temp_score) {
                max_score = temp_score;
                best_pos = check_point;
            }

            dlog.addCircle(Logger::TEAM,
                           check_point, 0.5, "#0ff000", false);

            table_final_score.back().push_back(temp_score);
        }

    }


    log_table(table_near_to_goal, "table_near_to_goal ");
    log_table(table_body_dir, "table_body_dir ");
    log_table(table_final_score, "final score ");


    dlog.addCircle(Logger::TEAM,
                   best_pos, 0.5, "#000000", true);


    dlog.addCircle(Logger::TEAM,
                   ball_lord_pos, 0.5, "#ffffff", true);

    return best_pos;
}


void Bhv_PassPosition::fastICConfig(FastIC *fastIC, rcsc::PlayerAgent *agent) {


    const ServerParam &SP = ServerParam::i();
    const WorldModel &wm = agent->world();

    fastIC->reset();

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

bool Bhv_PassPosition::checkPosIsValid(rcsc::Vector2D check_point, rcsc::Vector2D opp_pos, rcsc::Vector2D ball_lord_pos,
                                       double offside_x) {
    const ServerParam &SP = ServerParam::i();
    const CafeModel &cm = CafeModel::i();


    if(check_point.x > offside_x){
        return false;
    }

    return true;
}


double Bhv_PassPosition::nearToBodyDir(rcsc::Vector2D check_point, rcsc::Vector2D self_pos,
                                       rcsc::Vector2D ball_pos, double max_radius2, rcsc::PlayerAgent *agent) {

    const WorldModel &wm = agent->world();
    const AngleDeg &body_dir = wm.self().body();
    Vector2D terminal;
    terminal = terminal.setPolar(50, body_dir) + self_pos;

    const double max_dist = std::sqrt(2 * std::pow(max_radius2, 2));

    Segment2D self_dir(self_pos, terminal);

    Vector2D foot = geoUtils::findFoot(self_dir, check_point);
    double dist_to_goal_line = foot.dist(check_point);
    return dist_to_goal_line < 1.1 ? 1 : 0;
}


double Bhv_PassPosition::nearToGoal(rcsc::Vector2D check_point, double max_radius2) {
    const ServerParam &SP = ServerParam::i();
    const Vector2D our_goal = SP.theirTeamGoalPos();
    double dist_from_goal = our_goal.dist(check_point);
    return (100 - dist_from_goal) / 100;
}

#include "../../utils/utils.cpp"

void Bhv_PassPosition::log_table(std::vector <std::vector<double> > table, std::string name) {
    dlog.addText(Logger::TEAM,
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
        temp += "   \t";
        for (int j = 0; j < max_d; j++) {
            temp += patch::to_string(arr2[i][j]) + "\t";
        }
        rcsc::dlog.addText(rcsc::Logger::TEAM,
                           __FILE__" %s ", temp.c_str());
        temp = "";
    }

}