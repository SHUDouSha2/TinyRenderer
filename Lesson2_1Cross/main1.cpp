#include <iostream>
#include<cmath>
#include<vector>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

const int width = 200; //图像的宽
const int height = 200; //图像的高

//利用叉乘判断是否在三角形内部
//pts是三角形的三个点，P是平面内的任意一点
Vec3i crossProduct(Vec2i *pts,Vec2i P)
{
    //构造出三角形ABC三条边的向量
    Vec2i AB(pts[1].x - pts[0].x,pts[1].y - pts[0].y);
    Vec2i BC(pts[2].x - pts[1].x,pts[2].y - pts[1].y);
    Vec2i CA(pts[0].x - pts[2].x,pts[0].y - pts[2].y);
    
    //三角形三个顶点与P链接形成的向量
    Vec2i AP(P.x - pts[0].x,P.y - pts[0].y);
    Vec2i BP(P.x - pts[1].x,P.y - pts[1].y);
    Vec2i CP(P.x - pts[2].x,P.y - pts[2].y);

    //叉乘运算符的定义在geometry.h中
    //注意，官方版本的教学中，在geometry.h关于Vec2向量的运算中，是没有^叉乘这个运算符号的，需要手动加上这个运算符
    return Vec3i(AB^AP,BC^BP,CA^CP);
}

void drawSingleTriangle()
{
    TGAImage image(width,height,TGAImage::RGB);
    Vec2i pts[3] = {Vec2i(10,10),Vec2i(150,30),Vec2i(70,160)}; //手动指定三个点

    Vec2i boxmin(image.get_width() - 1 ,image.get_height() - 1); //包围矩形的左下角的点
    Vec2i boxmax(0,0); //包围矩形右上角的点
    Vec2i clamp(image.get_width() - 1 ,image.get_height() - 1);

    //求出包围三角形的最小的矩形
    for(int i = 0;i < 3;i++)
    {
        //以下代码理解起来略有难度，可以自己画图举例验证
        boxmin.x = std::max(0,std::min(boxmin.x,pts[i].x)); //max的作用是确保最小值不会小于0
        boxmin.y = std::max(0,std::min(boxmin.y,pts[i].y));

        boxmax.x = std::min(clamp.x,std::max(boxmax.x,pts[i].x)); //min的作用是确保最大值不会超出范围
        boxmax.y = std::min(clamp.y,std::max(boxmax.y,pts[i].y));
    }
    Vec2i P;

    //遍历矩形中所有像素
    for(P.x = boxmin.x;P.x <= boxmax.x;P.x++)
    {
        for(P.y = 0;P.y <= height - 1;P.y++)
            {
                Vec3i bc_screen = crossProduct(pts,P);
                if(bc_screen.x < 0 || bc_screen.y < 0 ||bc_screen .z < 0 )
                    continue;
                image.set(P.x,P.y,TGAColor(255,0,0,255));
            }
    }
    image.flip_vertically();
    image.write_tga_file("singleTriangle.tga");
}

int main()
{
    drawSingleTriangle();
}