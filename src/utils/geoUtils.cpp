//
// Created by armanaxh on ۲۰۱۹/۸/۲۶.
//


#include <iostream>
#include <rcsc/common/logger.h>
#include <rcsc/geom/vector_2d.h>
#include <rcsc/geom/segment_2d.h>
#include <rcsc/geom/polygon_2d.h>


#include "geoUtils.h"

using namespace rcsc;

const int MAX_POINTS = 25;

// Returns x-value of point of intersectipn of two
// lines
double geoUtils::x_intersect(double x1, double y1, double x2, double y2,
                             double x3, double y3, double x4, double y4) {
    double num = (x1 * y2 - y1 * x2) * (x3 - x4) -
                 (x1 - x2) * (x3 * y4 - y3 * x4);
    double den = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
    return num / den;
}

// Returns y-value of point of intersectipn of
// two lines
double geoUtils::y_intersect(double x1, double y1, double x2, double y2,
                             double x3, double y3, double x4, double y4) {
    double num = (x1 * y2 - y1 * x2) * (y3 - y4) -
                 (y1 - y2) * (x3 * y4 - y3 * x4);
    double den = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
    return num / den;
}

// This functions clips all the edges w.r.t one clip
// edge of clipping area
void geoUtils::clip(std::vector <Vector2D> &poly_points, int &poly_size,
                    double x1, double y1, double x2, double y2) {
    double new_points[MAX_POINTS][2];
    int new_poly_size = 0;

    // (ix,iy),(kx,ky) are the co-ordinate values of
    // the points
    for (int i = 0; i < poly_size; i++) {
        // i and k form a line in polygon
        int k = (i + 1) % poly_size;
        double ix = poly_points[i].x, iy = poly_points[i].y;
        double kx = poly_points[k].x, ky = poly_points[k].y;

        // Calculating position of first point
        // w.r.t. clipper line
        double i_pos = (x2 - x1) * (iy - y1) - (y2 - y1) * (ix - x1);

        // Calculating position of second point
        // w.r.t. clipper line
        double k_pos = (x2 - x1) * (ky - y1) - (y2 - y1) * (kx - x1);

        // Case 1 : When both points are inside
        if (i_pos < 0 && k_pos < 0) {
            //Only second point is added
            new_points[new_poly_size][0] = kx;
            new_points[new_poly_size][1] = ky;
            new_poly_size++;
        }

            // Case 2: When only first point is outside
        else if (i_pos >= 0 && k_pos < 0) {
            // Point of intersection with edge
            // and the second point is added
            new_points[new_poly_size][0] = x_intersect(x1,
                                                       y1, x2, y2, ix, iy, kx, ky);
            new_points[new_poly_size][1] = y_intersect(x1,
                                                       y1, x2, y2, ix, iy, kx, ky);
            new_poly_size++;

            new_points[new_poly_size][0] = kx;
            new_points[new_poly_size][1] = ky;
            new_poly_size++;
        }

            // Case 3: When only second point is outside
        else if (i_pos < 0 && k_pos >= 0) {
            //Only point of intersection with edge is added
            new_points[new_poly_size][0] = x_intersect(x1,
                                                       y1, x2, y2, ix, iy, kx, ky);
            new_points[new_poly_size][1] = y_intersect(x1,
                                                       y1, x2, y2, ix, iy, kx, ky);
            new_poly_size++;
        }

            // Case 4: When both points are outside
        else {
            //No points are added
        }
    }

    // Copying new points into original array
    // and changing the no. of vertices
    poly_points.clear();
    poly_size = new_poly_size;
    for (int i = 0; i < poly_size; i++) {
        poly_points.push_back(Vector2D(
                new_points[i][0],
                new_points[i][1]));
    }


}

// Implements Sutherland–Hodgman algorithm
Polygon2D geoUtils::suthHodgClip(Polygon2D &poly, Polygon2D &clipper) {
    //i and k are two consecutive indexes
    //TODO not true
    polygonOrientation(poly);
    polygonOrientation(clipper);


    std::vector <Vector2D> poly_points = poly.vertices();
    std::vector <Vector2D> clipper_points = clipper.vertices();
    int poly_size = poly_points.size();
    int clipper_size = clipper_points.size();

    for (int i = 0; i < clipper_size; i++) {
        int k = (i + 1) % clipper_size;

        // We pass the current array of vertices, it's size
        // and the end points of the selected clipper line
        clip(poly_points, poly_size, clipper_points[i].x,
             clipper_points[i].y, clipper_points[k].x,
             clipper_points[k].y);

    }

    return Polygon2D(poly_points);
}


//Driver code
void geoUtils::test() {
    // Defining polygon vertices in clockwise order
//    int poly_size = 3;
//    int poly_points[20][2] = {{100,150}, {200,250},
//                              {300,200}};

    std::vector <Vector2D> poly_points;
    poly_points.push_back(Vector2D(0, 0));
    poly_points.push_back(Vector2D(10, 0));
    poly_points.push_back(Vector2D(10, 5));
    poly_points.push_back(Vector2D(20, 5));
    poly_points.push_back(Vector2D(20, 20));
    poly_points.push_back(Vector2D(5, 20));
    poly_points.push_back(Vector2D(5, 10));
    poly_points.push_back(Vector2D(0, 10));

    Polygon2D poly(poly_points);

    // Defining clipper polygon vertices in clockwise order
    // 1st Example with square clipper
//    int clipper_size = 4;
//    int clipper_points[][2] = {{150,150}, {150,200},
//                               {200,200}, {200,150} };


    std::vector <Vector2D> clipper_points;
    clipper_points.push_back(Vector2D(5, 5));
    clipper_points.push_back(Vector2D(5, 15));
    clipper_points.push_back(Vector2D(15, 15));
    clipper_points.push_back(Vector2D(15, 5));


    Polygon2D cilpper(clipper_points);

    // 2nd Example with triangle clipper
    /*int clipper_size = 3;
    int clipper_points[][2] = {{100,300}, {300,300},
                                {200,100}};*/


    drawPolygon(poly, "ffffff");
    drawPolygon(cilpper, "0000ff");
    //Calling the clipping function
    poly = suthHodgClip(poly, cilpper);
    drawPolygon(poly, "#000000");

//    drawPolygon(poly, "00ff00");

}


void geoUtils::drawPolygon(Polygon2D poly, const char *color) {
    int i = 0;
    std::vector <Vector2D> temp_vec = poly.vertices();
    for (i = 0; i < temp_vec.size(); i++) {
        rcsc::dlog.addLine(Logger::TEAM,
                           temp_vec[i], temp_vec[(i + 1) % temp_vec.size()],
                           color);
    }
}


#include <bits/stdc++.h>


// A global point needed for sorting points with reference
// to the first point. Used in compare function of qsort()
Vector2D p0;
int cwo;

// A utility function to swap two points
void geoUtils::swap(Vector2D &p1, Vector2D &p2) {
    Vector2D temp = p1;
    p1 = p2;
    p2 = temp;
}

// A utility function to return square of distance between
// p1 and p2
double geoUtils::dist(Vector2D p1, Vector2D p2) {
    return (p1.x - p2.x) * (p1.x - p2.x) +
           (p1.y - p2.y) * (p1.y - p2.y);
}

// To find orientation of ordered triplet (p, q, r).
// The function returns following values
// 0 --> p, q and r are colinear
// 1 --> Clockwise
// 2 --> Counterclockwise
int geoUtils::orientation(Vector2D p, Vector2D q, Vector2D r) {
    double val = (q.y - p.y) * (r.x - q.x) -
                 (q.x - p.x) * (r.y - q.y);

    if (val == 0) return 0; // colinear
    return (val > 0) ? 1 : 2; // clockwise or counterclock wise
}

// A function used by library function qsort() to sort
// an array of points with respect to the first point
int geoUtils::compareSlopeP0(const void *vp1, const void *vp2) {
    Vector2D *p1 = (Vector2D *) vp1;
    Vector2D *p2 = (Vector2D *) vp2;

// Find orientation
    int o = orientation(p0, *p1, *p2);
    if (o == 0)
        return (dist(p0, *p2) >= dist(p0, *p1)) ? -1 : 1;

    return (o == cwo) ? -1 : 1;
}

// Prints simple closed path for a set of n points.
void geoUtils::orientationSort(std::vector <Vector2D> &points, bool clockwise) {
// Find the bottommost point
    int n = points.size();
    cwo = (clockwise == true) ? 1 : 2;
    int ymin = points[0].y, min = 0;
    for (int i = 1; i < n; i++) {
        int y = points[i].y;

        // Pick the bottom-most. In case of tie, chose the
        // left most point
        if ((y < ymin) || (ymin == y &&
                           points[i].x < points[min].x))
            ymin = points[i].y, min = i;
    }

// Place the bottom-most point at first position
    swap(points[0], points[min]);

// Sort n-1 points with respect to the first point.
// A point p1 comes before p2 in sorted ouput if p2
// has larger polar angle (in counterclockwise
// direction) than p1
    p0 = points[0];
    qsort(&points[1], n - 1, sizeof(Vector2D), compareSlopeP0);

}


void geoUtils::polygonOrientation(Polygon2D &poly) {
    std::vector <Vector2D> temp_point = poly.vertices();
    orientationSort(temp_point);
    poly.assign(temp_point);
}
