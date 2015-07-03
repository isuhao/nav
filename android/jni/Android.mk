LOCAL_PATH:= $(call my-dir)/../..
LOCAL_CODE_PATH 		:= $(call my-dir)



include $(CLEAR_VARS)
LOCAL_C_INCLUDES:= $(LOCAL_PATH)/FxKynapse/Interface\
$(LOCAL_PATH)/FxKynapse/RecastNavigation/DebugUtils/Include\
$(LOCAL_PATH)/FxKynapse/RecastNavigation/Detour/Include\
$(LOCAL_PATH)/FxKynapse/RecastNavigation/DetourCrowd/Include\
$(LOCAL_PATH)/FxKynapse/RecastNavigation/DetourTileCache/Include\
$(LOCAL_PATH)/FxKynapse/RecastNavigation/Recast/Include\
$(LOCAL_PATH)/FxKynapse/FxRecastNavigation/fastlz
LOCAL_CODE_PATH	:= FxKynapse/FxRecastNavigation/
LOCAL_MODULE    := libfxrecastnavigation
LOCAL_SRC_FILES := \
$(LOCAL_CODE_PATH)fastlz/fastlz.c\
$(LOCAL_CODE_PATH)RecastNavMeshBuilder.cpp\
$(LOCAL_CODE_PATH)RecastNavigationManager.cpp\
$(LOCAL_CODE_PATH)rcDefaultFileIO.cpp\
$(LOCAL_CODE_PATH)rcBotAgent.cpp\
$(LOCAL_CODE_PATH)rcObstacle.cpp\
$(LOCAL_CODE_PATH)PerfTimer.cpp\
$(LOCAL_CODE_PATH)MeshLoaderTerrain.cpp\
$(LOCAL_CODE_PATH)InputTerrainGeom.cpp\
$(LOCAL_CODE_PATH)DebugDrawPool.cpp\
$(LOCAL_CODE_PATH)CoordSystem.cpp\
$(LOCAL_CODE_PATH)ChunkyTriMesh.cpp
include $(BUILD_STATIC_LIBRARY)



include $(CLEAR_VARS)
LOCAL_C_INCLUDES:= $(LOCAL_PATH)/FxKynapse/Interface\
$(LOCAL_PATH)/FxKynapse/Navigation/sdk/include\
$(LOCAL_PATH)/../mobile/math\
$(LOCAL_PATH)/../client
LOCAL_CODE_PATH	:= FxKynapse/FxNavigation/
LOCAL_MODULE    := libfxnavigation
LOCAL_SRC_FILES := \
$(LOCAL_CODE_PATH)NavigationBot.cpp\
$(LOCAL_CODE_PATH)navigationmanager.cpp\
$(LOCAL_CODE_PATH)NavigationObstacle.cpp

$(info ----------------------------Compile infomation----------------------------)
$(info TARGET_ARCH     = $(TARGET_ARCH))
$(info TARGET_ARCH_ABI = $(TARGET_ARCH_ABI))
$(info TARGET_ABI      = $(TARGET_ABI))
$(info APP_CFLAGS      = $(APP_CFLAGS))
$(info APP_ABI         = $(APP_ABI))
$(info --------------------------------------------------------------------------)
include $(BUILD_STATIC_LIBRARY)