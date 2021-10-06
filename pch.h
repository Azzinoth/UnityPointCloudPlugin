#ifndef PCH_H
#define PCH_H

#include "undoManager.h"

#include "framework.h"
#include <iostream>

#include "comdef.h"

#include "thirdparty/laszip/include/laszip_api.h"

void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginLoad(IUnityInterfaces* unityInterfaces);
void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginUnload();
//UNITY_INTERFACE_EXPORT char* UNITY_INTERFACE_API RequestPointCloudSpatialInfoFromUnity(int pointCloudIndex);



#endif //PCH_H
