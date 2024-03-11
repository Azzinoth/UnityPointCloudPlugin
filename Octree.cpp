#include "Octree.h"

octreeNode::octreeNode()
{
    depth = 0;
    size = 0.0f;
    center = glm::vec3(0.0f);
    diagonal = 0.0f;
    childNodes = nullptr;
    parent = nullptr;
}

octreeNode::octreeNode(float Size, glm::vec3 Center, int Depth)
{
    if (debugMaxNodeDepth < Depth)
        debugMaxNodeDepth = Depth;
    depth = Depth;
    size = Size;
    center = Center;
    diagonal = sqrtTwo * size;
    childNodes = nullptr;
    parent = nullptr;
    nodeAABB = AABB(Center, Size);
}

octreeNode::octreeNode(const octreeNode& ref)
{
    center = ref.center;
    size = ref.size;
    diagonal = ref.diagonal;

    parent = nullptr;
    //parent = ref.parent;
    if (ref.childNodes != nullptr)
    {
        for (size_t i = 0; i < 8; i++)
        {
            childNodes[i] = ref.childNodes[i];
        }
    }
    objects = ref.objects;
    depth = ref.depth;
    nodeAABB = ref.nodeAABB;
}

octreeNode::~octreeNode()
{
    if (childNodes != nullptr)
    {
        for (int i = 0; i < 8; i++)
        {
            delete childNodes[i];
            //debugLog::getInstance().addToLog("child " + std::to_string(i) + " was deleted", "OctreeMemory");
        }
    }
}

void octreeNode::initChilds()
{
    debugNodeCount += 8;
#ifdef FULL_LOGGING
    //debugLog::getInstance().addToLog("8 nodes added. In total: " + std::to_string(debugNodeCount), "OctreeEvents");
#endif
    childNodes = new octreeNode * [8];

    // upper top left
    childNodes[0] = new octreeNode(size / 2.0f, center + glm::vec3(-size / 4.0f, size / 4.0f, -size / 4.0f), depth + 1);
    childNodes[0]->parent = this;
    // upper top right
    childNodes[1] = new octreeNode(size / 2.0f, center + glm::vec3(size / 4.0f, size / 4.0f, -size / 4.0f), depth + 1);
    childNodes[1]->parent = this;
    // upper bottom left
    childNodes[2] = new octreeNode(size / 2.0f, center + glm::vec3(-size / 4.0f, size / 4.0f, size / 4.0f), depth + 1);
    childNodes[2]->parent = this;
    // upper bottom right
    childNodes[3] = new octreeNode(size / 2.0f, center + glm::vec3(size / 4.0f, size / 4.0f, size / 4.0f), depth + 1);
    childNodes[3]->parent = this;

    // down top left
    childNodes[4] = new octreeNode(size / 2.0f, center + glm::vec3(-size / 4.0f, -size / 4.0f, -size / 4.0f), depth + 1);
    childNodes[4]->parent = this;
    // down top right
    childNodes[5] = new octreeNode(size / 2.0f, center + glm::vec3(size / 4.0f, -size / 4.0f, -size / 4.0f), depth + 1);
    childNodes[5]->parent = this;
    // down bottom left
    childNodes[6] = new octreeNode(size / 2.0f, center + glm::vec3(-size / 4.0f, -size / 4.0f, size / 4.0f), depth + 1);
    childNodes[6]->parent = this;
    // down bottom right
    childNodes[7] = new octreeNode(size / 2.0f, center + glm::vec3(size / 4.0f, -size / 4.0f, size / 4.0f), depth + 1);
    childNodes[7]->parent = this;
}

bool octreeNode::addObject(octreeSearchInfo info)
{
#ifdef FULL_LOGGING
    if (info.position.x > 578.1 && info.position.x < 578.3 &&
        info.position.y > -830.4 && info.position.y < -830.32 &&
        info.position.z > 558.5 && info.position.z < 558.9)
    {
        debugLog::getInstance().addToLog("point to add: ", point, "OctreeEvents");
    }
#endif

    bool bIntersect = true;
    if (info.position.x < nodeAABB.min[0] - 0.01f || info.position.x > nodeAABB.max[0] + 0.01f) bIntersect = false;
    if (info.position.y < nodeAABB.min[1] - 0.01f || info.position.y > nodeAABB.max[1] + 0.01f) bIntersect = false;
    if (info.position.z < nodeAABB.min[2] - 0.01f || info.position.z > nodeAABB.max[2] + 0.01f) bIntersect = false;

#ifdef FULL_LOGGING
    if (info.position.x > 578.1 && info.position.x < 578.3 &&
        info.position.y > -830.4 && info.position.y < -830.32 &&
        info.position.z > 558.5 && info.position.z < 558.9)
    {
        debugLog::getInstance().addToLog("point was: " + bIntersect ? "accepted" : "rejected", "OctreeEvents");
        if (!bIntersect)
        {
            debugLog::getInstance().addToLog("point was: rejected", "OctreeEvents");
            debugLog::getInstance().addToLog("point:", info.position, "OctreeEvents");
            debugLog::getInstance().addToLog("nodeAABB.min:", nodeAABB.min, "OctreeEvents");
            debugLog::getInstance().addToLog("nodeAABB.max:", nodeAABB.max, "OctreeEvents");
        }
    }
#endif

    if (objects.size() < capacity && bIntersect)
    {
        objects.push_back(info);
        debugPointsInserted++;
        return true;
    }
    else if (bIntersect)
    {
        if (childNodes == nullptr)
            initChilds();

        for (int i = 0; i < 8; i++)
        {
            if (childNodes[i]->addObject(info))
                return true;
        }
    }

    return false;
}

octreeNode* octreeNode::getNodeContaining(glm::vec3 point)
{
    bool bIntersect = true;
    if (point.x < nodeAABB.min[0] - 0.01f || point.x > nodeAABB.max[0] + 0.01f) bIntersect = false;
    if (point.y < nodeAABB.min[1] - 0.01f || point.y > nodeAABB.max[1] + 0.01f) bIntersect = false;
    if (point.z < nodeAABB.min[2] - 0.01f || point.z > nodeAABB.max[2] + 0.01f) bIntersect = false;

    if (bIntersect)
    {
        for (size_t i = 0; i < objects.size(); i++)
        {
            if (objects[i].position.x == point.x &&
                objects[i].position.y == point.y &&
                objects[i].position.z == point.z)
                return this;
        }

        if (childNodes != nullptr)
        {
            for (int i = 0; i < 8; i++)
            {
                octreeNode* result = nullptr;
                result = childNodes[i]->getNodeContaining(point);

                if (result != nullptr)
                    return result;
            }
        }
    }

    return nullptr;
}

void octreeNode::searchForObjects(glm::vec3& Center, float Radius, std::vector<int>& foundObjects)
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

    if (childNodes != nullptr)
    {
        for (int i = 0; i < 8; i++)
        {
            childNodes[i]->searchForObjects(Center, Radius, foundObjects);
        }
    }
}

void octreeNode::addAllObjects(octreeNode* node, std::vector<int>& foundObjects)
{
    for (int i = 0; i < objects.size(); i++)
    {
        //deletedCount++;
        foundObjects.push_back(objects[i].hadIndex);
    }

    if (childNodes != nullptr)
    {
        for (int i = 0; i < 8; i++)
        {
            childNodes[i]->addAllObjects(this, foundObjects);
        }
    }
}

void octreeNode::debugGetNodesPositionAndSize(std::vector<debugNodeBox>& result)
{
    result.push_back(debugNodeBox(this->center, this->size, this->depth));

    if (childNodes != nullptr)
    {
        for (int i = 0; i < 8; i++)
        {
            childNodes[i]->debugGetNodesPositionAndSize(result);
        }
    }
}

void octreeNode::isAtleastOnePointInSphere(glm::vec3& Center, float Radius)
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

        if (childNodes != nullptr)
        {
            for (int i = 0; i < 8; i++)
            {
                childNodes[i]->isAtleastOnePointInSphere(Center, Radius);
            }
        }
    }
}

octree::octree()
{
    root = nullptr;
}

octree::octree(float worldSize, glm::vec3 worldCenter)
{
    initialize(worldSize, worldCenter);
}

octree::octree(const octree& ref)
{
    root = ref.root;
}

octree::~octree()
{
    delete root;
}

void octree::initialize(float worldSize, glm::vec3 worldCenter)
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

bool octree::addObject(VertexData vertex, int index)
{
    //#ifdef FULL_LOGGING
    //        debugLog::getInstance().addToLog("point to add: ", vertex.position, "OctreeEvents");
    //#endif

    return root->addObject(octreeSearchInfo(vertex, index));
}

void octree::deleteObjects(glm::vec3& Center, float Radius)
{
#ifdef MAIN_EVENTS_LOGGING
    DWORD time = GetTickCount();
    deletedCount = 0;
#endif
    //root->deleteObjects(Center, Radius, this->PointnsInSphere);
    this->PointnsInSphere.clear();
    root->searchForObjects(Center, Radius, this->PointnsInSphere);
#ifdef MAIN_EVENTS_LOGGING
    debugLog::getInstance().addToLog("========= BEGIN OF DELETE EVENT =========", "OctreeEvents");
    debugLog::getInstance().addToLog("Center at ", Center, "OctreeEvents");
    debugLog::getInstance().addToLog("Radius: " + std::to_string(Radius), "OctreeEvents");
    debugLog::getInstance().addToLog("Points was deleted: " + std::to_string(deletedCount), "OctreeEvents");
    debugLog::getInstance().addToLog("Time was spent: " + std::to_string(time - GetTickCount()) + " ms", "OctreeEvents");
    debugLog::getInstance().addToLog("========= END OF DELETE EVENT =========", "OctreeEvents");
#endif
}

void octree::searchForObjects(glm::vec3& Center, float Radius, std::vector<int>& foundObjects)
{
    root->searchForObjects(Center, Radius, foundObjects);
}

std::vector<debugNodeBox>& octree::debugGetNodesPositionAndSize()
{
    root->debugGetNodesPositionAndSize(nodesPositionAndSize);
    return nodesPositionAndSize;
}

int octree::getDebugNodeCount()
{
    return debugNodeCount;
}

int octree::getDebugMaxNodeDepth()
{
    return debugMaxNodeDepth;
}

int octree::getPointsInserted()
{
    return debugPointsInserted;
}

bool octree::isInOctreeBound(glm::vec3& Center, float Radius)
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

bool octree::isAtleastOnePointInSphere(glm::vec3& Center, float Radius)
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

glm::vec3* octree::closestPointsInNode(octreeNode* node, glm::vec3& referencePoint, int& neighborsInRangeCount, float discardDistance, int minNeighborsInRange)
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

glm::vec3* octree::closestPointInNode(octreeNode* node, glm::vec3& referencePoint, float discardDistance)
{
    glm::vec3* result = nullptr;

    float minDistance = FLT_MAX;
    for (size_t i = 0; i < node->objects.size(); i++)
    {
        if (node->objects[i].position.x == referencePoint.x &&
            node->objects[i].position.y == referencePoint.y &&
            node->objects[i].position.z == referencePoint.z)
            continue;

        if (node->objects[i].position.x == DELETED_POINTS_COORDINATE &&
            node->objects[i].position.y == DELETED_POINTS_COORDINATE &&
            node->objects[i].position.z == DELETED_POINTS_COORDINATE)
            continue;

        if (node->objects[i].color[0] == 255 &&
            node->objects[i].color[1] == 0 &&
            node->objects[i].color[2] == 0 &&
            node->objects[i].color[3] == 255)
            continue;

        float distance = glm::distance(node->objects[i].position, referencePoint);
        if (distance < minDistance)
        {
            minDistance = distance;
            result = &node->objects[i].position;

            /*if (distance < discardDistance)
                return result;*/
        }
    }

    return result;
}

float octree::closestPointDistance(glm::vec3& referencePoint, float discardDistance, int minNeighborsInRange)
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

    glm::vec3* ptr = closestPointsInNode(node, referencePoint, neighborsInRangeCount, discardDistance, minNeighborsInRange);
    closestPoints[arrayIndex++] = ptr;

    if (node->parent != nullptr)
    {
        ptr = closestPointsInNode(node->parent, referencePoint, neighborsInRangeCount, discardDistance, minNeighborsInRange);
        closestPoints[arrayIndex++] = ptr;

        for (size_t i = 0; i < 8; i++)
        {
            glm::vec3* ptr = closestPointsInNode(node->parent->childNodes[i], referencePoint, neighborsInRangeCount, discardDistance, minNeighborsInRange);
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

float octree::closestPointDistance(glm::vec3& referencePoint, float discardDistance)
{
    int arrayIndex = 0;
    glm::vec3 closestPoint;

    octreeNode* node = root->getNodeContaining(referencePoint);
    if (node == nullptr)
        return FLT_MAX;

    glm::vec3* closestPoints[10];
    for (size_t i = 0; i < 10; i++)
    {
        closestPoints[i] = nullptr;
    }

    glm::vec3* ptr = closestPointInNode(node, referencePoint, discardDistance);
    closestPoints[arrayIndex++] = ptr;

    if (node->parent != nullptr)
    {
        ptr = closestPointInNode(node->parent, referencePoint, discardDistance);
        closestPoints[arrayIndex++] = ptr;

        for (size_t i = 0; i < 8; i++)
        {
            glm::vec3* ptr = closestPointInNode(node->parent->childNodes[i], referencePoint, discardDistance);
            closestPoints[arrayIndex++] = ptr;
        }
    }

    float minDistance = FLT_MAX;
    for (size_t i = 0; i < 10; i++)
    {
        if (closestPoints[i] == nullptr)
            continue;

        float distance = glm::distance(*closestPoints[i], referencePoint);
        if (distance < minDistance)
        {
            minDistance = distance;
            closestPoint = *closestPoints[i];
        }
    }

    return minDistance;
}