#ifndef SMOOTHING_H
#define SMOOTHING_H

#include "Triangle.h"
#include "Mesh.h"

namespace h7 {

//https://blog.csdn.net/chenweiyu11962/article/details/112210226
class Smoothing
{
private:
    Mesh* mesh;
public:
    Smoothing(Mesh* m)
    {
        this->mesh = m;
        //m->InitPerVertexVertexAdj();
    }
    ~Smoothing()
    {
        this->mesh = NULL;
    }
    void Laplacian(bool x, bool y, bool z);
    void CurvatureFlow(int iterationTimes);
    Point3d GetSmoothedVertex_CurvatureFlow(size_t index);

    float GetCos(Point3d& ps,Point3d& pe1,Point3d& pe2);
    Point3d GetSmoothedVertex_Laplacian(size_t index, bool x, bool y, bool z);
    float GetWeight(size_t index,int adjindex, std::vector<int>& adjFaces);
    float GetCot(Triangle& t,int index);

    //lambda: 0~1, mu: -1~0
    void Taubin(int iterationTime,float lambda,float mu);
    Point3d GetSmoothedVertex_Taubin_Step(size_t index,float lambda);
};


}

#endif // SMOOTHING_H
