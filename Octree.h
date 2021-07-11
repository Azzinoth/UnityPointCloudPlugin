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

static int deletedCount = 0;
static const int capacity = 250;
static float sqrtTwo = sqrt(2.0f);
static int count = 0;
static int debugMaxNodeDepth = 0;
static int debugNodeCount = 0;

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
    bool childsInitialized = false;
    octreeNode* parent;
    octreeNode** childs;
    std::vector<int> objects;
    int depth = 0;
    AABB nodeAABB;
    std::vector<MeshVertex>* rawData;

    octreeNode()
    {
        depth = 0;
        size = 0.0f;
        center = glm::vec3(0.0f);
        diagonal = 0.0f;
        childs = nullptr;
        parent = nullptr;
        rawData = nullptr;
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

    ~octreeNode()
    {
        if (childsInitialized)
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
        debugLog::getInstance().addToLog("8 nodes added. In total: " + std::to_string(debugNodeCount), "OctreeEvents");
#endif
        childsInitialized = true;
        childs = new octreeNode*[8];

        // upper top left
        childs[0] = new octreeNode(size / 2.0f, center + glm::vec3(-size / 4.0f, size / 4.0f, -size / 4.0f), depth + 1);
        childs[0]->parent = this;
        childs[0]->rawData = rawData;
        // upper top right
        childs[1] = new octreeNode(size / 2.0f, center + glm::vec3(size / 4.0f, size / 4.0f, -size / 4.0f), depth + 1);
        childs[1]->parent = this;
        childs[1]->rawData = rawData;
        // upper bottom left
        childs[2] = new octreeNode(size / 2.0f, center + glm::vec3(-size / 4.0f, size / 4.0f, size / 4.0f), depth + 1);
        childs[2]->parent = this;
        childs[2]->rawData = rawData;
        // upper bottom right
        childs[3] = new octreeNode(size / 2.0f, center + glm::vec3(size / 4.0f, size / 4.0f, size / 4.0f), depth + 1);
        childs[3]->parent = this;
        childs[3]->rawData = rawData;

        // down top left
        childs[4] = new octreeNode(size / 2.0f, center + glm::vec3(-size / 4.0f, -size / 4.0f, -size / 4.0f), depth + 1);
        childs[4]->parent = this;
        childs[4]->rawData = rawData;
        // down top right
        childs[5] = new octreeNode(size / 2.0f, center + glm::vec3(size / 4.0f, -size / 4.0f, -size / 4.0f), depth + 1);
        childs[5]->parent = this;
        childs[5]->rawData = rawData;
        // down bottom left
        childs[6] = new octreeNode(size / 2.0f, center + glm::vec3(-size / 4.0f, -size / 4.0f, size / 4.0f), depth + 1);
        childs[6]->parent = this;
        childs[6]->rawData = rawData;
        // down bottom right
        childs[7] = new octreeNode(size / 2.0f, center + glm::vec3(size / 4.0f, -size / 4.0f, size / 4.0f), depth + 1);
        childs[7]->parent = this;
        childs[7]->rawData = rawData;
    }

    bool addObject(glm::vec3& point, int index)
    {
#ifdef FULL_LOGGING
        debugLog::getInstance().addToLog("point to add: ", point, "OctreeEvents");
#endif

        bool instersect = true;
        if (point.x < nodeAABB.min[0] || point.x > nodeAABB.max[0]) instersect = false;
        if (point.y < nodeAABB.min[1] || point.y > nodeAABB.max[1]) instersect = false;
        if (point.z < nodeAABB.min[2] || point.z > nodeAABB.max[2]) instersect = false;

#ifdef FULL_LOGGING
        debugLog::getInstance().addToLog("point was: " + instersect ? "accepted" : "rejected", "OctreeEvents");
        if (!instersect)
        {
            debugLog::getInstance().addToLog("point was: rejected", "OctreeEvents");
            debugLog::getInstance().addToLog("point:", point, "OctreeEvents");
            debugLog::getInstance().addToLog("nodeAABB.min:", nodeAABB.min, "OctreeEvents");
            debugLog::getInstance().addToLog("nodeAABB.max:", nodeAABB.max, "OctreeEvents");
        }
#endif

        if (objects.size() < capacity && instersect)
        {
            objects.push_back(index);
            return true;
        }
        else if (instersect)
        {
            if (!childsInitialized)
            {
                initChilds();
            }

            for (int i = 0; i < 8; i++)
            {
                if (childs[i]->addObject(point, index))
                    return true;
            }
        }

        return false;
    }

    //void deleteObjects(glm::vec3& Center, float Radius, std::vector<int>& pointsToDelete)
    //{
    //    /*debugLog::getInstance().addToLog("========= BEGIN OF DELETE EVENT =========", "deleteEvents");
    //    debugLog::getInstance().addToLog("Center at ", Center, "deleteEvents");
    //    debugLog::getInstance().addToLog("Radius: " + std::to_string(Radius), "deleteEvents");
    //    debugLog::getInstance().addToLog("Depth: " + std::to_string(depth), "deleteEvents");*/

    //    double currentDistance = distance(center, Center);
    //    /*debugLog::getInstance().addToLog("currentDistance  distance(center, Center): " + std::to_string(currentDistance), "deleteEvents");
    //    debugLog::getInstance().addToLog("diagonal: " + std::to_string(diagonal), "deleteEvents");*/

    //    if (currentDistance > Radius + diagonal)
    //    {
    //        /*debugLog::getInstance().addToLog("Rejected", "deleteEvents");
    //        debugLog::getInstance().addToLog("========= END OF DELETE EVENT =========", "deleteEvents");*/
    //        return;
    //    }
    //    else if (Radius > currentDistance + diagonal)
    //    {
    //        /*debugLog::getInstance().addToLog("Approved total deletion", "deleteEvents");
    //        debugLog::getInstance().addToLog("========= END OF DELETE EVENT =========", "deleteEvents");*/
    //        deleteAllObjects(this, pointsToDelete);
    //        return;
    //    }

    //    for (int i = 0; i < objects.size(); i++)
    //    {
    //        currentDistance = distance(rawData->operator[](objects[i]).position, Center);
    //        //debugLog::getInstance().addToLog("currentDistance  distance(rawData->operator[](objects[i]).position, Center): " + std::to_string(currentDistance), "deleteEvents");
    //        if (currentDistance < Radius)
    //        {
    //            deletedCount++;
    //            pointsToDelete.push_back(objects[i]);
    //        }
    //    }

    //    if (childsInitialized)
    //    {
    //        for (int i = 0; i < 8; i++)
    //        {
    //            childs[i]->deleteObjects(Center, Radius, pointsToDelete);
    //        }
    //    }
    //}

    //void deleteAllObjects(octreeNode* node, std::vector<int>& pointsToDelete)
    //{
    //    for (int i = 0; i < objects.size(); i++)
    //    {
    //        deletedCount++;
    //        pointsToDelete.push_back(objects[i]);
    //    }

    //    if (childsInitialized)
    //    {
    //        for (int i = 0; i < 8; i++)
    //        {
    //            childs[i]->deleteAllObjects(this, pointsToDelete);
    //        }
    //    }
    //}

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
            currentDistance = distance(rawData->operator[](objects[i]).position, Center);
            if (currentDistance < Radius)
            {
                //deletedCount++;
                foundObjects.push_back(objects[i]);
            }
        }

        if (childsInitialized)
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
            foundObjects.push_back(objects[i]);
        }

        if (childsInitialized)
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

        if (childsInitialized)
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
                currentDistance = distance(rawData->operator[](objects[i]).position, Center);
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

            if (childsInitialized)
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

    octree(float worldSize, glm::vec3 worldCenter, std::vector<MeshVertex>* RawData)
    {
        initialize(worldSize, worldCenter, RawData);
    }

    ~octree()
    {
        delete root;
    }

    void initialize(float worldSize, glm::vec3 worldCenter, std::vector<MeshVertex>* RawData)
    {
        // Reset all debug counters
        deletedCount = 0;
        debugMaxNodeDepth = 0;
        debugNodeCount = 0;

        root = new octreeNode(worldSize, worldCenter, 0);
        root->parent = root;
        root->rawData = RawData;
        //rawDataPointer = RawData;

#ifdef MAIN_EVENTS_LOGGING
        debugLog::getInstance().addToLog("creating octree...", "OctreeEvents");
        debugLog::getInstance().addToLog("World size: " + std::to_string(worldSize), "OctreeEvents");
        debugLog::getInstance().addToLog("World center: ", worldCenter, "OctreeEvents");
        debugLog::getInstance().addToLog("Raw data size: " + std::to_string(RawData->size()), "OctreeEvents");
#endif
    }

    bool addObject(int index)
    {
//#ifdef FULL_LOGGING
//        debugLog::getInstance().addToLog("point to add: ", rawData[index], "OctreeEvents");
//#endif
        return root->addObject(root->rawData->operator[](index).position, index);
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
};