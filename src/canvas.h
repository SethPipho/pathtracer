#ifndef CANVAS_H
#define CANVAS_H

#include <fstream>

class Pixel {
    public:
        int r,g,b;
        Pixel(){r = 0; b = 0; g = 0;}
        Pixel(int _r, int _g, int _b) {r = _r; b = _b; g = _g;}

};


class Canvas {
    private:
        Pixel* pixels;
    public:
        int width, height;
        Canvas(int w, int h) {
            width = w;
            height = h;
            pixels = new Pixel[w * h];
        }

        void setPixel(int x, int y,int r,int g, int b){
            int index = y * width + x;
            pixels[index].r = r;
            pixels[index].g = g;
            pixels[index].b = b; 
        }

        void savePPM(std::string path){
            std::ofstream f;
            f.open(path);

            f << "P3 \n";
            f << width << " " << height << "\n";
            f << 255 << "\n";
            for (int y = 0; y < height; y++){
                for (int x = 0; x < width; x++){
                    Pixel p = pixels[y * width + x];
                    f << p.r << " " << p.g << " " << p.b << "\n";
                }
            
            }

            f.close();
        }
};



#endif