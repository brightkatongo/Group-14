#include <iostream>
using namespace std;
const double pi = 3.142;
class cylinder {
     double radius;
     double hight;

     //constructor
     cylinder (double radius = 0, double height =0){
         this ->radius = radius;
         this ->height= height;
     }
 double caculateVolume(){
     return pi*radius*radius*height;
 }
 double caculateArea(){
   return 2*pi*radius*radius + 2*pi*radius*height;
 }


 int main(){

   cylinder cyl (8,9);
       cout <<"volume="<<cyl.caculateVolume();
return 0;

}
