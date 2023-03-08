#ifndef MESH_H
#define MESH_H

#include <vector>
#include <exception>
#include <algorithm>
#include "Triangle.h"

namespace h7 {

//点邻接信息结构，每一个顶点对应一个这个结构
struct PointAttachmentInfo
{
public:
    PointAttachmentInfo();
    ~PointAttachmentInfo();
    void ClearVertexAdj();
    void ClearTriangleAdj();
    std::vector<int>* VertexAdjacencyList;//该顶点的相邻点索引
    std::vector<int>* TriangleAdjacencyList;//该顶点的相邻面索引
};
PointAttachmentInfo::PointAttachmentInfo()
{
    VertexAdjacencyList=NULL;
    TriangleAdjacencyList=NULL;
}
PointAttachmentInfo::~PointAttachmentInfo()
{
    ClearVertexAdj();
    ClearTriangleAdj();
}
void PointAttachmentInfo::ClearVertexAdj()
{
    if(VertexAdjacencyList!=NULL)
    {
        delete VertexAdjacencyList;
        VertexAdjacencyList=NULL;
    }
}
void PointAttachmentInfo::ClearTriangleAdj()
{
    if(TriangleAdjacencyList!=NULL)
    {
        delete TriangleAdjacencyList;
        VertexAdjacencyList=NULL;
    }
}
//带各个顶点邻接信息的Mesh结构
class Mesh
{
public:
    std::vector<Point3d> Vertices;//顶点列表
    std::vector<Triangle> Faces;//面列表
    std::vector<PointAttachmentInfo> AdjInfos;//顶点邻接信息表，长度等于顶点列表
    bool GetIsPerVertexVertexInfoEnabled();
    bool GetIsPerVertexTriangleInfoEnabled();
    Mesh();
    ~Mesh();
    long AddVertex(Point3d& toAdd);
    long AddFace(Triangle& tri);
    void InitPerVertexVertexAdj();//初始化点点邻接关系
    void InitPerVertexTriangleAdj();//初始化点面邻接关系
    void ClearPerVertexVertexAdj();
    void ClearPerVertexTriangleAdj();
private:
    bool IsPerVertexVertexInfoEnabled;
    bool IsPerVertexTriangleInfoEnabled;
};
Mesh::Mesh()
{
    IsPerVertexTriangleInfoEnabled=false;
    IsPerVertexVertexInfoEnabled=false;
}
Mesh::~Mesh()
{
    if(IsPerVertexTriangleInfoEnabled)
        ClearPerVertexTriangleAdj();
    if(IsPerVertexVertexInfoEnabled)
        ClearPerVertexVertexAdj();
}
long Mesh::AddVertex(Point3d& toAdd)
{
    long index = (long)Vertices.size();
    Vertices.push_back(toAdd);
    return index;
}
long  Mesh::AddFace(Triangle& tri)
{
    long  index = (long)Faces.size();
    Faces.push_back(tri);
    return index;
}
void Mesh::InitPerVertexVertexAdj()
{
    if(IsPerVertexVertexInfoEnabled)
        ClearPerVertexVertexAdj();
    IsPerVertexVertexInfoEnabled = true;
    if(AdjInfos.size()!=Vertices.size())
        AdjInfos.resize(Vertices.size());
    size_t vcount=Vertices.size();
    size_t fcount= Faces.size();
    for (size_t i = 0; i < vcount; i++)
    {
        std::vector<int>* vertexAdjacencyList= new std::vector<int>();
        if(vertexAdjacencyList==NULL){
            return;
        }
        vertexAdjacencyList->reserve(6);
        AdjInfos[i].VertexAdjacencyList=vertexAdjacencyList;
    }
    for (size_t i = 0; i < fcount; i++)
    {
        Triangle &t = Faces[i];
        std::vector<int> *p0list= AdjInfos[t.P0Index].VertexAdjacencyList;
        std::vector<int> *p1list= AdjInfos[t.P1Index].VertexAdjacencyList;
        std::vector<int> *p2list= AdjInfos[t.P2Index].VertexAdjacencyList;

        if (std::find(p0list->begin(), p0list->end(), t.P1Index)==p0list->end())
            p0list->push_back(t.P1Index);
        if (std::find(p0list->begin(), p0list->end(), t.P2Index)==p0list->end())
            p0list->push_back(t.P2Index);

        if (std::find(p1list->begin(), p1list->end(), t.P0Index)==p1list->end())
            p1list->push_back(t.P0Index);
        if (std::find(p1list->begin(), p1list->end(), t.P2Index)==p1list->end())
            p1list->push_back(t.P2Index);

        if (std::find(p2list->begin(), p2list->end(), t.P0Index)==p2list->end())
            p2list->push_back(t.P0Index);
        if (std::find(p2list->begin(), p2list->end(), t.P1Index)==p2list->end())
            p2list->push_back(t.P1Index);
    }
}
void Mesh::InitPerVertexTriangleAdj()
{
    if(IsPerVertexTriangleInfoEnabled)
        ClearPerVertexTriangleAdj();
    IsPerVertexTriangleInfoEnabled = true;
    if(AdjInfos.size()!=Vertices.size())
        AdjInfos.resize(Vertices.size());
    for (size_t i = 0; i < Vertices.size(); i++)
    {
        std::vector<int>* triangleAdjacencyList = new std::vector<int>();
        if(triangleAdjacencyList==NULL){
            return;
        }
        triangleAdjacencyList->reserve(6);
        AdjInfos[i].TriangleAdjacencyList=triangleAdjacencyList;
    }
    for (size_t i = 0; i < Faces.size(); i++)
    {
        Triangle& t = Faces[i];
        std::vector<int> *t0list= AdjInfos[t.P0Index].TriangleAdjacencyList;
        std::vector<int> *t1list= AdjInfos[t.P1Index].TriangleAdjacencyList;
        std::vector<int> *t2list= AdjInfos[t.P2Index].TriangleAdjacencyList;
        t0list->push_back(i);
        t1list->push_back(i);
        t2list->push_back(i);
    }
}
void Mesh::ClearPerVertexVertexAdj()
{
    if(!IsPerVertexVertexInfoEnabled)
        return;
    for (size_t i = 0; i < Vertices.size(); i++)
    {
        if (AdjInfos[i].VertexAdjacencyList != NULL)
        {
            delete AdjInfos[i].VertexAdjacencyList;
            AdjInfos[i].VertexAdjacencyList=NULL;
        }
    }
    IsPerVertexVertexInfoEnabled = false;
}
void Mesh::ClearPerVertexTriangleAdj()
{
    if(!IsPerVertexTriangleInfoEnabled)
        return;
    for (size_t i = 0; i < Vertices.size(); i++)
    {
        if (AdjInfos[i].TriangleAdjacencyList != NULL)
        {
            delete AdjInfos[i].TriangleAdjacencyList;
            AdjInfos[i].TriangleAdjacencyList=NULL;
        }
    }
    IsPerVertexTriangleInfoEnabled = false;
}
bool Mesh::GetIsPerVertexVertexInfoEnabled()
{
    return IsPerVertexVertexInfoEnabled;
}
bool Mesh::GetIsPerVertexTriangleInfoEnabled()
{
    return IsPerVertexTriangleInfoEnabled;
}

}

#endif // MESH_H
