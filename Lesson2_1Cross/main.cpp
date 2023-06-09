#include <iostream>
#include <vector>
#include <cmath>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);
Model *model = NULL;
const int width  = 800;
const int height = 800;

// 利用叉乘判断点是否在三角形内部
Vec3i crossProduct(Vec2i *pts, Vec2i P) {
    // 构建出三角形 ABC 三条边的向量
    Vec2i AB(pts[1].x - pts[0].x, pts[1].y - pts[0].y);
    Vec2i BC(pts[2].x - pts[1].x, pts[2].y - pts[1].y);
    Vec2i CA(pts[0].x - pts[2].x, pts[0].y - pts[2].y);
    
    // 三角形三个顶点和 P 链接形成的向量
    Vec2i AP(P.x - pts[0].x, P.y - pts[0].y);
    Vec2i BP(P.x - pts[1].x, P.y - pts[1].y);
    Vec2i CP(P.x - pts[2].x, P.y - pts[2].y);
    
    return Vec3i(AB^AP, BC^BP, CA^CP);
}

// 利用重心坐标判断点是否在三角形内部
Vec3f barycentric(Vec2i *pts, Vec2i P) {
    Vec3i x(pts[1].x - pts[0].x, pts[2].x - pts[0].x, pts[0].x - P.x);
    Vec3i y(pts[1].y - pts[0].y, pts[2].y - pts[0].y, pts[0].y - P.y);
    
    // u 向量和 x y 向量的点积为 0，所以 x y 向量叉乘可以得到 u 向量
    Vec3i u = x^y;
    
    // 由于 A, B, C, P 的坐标都是 int 类型，所以 u.z 必定是 int 类型，取值范围为 ..., -2, -1, 0, 1, 2, ...
    // 所以 u.z 绝对值小于 1 意味着三角形退化了，需要舍弃
    if(std::abs(u.z) < 1) {
        return Vec3f(-1, 1, 1);
    }
    return Vec3f(1.f- (u.x+u.y) / (float)u.z, u.x / (float)u.z, u.y / (float)u.z);
}

// 自己实现的三角形光栅化函数
// 主要思路是遍历 Box 中的每一个像素，和三角形的三个点做叉乘，如果叉乘均为正，着色；有一个负数，不着色
void triangle(Vec2i *pts, TGAImage &image, TGAColor color) {
    // 步骤 1: 找出包围盒
    Vec2i boxmin(image.get_width() - 1, image.get_height() - 1);
    Vec2i boxmax(0, 0);
    Vec2i clamp(image.get_width() - 1, image.get_height() - 1); // 图片的边界
    // 查找包围盒边界
    for (int i = 0; i < 3; i++) {
        // 第一层循环，遍历三角形的三个顶点
        for (int j = 0; j < 2; j++) {
            // 第二层循环，根据顶点数值缩小包围盒的范围
            boxmin.x = std::max(0,        std::min(boxmin.x, pts[i].x));
            boxmin.y = std::max(0,        std::min(boxmin.y, pts[i].y));
            
            boxmax.x = std::min(clamp.x, std::max(boxmax.x, pts[i].x));
            boxmax.y = std::min(clamp.y, std::max(boxmax.y, pts[i].y));
        }
    }
    
    std::cout << "boxmin: " << boxmin << "boxmax: "  << boxmax << std::endl;
    
    
    // 步骤 2: 包围盒内的每一个像素和三角形顶点连线做叉乘
    Vec2i P;
    for (P.x = boxmin.x; P.x <= boxmax.x; P.x++) {
        for (P.y = boxmin.y; P.y <= boxmax.y; P.y++) {
            
            // Vec3i bc_screen  = crossProduct(pts, P);
            Vec3f bc_screen  = barycentric(pts, P);
            
            // bc_screen 某个分量小于 0 则表示此点在三角形外（认为边也是三角形的一部分）
            if (bc_screen.x<0 || bc_screen.y<0 || bc_screen.z<0) {
                continue;
            }
            
            image.set(P.x, P.y, color);
        }
    }
}

void drawSingleTriangle() {
    TGAImage frame(200, 200, TGAImage::RGB);
    Vec2i pts[3] = {Vec2i(10, 10), Vec2i(150, 30), Vec2i(70, 160)};
    
    triangle(pts, frame, red);
    
    frame.flip_vertically();
    frame.write_tga_file("triangle.tga");
}
/*
void drawModelTriangle() {
    TGAImage frame(width, height, TGAImage::RGB);
    
    // 遍历模型里的每个三角形，然后随机着色
//    for (int i = 0; i < model->nfaces(); i++) {
//        std::vector<int> face = model->face(i);
//        Vec2i screen_coords[3];
//        for (int j = 0; j < 3; j++) {
//            Vec3f world_coords = model->vert(face[j]);
//            screen_coords[j] = Vec2i((world_coords.x + 1.) * width / 2., (world_coords.y + 1.) * height / 2.);
//        }
//        triangle(screen_coords, frame, TGAColor(rand() % 255, rand() % 255, rand() % 255, 255));
//    }
    
    // 这个是用一个模拟光照对三角形进行着色
    Vec3f light_dir(0, 0, -1); // 假设光是垂直屏幕的
    for (int i = 0; i < model->nfaces(); i++) {
        std::vector<int> face = model->face(i);
        Vec2i screen_coords[3];
        Vec3f world_coords[3];

        // 计算世界坐标和屏幕坐标
        for (int j = 0; j < 3; j++) {
            Vec3f v = model->vert(face[j]);
            // 投影为正交投影，而且只做了个简单的视口变换
            screen_coords[j] = Vec2i((v.x + 1.) * width / 2., (v.y + 1.) * height / 2.);
            world_coords[j]  = v;
        }

        // 计算世界坐标中某个三角形的法线（法线 = 三角形任意两条边做叉乘）
        Vec3f n = (world_coords[2] - world_coords[0]) ^ (world_coords[1] - world_coords[0]);
        n.normalize(); // 对 n 做归一化处理

        // 三角形法线和光照方向做点乘，点乘值大于 0，说明法线方向和光照方向在同一侧
        // 值越大，说明越多的光照射到三角形上，颜色越白
        float intensity = n * light_dir;
        if (intensity > 0) {
            triangle(screen_coords, frame, TGAColor(intensity * 255, intensity * 255, intensity * 255, 255));
        }
    }
    
    frame.flip_vertically();
    frame.write_tga_file("output/day03_light_model.tga");
    
    delete model;
}
*/

int main(int argc, char** argv) {
    
    drawSingleTriangle();
    //drawModelTriangle();

    return 0;
}
