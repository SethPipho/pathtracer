#ifndef VEC_H
#define VEC_H

#include <iostream>
#include <math.h>

class vec3 {
    public:
        double x,y,z;
        vec3(){x =0; y = 0; z = 0;}
        vec3(double _x, double _y, double _z){x = _x; y = _y; z = _z;}

        inline vec3& operator+=(const vec3 &v);
        inline vec3& operator+=(const double s);
        inline vec3& operator/=(const double s);

        inline double length(){
            return sqrt(x * x + y * y + z * z);
        }


       

};


 inline vec3 operator+(const vec3 a, const vec3 b){
     return vec3(a.x + b.x, a.y + b.y, a.z + b.z);
 }



inline vec3 operator-(const vec3 a, const vec3 b){
     return vec3(a.x - b.x, a.y - b.y, a.z - b.z);
 }

 inline vec3 operator*(const vec3 a, const vec3 b){
     return vec3(a.x * b.x, a.y * b.y, a.z * b.z);
 }

 inline vec3 operator*(const vec3 v, const double c){
     return vec3(v.x * c, v.y * c, v.z * c);
 }

 inline vec3 operator/(const vec3 v, const double c){
     return vec3(v.x / c, v.y / c, v.z / c);
 }

 inline vec3 unit(vec3 v){
    return v / v.length();
}


 inline double dot(const vec3 a, const vec3 b){
     return a.x * b.x + a.y * b.y + a.z * b.z; 
 }

 inline std::ostream& operator<<(std::ostream &os, const vec3 &t){
     os <<" ("<< t.x << " " << t.y << " " << t.z << ") ";
     return os;
 }

 inline vec3& vec3::operator+=(const vec3 &v){
     x += v.x;
     y += v.y;
     z += v.z;
     return *this;
 }

  inline vec3& vec3::operator+=(const double s){
     x += s;
     y += s;
     z += s;
     return *this;
 }

  inline vec3& vec3::operator/=(const double s){
     x /= s;
     y /= s;
     z /= s;
     return *this;
 }



 #endif