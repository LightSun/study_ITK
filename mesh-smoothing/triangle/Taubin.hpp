#ifndef TAUBIN_HPP
#define TAUBIN_HPP

#include <math.h>
#include "Mesh.h"

namespace h7 {

class TaubinSmoothing
{
private:
    Mesh* mesh;
public:
    TaubinSmoothing(Mesh* m)
    {
        this->mesh=m;
        m->InitPerVertexVertexAdj();
    }
    ~TaubinSmoothing()
    {
        this->mesh=NULL;
    }
public:
    void Taubin(int iterationTime,float lambda,float mu)
    {
        Point3d* tempList=new Point3d[mesh->Vertices.size()];
        for(int c=0;c<iterationTime;c++)
        {
            for(size_t i=0;i<mesh->Vertices.size();i++)
            {
                tempList[i]=GetSmoothedVertex_Taubin_Step(i,lambda);
            }
            for(size_t i=0;i<mesh->Vertices.size();i++)
            {
                mesh->Vertices[i]=tempList[i];
            }
            for(size_t i=0;i<mesh->Vertices.size();i++)
            {
                tempList[i]=GetSmoothedVertex_Taubin_Step(i,mu);
            }
            for(size_t i=0;i<mesh->Vertices.size();i++)
            {
                mesh->Vertices[i]=tempList[i];
            }
        }
        delete[] tempList;
    }
private:
    Point3d GetSmoothedVertex_Taubin_Step(size_t index,float lambda)
    {
        float dx=0,dy=0,dz=0;
        std::vector<int>& adjVertices=*(this->mesh->AdjInfos[index].VertexAdjacencyList);
        Point3d& p=mesh->Vertices[index];
        if(adjVertices.size()==0)
            return mesh->Vertices[index];
        for(size_t i=0;i<adjVertices.size();i++)
        {
            Point3d& t=mesh->Vertices[adjVertices[i]];
            dx+=(t.X-p.X);
            dy+=(t.Y-p.Y);
            dz+=(t.Z-p.Z);
        }
        dx/=adjVertices.size();
        dy/=adjVertices.size();
        dz/=adjVertices.size();
        float newx=lambda*dx+p.X;
        float newy=lambda*dy+p.Y;
        float newz=lambda*dz+p.Z;
        return Point3d(newx,newy,newz);
    }
};
}

#endif // TAUBIN_HPP
