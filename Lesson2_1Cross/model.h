#ifndef __MODEL_H__
#define __MODEL_H__
//标识的命名规则一般是头文件名全大写，前面加下划线，并把文件名中的"."也变成下划线，如：stdio.h
#include <vector>
#include "geometry.h"

class Model {
private:
	std::vector<Vec3f> verts_;
	std::vector<std::vector<int> > faces_;
public:
	Model(const char *filename);
	~Model();
	int nverts(); //返回顶点数量
	int nfaces(); //返回面数量
	Vec3f vert(int i); //通过序号 返回顶点
	std::vector<int> face(int idx); //返回一个面的三个顶点
};

#endif //__MODEL_H__
