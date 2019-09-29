//
// Created by armanaxh on ۲۰۱۹/۹/۲۲.
//

#ifndef CAFE_2D_PASS_NEIGHBOUR_ALLOCATOR_H
#define CAFE_2D_PASS_NEIGHBOUR_ALLOCATOR_H

#include <rcsc/player/world_model.h>
#include <rcsc/common/logger.h>
#include <rcsc/common/server_param.h>
#include "../strategy.h"
#include <iostream>
#include <limits>
#include <map>
#include <rcsc/geom/delaunay_triangulation.h>
#include <rcsc/player/player_object.h>

#define INT_MAX 5235232342

class PlayerRelationship {

    const rcsc::WorldModel &wm;
    const rcsc::ServerParam &SP;
    const Strategy &stra;

    rcsc::DelaunayTriangulation M_triangulation;
    rcsc::AbstractPlayerCont relationships[11];
    double graph[11][11];

    static std::vector<int> path_to_ [11];


public:
    PlayerRelationship(const rcsc::WorldModel &wm) :
            wm(wm),
            SP(rcsc::ServerParam::i()),
            stra(Strategy::i()) {}


    void calc(rcsc::PlayerAgent *agent);

    rcsc::AbstractPlayerCont getNeighbors(const int unum) const;

    rcsc::AbstractPlayerCont getPassPath(const int sender, const int resiver) const;



private:
    void addVertexs();

    void calcRelations();

    const rcsc::AbstractPlayerObject *getPlayerInPos(const rcsc::Vector2D pos);

    const void createGraph();

    const void *processPath(const int ball_lord_unum) const;

    const double getCost(int unum_start,const  rcsc::AbstractPlayerObject * end_player);

    const int minDistance(int dist[], bool sptSet[]) const;



    void printGraph();
    void printSolution(int dist[],  int path[11]) const;
};


#endif //CAFE_2D_PASS_NEIGHBOUR_ALLOCATOR_H
