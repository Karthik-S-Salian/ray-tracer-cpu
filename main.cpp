#include <iostream>
#include "vec3.h"
#include "color.h"



int main(){

    int image_width  = 256,image_height = 256;

    std::cout<<"P3\n"<<image_width<<' '<<image_height<<"\n255\n";

    for(int j=0;j<image_height;++j){
        std::clog << "\rScanlines remaining: " << (image_height - j) << ' ' << std::flush;
        for(int i=0;i<image_width;++i){
            // auto r = double(i)/(image_width-1);
            // auto g = double(j)/(image_height-1);
            // auto b = 0;

            // int ir = static_cast<int>(255.9999*r);
            // int ig = static_cast<int>(255.9999*g);
            // int ib = static_cast<int>(255.9999*b);

            // std::cout<<ir<<' '<<ig<<' '<<ib<<'\n';

            auto pixel_color =  color(double(i)/(image_width-1),double(j)/(image_height-1),0);
            write_color(std::cout,pixel_color);
        }
    }

    std::clog << "\rDone              .\n";
}