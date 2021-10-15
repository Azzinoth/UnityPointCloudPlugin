#pragma once

//#define FULL_LOGGING
//#define MAIN_EVENTS_LOGGING

#include <vector>
#define GLM_FORCE_XYZW_ONLY
#include "glm/gtc/type_ptr.hpp"
#include "glm/mat4x4.hpp"
#include "glm/vec3.hpp"

#include <d3d11.h>

struct MeshVertex
{
    glm::vec3 position;
    byte color[4];
};

struct octreeSearchInfo
{
    glm::vec3 position;
    byte color[4];

    int hadIndex = -1;

    octreeSearchInfo(MeshVertex vertex, int index)
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

    octreeNode()
    {
        depth = 0;
        size = 0.0f;
        center = glm::vec3(0.0f);
        diagonal = 0.0f;
        childs = nullptr;
        parent = nullptr;
    }

    octreeNode(float Size, glm::vec3 Center, int Depth)
    {
        if (debugMaxNodeDepth < Depth)
            debugMaxNodeDepth = Depth;
        depth = Depth;
        size = Size;
        center = Center;
        diagonal = sqrtTwo * size;
        childs = nullptr;
        parent = nullptr;
        nodeAABB = AABB(Center, Size);
    }

    octreeNode(const octreeNode& ref)
    {
        center = ref.center;
        size = ref.size;
        diagonal = ref.diagonal;

        parent = nullptr;
        //parent = ref.parent;
        if (ref.childs != nullptr)
        {
            for (size_t i = 0; i < 8; i++)
            {
                childs[i] = ref.childs[i];
            }
        }
        objects = ref.objects;
        depth = ref.depth;
        nodeAABB = ref.nodeAABB;
    }

    ~octreeNode()
    {
        if (childs != nullptr)
        {
            for (int i = 0; i < 8; i++)
            {
                delete childs[i];
                //debugLog::getInstance().addToLog("child " + std::to_string(i) + " was deleted", "OctreeMemory");
            }
        }
    }

    void initChilds()
    {
        debugNodeCount += 8;
#ifdef FULL_LOGGING
        //debugLog::getInstance().addToLog("8 nodes added. In total: " + std::to_string(debugNodeCount), "OctreeEvents");
#endif
        childs = new octreeNode * [8];

        // upper top left
        childs[0] = new octreeNode(size / 2.0f, center + glm::vec3(-size / 4.0f, size / 4.0f, -size / 4.0f), depth + 1);
        childs[0]->parent = this;
        // upper top right
        childs[1] = new octreeNode(size / 2.0f, center + glm::vec3(size / 4.0f, size / 4.0f, -size / 4.0f), depth + 1);
        childs[1]->parent = this;
        // upper bottom left
        childs[2] = new octreeNode(size / 2.0f, center + glm::vec3(-size / 4.0f, size / 4.0f, size / 4.0f), depth + 1);
        childs[2]->parent = this;
        // upper bottom right
        childs[3] = new octreeNode(size / 2.0f, center + glm::vec3(size / 4.0f, size / 4.0f, size / 4.0f), depth + 1);
        childs[3]->parent = this;

        // down top left
        childs[4] = new octreeNode(size / 2.0f, center + glm::vec3(-size / 4.0f, -size / 4.0f, -size / 4.0f), depth + 1);
        childs[4]->parent = this;
        // down top right
        childs[5] = new octreeNode(size / 2.0f, center + glm::vec3(size / 4.0f, -size / 4.0f, -size / 4.0f), depth + 1);
        childs[5]->parent = this;
        // down bottom left
        childs[6] = new octreeNode(size / 2.0f, center + glm::vec3(-size / 4.0f, -size / 4.0f, size / 4.0f), depth + 1);
        childs[6]->parent = this;
        // down bottom right
        childs[7] = new octreeNode(size / 2.0f, center + glm::vec3(size / 4.0f, -size / 4.0f, size / 4.0f), depth + 1);
        childs[7]->parent = this;
    }

    bool addObject(octreeSearchInfo info)
    {
#ifdef FULL_LOGGING
        if (info.position.x > 578.1 && info.position.x < 578.3 &&
            info.position.y > -830.4 && info.position.y < -830.32 &&
            info.position.z > 558.5 && info.position.z < 558.9)
        {
            debugLog::getInstance().addToLog("point to add: ", point, "OctreeEvents");
        }
#endif

        bool instersect = true;
        if (info.position.x < nodeAABB.min[0] - 0.01f || info.position.x > nodeAABB.max[0] + 0.01f) instersect = false;
        if (info.position.y < nodeAABB.min[1] - 0.01f || info.position.y > nodeAABB.max[1] + 0.01f) instersect = false;
        if (info.position.z < nodeAABB.min[2] - 0.01f || info.position.z > nodeAABB.max[2] + 0.01f) instersect = false;

#ifdef FULL_LOGGING
        if (info.position.x > 578.1 && info.position.x < 578.3 &&
            info.position.y > -830.4 && info.position.y < -830.32 &&
            info.position.z > 558.5 && info.position.z < 558.9)
        {
            debugLog::getInstance().addToLog("point was: " + instersect ? "accepted" : "rejected", "OctreeEvents");
            if (!instersect)
            {
                debugLog::getInstance().addToLog("point was: rejected", "OctreeEvents");
                debugLog::getInstance().addToLog("point:", info.position, "OctreeEvents");
                debugLog::getInstance().addToLog("nodeAABB.min:", nodeAABB.min, "OctreeEvents");
                debugLog::getInstance().addToLog("nodeAABB.max:", nodeAABB.max, "OctreeEvents");
            }
        }
#endif

        if (objects.size() < capacity && instersect)
        {
            objects.push_back(info);
            debugPointsInserted++;
            return true;
        }
        else if (instersect)
        {
            if (childs == nullptr)
                initChilds();

            for (int i = 0; i < 8; i++)
            {
                if (childs[i]->addObject(info))
                    return true;
            }
        }

        return false;
    }

    octreeNode* getNodeContaining(glm::vec3 point)
    {
        bool instersect = true;
        if (point.x < nodeAABB.min[0] - 0.01f || point.x > nodeAABB.max[0] + 0.01f) instersect = false;
        if (point.y < nodeAABB.min[1] - 0.01f || point.y > nodeAABB.max[1] + 0.01f) instersect = false;
        if (point.z < nodeAABB.min[2] - 0.01f || point.z > nodeAABB.max[2] + 0.01f) instersect = false;

        if (instersect)
        {
            for (size_t i = 0; i < objects.size(); i++)
            {
                if (objects[i].position.x == point.x &&
                    objects[i].position.y == point.y &&
                    objects[i].position.z == point.z)
                    return this;
            }

            if (childs != nullptr)
            {
                for (int i = 0; i < 8; i++)
                {
                    octreeNode* result = nullptr;
                    result = childs[i]->getNodeContaining(point);

                    if (result != nullptr)
                        return result;
                }
            }
        }

        return nullptr;
    }

    void searchForObjects(glm::vec3& Center, float Radius, std::vector<int>& foundObjects)
    {
        double currentDistance = distance(center, Center);

        if (currentDistance > Radius + diagonal)
        {
            return;
        }
        else if (Radius > currentDistance + diagonal)
        {
            addAllObjects(this, foundObjects);
            return;
        }

        for (int i = 0; i < objects.size(); i++)
        {
            currentDistance = distance(objects[i].position, Center);
            if (currentDistance < Radius)
            {
                //deletedCount++;
                foundObjects.push_back(objects[i].hadIndex);
            }
        }

        if (childs != nullptr)
        {
            for (int i = 0; i < 8; i++)
            {
                childs[i]->searchForObjects(Center, Radius, foundObjects);
            }
        }
    }

    void addAllObjects(octreeNode* node, std::vector<int>& foundObjects)
    {
        for (int i = 0; i < objects.size(); i++)
        {
            //deletedCount++;
            foundObjects.push_back(objects[i].hadIndex);
        }

        if (childs != nullptr)
        {
            for (int i = 0; i < 8; i++)
            {
                childs[i]->addAllObjects(this, foundObjects);
            }
        }
    }

    void debugGetNodesPositionAndSize(std::vector<debugNodeBox>& result)
    {
        result.push_back(debugNodeBox(this->center, this->size, this->depth));

        if (childs != nullptr)
        {
            for (int i = 0; i < 8; i++)
            {
                childs[i]->debugGetNodesPositionAndSize(result);
            }
        }
    }

    void isAtleastOnePointInSphere(glm::vec3& Center, float Radius)
    {
        isAtleastOnePointInSphereCounter++;

        if (isAtleastOnePointInSphereResult)
            return;

        double currentDistance = distance(center, Center);

        if (currentDistance > Radius + diagonal)
        {
        }
        else if (Radius > currentDistance + diagonal)
        {
            if (objects.size() > 0)
            {
                //debugLog::getInstance().addToLog("found on counter(if (objects.size() > 0)): " + std::to_string(isAtleastOnePointInSphereCounter), "isAtleastOnePointInSphere");
                //debugLog::getInstance().addToLog("time stamp: " + std::to_string(GetTickCount()), "isAtleastOnePointInSphere");
                //debugLog::getInstance().addToLog("node depth: " + std::to_string(this->depth), "isAtleastOnePointInSphere");
                //debugLog::getInstance().addToLog("bool isAtleastOnePointInSphereResult: " + std::to_string(isAtleastOnePointInSphereResult), "isAtleastOnePointInSphere");
                isAtleastOnePointInSphereResult = true;
                return;
            }
        }
        else
        {
            for (int i = 0; i < objects.size(); i++)
            {
                currentDistance = distance(objects[i].position, Center);
                isAtleastOnePointInSphereChecksCounter++;

                if (currentDistance < Radius)
                {
                    /*debugLog::getInstance().addToLog("found on counter(if (currentDistance < Radius)): " + std::to_string(isAtleastOnePointInSphereCounter), "isAtleastOnePointInSphere");
                    debugLog::getInstance().addToLog("time stamp: " + std::to_string(GetTickCount()), "isAtleastOnePointInSphere");
                    debugLog::getInstance().addToLog("node depth: " + std::to_string(this->depth), "isAtleastOnePointInSphere");
                    debugLog::getInstance().addToLog("bool isAtleastOnePointInSphereResult: " + std::to_string(isAtleastOnePointInSphereResult), "isAtleastOnePointInSphere");*/
                    isAtleastOnePointInSphereResult = true;
                    return;
                }
            }

            if (childs != nullptr)
            {
                for (int i = 0; i < 8; i++)
                {
                    childs[i]->isAtleastOnePointInSphere(Center, Radius);
                }
            }
        }
    }
};

class octree
{
    friend octreeNode;
public:
    octreeNode* root;
    std::vector<int> pointsToDelete;

    octree()
    {
        root = nullptr;
    }

    octree(float worldSize, glm::vec3 worldCenter)
    {
        initialize(worldSize, worldCenter);
    }

    octree(const octree& ref)
    {
        root = ref.root;
    }

    ~octree()
    {
        delete root;
    }

    void initialize(float worldSize, glm::vec3 worldCenter)
    {
        // Reset all debug counters
        deletedCount = 0;
        debugMaxNodeDepth = 0;
        debugNodeCount = 0;
        debugPointsInserted = 0;

        root = new octreeNode(worldSize, worldCenter, 0);
        root->parent = root;

#ifdef MAIN_EVENTS_LOGGING
        debugLog::getInstance().addToLog("creating octree...", "OctreeEvents");
        debugLog::getInstance().addToLog("World size: " + std::to_string(worldSize), "OctreeEvents");
        debugLog::getInstance().addToLog("World center: ", worldCenter, "OctreeEvents");
        debugLog::getInstance().addToLog("Raw data size: " + std::to_string(RawData->size()), "OctreeEvents");
#endif
    }

    bool addObject(MeshVertex vertex, int index)
    {
        //#ifdef FULL_LOGGING
        //        debugLog::getInstance().addToLog("point to add: ", vertex.position, "OctreeEvents");
        //#endif

        return root->addObject(octreeSearchInfo(vertex, index));
    }

    void deleteObjects(glm::vec3& Center, float Radius)
    {
#ifdef MAIN_EVENTS_LOGGING
        DWORD time = GetTickCount();
        deletedCount = 0;
#endif
        //root->deleteObjects(Center, Radius, this->pointsToDelete);
        root->searchForObjects(Center, Radius, this->pointsToDelete);
#ifdef MAIN_EVENTS_LOGGING
        debugLog::getInstance().addToLog("========= BEGIN OF DELETE EVENT =========", "OctreeEvents");
        debugLog::getInstance().addToLog("Center at ", Center, "OctreeEvents");
        debugLog::getInstance().addToLog("Radius: " + std::to_string(Radius), "OctreeEvents");
        debugLog::getInstance().addToLog("Points was deleted: " + std::to_string(deletedCount), "OctreeEvents");
        debugLog::getInstance().addToLog("Time was spent: " + std::to_string(time - GetTickCount()) + " ms", "OctreeEvents");
        debugLog::getInstance().addToLog("========= END OF DELETE EVENT =========", "OctreeEvents");
#endif
    }

    void searchForObjects(glm::vec3& Center, float Radius, std::vector<int>& foundObjects)
    {
        root->searchForObjects(Center, Radius, foundObjects);
    }

    std::vector<debugNodeBox>& debugGetNodesPositionAndSize()
    {
        root->debugGetNodesPositionAndSize(nodesPositionAndSize);
        return nodesPositionAndSize;
    }

    int getDebugNodeCount()
    {
        return debugNodeCount;
    }

    int getDebugMaxNodeDepth()
    {
        return debugMaxNodeDepth;
    }

    int getPointsInserted()
    {
        return debugPointsInserted;
    }

    bool isInOctreeBound(glm::vec3& Center, float Radius)
    {
#ifdef MAIN_EVENTS_LOGGING
        debugLog::getInstance().addToLog("========= BEGIN OF isInOctreeBound EVENT =========", "OctreeEvents");
        debugLog::getInstance().addToLog("Center at ", Center, "OctreeEvents");
        debugLog::getInstance().addToLog("Radius: " + std::to_string(Radius), "OctreeEvents");

        if (distance(root->center, Center) > Radius + root->diagonal)
        {
            debugLog::getInstance().addToLog("Result: false", "OctreeEvents");
            debugLog::getInstance().addToLog("========= END OF isInOctreeBound EVENT =========", "OctreeEvents");
            return false;
        }

        debugLog::getInstance().addToLog("Result: true", "OctreeEvents");
        debugLog::getInstance().addToLog("========= END OF isInOctreeBound EVENT =========", "OctreeEvents");
        return true;
#else
        if (distance(root->center, Center) > Radius + root->diagonal)
            return false;

        return true;
#endif
    }

    bool isAtleastOnePointInSphere(glm::vec3& Center, float Radius)
    {
        isAtleastOnePointInSphereCounter = 0;
        isAtleastOnePointInSphereChecksCounter = 0;

        isAtleastOnePointInSphereResult = false;
        root->isAtleastOnePointInSphere(Center, Radius);

        //if (GetTickCount() - time > 0)
        //{
            /*debugLog::getInstance().addToLog("isAtleastOnePointInSphereCounter: " + std::to_string(isAtleastOnePointInSphereCounter), "isAtleastOnePointInSphere");
            debugLog::getInstance().addToLog("isAtleastOnePointInSphereChecksCounter: " + std::to_string(isAtleastOnePointInSphereChecksCounter), "isAtleastOnePointInSphere");
            debugLog::getInstance().addToLog("Radius: " + std::to_string(Radius), "isAtleastOnePointInSphere");
            debugLog::getInstance().addToLog("Time spent on isAtleastOnePointInSphere: " + std::to_string(GetTickCount() - time) + " ms", "isAtleastOnePointInSphere");
            debugLog::getInstance().addToLog("bool isAtleastOnePointInSphereResult: " + std::to_string(isAtleastOnePointInSphereResult), "isAtleastOnePointInSphere");
            debugLog::getInstance().addToLog("===============================", "isAtleastOnePointInSphere");*/
            //}

        return isAtleastOnePointInSphereResult;
    }

    glm::vec3* closestPointInNode(octreeNode* node, glm::vec3& referencePoint, int& neighborsInRangeCount, float discardDistance, int minNeighborsInRange)
    {
        glm::vec3* result = nullptr;

        float minDistance = FLT_MAX;
        for (size_t i = 0; i < node->objects.size(); i++)
        {
            if (node->objects[i].position.x == referencePoint.x &&
                node->objects[i].position.y == referencePoint.y &&
                node->objects[i].position.z == referencePoint.z)
                continue;

            float distance = glm::distance(node->objects[i].position, referencePoint);
            if (distance < minDistance)
            {
                minDistance = distance;
                result = &node->objects[i].position;

                if (distance < discardDistance)
                {
                    neighborsInRangeCount++;
                    if (neighborsInRangeCount >= minNeighborsInRange)
                        return result;
                }
            }
        }

        return result;
    }

    float closestPointDistance(glm::vec3& referencePoint, float discardDistance, int minNeighborsInRange)
    {
        int neighborsInRangeCount = 0;
        int arrayIndex = 0;
        glm::vec3 closestPoint;

        octreeNode* node = root->getNodeContaining(referencePoint);
        if (node == nullptr)
        {
            return FLT_MAX;
        }

        glm::vec3* closestPoints[10];
        for (size_t i = 0; i < 10; i++)
        {
            closestPoints[i] = nullptr;
        }

        glm::vec3* ptr = closestPointInNode(node, referencePoint, neighborsInRangeCount, discardDistance, minNeighborsInRange);
        closestPoints[arrayIndex++] = ptr;

        if (node->parent != nullptr)
        {
            ptr = closestPointInNode(node->parent, referencePoint, neighborsInRangeCount, discardDistance, minNeighborsInRange);
            closestPoints[arrayIndex++] = ptr;

            for (size_t i = 0; i < 8; i++)
            {
                glm::vec3* ptr = closestPointInNode(node->parent->childs[i], referencePoint, neighborsInRangeCount, discardDistance, minNeighborsInRange);
                closestPoints[arrayIndex++] = ptr;
            }
        }

        float minDistance = FLT_MAX;
        for (size_t i = 0; i < 10; i++)
        {
            if (closestPoints[i] == nullptr)
                continue;

            float distance = glm::distance(*closestPoints[i], referencePoint);
            if (distance < minDistance && neighborsInRangeCount >= minNeighborsInRange)
            {
                minDistance = distance;
                closestPoint = *closestPoints[i];
            }
        }

        return minDistance;
    }
};