//
// Created by armanaxh on ۲۰۱۹/۸/۲۶.
//

#ifndef RCSC_AGENT_GEOUTILS_H
#define RCSC_AGENT_GEOUTILS_H

#include <iostream>
#include <vector>
#include <rcsc/geom/vector_2d.h>
#include <rcsc/geom/segment_2d.h>
#include <rcsc/geom/polygon_2d.h>

class geoUtils {
    geoUtils() {}

public:

    static double x_intersect(double x1, double y1, double x2, double y2,
                              double x3, double y3, double x4, double y4);

    static double y_intersect(double x1, double y1, double x2, double y2,
                              double x3, double y3, double x4, double y4);

    static void clip(std::vector <Vector2D> &poly_points, int &poly_size,
                     double x1, double y1, double x2, double y2);

    static Polygon2D suthHodgClip(Polygon2D &poly, Polygon2D &clipper);

    static void test();

    static void drawPolygon(Polygon2D poly, const char *color = "#ff00000");

    static void swap(Vector2D &p1, Vector2D &p2);

    static double dist(Vector2D p1, Vector2D p2);

    static int orientation(Vector2D p, Vector2D q, Vector2D r);

    static void orientationSort(std::vector <Vector2D> &points, bool clockwise = true);

    static int compareSlopeP0(const void *vp1, const void *vp2);

    static void polygonOrientation(Polygon2D &poly);

    };


#endif //RCSC_AGENT_GEOUTILS_H
