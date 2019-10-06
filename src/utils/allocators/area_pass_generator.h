//
// Created by armanaxh on ۲۰۱۹/۱۰/۱.
//

#ifndef CAFE_2D_AREA_PASS_GENERATOR_H
#define CAFE_2D_AREA_PASS_GENERATOR_H

#include <rcsc/player/world_model.h>
#include <rcsc/player/player_object.h>
#include <rcsc/geom/vector_2d.h>
#include "../estimators/HERMES_FastIC.h"

typedef std::vector<std::pair< const AbstractPlayerObject * , rcsc::Vector2D> > AreaPassCont;

class AreaPassGenerator {


    const rcsc::WorldModel &wm;
    static FastIC *fic;
    rcsc::AbstractPlayerCont *relationships;

    AreaPassCont area_pass[11];


public:
    AreaPassGenerator(rcsc::AbstractPlayerCont rel[11], const rcsc::WorldModel &wm, FastIC *fic) :
            relationships(rel), wm(wm) {
        this->fic = fic;
        fic->setByWorldModel();
        fic->setMaxCycleAfterFirstFastestPlayer(4);
    }


    void generate();

    AreaPassCont getAreaPass(const int unum) const {
        if(unum < 1 || unum > 11){
            std::cerr << __FILE__" unum is not True!" << std::endl;
            exit(0);
        }
        return area_pass[unum -1];
    }



private:

    const rcsc::Vector2D generateAreaPass(const rcsc::AbstractPlayerObject *sender,
                                          const rcsc::AbstractPlayerObject *resiver);

    bool checkPosIsValid(const rcsc::Vector2D check_point, const rcsc::Vector2D sender_pos,
                         const rcsc::Vector2D resiver_pos, double x_offside, const int resiver_unum);


    double nearGoal(rcsc::Vector2D check_point, rcsc::Vector2D sender_pos, rcsc::Vector2D resiver_pos,
                    double max_radius2, double start_x);

    double nearResiver(rcsc::Vector2D check_point, rcsc::Vector2D sender_pos, rcsc::Vector2D resiver_pos,
                       double max_radius2);

    void log_table(std::vector <std::vector<double> > table, std::string name);
};


#endif //CAFE_2D_AREA_PASS_GENERATOR_H