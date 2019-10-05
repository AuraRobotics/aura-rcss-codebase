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
#include "../estimators/HERMES_FastIC.h"
#include "./area_pass_generator.h"

class PlayerRelationship {

    const rcsc::WorldModel &wm;
    const rcsc::ServerParam &SP;
    const Strategy &stra;

    static FastIC *fic;

    rcsc::DelaunayTriangulation M_triangulation;
    rcsc::AbstractPlayerCont relationships[11];
    double graph_full[11][11];
    double graph_pass[11][11];

    static std::vector<int> path_to_[11];

    rcsc::AbstractPlayerCont short_pass[11];
    AreaPassCont area_pass[11];



public:
    PlayerRelationship(const rcsc::WorldModel &wm) :
            wm(wm),
            SP(rcsc::ServerParam::i()),
            stra(Strategy::i()) {}


    void calc(rcsc::PlayerAgent *agent, FastIC *fic);



    rcsc::AbstractPlayerCont getNeighbors(const int unum) const;

    rcsc::AbstractPlayerCont getPassPath(const int sender, const int resiver) const;

    rcsc::AbstractPlayerCont getShortPass(const int unum) const;

    AreaPassCont getAreaPass(const int unum) const;

private:
    void addVertexs();

    void calcRelations();

    const rcsc::AbstractPlayerObject *getPlayerInPos(const rcsc::Vector2D pos);

    const void createGraph();

    const void *processPath(const int ball_lord_unum) const;

    bool ignoreIterceptPass(int unum_first, const rcsc::AbstractPlayerObject *player_second);

    const double getCost(int unum_first, const rcsc::AbstractPlayerObject *player_second);

    const int minDistance(int dist[], bool sptSet[]) const;


    void printGraph(double (*graph)[11], std::string name);

    void printSolution(int dist[], int path[11]) const;
};


#endif //CAFE_2D_PASS_NEIGHBOUR_ALLOCATOR_H
