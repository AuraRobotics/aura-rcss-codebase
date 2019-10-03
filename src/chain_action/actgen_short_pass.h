//
// Created by armanaxh on ۲۰۱۹/۹/۳۰.
//

#ifndef CAFE_2D_ACTGEN_SHORT_PASS_H
#define CAFE_2D_ACTGEN_SHORT_PASS_H


#include "action_generator.h"

class ActGen_ShortPass
        : public ActionGenerator {

public:
    virtual
    void generate( std::vector< ActionStatePair > * result,
                   const PredictState & state,
                   const rcsc::WorldModel & wm,
                   const std::vector< ActionStatePair > & path ) const;
};


#endif //CAFE_2D_ACTGEN_SHORT_PASS_H
