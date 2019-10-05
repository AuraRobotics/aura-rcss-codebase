//
// Created by armanaxh on ۲۰۱۹/۱۰/۳.
//

#ifndef CAFE_2D_DIRECT_PASS_GENERATOR_H
#define CAFE_2D_DIRECT_PASS_GENERATOR_H

#include <rcsc/player/world_model.h>
#include <rcsc/player/player_object.h>
#include <rcsc/geom/vector_2d.h>
#include "../estimators/HERMES_FastIC.h"

typedef std::vector <std::pair<const AbstractPlayerObject *, rcsc::Vector2D> > DeepPassCont;

class DeepPassGenerator {


    const rcsc::WorldModel &wm;
    static FastIC *fic;
    rcsc::AbstractPlayerCont *relationships;

    DeepPassCont direct_pass[11];


public:
    DeepPassGenerator(rcsc::AbstractPlayerCont rel[11], const rcsc::WorldModel &wm, FastIC *fic) :
            relationships(rel), wm(wm) {
        this->fic = fic;
        fic->setByWorldModel();
    }


    void generate();

    DeepPassCont getDirectPass(const int unum) const {
        if (unum < 1 || unum > 11) {
            std::cerr << __FILE__" unum is not True!" << std::endl;
        }
        return direct_pass[unum - 1];
    }


private:
    const rcsc::Vector2D generateDeepPass(const rcsc::AbstractPlayerObject *sender,
                                          const rcsc::AbstractPlayerObject *resiver);


};

#endif //CAFE_2D_DIRECT_PASS_GENERATOR_H
