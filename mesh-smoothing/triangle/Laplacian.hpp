#ifndef LAPLACIAN_HPP
#define LAPLACIAN_HPP

#include <math.h>
#include "Mesh.h"

namespace h7 {
class LaplacianSmoothing
{
private:
    Mesh* mesh;
public:
    LaplacianSmoothing(Mesh* m)
    {
        this->mesh=m;
        m->InitPerVertexVertexAdj();
    }
    ~LaplacianSmoothing()
    {
        this->mesh=NULL;
    }
public:
    void ScaleDependentLaplacian(int iterationTime,float lambda=1.0)
    {
        Point3d* tempList=new Point3d[mesh->Vertices.size()];
        for(int c=0;c<iterationTime;c++)
        {
            for(size_t i=0;i<mesh->Vertices.size();i++)
            {
                tempList[i]=GetSmoothedVertex_ScaleDependentLaplacian(i,lambda);
            }
            for(size_t i=0;i<mesh->Vertices.size();i++)
            {
                mesh->Vertices[i]=tempList[i];
            }
        }
        delete[] tempList;
    }
private:
    Point3d GetSmoothedVertex_ScaleDependentLaplacian(size_t index,float lambda=1.0f)
    {
        float dx=0,dy=0,dz=0;
        std::vector<int>& adjVertices=*(this->mesh->AdjInfos[index].VertexAdjacencyList);
        Point3d& p=mesh->Vertices[index];
        if(adjVertices.size()==0)
            return mesh->Vertices[index];
        float sumweight=0;
        for(size_t i=0;i<adjVertices.size();i++)
        {
            Point3d& t=mesh->Vertices[adjVertices[i]];
            float weight=GetDistence(p,t);
            dx+=weight*(t.X-p.X);
            dy+=weight*(t.Y-p.Y);
            dz+=weight*(t.Z-p.Z);
            sumweight+=weight;
        }
        dx/=sumweight;
        dy/=sumweight;
        dz/=sumweight;
        float newx=lambda*dx+p.X;
        float newy=lambda*dy+p.Y;
        float newz=lambda*dz+p.Z;
        return Point3d(newx,newy,newz);
    }
    float GetWeight(size_t index,size_t adjIndex,std::vector<long>& adjVertices)
    {
        Point3d& p=mesh->Vertices[index];
        Point3d& t=mesh->Vertices[adjVertices[adjIndex]];
        return 1.0f/(GetDistence(p,t));
    }
    float GetDistence(Point3d& p1,Point3d& p2)
    {
        return (float)sqrt((p1.X-p2.X)*(p1.X-p2.X)
                           +(p1.Y-p2.Y)*(p1.Y-p2.Y)
                           +(p1.Z-p2.Z)*(p1.Z-p2.Z));
    }
};
}

#endif // LAPLACIAN_HPP
