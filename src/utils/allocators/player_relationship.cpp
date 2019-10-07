//
// Created by armanaxh on ۲۰۱۹/۹/۲۲.
//

#include "player_relationship.h"
#include <vector>
#include <map>
#include <iostream>
#include "../rcsc_utils.h"
#include "./area_pass_generator.h"
#include "./deep_pass_generator.h"

using namespace rcsc;

std::vector<int> PlayerRelationship::path_to_[11];
FastIC *PlayerRelationship::fic;

void PlayerRelationship::calc(PlayerAgent *agent, FastIC *fic) {


    this->fic = fic;
    fic->setByWorldModel();
    fic->setMaxCycleAfterFirstFastestPlayer(5);

    M_triangulation.clear();
    for (int i = 0; i < 11; i++) {
        relationships[i].clear();
        area_pass[i].clear();
        deep_pass[i].clear();
    }

    addVertexs();

    M_triangulation.compute();
    M_triangulation.updateVoronoiVertex();

    calcRelations();
    createGraph();
    calcKickable();
}

void PlayerRelationship::calcKickable() {
    if(!wm.self().isKickable() && wm.self().unum() != 8){
        return;
    }

    clock_t start_time = clock();


    AreaPassGenerator area_pass_generator(relationships, wm, fic);
    area_pass_generator.generate();

    for (int i = 0; i < 11; i++) {
        area_pass[i] = area_pass_generator.getAreaPass(i + 1);
    }

//    std::cout << " time area pass : " << clock() - start_time << std::endl;
//    start_time = clock();

    DeepPassGenerator deep_pass_generator(relationships, wm, fic);
    deep_pass_generator.generate();

    for (int i = 0; i < 11; i++) {
        deep_pass[i] = deep_pass_generator.getDirectPass(i + 1);
    }
//    std::cout << " time deep pass : " << clock() - start_time << std::endl;
//    start_time = clock();
}

void PlayerRelationship::addVertexs() {
    const SideID our = wm.ourSide();


    const std::vector <rcsc::Vector2D> &form_positions = stra.getPositions();
    for (int i = 0; i < form_positions.size(); i++) {
        M_triangulation.addVertex(form_positions[i]);
    }


}

//TODO performance
void PlayerRelationship::calcRelations() {
    typedef std::map<const Vector2D *, std::vector < Vector2D> > GraphPos;
    GraphPos relations_pos;


    for (DelaunayTriangulation::VertexCont::const_iterator v = M_triangulation.vertices().begin(),
                 end = M_triangulation.vertices().end();
         v != end;
         ++v) {
        relations_pos[&v->pos()] = std::vector<Vector2D>();
    }


    for (DelaunayTriangulation::EdgeCont::const_iterator it = M_triangulation.edges().begin(),
                 end = M_triangulation.edges().end();
         it != end;
         ++it) {

        const Vector2D &origin = it->second->vertex(0)->pos();
        const Vector2D &terminal = it->second->vertex(1)->pos();

        relations_pos[&origin].push_back(terminal);
        relations_pos[&terminal].push_back(origin);


    }


    for (GraphPos::const_iterator it = relations_pos.begin(); it != relations_pos.end(); it++) {
        const Vector2D main = (*it->first);
        const AbstractPlayerObject *main_player = getPlayerInPos(main);
        if (main_player == NULL) {
            continue;
        }
        int main_player_unum = main_player->unum();
        if (main_player_unum == -1) {
            continue;
        }

        const std::vector <Vector2D> &list = it->second;
        for (int i = 0; i < list.size(); i++) {
            const AbstractPlayerObject *player_object = getPlayerInPos(list[i]);

            if (player_object != NULL && player_object->unum() != -1) {
                relationships[main_player_unum - 1].push_back(player_object);
            }
        }


    }






}

/*
 *
 */
const rcsc::AbstractPlayerObject *PlayerRelationship::getPlayerInPos(const Vector2D pos) {
    const std::vector <rcsc::Vector2D> &form_positions = stra.getPositions();
    for (int i = 0; i < form_positions.size(); i++) {


        double x = form_positions[i].x;
        double y = form_positions[i].y;

        if (std::pow(pos.x - x, 2) + std::pow(pos.y - y, 2) < 1.0e-3) {
            // detect same coordinate vertex
            return wm.ourPlayer(i + 1);
        }
    }
    return NULL;
}

/*
 *
 */
rcsc::AbstractPlayerCont PlayerRelationship::getNeighbors(const int unum) const {
    return relationships[unum - 1];
}

rcsc::AbstractPlayerCont PlayerRelationship::getShortPass(const int unum) const {
    return short_pass[unum - 1];
}

AreaPassCont PlayerRelationship::getAreaPass(const int unum) const {
    return area_pass[unum - 1];
}

DeepPassCont PlayerRelationship::getDeepPass(const int unum) const {
    return deep_pass[unum - 1];
}

rcsc::AbstractPlayerCont PlayerRelationship::getPassPath(const int sender, const int resiver) const {
    static int last_sender = -1;
    if (last_sender != sender) {
        processPath(sender);
        last_sender = sender;
    }
    std::vector<int> path_pass = path_to_[resiver - 1];
    AbstractPlayerCont path_pass_objects;
    for (int i = 0; i < path_pass.size(); i++) {
        const AbstractPlayerObject *object_player = wm.ourPlayer(path_pass[i] + 1);
        path_pass_objects.push_back(object_player);
    }

    return path_pass_objects;
}

/*
 *
 */
static FastIC *fic;

const void PlayerRelationship::createGraph() {

    for (int i = 0; i < 11; i++) {
        for (int j = 0; j < 11; j++) {
            graph_full[i][j] = INT_MAX;
            graph_pass[i][j] = INT_MAX;
        }

        short_pass[i].clear();
    }


    for (int i = 1; i < 11; i++) {
        for (int j = 0; j < relationships[i].size(); j++) {
            int temp_unum = relationships[i][j]->unum();
            if (temp_unum > 0) {
                double cost = getCost(i + 1, relationships[i][j]);
                graph_full[i][temp_unum - 1] = cost;
                if (ignoreIterceptPass(i + 1, relationships[i][j])) {
                    continue;
                }
                graph_pass[i][temp_unum - 1] = cost;
                short_pass[i].push_back(relationships[i][j]);
            }

        }
    }


//    printGraph(graph_full, "graph full");
//    printGraph(graph_pass, "graph pass");

    //////////////////////////////

    const AbstractPlayerCont::const_iterator pass_end = short_pass[wm.self().unum() - 1].end();
    for (AbstractPlayerCont::const_iterator pass_resiver_it = short_pass[wm.self().unum() - 1].begin();
         pass_resiver_it != pass_end;
         ++pass_resiver_it) {

        dlog.addLine(Logger::PASS,
                     wm.self().pos(), (*pass_resiver_it)->pos(),
                     "#42a7f5");
    }

    //////////////////////


}

/*
 *
 */

bool PlayerRelationship::ignoreIterceptPass(int unum_first, const rcsc::AbstractPlayerObject *player_second) {

    const AbstractPlayerObject *player_first = wm.ourPlayer(unum_first);
    const Vector2D &player_first_pos = player_first->pos();
    const Vector2D &player_second_pos = player_second->pos();

    double x_offside = wm.offsideLineX();


    if (x_offside < player_second_pos.x) {
        return true;
    }

    if (player_first_pos.dist(player_second_pos) < 4) {
        return true;
    }
    if (player_second->goalie()) {
        return true;
    }


    double pass_dist = player_first_pos.dist(player_second_pos);
    const double max_receive_ball_speed = 1.45;

    double pass_speed = rcscUtils::first_speed_pass(pass_dist, max_receive_ball_speed);

    const int pass_cycle = rcscUtils::ballCycle(pass_dist, pass_speed);
    Vector2D donor_to_me_vel = player_second_pos - player_first_pos;
    donor_to_me_vel.setLength(pass_speed);

    fic->refresh();
    fic->setBall(player_first_pos + donor_to_me_vel, donor_to_me_vel, 0); //TODO donor_to_me_vel
    fic->calculate();

    const int fastest_opp_cycle = fic->getFastestOpponentReachCycle();


    if (fastest_opp_cycle < pass_cycle) {


        const AbstractPlayerObject *fastest_opp = fic->getFastestOpponent();
        int opp_unum = -1;
        if (fastest_opp != NULL) {
            opp_unum = fastest_opp->unum();
            dlog.addText(Logger::TEAM,
                         __FILE__":   ignore edge of  %d -> %d      with %d      opp_cycle : %d , pass_cycle : %d    mate_cycle : %d",
                         unum_first,
                         player_second->unum(), opp_unum, fastest_opp_cycle, pass_cycle,
                         fic->getFastestTeammateReachCycle());
        }
        return true;
    }


    return false;
}

/*
 *
 */
const double PlayerRelationship::getCost(int unum_first, const rcsc::AbstractPlayerObject *player_second) {
    const AbstractPlayerObject *player_first = wm.ourPlayer(unum_first);
    const Vector2D &player_first_pos = player_first->pos();
    const Vector2D &player_second_pos = player_second->pos();

    if (player_second->goalie() || player_second->unum() == 1) {
        return INT_MAX;
    }

    double pass_dist = player_first_pos.dist(player_second_pos);
    const double max_receive_ball_speed = 1.24;

    double pass_speed = rcscUtils::first_speed_pass(pass_dist, max_receive_ball_speed);

    double cost = 0;
    const int pass_cycle = rcscUtils::ballCycle(pass_dist, pass_speed);
    cost += pass_cycle * 10;

    //////////////////////////////////  PERFORMANCE
//    dlog.addLine(Logger::WORLD,
//                 player_first_pos, player_second_pos,
//                 "#42a7f5");
    ///////////////////////

    return cost;
}
///////////
/*
 *
 */
#include "../utils.cpp"

void PlayerRelationship::printGraph(double (*graph)[11], std::string name) {
    dlog.addText(Logger::TEAM,
                 __FILE__":   %s -----------------", name);
    std::string temp;
    for (int i = 0; i < 11; i++) {
        temp += "   \t";
        for (int j = 0; j < 11; j++) {
            int cost = graph[i][j];
            if (cost == INT_MAX)
                cost = 0;
            temp += patch::to_string(graph[i][j]) + "\t";
        }
        rcsc::dlog.addText(rcsc::Logger::TEAM,
                           __FILE__" %s ", temp.c_str());
        temp = "";
    }
}
///////////
/*
 *
 */


const int V = 11;

const void *PlayerRelationship::processPath(const int ball_lord_unum) const {

    const double (*graph)[11] = graph_full;

    int dist[V]; // The output array. dist[i] will hold the shortest
    // distance from src to i

    int path[11] = {-1};

    bool sptSet[V]; // sptSet[i] will be true if vertex i is included in shortest
    // path tree or shortest distance from src to i is finalized

    // Initialize all distances as INFINITE and stpSet[] as false
    for (int i = 0; i < V; i++) {
        dist[i] = INT_MAX;
        sptSet[i] = false;
    }

    // Distance of source vertex from itself is always 0
    dist[ball_lord_unum - 1] = 0;

// Find shortest path for all vertices
    for (int count = 0; count < V - 1; count++) {
        // Pick the minimum distance vertex from the set of vertices not
        // yet processed. u is always equal to src in the first iteration.
        int u = minDistance(dist, sptSet);
        // Mark the picked vertex as processed
        sptSet[u] = true;

        // Update dist value of the adjacent vertices of the picked vertex.
        for (int v = 0; v < V; v++) {
            if (!sptSet[v] && graph[u][v] && dist[u] < 100000
                && dist[u] + graph[u][v] < dist[v]) {
                dist[v] = dist[u] + graph[u][v];
                path[v] = u;
            }
        }

    }


    for (int i = 0; i < 11; i++) {
        int parent = i;
        path_to_[i].clear();
        path_to_[i].push_back(parent);
        while (parent != ball_lord_unum - 1) {
            if (parent == -1) {
                path_to_[i].clear();
                break;
            }
            parent = path[parent];
            path_to_[i].push_back(parent);
        }
    }

    printSolution(dist, path);
}

/////////////////////////////////////////////////////////
void PlayerRelationship::printSolution(int dist[], int path[11]) const {

    dlog.addText(Logger::PLAN,
                 __FILE__"Vertex \t\t Distance from Source\n");
    for (int i = 0; i < V; i++) {

        dlog.addText(Logger::PLAN,
                     __FILE__"%d \t\t %d \n", i, dist[i]);
    }


    for (int i = 0; i < 11; i++) {
        dlog.addText(Logger::PLAN,
                     __FILE__"Vertex \t\t PATH from Source %d:\n   ", i + 1);
        std::string temp;
        for (int j = 0; j < path_to_[i].size(); j++) {
            temp += patch::to_string(path_to_[i][j] + 1) + "\t";
        }
        dlog.addText(Logger::PLAN,
                     __FILE__"\t\t%s \t", temp.c_str());
    }


}
///////////////////////////////////////////////

const int PlayerRelationship::minDistance(int dist[], bool sptSet[]) const {
    // Initialize min value
    int min = INT_MAX;
    int min_index = -1;

    for (int v = 0; v < V; v++)
        if (sptSet[v] == false && dist[v] <= min)
            min = dist[v], min_index = v;

    return min_index;
}