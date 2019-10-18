//
// Created by armanaxh on ۲۰۱۹/۹/۵.
//

#include "bhv_mark_man.h"

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
#include <rcsc/action/neck_turn_to_ball_and_player.h>
#include <rcsc/action/neck_scan_players.h>

#include <rcsc/geom/vector_2d.h>
#include <rcsc/geom/angle_deg.h>


#include <iostream>

using namespace rcsc;

bool Bhv_MarkMan::execute(rcsc::PlayerAgent *agent) {


    const WorldModel &wm = agent->world();
    const ServerParam &SP = ServerParam::i();
    const Strategy &stra = Strategy::i();
    const CafeModel &cm = CafeModel::i();

    if (!target_opp) {
        return false;
    }
    Vector2D target_point = getDefensivePos(agent);
    if (target_point == Vector2D::INVALIDATED) {
        return false;
    }

    const double dash_power = Strategy::get_normal_dash_power(wm, stra);

    double dist_target = target_point.dist(wm.self().pos());
    double dist_thr =  dist_target > 10 ?  dist_target * 0.1 * 3 : dist_target * 0.1 * 1.2;
    if (dist_thr < 1.0) dist_thr = 1.0;

    dlog.addText(Logger::TEAM,
                 __FILE__": Bhv_MarkMan target=(%.1f %.1f) dist_thr=%.2f",
                 target_point.x, target_point.y,
                 dist_thr);


    if (!Body_GoToPoint(target_point, dist_thr, dash_power
    ).execute(agent)) {
        Body_TurnToBall().execute(agent);
    }

    if (!Neck_TurnToBallAndPlayer( target_opp ).execute( agent )) {
//
//        std::cout << "scannnnnnnnnnnnnn in mark men problem -----" << std::endl;
        agent->setNeckAction(new Neck_TurnToBallOrScan());
    }
    agent->doChangeView(ViewWidth::NARROW);


    return true;
}


///////////////////////DEBUG
static Polygon2D max_covered_denger_poly;
static double max_area_cover = INT_MIN;

static std::vector <std::vector<double> >  table_cover;
static std::vector <std::vector<double> >  table_goal_line;
static std::vector <std::vector<double> >  table_near_penalty;
static std::vector <std::vector<double> >  table_near_goal;
static std::vector <std::vector<double> >  table_near_target;
static std::vector <std::vector<double> >  table_between_goal_angle;
static std::vector <std::vector<double> >  table_near_to_pass_line;
static std::vector <std::vector<double> >  table_final_score;

///////////////////////////////

Vector2D Bhv_MarkMan::getDefensivePos(rcsc::PlayerAgent *agent) {


    const ServerParam &SP = ServerParam::i();
    const WorldModel &wm = agent->world();
    const CafeModel &cm = CafeModel::i();

    Vector2D self_form_pos = Strategy::i().getPosition(wm.self().unum()); // TODO check self_pos is ok
    Vector2D self_pos = wm.self().pos();
    Vector2D ball_pos = cm.getBallLordPos();//wm.ball().pos();
    Vector2D opp_pos = target_opp->pos() + target_opp->vel();
    Vector2D opp_vel = target_opp->vel();


    dlog.addText(Logger::TEAM,
                 __FILE__": Opponent target pos =(%.1f %.1f), vel =(%.1f %.1f) ",
                 opp_pos.x, opp_pos.y,
                 opp_vel.x, opp_vel.y);


    int x_offside = cm.getOurOffsideLine(); //TODO change name

    double path_dist = rcscUtils::ballPathDistWithMaxSpeed(20);//TODO why 20?
    rcsc::Polygon2D dengerArea = getDengerArea(ball_pos, opp_pos);


    double search_radius = 15;//TODO dynamic
    std::pair<double, double> check_line_y(self_form_pos.y - search_radius, self_form_pos.y + search_radius);
    std::pair<double, double> check_line_x(self_form_pos.x - search_radius, self_form_pos.x + search_radius);


    double max_score = INT_MIN;
    Vector2D best_pos = Vector2D::INVALIDATED;


    Vector2D ball_next_pos = cm.getBallLordPos();


    ////DEBUG
    table_cover.clear();
    table_near_penalty.clear();
    table_near_goal.clear();
    table_goal_line.clear();
    table_near_target.clear();
    table_between_goal_angle.clear();
    table_near_to_pass_line.clear();
    table_final_score.clear();
    ///////////////////////////////////////

    for (int i = check_line_x.first; i <= check_line_x.second; i++) {
        //////DEBUGG
        table_cover.push_back(std::vector<double>(0));
        table_goal_line.push_back(std::vector<double>(0));
        table_near_goal.push_back(std::vector<double>(0));
        table_near_penalty.push_back(std::vector<double>(0));
        table_final_score.push_back(std::vector<double>(0));
        table_between_goal_angle.push_back(std::vector<double>(0));
        table_near_to_pass_line.push_back(std::vector<double>(0));
        table_near_target.push_back(std::vector<double>(0));
        /////DEBUG
        for (int j = check_line_y.first; j <= check_line_y.second; j++) {
            Vector2D check_point(i, j);


            if (!checkPosIsValid(check_point, self_pos, opp_pos, ball_next_pos, x_offside)) {
                continue;
            }

            double temp_score = 0;

//
//            double cover_danger =
//                    coverDengerPassArea(check_point, self_pos, opp_pos, ball_next_pos, dengerArea, path_dist) * 0.5;
            double near_to_goal_line =
                    nearToGoalLine(check_point, self_pos, opp_pos, ball_next_pos, 2 * search_radius) * 0.8;
//            double near_to_penalt =
//                    nearToPenaltyArea(check_point, self_pos, opp_pos, ball_next_pos, 2 * search_radius) * 0;
            double near_to_goal = nearToGoal(check_point, check_line_x.first, 2 * search_radius) * 0.3;
//
            double near_to_target = nearToTarget(check_point, self_pos, opp_pos, ball_next_pos, 2 * search_radius) * 0.5;

//            double near_to_pass_line = nearToPassLine(check_point, self_pos, opp_pos, ball_next_pos, 2 * search_radius) * 0.4;
            double near_to_body_dir =
                    nearToBodyDir(check_point, self_pos, opp_pos, ball_next_pos, 2 * search_radius, agent) * 0.1;


//            temp_score += cover_danger;
            temp_score += near_to_goal_line;
//            temp_score += near_to_penalt;
            temp_score += near_to_goal;
            temp_score += near_to_target;
//            temp_score += near_to_pass_line;
            temp_score += near_to_body_dir;


            ///////////
//            table_cover.back().push_back(cover_danger);
            table_goal_line.back().push_back(near_to_goal_line);
//            table_near_penalty.back().push_back(near_to_penalt);
            table_near_goal.back().push_back(near_to_goal);
//            table_near_target.back().push_back(near_to_target);
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

    log_table(table_cover, "table cover");
    log_table(table_goal_line, "near to goal line ");
    log_table(table_near_penalty, "near to penalty ");
    log_table(table_near_goal, "near to goal");
    log_table(table_near_target, "near to target");
    log_table(table_between_goal_angle, "table_between_goal_angle");
    log_table(table_near_to_pass_line, "table_near_to_pass_line");
    log_table(table_final_score, "final score ");


    dlog.addCircle(Logger::TEAM,
                   best_pos, 0.5, "#000000", true);


    dlog.addCircle(Logger::TEAM,
                   ball_next_pos, 0.5, "#ffffff", true);

    dlog.addCircle(Logger::TEAM,
                   opp_pos, 0.5, "#ff0000", true);


    dlog.addText(Logger::TEAM,
                 __FILE__": polygon size=%d",
                 dengerArea.vertices().size());
    geoUtils::drawPolygon(dengerArea, "#0000ff");


    return best_pos;
}


bool Bhv_MarkMan::checkPosIsValid(rcsc::Vector2D check_point, rcsc::Vector2D self_pos,
                                  rcsc::Vector2D opp_pos, rcsc::Vector2D ball_pos, double our_offside_x) {
    const ServerParam &SP = ServerParam::i();
    const CafeModel &cm = CafeModel::i();

    double radius_offside_cover = 2;


    if (check_point.x + radius_offside_cover < our_offside_x) {
        return false;
    }

    double radius_opp_cover = -1.1;
    if(check_point.x <= -46){
        radius_opp_cover = -0.02;
    }

    if (opp_pos.x + radius_opp_cover < check_point.x) {
        return false;
    }

    if(opp_pos.dist(check_point) > 3.3){
        return false;
    }
    return true;
}

double Bhv_MarkMan::nearToTarget(rcsc::Vector2D check_point, rcsc::Vector2D self_pos, rcsc::Vector2D opp_pos,
                                 rcsc::Vector2D ball_pos, double max_radius2) {

    const double max_dist = std::sqrt(2 * std::pow(max_radius2, 2));
    return (max_dist - check_point.dist(opp_pos)) / max_dist;
}

double Bhv_MarkMan::betweenGoalAngle(rcsc::Vector2D check_point, rcsc::Vector2D self_pos, rcsc::Vector2D opp_pos,
                                     rcsc::Vector2D ball_pos, double max_radius2) {

    const ServerParam &SP = ServerParam::i();
    const Vector2D &our_goal = SP.ourTeamGoalPos();

    double origin_goal_angle = (Vector2D(our_goal.x, our_goal.y - 7).th() - opp_pos.th()).degree();
    double terminal_goal_angle = (Vector2D(our_goal.x, our_goal.y + 7).th() - opp_pos.th()).degree();

    double delta_angle_check = (check_point.th() - opp_pos.th()).degree();
    double delta_angle_teminal = origin_goal_angle - terminal_goal_angle;


    dlog.addText(Logger::TEAM,
                 __FILE__" origin_goal_angle       %.1f , terminal_goal_angle:  %.1f , delta_angle_check: %.1f ",
                 origin_goal_angle, terminal_goal_angle, delta_angle_check
    );


    if (delta_angle_check > 0 && delta_angle_check < delta_angle_teminal) {
        return 1;
    }
    return 0;
}

double Bhv_MarkMan::nearToGoalLine(rcsc::Vector2D check_point, rcsc::Vector2D self_pos,
                                   rcsc::Vector2D opp_pos, rcsc::Vector2D ball_pos, double max_radius2) {

    const ServerParam &SP = ServerParam::i();
    const Vector2D &our_goal = SP.ourTeamGoalPos();

    const double max_dist = 6;//std::sqrt(2 * std::pow(max_radius2, 2));

    Line2D opp_to_goal(opp_pos, our_goal);
    double dist_to_goal_line = opp_to_goal.dist(check_point);
    return (max_dist - dist_to_goal_line) / max_dist;
}

double Bhv_MarkMan::nearToBodyDir(rcsc::Vector2D check_point, rcsc::Vector2D self_pos, rcsc::Vector2D opp_pos,
                                  rcsc::Vector2D ball_pos, double max_radius2, rcsc::PlayerAgent *agent) {

    const WorldModel &wm = agent->world();
    const AngleDeg &body_dir = wm.self().body();
    Vector2D terminal;
    terminal = terminal.setPolar(50, body_dir) + self_pos;

    const double max_dist = std::sqrt(2 * std::pow(max_radius2, 2));

    Segment2D self_dir(self_pos, terminal);

    dlog.addLine(Logger::TEAM,
                 self_pos, terminal,
                 "#f00fff");


    Vector2D foot = geoUtils::findFoot(self_dir, check_point);
    double dist_to_goal_line = foot.dist(check_point);
    return dist_to_goal_line < 1.1 ? 1 : 0;
}

double Bhv_MarkMan::nearToPassLine(rcsc::Vector2D check_point, rcsc::Vector2D self_pos,
                                   rcsc::Vector2D opp_pos, rcsc::Vector2D ball_pos, double max_radius2) {

    const double max_dist = 6;//;std::sqrt(2 * std::pow(max_radius2, 2));

    Line2D opp_to_ball(opp_pos, ball_pos);
    double dist_to_pass_line = opp_to_ball.dist(check_point);
    return (max_dist - dist_to_pass_line) / max_dist;
}

double Bhv_MarkMan::nearToPenaltyArea(rcsc::Vector2D check_point, rcsc::Vector2D self_pos,
                                      rcsc::Vector2D opp_pos, rcsc::Vector2D ball_pos,
                                      double max_radius2) {
    const double max_dist = std::sqrt(2 * std::pow(max_radius2, 2));

    Line2D opp_to_penalty(opp_pos, Vector2D(0, opp_pos.y)); //TODO -35
    double dist_to_penalty_line = opp_to_penalty.dist(check_point);

    return (max_dist - dist_to_penalty_line) / max_dist;
}

double Bhv_MarkMan::nearToGoal(rcsc::Vector2D check_point, double start_x, double max_radius2) {
    return (max_radius2 - check_point.x + start_x) / max_radius2;
}

double Bhv_MarkMan::coverDengerPassArea(rcsc::Vector2D check_point, rcsc::Vector2D self_pos,
                                        rcsc::Vector2D opp_pos, rcsc::Vector2D ball_pos,
                                        rcsc::Polygon2D denger_area, double path_dist) {

    if (check_point.dist(ball_pos) > path_dist) {
        return 0;
    }
    double defense_radius = 3;//TODO  dynamic

    Polygon2D temp_cover_poly;

    Vector2D origin = Vector2D(check_point.x, check_point.y - defense_radius);
    Vector2D terminal = Vector2D(check_point.x, check_point.y + defense_radius);

    //TODO check sentax
    origin = (check_point - ball_pos);
    origin = origin.setLength(defense_radius);
    terminal = origin;
    origin = origin.rotate(90) + check_point;
    terminal = terminal.rotate(-90) + check_point;


    Vector2D first_vec = (origin - ball_pos);
    first_vec = first_vec.setLength(path_dist) + ball_pos;
    Vector2D second_vec = (terminal - ball_pos);
    second_vec = second_vec.setLength(path_dist) + ball_pos;

    temp_cover_poly.addVertex(origin);
    temp_cover_poly.addVertex(terminal);
    temp_cover_poly.addVertex(first_vec);
    temp_cover_poly.addVertex(second_vec);


    Polygon2D temp_covered_denger_poly = geoUtils::suthHodgClip(denger_area, temp_cover_poly);
    double covered_score = temp_covered_denger_poly.area() / denger_area.area();

    ////DEBUGGGGGG
    double temp_area = temp_covered_denger_poly.area();
    if (max_area_cover < temp_area) {
        max_area_cover = temp_area;
        max_covered_denger_poly = temp_cover_poly;
    }

    ////////////


    return covered_score;
}

rcsc::Polygon2D Bhv_MarkMan::getDengerArea(Vector2D ball_pos,
                                           Vector2D opp_pos) {

    const ServerParam &SP = ServerParam::i();

    /////////////TODO to utils
    int path_cycle = 6;
    double max_speed = SP.ballSpeedMax();
    double dencay = SP.ballDecay();

    if (dencay == 1) {
        std::cerr << "dancay is 1 ..." << std::endl;
        std::exit(EXIT_FAILURE);
    }
    double path_dist = max_speed * (1 - std::pow(dencay, path_cycle)) / (1 - dencay);
    ////////////////////////////////////////////////

    const Vector2D our_goal = SP.ourTeamGoalPos();
    Vector2D start_point_goal(our_goal.x, our_goal.y - 10);
    Vector2D end_point_goal(our_goal.x, our_goal.y + 10);


    Vector2D start_point_opp(opp_pos.x, opp_pos.y - 5);
    Vector2D end_point_opp(opp_pos.x, opp_pos.y + 5);


    Vector2D start_point_danger = (start_point_goal - start_point_opp);
    start_point_danger.setLength(path_dist);
    start_point_danger += start_point_opp;

    Vector2D end_point_danger = (end_point_goal - end_point_opp);
    end_point_danger.setLength(path_dist);
    end_point_danger += end_point_opp;


    std::vector <rcsc::Vector2D> rect;

    rect.push_back(start_point_opp);
    rect.push_back(end_point_opp);
    rect.push_back(start_point_danger);
    rect.push_back(end_point_danger);

    rcsc::Polygon2D dengerArea(rect);
    geoUtils::polygonOrientation(dengerArea);//TODO true ?


    return dengerArea;//TODO is not true , ref or pointer
}


#include "../../utils/utils.cpp"

void Bhv_MarkMan::log_table(std::vector <std::vector<double> >  table, std::string name) {
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
        temp += "    ";
        for (int j = 0; j < max_d; j++) {
            temp += patch::to_string(arr2[i][j]) + "\t";
        }
        rcsc::dlog.addText(rcsc::Logger::TEAM,
                           __FILE__" %s ", temp.c_str());
        temp = "";
    }

}