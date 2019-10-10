//
// Created by armanaxh on ۲۰۱۹/۱۰/۱۰.
//

#ifndef CAFE_2D_BHV_SHOOT_H
#define CAFE_2D_BHV_SHOOT_H


class Bhv_Shoot
        : public rcsc::SoccerBehavior {
public:
    Bhv_Shoot() {}

    bool execute(rcsc::PlayerAgent *agent);

private:


};


#endif //CAFE_2D_BHV_SHOOT_H
