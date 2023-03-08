#ifndef CURVATUREFLOW_HPP
#define CURVATUREFLOW_HPP

#include <math.h>
#include "Mesh.h"

namespace h7 {
class CurvatureFlowSmoothing
{
private:
    Mesh* mesh;
public:
    CurvatureFlowSmoothing(Mesh* m)
    {
        this->mesh=m;
        m->InitPerVertexVertexAdj();
    }
    ~CurvatureFlowSmoothing()
    {
        this->mesh=NULL;
    }
public:
    void CurvatureFlow(int iterationTime)
    {
        if(!mesh->GetIsPerVertexTriangleInfoEnabled())
            mesh->InitPerVertexTriangleAdj();
        Point3d* tempList=new Point3d[mesh->Vertices.size()];
        for(int c=0;c<iterationTime;c++)
        {
            for(size_t i=0;i<mesh->Vertices.size();i++)
            {
                tempList[i]=GetSmoothedVertex_CurvatureFlow(i);
            }
            for(size_t i=0;i<mesh->Vertices.size();i++)
            {
                mesh->Vertices[i]=tempList[i];
            }
        }
        delete[] tempList;
    }
private:
    Point3d GetSmoothedVertex_CurvatureFlow(size_t index)
    {
        float dx=0,dy=0,dz=0;
        std::vector<int>& adjVertices=*(this->mesh->AdjInfos[index].VertexAdjacencyList);
        std::vector<int>& adjFaces=*(this->mesh->AdjInfos[index].TriangleAdjacencyList);
        Point3d& p=mesh->Vertices[index];
        if(adjVertices.size()==0||adjVertices.size()!=adjFaces.size())
            return mesh->Vertices[index];
        float sumweight=0;
        for(size_t i=0;i<adjVertices.size();i++)
        {
            Point3d& t=mesh->Vertices[adjVertices[i]];
            float cotWeight=GetWeight(index,adjVertices[i],adjVertices,adjFaces);
            dx+=cotWeight*(t.X-p.X);
            dy+=cotWeight*(t.Y-p.Y);
            dz+=cotWeight*(t.Z-p.Z);
            sumweight+=cotWeight;
        }
        dx/=sumweight;
        dy/=sumweight;
        dz/=sumweight;
        float newx=dx+p.X;
        float newy=dy+p.Y;
        float newz=dz+p.Z;
        return Point3d(newx,newy,newz);
    }
    float GetWeight(size_t index,int adjindex,std::vector<int>& adjVertices,
                    std::vector<int>& adjFaces)
    {
        float w=0;
        int count=0;
        for(size_t i=0;i<adjFaces.size();i++)
        {
            Triangle& t=mesh->Faces[adjFaces[i]];
            if(t.HasVertex(adjindex))
            {
                long otherIndex=t.GetOtherIndex(index,adjindex);
                float cot=GetCot(t,otherIndex);
                w+=cot;
                count++;
            }
        }
        if(count==0)
            return 0;
        w=w/count;
        return w;
    }
    float GetCot(Triangle& t,long index)
    {
        std::vector<Point3d>& v=mesh->Vertices;
        float cos;
        if(t.P0Index==index)
        {
            cos=GetCos(v[t.P0Index],v[t.P1Index],v[t.P2Index]);
        }else
            if(t.P1Index==index)
            {
                cos=GetCos(v[t.P1Index],v[t.P0Index],v[t.P2Index]);
            }else
            if(t.P2Index==index)
            {
                cos=GetCos(v[t.P2Index],v[t.P1Index],v[t.P0Index]);
            }
            return cos/sqrt(1-cos*cos);
    }
    float GetCos(Point3d& ps,Point3d& pe1,Point3d& pe2)
    {
        Point3d pse1(pe1.X-ps.X,pe1.Y-ps.Y,pe1.Z-ps.Z);
        Point3d pse2(pe2.X-ps.X,pe2.Y-ps.Y,pe2.Z-ps.Z);
        float mo1=sqrt(pse1.X*pse1.X+pse1.Y*pse1.Y+pse1.Z*pse1.Z);
        float mo2=sqrt(pse2.X*pse2.X+pse2.Y*pse2.Y+pse2.Z*pse2.Z);
        float mul=pse1.X*pse2.X+pse1.Y*pse2.Y+pse1.Z*pse2.Z;
        return mul/(mo1*mo2);
    }
};
}

#endif // CURVATUREFLOW_HPP
