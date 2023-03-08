#include "Smoothing.h"
#include <math.h>

using namespace h7;

void Smoothing::Laplacian(bool x, bool y, bool z)
{
    Point3d* tempList = new Point3d[mesh->Vertices.size()];
    for(size_t i=0;i<mesh->Vertices.size();i++)
    {
        tempList[i]=GetSmoothedVertex_Laplacian(i, x, y, z);
    }
    for(size_t i=0;i<mesh->Vertices.size();i++)
    {
        mesh->Vertices[i]=tempList[i];
    }
    delete[] tempList;
}

Point3d Smoothing::GetSmoothedVertex_Laplacian(size_t index, bool x, bool y, bool z)
{
    float nx=0,ny=0,nz=0;
    std::vector<int>& adjVertices = *(this->mesh->AdjInfos[index].VertexAdjacencyList);
    if(adjVertices.size()==0)
        return mesh->Vertices[index];
    //Point3d& P=mesh->Vertices[index];
    for(size_t i=0;i<adjVertices.size();i++)
    {
        nx+=mesh->Vertices[adjVertices[i]].GetX();
        ny+=mesh->Vertices[adjVertices[i]].GetY();
        nz+=mesh->Vertices[adjVertices[i]].GetZ();
    }
    if(x) {
        nx/=adjVertices.size();
    }
    else {
        nx = mesh->Vertices[index].GetX();
    }
    if(y) {
        ny/=adjVertices.size();
    }
    else {
        ny = mesh->Vertices[index].GetY();
    }
    if(z) {
        nz/=adjVertices.size();
    }
    else {
        nz = mesh->Vertices[index].GetZ ();
    }
    return Point3d(nx,ny,nz);
}

void Smoothing::CurvatureFlow(int iterationTimes) {
    if(!mesh->GetIsPerVertexTriangleInfoEnabled())
        mesh->InitPerVertexTriangleAdj();
    Point3d* tempList=new Point3d[mesh->Vertices.size()];
    for(int c=0;c<iterationTimes;c++)
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

Point3d Smoothing::GetSmoothedVertex_CurvatureFlow(size_t index) {
    float dx=0,dy=0,dz=0;
    std::vector<int>& adjVertices=*this->mesh->AdjInfos[index].VertexAdjacencyList;
    //std::vector<int>& adjFaces=this->mesh->AdjInfos[index].TriangleAdjacencyList;
    Point3d& p=mesh->Vertices[index];
    if(adjVertices.size()==0
        //||adjVertices.size()!=adjFaces.size()
        )
        return mesh->Vertices[index];
    float sumweight=0;
    for(size_t i=0;i<adjVertices.size();i++)
    {
        Point3d& t=mesh->Vertices[adjVertices[i]];
        //float cotWeight=GetWeight(index,adjVertices[i],adjFaces);
        float cotWeight = 1.0f / adjVertices.size();
        dx+=cotWeight*(t.GetX()-p.GetX());
        dy+=cotWeight*(t.GetY()-p.GetY());
        //dz+=cotWeight*(t.GetZ()-p.GetZ());
        sumweight+=cotWeight;
    }
    dx/=sumweight;
    dy/=sumweight;
    //dz/=sumweight;
    float newx=dx+p.GetX();
    float newy=dy+p.GetY();
    float newz=dz+p.GetZ();
    return Point3d(newx,newy,newz);
}

float Smoothing::GetWeight(size_t index,int adjindex, std::vector<int>& adjFaces)
{
    float w=0;
    int count=0;
    for(size_t i=0;i<adjFaces.size();i++)
    {
        Triangle& t=mesh->Faces[adjFaces[i]];
        if(t.HasVertex(adjindex))
        {
            int otherIndex=t.GetOtherIndex(index,adjindex);
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
float Smoothing::GetCot(Triangle& t,int index)
{
    std::vector<Point3d>& v=mesh->Vertices;
    float cos = 0.0f;
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
float Smoothing::GetCos(Point3d& ps,Point3d& pe1,Point3d& pe2)
{
    Point3d pse1(pe1.GetX()-ps.GetX(),pe1.GetY()-ps.GetY(),pe1.GetZ()-ps.GetZ());
    Point3d pse2(pe2.GetX()-ps.GetX(),pe2.GetY()-ps.GetY(),pe2.GetZ()-ps.GetZ());
    float mo1=sqrt(pse1.GetX()*pse1.GetX()+pse1.GetY()*pse1.GetY()+pse1.GetZ()*pse1.GetZ());
    float mo2=sqrt(pse2.GetX()*pse2.GetX()+pse2.GetY()*pse2.GetY()+pse2.GetZ()*pse2.GetZ());
    float mul=pse1.GetX()*pse2.GetX()+pse1.GetY()*pse2.GetY()+pse1.GetZ()*pse2.GetZ();
    return mul/(mo1*mo2);
}

/**
        关于lambda mu值的几个注意事项lambda是λ ，mu是μ
        Taubin平滑的原理基于了一部分数字信号处理方面的知识，用简单的话来表述，
就是使用一个负收缩因子μ将拉普拉斯平滑照成的收缩再放大回去( 防止过分的出现变形失真)，
 其算法的主体采用了2个与拉普拉斯平滑算法相似的过程，一个过程采用正因子λ（0~1），
 另一个过程采用负因子μ（-1~0），每一次迭代都必须重复这两个过程
        我们假设0<λ，并且μ是一个负比例因子，使得μ<-λ。
        保持μ+λ<0（例如绝对值mu更大）
        让kpb为通带频率，taubin说：
        kpb=1/λ+1/μ>0
        kpb值在0.01～0.1之间，与原文献相比，结果良好。
        kpb*μ-μ/λ=1
        μ=1/（kpb-1/λ）
        所以如果
        *λ==0.5，kpb==0.1->mu=1/（0.1-2）=-0.526
        *λ==0.5，kpb==0.01->mu=1/（0.01-2）=-0.502
即有：
 */
void Smoothing::Taubin(int iterationTime,float lambda,float mu)
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

Point3d Smoothing::GetSmoothedVertex_Taubin_Step(size_t index,float lambda)
{
    float dx=0,dy=0,dz=0;
    std::vector<int>& adjVertices = *this->mesh->AdjInfos[index].VertexAdjacencyList;
    Point3d& p = mesh->Vertices[index];
    if(adjVertices.size()==0)
        return mesh->Vertices[index];
    for(size_t i=0;i<adjVertices.size();i++)
    {
        Point3d& t=mesh->Vertices[adjVertices[i]];
        dx+=(t.GetX()-p.GetX());
        dy+=(t.GetY()-p.GetY());
        //dz+=(t.GetZ()-p.GetZ());
    }
    dx/=adjVertices.size();
    dy/=adjVertices.size();
    dz/=adjVertices.size();
    float newx=lambda*dx+p.GetX();
    float newy=lambda*dy+p.GetY();
    float newz=lambda*dz+p.GetZ();
    return Point3d(newx,newy,newz);
}

