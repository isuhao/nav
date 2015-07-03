LOCAL_PATH:= $(call my-dir)/../..
LOCAL_CODE_PATH 		:= LOCAL_PATH;

include $(CLEAR_VARS)
LOCAL_C_INCLUDES:= $(LOCAL_PATH)/DebugUtils/Include\
$(LOCAL_PATH)/Detour/Include\
$(LOCAL_PATH)/DetourCrowd/Include\
$(LOCAL_PATH)/DetourTileCache/Include\
$(LOCAL_PATH)/Recast/Include
LOCAL_MODULE    := librecastnavigation
LOCAL_SRC_FILES := \
Recast/Source/RecastRegion.cpp\
Recast/Source/RecastRasterization.cpp\
Recast/Source/RecastMeshDetail.cpp\
Recast/Source/RecastMesh.cpp\
Recast/Source/RecastLayers.cpp\
Recast/Source/RecastFilter.cpp\
Recast/Source/RecastContour.cpp\
Recast/Source/RecastArea.cpp\
Recast/Source/RecastAlloc.cpp\
Recast/Source/Recast.cpp\
DetourTileCache/Source/DetourTileCacheBuilder.cpp\
DetourTileCache/Source/DetourTileCache.cpp\
DetourCrowd/Source/DetourProximityGrid.cpp\
DetourCrowd/Source/DetourPathQueue.cpp\
DetourCrowd/Source/DetourPathCorridor.cpp\
DetourCrowd/Source/DetourObstacleAvoidance.cpp\
DetourCrowd/Source/DetourLocalBoundary.cpp\
DetourCrowd/Source/DetourCrowd.cpp\
Detour/Source/DetourNode.cpp\
Detour/Source/DetourNavMeshQuery.cpp\
Detour/Source/DetourNavMeshBuilder.cpp\
Detour/Source/DetourNavMesh.cpp\
Detour/Source/DetourCommon.cpp\
Detour/Source/DetourAlloc.cpp

$(info ----------------------------Compile infomation----------------------------)
$(info TARGET_ARCH     = $(TARGET_ARCH))
$(info TARGET_ARCH_ABI = $(TARGET_ARCH_ABI))
$(info TARGET_ABI      = $(TARGET_ABI))
$(info APP_CFLAGS      = $(APP_CFLAGS))
$(info APP_ABI         = $(APP_ABI))
$(info --------------------------------------------------------------------------)
include $(BUILD_STATIC_LIBRARY)