//
// Created by armanaxh on ۲۰۱۹/۹/۲۲.
//

#include "player_relationship.h"
#include <vector>
#include <map>
#include <iostream>

using namespace rcsc;


void PlayerRelationship::calc() {

    addVertexs();


    M_triangulation.compute();
    M_triangulation.updateVoronoiVertex();

    calcRelations();
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

    for (int i = 1; i <= 11; i++) {
        relationships.push_back(rcsc::AbstractPlayerCont());
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

///////////
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
///////////
/*
 *
 */
rcsc::AbstractPlayerCont PlayerRelationship::getNeighbors(const int unum) {

    return relationships[unum - 1];
}

