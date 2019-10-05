//
// Created by armanaxh on ۲۰۱۹/۱۰/۱.
//

#ifndef CAFE_2D_ACTGEN_AREA_PASS_H
#define CAFE_2D_ACTGEN_AREA_PASS_H

#include "action_generator.h"
#include <rcsc/player/player_agent.h>
#include <rcsc/geom/vector_2d.h>


class ActGen_AreaPass
        : public ActionGenerator {

public:
    virtual
    void generate(std::vector <ActionStatePair> *result,
                  const PredictState &state,
                  const rcsc::WorldModel &wm,
                  const std::vector <ActionStatePair> &path) const;

private:
};


#endif //CAFE_2D_ACTGEN_AREA_PASS_H
