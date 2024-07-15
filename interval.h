#ifndef INTERVAL_H 
#define INTERVAL_H

// #include "rtweekend.h"  //may cause circular dependency placed here jsut for error by vscode syntax highlighter

class interval {
  public:
    double min, max;

    interval() : min(+infinity), max(-infinity) {} // Default interval is empty

    interval(double _min, double _max) : min(_min), max(_max) {}

    interval(const interval &a,const interval &b){
        max = a.max>=b.max?a.max:b.max;
        min = a.min<=b.min?a.min:b.min;
    }

    bool contains(double x) const {
        return min <= x && x <= max;
    }

    bool surrounds(double x) const {
        return min < x && x < max;
    }

    double clamp(double x) const {
        if (x < min) return min;
        if (x > max) return max;
        return x;
    }

    interval expand(double delta){
        auto padding = delta/2;
        return interval(min-padding,max+padding);
    }

    double size() const {
        return max - min;
    }

    static const interval empty, universe;
};

// const static interval empty(+infinity, -infinity);
// const static interval universe(-infinity, +infinity);

const  interval interval::empty  = interval(+infinity, -infinity);
const  interval interval::universe = interval(-infinity, +infinity);

#endif