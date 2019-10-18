//
// Created by armanaxh on ۲۰۱۹/۹/۵.
//

#ifndef CAFE_2D_RCSC_UTILS_H
#define CAFE_2D_RCSC_UTILS_H

class rcscUtils {

    rcscUtils() {}

public:
    static double ballPathDistWithMaxSpeed(int cycle);
    static double ballPathDist(int cycle, double speed);

    static double maxDistBall(double initial_speed);
    static double maxDistBall();//with max_speed_ball
    static int ballCycle(double dist, double speed = -1);

    static double ball_speed_after_dist(double dist, double speed);
    static double first_speed_pass(double dist, double speed_target);
};


#endif //CAFE_2D_RCSC_UTILS_H
