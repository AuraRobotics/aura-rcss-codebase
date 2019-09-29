//
// Created by armanaxh on ۲۰۱۹/۹/۲۲.
//

#include "player_relationship.h"
#include <vector>
#include <map>
#include <iostream>
#include "../rcsc_utils.h"

using namespace rcsc;

std::vector<int> PlayerRelationship::path_to_[11];

void PlayerRelationship::calc(PlayerAgent *agent) {

    M_triangulation.clear();
    for (int i = 0; i < 11; i++) {
        relationships[i].clear();
    }

    addVertexs();

    M_triangulation.compute();
    M_triangulation.updateVoronoiVertex();

    calcRelations();
    createGraph();
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




rcsc::AbstractPlayerCont PlayerRelationship::getPassPath(const int sender, const int resiver) const {
    processPath(sender);
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
const void PlayerRelationship::createGraph() {

    for (int i = 0; i < 11; i++) {
        for (int j = 0; j < 11; j++) {
            graph[i][j] = 999;
        }
    }

    for (int i = 0; i < 11; i++) {
        for (int j = 0; j < relationships[i].size(); j++) {
            int temp_unum = relationships[i][j]->unum();
            if (temp_unum > 0)
                graph[i][temp_unum - 1] = getCost(i + 1, relationships[i][j]);
        }
    }


    printGraph();
}


/*
 *
 */
const double PlayerRelationship::getCost(int unum_first, const rcsc::AbstractPlayerObject *player_second) {
    const AbstractPlayerObject *player_first = wm.ourPlayer(unum_first);
    Vector2D player_first_pos = player_first->pos();
    Vector2D player_second_pos = player_second->pos();

    double dist_ = player_first_pos.dist(player_second_pos);

    double cost = 0;
    cost += rcscUtils::ballCycle(dist_) * 10;

    return cost;
}
///////////
/*
 *
 */
#include "../utils.cpp"

void PlayerRelationship::printGraph() {
    dlog.addText(Logger::TEAM,
                 __FILE__":   %s -----------------", "graph");
    std::string temp;
    for (int i = 0; i < 11; i++) {
        temp += "   \t";
        for (int j = 0; j < 11; j++) {
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

    int dist[V]; // The output array. dist[i] will hold the shortest
    // distance from src to i

    int path[11] = {-1};

    bool sptSet[V]; // sptSet[i] will be true if vertex i is included in shortest
    // path tree or shortest distance from src to i is finalized

    // Initialize all distances as INFINITE and stpSet[] as false
    for (int i = 0; i < V; i++) {
        dist[i] = 10000;//MAX_INT
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
            if (!sptSet[v] && graph[u][v] && dist[u] != INT_MAX
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

    dlog.addText(Logger::TEAM,
                 __FILE__"Vertex \t\t Distance from Source\n");
    for (int i = 0; i < V; i++) {

        dlog.addText(Logger::TEAM,
                     __FILE__"%d \t\t %d \n", i, dist[i]);
    }


    for (int i = 0; i < 11; i++) {
        dlog.addText(Logger::TEAM,
                     __FILE__"Vertex \t\t PATH from Source %d:\n   ", i + 1);
        std::string temp;
        for (int j = 0; j < path_to_[i].size(); j++) {
            temp += patch::to_string(path_to_[i][j] + 1) + "\t";
        }
        dlog.addText(Logger::TEAM,
                     __FILE__"\t\t%s \t", temp.c_str());
    }


}
///////////////////////////////////////////////

const int PlayerRelationship::minDistance(int dist[], bool sptSet[]) const {
    // Initialize min value
    int min = 100000;//TODO
    int min_index = -1;

    for (int v = 0; v < V; v++)
        if (sptSet[v] == false && dist[v] <= min)
            min = dist[v], min_index = v;

    return min_index;
}