//
// Created by armanaxh on ۲۰۱۹/۹/۲۲.
//

#ifndef CAFE_2D_PASS_NEIGHBOUR_ALLOCATOR_H
#define CAFE_2D_PASS_NEIGHBOUR_ALLOCATOR_H

#include <rcsc/player/world_model.h>
#include <rcsc/common/logger.h>
#include <rcsc/common/server_param.h>
#include "../cafe_model.h"
#include "../strategy.h"
#include <iostream>
#include <map>
#include <rcsc/geom/delaunay_triangulation.h>
#include <rcsc/player/player_object.h>

class PlayerRelationship {

    const WorldModel &wm;
    const ServerParam &SP;
    const Strategy &stra;
    const CafeModel &cm;

    DelaunayTriangulation M_triangulation;
    std::vector <rcsc::AbstractPlayerCont> relationships;


public:
    PlayerRelationship(const WorldModel &wm) :
            wm(wm),
            SP(ServerParam::i()),
            stra(Strategy::i()),
            cm(CafeModel::i()) {}


    void calc();

    rcsc::AbstractPlayerCont getNeighbors(const int unum);

private:
    void addVertexs();

    void calcRelations();

    const rcsc::AbstractPlayerObject *getPlayerInPos(const Vector2D pos);

};


#endif //CAFE_2D_PASS_NEIGHBOUR_ALLOCATOR_H
