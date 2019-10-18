//
// Created by armanaxh on ۲۰۱۹/۱۰/۱۶.
//

#ifndef CAFE_2D_ACTGEN_DEG_PASS_H
#define CAFE_2D_ACTGEN_DEG_PASS_H

#include "action_generator.h"
#include <rcsc/player/player_agent.h>
#include <rcsc/geom/vector_2d.h>
#include "../utils/allocators/area_pass_generator.h"


class ActGen_DegPass
        : public ActionGenerator {

public:

    virtual
    void generate( std::vector< ActionStatePair > * result,
                   const PredictState & state,
                   const rcsc::WorldModel & current_wm,
                   const std::vector< ActionStatePair > & path ) const;

private:
    void getDegPass(const rcsc::WorldModel & current_wm, const PredictState &state, std::vector< ActionStatePair > * result) const;
    void configFastIC(FastIC * fic, const rcsc::WorldModel &wm) const;
};

#endif //CAFE_2D_ACTGEN_DEG_PASS_H
