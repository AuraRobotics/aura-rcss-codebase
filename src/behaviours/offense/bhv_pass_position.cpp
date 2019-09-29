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


static std::vector <std::vector<double> > table_final_score;

///////////////////////////////

Vector2D Bhv_PassPosition::getPassPos(rcsc::PlayerAgent *agent) {


    const ServerParam &SP = ServerParam::i();
    const WorldModel &wm = agent->world();
    const CafeModel &cm = CafeModel::i();

    Vector2D self_pos = Strategy::i().getPosition(wm.self().unum()); // TODO check self_pos is ok
    Vector2D ball_pos = cm.getBallLordPos();//wm.ball().pos();

    int x_offside = cm.getOurOffsideLine(); //TODO change name

    double path_dist = rcscUtils::ballPathDistWithMaxSpeed(20);//TODO why 20?


    double search_radius = 5;//TODO dynamic
    std::pair<double, double> check_line_y(self_pos.y - search_radius, self_pos.y + search_radius);
    std::pair<double, double> check_line_x(self_pos.x - search_radius, self_pos.x + search_radius);


    double max_score = 0;
    Vector2D best_pos = Vector2D::INVALIDATED;


    Vector2D ball_lord_pos = cm.getBallLordPos();


    ////DEBUG

    table_final_score.clear();
    ///////////////////////////////////////

    for (int i = check_line_x.first; i <= check_line_x.second; i++) {
        //////DEBUGG

        table_final_score.push_back(std::vector<double>());
        /////DEBUG
        for (int j = check_line_y.first; j <= check_line_y.second; j++) {
            Vector2D check_point(i, j);


            if (!checkPosIsValid(check_point, self_pos, ball_lord_pos, x_offside)) {
                continue;
            }

            double temp_score = 0;


            temp_score += 0;


            ///////////

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


    log_table(table_final_score, "final score ");


    dlog.addCircle(Logger::TEAM,
                   best_pos, 0.5, "#000000", true);


    dlog.addCircle(Logger::TEAM,
                   ball_lord_pos, 0.5, "#ffffff", true);

    return best_pos;
}


bool Bhv_PassPosition::checkPosIsValid(rcsc::Vector2D check_point,
                                       rcsc::Vector2D opp_pos, rcsc::Vector2D ball_lord_pos, double our_offside_x) {
    const ServerParam &SP = ServerParam::i();
    const CafeModel &cm = CafeModel::i();


    return true;
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