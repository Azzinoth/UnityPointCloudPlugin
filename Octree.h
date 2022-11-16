#pragma once

//#define FULL_LOGGING
//#define MAIN_EVENTS_LOGGING

#include <vector>
#define GLM_FORCE_XYZW_ONLY
#include "glm/gtc/type_ptr.hpp"
#include "glm/mat4x4.hpp"
#include "glm/vec3.hpp"

#include <d3d11.h>

#define DELETED_POINTS_COORDINATE -200000.0f

struct VertexData
{
    glm::vec3 position;
    byte color[4];
    byte classification;
};

struct octreeSearchInfo
{
    glm::vec3 position;
    byte color[4];

    int hadIndex = -1;

    octreeSearchInfo(VertexData vertex, int index)
    {
        position = vertex.position;

        color[0] = vertex.color[0];
        color[1] = vertex.color[1];
        color[2] = vertex.color[2];
        color[3] = vertex.color[3];

        hadIndex = index;
    }
};

static int deletedCount = 0;
static const int capacity = 500;
static float sqrtTwo = sqrt(2.0f);
static int count = 0;
static int debugMaxNodeDepth = 0;
static int debugNodeCount = 0;
static int debugPointsInserted = 0;

static int isAtleastOnePointInSphereCounter = 0;
static int isAtleastOnePointInSphereChecksCounter = 0;
static bool isAtleastOnePointInSphereResult = false;

class AABB
{
public:
    glm::vec3 min = glm::vec3(0.0f);
    glm::vec3 max = glm::vec3(0.0f);

    float size = 0.0;

    AABB() {}

    AABB(glm::vec3 center, float Size)
    {
        float halfSize = Size / 2.0f;

        min[0] = center[0] - halfSize;
        min[1] = center[1] - halfSize;
        min[2] = center[2] - halfSize;

        max[0] = center[0] + halfSize;
        max[1] = center[1] + halfSize;
        max[2] = center[2] + halfSize;

        size = abs(max.x - min.x);
        if (abs(max.y - min.y) > size)
            size = abs(max.y - min.y);

        if (abs(max.z - min.z) > size)
            size = abs(max.z - min.z);
    }
};

class debugNodeBox
{
public:
    glm::vec3 center;
    float size;
    int depth;

    debugNodeBox(glm::vec3 center, float size, int depth)
    {
        this->center = center;
        this->size = size;
        this->depth = depth;
    }
};

static std::vector<debugNodeBox> nodesPositionAndSize;

class octree;
class octreeNode
{
public:
    glm::vec3 center = glm::vec3(0.0f, 0.0f, 0.0f);
    float size = 0.0f;
    float diagonal = 0.0f;
    octreeNode* parent;
    octreeNode** childs;
    std::vector<octreeSearchInfo> objects;
    int depth = 0;
    AABB nodeAABB;

    octreeNode();
    octreeNode(float Size, glm::vec3 Center, int Depth);
    octreeNode(const octreeNode& ref);

    ~octreeNode();

    void initChilds();

    bool addObject(octreeSearchInfo info);
    octreeNode* getNodeContaining(glm::vec3 point);

    void searchForObjects(glm::vec3& Center, float Radius, std::vector<int>& foundObjects);
    void addAllObjects(octreeNode* node, std::vector<int>& foundObjects);

    void debugGetNodesPositionAndSize(std::vector<debugNodeBox>& result);
    void isAtleastOnePointInSphere(glm::vec3& Center, float Radius);
};

class octree
{
    friend octreeNode;
public:
    octreeNode* root;
    std::vector<int> pointsToDelete;

    octree();
    octree(float worldSize, glm::vec3 worldCenter);
    octree(const octree& ref);
    ~octree();

    void initialize(float worldSize, glm::vec3 worldCenter);

    bool addObject(VertexData vertex, int index);
    void deleteObjects(glm::vec3& Center, float Radius);
    void searchForObjects(glm::vec3& Center, float Radius, std::vector<int>& foundObjects);

    std::vector<debugNodeBox>& debugGetNodesPositionAndSize();

    int getDebugNodeCount();
    int getDebugMaxNodeDepth();
    int getPointsInserted();

    bool isInOctreeBound(glm::vec3& Center, float Radius);
    bool isAtleastOnePointInSphere(glm::vec3& Center, float Radius);

    glm::vec3* closestPointsInNode(octreeNode* node, glm::vec3& referencePoint, int& neighborsInRangeCount, float discardDistance, int minNeighborsInRange);
    glm::vec3* closestPointInNode(octreeNode* node, glm::vec3& referencePoint, float discardDistance);

    float closestPointDistance(glm::vec3& referencePoint, float discardDistance, int minNeighborsInRange);
    float closestPointDistance(glm::vec3& referencePoint, float discardDistance);
};