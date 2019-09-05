//
// Created by armanaxh on ۲۰۱۹/۸/۲۹.
//


#include <algorithm>
#include <iostream>
#include <rcsc/action/basic_actions.h>
#include <rcsc/action/body_go_to_point.h>
#include <rcsc/action/neck_turn_to_ball_or_scan.h>
#include <rcsc/action/neck_turn_to_low_conf_teammate.h>


#include "bhv_defensive_positioning.h"
#include <rcsc/player/player_agent.h>
#include <rcsc/common/logger.h>
#include <rcsc/common/server_param.h>
#include "../cafe_model.h"
#include "../strategy.h"
#include "../utils/algo_utils.h"
#include "../utils/geo_utils.h"
#include "../utils/rcsc_utils.h"

using namespace rcsc;


bool Bhv_DefensivePositioning::execute(rcsc::PlayerAgent *agent) {


    if (defendTheDefendLine(agent)) {
        return true;
    } else if (positioningDengerArea(agent)) {
        return true;
    }

    return false;
}


bool Bhv_DefensivePositioning::defendTheDefendLine(rcsc::PlayerAgent *agent) {

    const WorldModel &wm = agent->world();
    const ServerParam &SP = ServerParam::i();
    const Strategy &stra = Strategy::i();
    const CafeModel &cm = CafeModel::i();


    std::vector<int> defensive_players_unum = stra.getGroupPlayer(Defense);

    if (std::find(defensive_players_unum.begin(), defensive_players_unum.end(), wm.self().unum()) ==
        defensive_players_unum.end()) {
        return false;
    }

    /////danger opponents
    ConstPlayerPtrCont defensive_players = cm.getOurPlayersByUnum(defensive_players_unum);
    PlayerPtrCont denger_opp_on_line = getDengerOpponent(agent);

    if (denger_opp_on_line.empty()) {
        return false;
    }

    const PlayerObject *target_opp = assignOpponent(defensive_players, denger_opp_on_line,
                                                    wm.self().unum());//TODO NULL check
    if (target_opp == NULL) {
        return false;
    }

    dlog.addLine(Logger::TEAM,
                 wm.self().pos(), target_opp->pos(),
                 "#ff0010");

    dlog.addRect(Logger::TEAM,
                 target_opp->pos().x - 2, target_opp->pos().y - 2, 4, 4,
                 "#ff2200");


    Vector2D target_point = getDefensivePos(agent, target_opp);
    if (target_point == Vector2D::INVALIDATED) {
        return false;
    }

    const double dash_power = Strategy::get_normal_dash_power(wm, stra);

    double dist_thr = wm.ball().distFromSelf() * 0.04;
    if (dist_thr < 1.0) dist_thr = 1.0;

    dlog.addText(Logger::TEAM,
                 __FILE__": Bhv_Defencive_Positioning target=(%.1f %.1f) dist_thr=%.2f",
                 target_point.x, target_point.y,
                 dist_thr);


    if (!Body_GoToPoint(target_point, dist_thr, dash_power
    ).execute(agent)) {
        Body_TurnToBall().execute(agent);
    }

    if (wm.existKickableOpponent()
        && wm.ball().distFromSelf() < 18.0) {
        agent->setNeckAction(new Neck_TurnToBall());
    } else {
        agent->setNeckAction(new Neck_TurnToBallOrScan());
    }

    return true;

}


rcsc::PlayerPtrCont Bhv_DefensivePositioning::getDengerOpponent(rcsc::PlayerAgent *agent) {

    const CafeModel &cm = CafeModel::i();
    const WorldModel &wm = agent->world();
    const Strategy &stra = Strategy::i();

    const Vector2D ball_pos = wm.ball().pos();

    double defence_line = stra.getDeffanceLine() - 3;//TODO goto getPlayerInRange
    double offside_line = cm.getOurOffsideLine();
    double defence_radus = 13;//TODO 5 cycle

    double x_start = std::min(defence_line, offside_line);
    double x_end = std::max(x_start + defence_radus, offside_line + defence_radus);

    Strategy::BallArea ball_area = stra.get_ball_area(wm);

    if (ball_area == Strategy::BA_Danger || ball_area == Strategy::BA_CrossBlock) {
        x_start = -60;
        x_end = -33;
        return PlayerPtrCont();//TODO fix return NULL
    } else {
        x_end = std::min(x_end, ball_pos.x + 3);
    }


    PlayerPtrCont opponents = cm.getPlayerInRangeX(x_start, x_end, false);


    ///////////////////////////////////////////////// DEBUG
    dlog.addLine(Logger::TEAM,
                 Vector2D(x_start, -30), Vector2D(x_start, 30),
                 "#00ffff");
    dlog.addLine(Logger::TEAM,
                 Vector2D(x_end, -30), Vector2D(x_end, 30),
                 "#00ffff");

    dlog.addText(Logger::TEAM,
                 __FILE__": Max dist ball : %.2f",
                 rcscUtils::maxDistBall());


////////////////////////////////////////////////////////////////////
    //TODO for now
    PlayerPtrCont denger_opp;

    const PlayerPtrCont::iterator end_t = opponents.end();
    for (PlayerPtrCont::iterator it = opponents.begin(); it != end_t; it++) {
        if ((*it)->distFromBall() < 4 || (*it)->isKickable()) {
            continue;
        }
        if ((*it)->distFromBall() > rcscUtils::maxDistBall()) {
            continue;
        }

        denger_opp.push_back(&(**it));
    }


    return denger_opp;
}

const PlayerObject *
Bhv_DefensivePositioning::assignOpponent(ConstPlayerPtrCont def_ps, PlayerPtrCont opp_ps, int self_unum) {

    Hungarian::Matrix cost_m;

    if (def_ps.empty() || opp_ps.empty()) {
        return NULL;
    }


    int i = 0;
    const ConstPlayerPtrCont::const_iterator end_dps = def_ps.end();
    for (ConstPlayerPtrCont::const_iterator itd = def_ps.begin();
         itd != end_dps; itd++) {
        int j = 0;
        cost_m.push_back(std::vector<int>());
        const PlayerPtrCont::const_iterator opp_ops = opp_ps.end();
        for (PlayerPtrCont::const_iterator ito = opp_ps.begin();
             ito != opp_ops; ito++) {
            double dist2 = (*itd)->pos().dist2((*ito)->pos());
            cost_m[i].push_back(dist2);
        }
        i++;
    }

    Hungarian::Result result = AlgoUtils::hungarianAssignment(cost_m, Hungarian::MODE_MINIMIZE_COST);

    if (result.success != true) {
        return NULL;
    }

    Hungarian::Matrix solution = result.assignment;
//    /////////////////////////////////TODO DEBUG
//    for (int i = 0; i < solution.size(); i++) {
//        for (int j = 0; j < solution[i].size(); j++) {
//            std::cout << solution[i][j] << " ,";
//        }
//        std::cout << std::endl;
//    }
//    //////////////////////////////////////////

    for (int j = 0; j < opp_ps.size(); j++) {
        for (int i = 0; i < solution.size(); i++) {
            if (solution[i][j]) {
                dlog.addLine(Logger::TEAM,
                             def_ps[i]->pos(), opp_ps[j]->pos(),
                             "#4400ff");
                if (def_ps[i]->unum() == self_unum) {
                    return opp_ps[j];
                }
                break;
            }
        }
    }

    return NULL;
}

///////////////////////DEBUG
static Polygon2D max_covered_denger_poly;
static double max_area_cover = INT_MIN;

static std::vector <std::vector<double> > table_cover;
static std::vector <std::vector<double> > table_goal_line;
static std::vector <std::vector<double> > table_near_penalty;
static std::vector <std::vector<double> > table_near_goal;
static std::vector <std::vector<double> > table_final_score;
///////////////////////////////

Vector2D Bhv_DefensivePositioning::getDefensivePos(rcsc::PlayerAgent *agent, const PlayerObject *opponent) {


    const ServerParam &SP = ServerParam::i();
    const WorldModel &wm = agent->world();
    const CafeModel &cm = CafeModel::i();

    Vector2D self_pos = Strategy::i().getPosition(wm.self().unum());
    Vector2D ball_pos = wm.ball().pos();
    Vector2D opp_pos = opponent->pos();

    int x_offside = cm.getOurOffsideLine(); //TODO change name

    double path_dist = rcscUtils::ballPathDistWithMaxSpeed(20);//TODO why 20?
    rcsc::Polygon2D dengerArea = getDengerArea(ball_pos, opp_pos);


    double search_radius = 5;//TODO dynamic
    std::pair<double, double> check_line_y(self_pos.y - search_radius, self_pos.y + search_radius);
    std::pair<double, double> check_line_x(self_pos.x - search_radius, self_pos.x + search_radius);


    double max_score = INT_MIN;
    Vector2D best_pos = Vector2D::INVALIDATED;

    double max_score_just_cover = INT_MIN;
    Vector2D best_pos_just_cover;



    ////DEBUG
    table_cover.clear();
    table_near_penalty.clear();
    table_near_goal.clear();
    table_goal_line.clear();
    table_final_score.clear();
    ///////////////////////////////////////

    for (int i = check_line_x.first; i <= check_line_x.second; i++) {
        //////DEBUGG
        table_cover.push_back(std::vector<double>());
        table_goal_line.push_back(std::vector<double>());
        table_near_goal.push_back(std::vector<double>());
        table_near_penalty.push_back(std::vector<double>());
        table_final_score.push_back(std::vector<double>());
        /////DEBUG
        for (int j = check_line_y.first; j <= check_line_y.second; j++) {
            Vector2D check_point(i, j);


            if (!checkPosIsValid(check_point, self_pos, opp_pos, ball_pos, x_offside)) {
                continue;
            }


            double temp_score = 0;
            double cover_danger =
                    coverDengerPassArea(check_point, self_pos, opp_pos, ball_pos, dengerArea, path_dist) * 0.6;
            ///////////
            table_cover.back().push_back(cover_danger);
            ///////////DEBUG
            if (max_score_just_cover < temp_score) {
                max_score_just_cover = temp_score;
                best_pos_just_cover = check_point;
            }
            /////////////////

            double near_to_goal_line =
                    nearToGoalLine(check_point, self_pos, opp_pos, ball_pos, 2 * search_radius) * 0.35;
            table_goal_line.back().push_back(near_to_goal_line);
            double near_to_penalt =
                    nearToPenaltyArea(check_point, self_pos, opp_pos, ball_pos, 2 * search_radius) * 0.35;
            table_near_penalty.back().push_back(near_to_penalt);
            double near_to_goal = nearToGoal(check_point, check_line_x.first, 2 * search_radius) * 0.2;
            table_near_goal.back().push_back(near_to_goal);


            temp_score += cover_danger;
            temp_score += near_to_goal_line;
            temp_score += near_to_penalt;
            temp_score += near_to_goal;


            if (max_score < temp_score) {
                max_score = temp_score;
                best_pos = check_point;
            }

            dlog.addCircle(Logger::TEAM,
                           check_point, 0.5, "#0ff000", false);

            table_final_score.back().push_back(temp_score);
        }

    }


    dlog.addCircle(Logger::TEAM,
                   best_pos, 0.5, "#000000", true);
    dlog.addCircle(Logger::TEAM,
                   best_pos_just_cover, 0.5, "#ffffff", true);


    dlog.addCircle(Logger::TEAM,
                   ball_pos, 0.5, "#ffffff", true);

    dlog.addCircle(Logger::TEAM,
                   opp_pos, 0.5, "#ff0000", true);


    dlog.addText(Logger::TEAM,
                 __FILE__": polygon size=%d",
                 dengerArea.vertices().size());
    geoUtils::drawPolygon(dengerArea, "#0000ff");

    /////DEBUG
    geoUtils::drawPolygon(max_covered_denger_poly, "#ffff55");
    max_area_cover = 0;
    max_covered_denger_poly = Polygon2D();
    //////


    log_table(table_cover, "table cover");
    log_table(table_goal_line, "near to goal line ");
    log_table(table_near_penalty, "near to penalty ");
    log_table(table_near_goal, "near to goal");
    log_table(table_final_score, "final score ");

    return best_pos;
}


bool Bhv_DefensivePositioning::checkPosIsValid(rcsc::Vector2D check_point, rcsc::Vector2D self_pos,
                                               rcsc::Vector2D opp_pos, rcsc::Vector2D ball_pos, double our_offside_x) {
    const ServerParam &SP = ServerParam::i();
    const CafeModel &cm = CafeModel::i();

    if (check_point.x < our_offside_x) {
        return false;
    }
    return true;
}


double Bhv_DefensivePositioning::nearToGoalLine(rcsc::Vector2D check_point, rcsc::Vector2D self_pos,
                                                rcsc::Vector2D opp_pos, rcsc::Vector2D ball_pos, double max_radius2) {

    const ServerParam &SP = ServerParam::i();
    const Vector2D our_goal = SP.ourTeamGoalPos();

    const double max_dist = std::sqrt(2 * std::pow(max_radius2, 2));

    Line2D opp_to_goal(opp_pos, our_goal);
    double dist_to_goal_line = opp_to_goal.dist(check_point);
    return (max_dist - dist_to_goal_line) / max_dist;
}

double Bhv_DefensivePositioning::nearToPenaltyArea(rcsc::Vector2D check_point, rcsc::Vector2D self_pos,
                                                   rcsc::Vector2D opp_pos, rcsc::Vector2D ball_pos,
                                                   double max_radius2) {
    const double max_dist = std::sqrt(2 * std::pow(max_radius2, 2));

    Line2D opp_to_penalty(opp_pos, Vector2D(0, opp_pos.y)); //TODO -35
    double dist_to_penalty_line = opp_to_penalty.dist(check_point);

    return (max_dist - dist_to_penalty_line) / max_dist;
}

double Bhv_DefensivePositioning::nearToGoal(rcsc::Vector2D check_point, double start_x, double max_radius2) {
    return (max_radius2 - check_point.x + start_x) / max_radius2;
}

double Bhv_DefensivePositioning::coverDengerPassArea(rcsc::Vector2D check_point, rcsc::Vector2D self_pos,
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

rcsc::Polygon2D Bhv_DefensivePositioning::getDengerArea(Vector2D ball_pos,
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


bool Bhv_DefensivePositioning::positioningDengerArea(rcsc::PlayerAgent *agent) {
    return false;
}


#include <stdio.h>
#include <string>
#include <sstream>

//TODO to Utils
namespace patch {
    template<typename T>
    std::string to_string(const T &n) {
        std::ostringstream stm;
        stm << n;
        return stm.str();
    }
}


void Bhv_DefensivePositioning::log_table(std::vector <std::vector<double> > table, std::string name) {
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
        for (int j = 0; j < max_d; j++) {
            temp += patch::to_string(arr2[i][j]) + "\t";
        }
        rcsc::dlog.addText(rcsc::Logger::TEAM,
                           __FILE__" %s ", temp.c_str());
        temp = "";
    }

}