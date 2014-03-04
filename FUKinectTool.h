#pragma once
//Kinect Includes
#include <Windows.h>
#include <NuiApi.h>
#include <KinectInteraction.h>
//Windows Includes
#include <strsafe.h>
#include <shlobj.h>
#include <Objbase.h>
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "Ole32.lib")
//STL Includes
#include <thread>
#include <vector>
#include <memory>
#include <map>
#include <iostream>
//Local Includes
#include "FUMath.h"
#define F_UNUSED(T) (void)T

class NuiInteractionClient : public INuiInteractionClient
{
public:
    NuiInteractionClient() {}
    ~NuiInteractionClient() {}
    STDMETHODIMP_(ULONG) AddRef() { return S_OK;     }
    STDMETHODIMP_(ULONG) Release() { return S_OK;     }
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv) {F_UNUSED(riid); F_UNUSED(ppv); return S_OK;  }
    STDMETHODIMP GetInteractionInfoAtLocation(DWORD skeletonTrackingId, NUI_HAND_TYPE handType, FLOAT x, FLOAT y, NUI_INTERACTION_INFO *pInteractionInfo)
    {
        F_UNUSED(skeletonTrackingId);
        F_UNUSED(handType);
        F_UNUSED(x);
        F_UNUSED(y);
        if(pInteractionInfo)
        {
            pInteractionInfo->IsGripTarget          = true;
            pInteractionInfo->PressTargetControlId  = 0;
            pInteractionInfo->PressAttractionPointX = 0.f;
            pInteractionInfo->PressAttractionPointY = 0.f;
            return S_OK;
        }
        return E_POINTER;
    }
};

/**
 * @brief Create it with NUI_INITIALIZE_FLAG_USES_SKELETON and NUI_INITIALIZE_FLAG_USES_DEPTH to use Interactions
 */
class FUKinectTool
{
public:
    enum KINECT_STATUS {
        INITIALIZING,//The device is connected, but still initializing.
        NOT_CONNECTED,//The device is not connected.
        NOT_GENUINE,//The device is not a valid Kinect.
        NOT_SUPPORTED,//The device is an unsupported model.
        INSUFFICENT_BANDWITH,//The device is connected to a hub without the necessary bandwidth requirements.
        NOT_POWERED,//The device is connected, but unpowered.
        NOT_READY,//There was some other unspecified error.
        DEVICE_IN_USE,
        NO_PROBLEM = S_OK//All is well! ;)
    };
    enum SKELETON_JOINTS {
        HIP_CENTER = NUI_SKELETON_POSITION_HIP_CENTER,
        SPINE = NUI_SKELETON_POSITION_SPINE,
        SHOULDER_CENTER = NUI_SKELETON_POSITION_SHOULDER_CENTER,
        HEAD = NUI_SKELETON_POSITION_HEAD,
        SHOULDER_LEFT = NUI_SKELETON_POSITION_SHOULDER_LEFT,
        ELBOW_LEFT = NUI_SKELETON_POSITION_ELBOW_LEFT,
        WRIST_LEFT = NUI_SKELETON_POSITION_WRIST_LEFT,
        HAND_LEFT = NUI_SKELETON_POSITION_HAND_LEFT,
        SHOULDER_RIGHT = NUI_SKELETON_POSITION_SHOULDER_RIGHT,
        ELBOW_RIGHT = NUI_SKELETON_POSITION_ELBOW_RIGHT,
        WRIST_RIGHT = NUI_SKELETON_POSITION_WRIST_RIGHT,
        HAND_RIGHT = NUI_SKELETON_POSITION_HAND_RIGHT,
        HIP_LEFT = NUI_SKELETON_POSITION_HIP_LEFT,
        KNEE_LEFT = NUI_SKELETON_POSITION_KNEE_LEFT,
        ANKLE_LEFT = NUI_SKELETON_POSITION_ANKLE_LEFT,
        FOOT_LEFT = NUI_SKELETON_POSITION_FOOT_LEFT,
        HIP_RIGHT = NUI_SKELETON_POSITION_HIP_RIGHT,
        KNEE_RIGHT = NUI_SKELETON_POSITION_KNEE_RIGHT,
        ANKLE_RIGHT = NUI_SKELETON_POSITION_ANKLE_RIGHT,
        FOOT_RIGHT = NUI_SKELETON_POSITION_FOOT_RIGHT,
    };

    enum SKELETON_STATE
    {
        SKELETON_TRACKED = NUI_SKELETON_TRACKED,
        SKELETON_NOT_TRACKED = NUI_SKELETON_NOT_TRACKED,
        SKELETON_POSITION_ONLY = NUI_SKELETON_POSITION_ONLY
    };

    enum SKELETONS {
        SKELETON_ONE,//The one on the right
        SKELETON_TWO,//The one on the left
        BOTH_SKELETONS,
        NONE
    };

public:
    FUKinectTool(DWORD flags);
    ~FUKinectTool(void);
    /**
     * @brief This functions checks if there are any depth|skeleton|interaction stream available and If there are, it does the necessary
     * processing. Use this function in only one place to keep updating the sensor.
     */
    void updateSensor();
    KINECT_STATUS getKinectStatus() {return mKinectErrorMessage;}

    bool detectRightHandUpPosture(NUI_SKELETON_DATA &skeletonData);
    bool detectLeftHandUpPosture(NUI_SKELETON_DATA &skeletonData);
    bool detectBothHandsUp(NUI_SKELETON_DATA &skeletonData);
    //TODO: add getHandPosition(HAND_TYPE)
    bool detectLeftHandDownPosture(NUI_SKELETON_DATA &skeletonData);
    bool detectRightHandDownPosture(NUI_SKELETON_DATA &skeletonData);
    bool detectOpenRightArm(NUI_SKELETON_DATA &skeletonData);
    bool detectOpenLeftArm(NUI_SKELETON_DATA &skeletonData);
    bool detectOpenArms(NUI_SKELETON_DATA &skeletonData);
    bool detectPush(DWORD skeletonTrackingID);
    bool detectGrip(DWORD skeletonTrackingID);
    bool detectLeanRight(NUI_SKELETON_DATA &skeletonData);//TODO: This probably doesn't work
    bool detectLeanLeft(NUI_SKELETON_DATA &skeletonData);//TODO: This probably doesn't work
    /**
     * @brief Takes the right shoulder as the origin of the circle and as the hand turns around it
     * returns the angle. When the right hand is down near the legs, the angle is 0 and it increases clockwise.
     * @param NUI_SKELETON_DATA & skeletonData
     * @return float --> The angle relative to the right shoulder as the origin
     */
    float getRightHandAngle(NUI_SKELETON_DATA &skeletonData);
    /**
     * @brief Takes the left shoulder as the origin of the circle and as the hand turns around
     * it returns the angle. When the left hand is down near the legs, the angle is 0 and it increases clockwise.
     * @param NUI_SKELETON_DATA & skeletonData
     * @return float --> The angle relative to the left shoulder as the origin
     */
    float getLeftHandAngle(NUI_SKELETON_DATA &skeletonData);
    /**
     * @brief This is the skeleton on the right side
     * @return
     */
    NUI_SKELETON_DATA* getSkeletonOne() {return mSkeletonDataOne;}
    /**
     * @brief This is the skeleton on the left
     * @return
     */
    NUI_SKELETON_DATA* getSkeletonTwo() {return mSkeletonDataTwo;}
    bool isSkeletonTracked(NUI_SKELETON_DATA &skeletonData);
    void takeColorShot() {mSaveScreenshot = true;}
    double getDistanceFromFloor(Vector4 jointPosition);
    bool detectJumping(NUI_SKELETON_DATA &skeletonData);
    /**
     * @brief Returns the position of the active hand in cantimeters
     * @param skeletonTrackingID to determine which user's position to return
     * @return
     */
    FUMath::FUVector2<float> getHandPosition(DWORD skeletonTrackingID);
    SKELETONS getWhichSkeletonLeftScene() {return mSkeletonLeftScene;}

private:
    /**
     * @brief The skeleton on the right or the only one that is tracked
     */
    NUI_SKELETON_DATA *mSkeletonDataOne;
    unsigned long mSkeletonOneTrackingID;
    /**
     * @brief The skeleton on the left. This isn't available when there's only one skeleton
     */
    NUI_SKELETON_DATA *mSkeletonDataTwo;
    unsigned long mSkeletonTwoTrackingID;
    HANDLE mHandleNextColorFrameEvent;
    HANDLE mHandleColorStream;
    HANDLE mHandleNextDepthFrameEvent;
    HANDLE mHandleDepthStream;
    HANDLE mHandleSkeletonStream;
    HANDLE mHandleNextSkeletonEvent;
    HANDLE mHandleNextHandEvent;
    INuiSensor *mNuiSensor;
    bool mSaveScreenshot;
    const int mColorWidth;
    const int mColorHeight;
    INuiInteractionStream *mNuiInteractionStream;
    NuiInteractionClient *mNuiInteractionClient;
    bool mIsPressedPlayerOne, mIsGrippingPlayerOne, mJumpedPlayerOne, mIsPressedPlayerTwo, mIsGrippingPlayerTwo, mJumpedPlayerTwo;
    DWORD mDWFlags;

    KINECT_STATUS mKinectErrorMessage;
    NUI_SKELETON_FRAME mSkeletonFrame;
    FUMath::FUVector2<float> mVectorSkeletonOneHandPosition;
    FUMath::FUVector2<float> mVectorSkeletonTwoHandPosition;

    SKELETONS mSkeletonLeftScene;

private:
    /**
     * @brief Finds and creates an instance with the firs found ready kinect. And initializes the Kinect with Skeleton tracking
     * and color stream. This function creates a handle that is signalled when a new skeleton data or color data is available.
     * @return HRESULT
     */
    HRESULT createFirstConnected();
    /**
      @brief A callback function that gets notified when the sensor connection status changes.
     */
    static void CALLBACK StatusProcCallback(HRESULT hrStatus, const OLECHAR *instanceName, const OLECHAR *uniqueDeviceName, void *pUserData);
    void safeReleaseSensor();

    HRESULT getScreenshotFileName(wchar_t *screenshotName, UINT screenshotNameSize);
    /**
     * @brief Save passed in image data to disk as a bitmap
     * @param pBitmapBits --> image data to save
     * @param lWidth --> width (in pixels) of input image data
     * @param lHeight --> height (in pixels) of input image data
     * @param wBitsPerPixel --> bits per pixel of image data
     * @param lpszFilePath --> full file path to output bitmap to
     * @return indicates success or failure
     */
    HRESULT saveBitmapToFile(BYTE* pBitmapBits, LONG lWidth, LONG lHeight, WORD wBitsPerPixel, LPCWSTR lpszFilePath);
    void processInteraction();
    void processDepth();
    void processColor();
    void processSkeleton();
    bool isSkeletonOnRight(NUI_SKELETON_DATA &skeletonDataOne, NUI_SKELETON_DATA &skeletonDataTwo);
    bool checkForSkeletonVisibility(NUI_SKELETON_DATA &skeletonData, NUI_SKELETON_FRAME &frame);
    int getSkeletonCount(NUI_SKELETON_FRAME &sFrame);
    bool isFloorVisible();
};

