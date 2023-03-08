#ifndef HCLAPLACIAN_HPP
#define HCLAPLACIAN_HPP

#include <math.h>
#include "Mesh.h"

namespace h7 {

class HCLaplacianSmoothing{

private:
    Mesh* mesh;

    float factor2;//TODO
    float factor1;
public:
    HCLaplacianSmoothing(Mesh* m)
    {
        this->mesh=m;
        m->InitPerVertexVertexAdj();
    }
    ~HCLaplacianSmoothing()
    {
        this->mesh=NULL;
    }
public:
    void HCLaplacian(int iterationTime,float lambda,float mu)
    {
        std::vector<Point3d> point_vector;
        std::vector<Point3d> startPoint;
        point_vector.resize(mesh->Vertices.size());
        startPoint.resize(mesh->Vertices.size());
        for(size_t i=0;i<mesh->Vertices.size();i++)
        {
            startPoint[i].X=mesh->Vertices[i].X;
            startPoint[i].Y=mesh->Vertices[i].Y;
            startPoint[i].Z=mesh->Vertices[i].Z;
        }
        for(int c=0;c<iterationTime;c++)
        {
            for(size_t i=0;i<mesh->Vertices.size();i++)
            {
                float dx = 0, dy = 0, dz = 0;
                std::vector<int>& adV=*(mesh->AdjInfos[i].VertexAdjacencyList);
                for (size_t j=0;j<adV.size();j++)
                {
                    Point3d& t=mesh->Vertices[adV[j]];
                    dx += t.X;
                    dy += t.Y;
                    dz += t.Z;
                }
                dx = dx / adV.size();
                dy = dy / adV.size();
                dz = dz / adV.size();
                point_vector[i].X=dx - (factor1 * startPoint[i].X + (1 - factor1) *mesh->Vertices[i].X) ;
                point_vector[i].Y=dy - (factor1 * startPoint[i].Y + (1 - factor1) *mesh->Vertices[i].Y) ;
                point_vector[i].Z=dz - (factor1 * startPoint[i].Z + (1 - factor1) *mesh->Vertices[i].Z) ;
                startPoint[i].X=dx;
                startPoint[i].Y=dy;
                startPoint[i].Z=dz;
            }
            for(size_t i=0;i<mesh->Vertices.size();i++)
            {
                mesh->Vertices[i].X=point_vector[i].X;
                mesh->Vertices[i].Y=point_vector[i].Y;
                mesh->Vertices[i].Z=point_vector[i].Z;
            }
            for(size_t i=0;i<mesh->Vertices.size();i++)
            {
                float dx = 0, dy = 0, dz = 0;
                std::vector<int>& adV=*(mesh->AdjInfos[i].VertexAdjacencyList);
                for (size_t j=0;j<adV.size();j++)
                {
                    Point3d& t=mesh->Vertices[adV[j]];
                    dx += t.X;
                    dy += t.Y;
                    dz += t.Z;
                }
                dx = (1.0f - factor2) * dx / adV.size();
                dy = (1.0f - factor2) * dy / adV.size();
                dz = (1.0f - factor2) * dz / adV.size();
                point_vector[i].X=startPoint[i].X - (factor2*mesh->Vertices[i].X +dx);
                point_vector[i].Y=startPoint[i].Y - (factor2*mesh->Vertices[i].Y +dy);
                point_vector[i].Z=startPoint[i].Z - (factor2*mesh->Vertices[i].Z +dz);
            }
            for(size_t i=0;i<mesh->Vertices.size();i++)
            {
                mesh->Vertices[i].X=point_vector[i].X;
                mesh->Vertices[i].Y=point_vector[i].Y;
                mesh->Vertices[i].Z=point_vector[i].Z;
            }
    }
    }
};

}

#endif // HCLAPLACIAN_HPP
