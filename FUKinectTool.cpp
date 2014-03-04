#include "FUKinectTool.h"

FUKinectTool::FUKinectTool(DWORD flags)
    : mSkeletonDataOne(nullptr)
    , mSkeletonDataTwo(nullptr)
    , mNuiSensor(nullptr)
    , mSaveScreenshot(false)
    , mColorWidth(1280)
    , mColorHeight(960)
    , mNuiInteractionClient(new NuiInteractionClient())
    , mIsPressedPlayerOne(false)
    , mIsPressedPlayerTwo(false)
    , mIsGrippingPlayerOne(false)
    , mIsGrippingPlayerTwo(false)
    , mJumpedPlayerOne(false)
    , mJumpedPlayerTwo(false)
    , mKinectErrorMessage(NO_PROBLEM)
    , mDWFlags(flags)
    , mSkeletonFrame()
    , mVectorSkeletonOneHandPosition(0, 0)
    , mVectorSkeletonTwoHandPosition(0, 0)
    , mSkeletonOneTrackingID(-1)
    , mSkeletonTwoTrackingID(-1)
    , mSkeletonLeftScene(SKELETONS::NONE)
{
    NuiSetDeviceStatusCallback(&FUKinectTool::StatusProcCallback, this);
    createFirstConnected();
}

FUKinectTool::~FUKinectTool(void)
{
    if (mNuiSensor) {
        mNuiSensor->NuiShutdown();
        mNuiSensor->Release();
        mNuiSensor = nullptr;
    }
    if (mHandleNextColorFrameEvent != INVALID_HANDLE_VALUE)
        CloseHandle(mHandleNextColorFrameEvent);
    if (mHandleNextSkeletonEvent && (mHandleNextSkeletonEvent != INVALID_HANDLE_VALUE))
        CloseHandle(mHandleNextSkeletonEvent);
    mSkeletonDataOne = nullptr;
    mSkeletonDataTwo = nullptr;
}

void CALLBACK FUKinectTool::StatusProcCallback(HRESULT hrStatus, const OLECHAR *instanceName, const OLECHAR *uniqueDeviceName, void *pUserData)
{
    (void)instanceName;
    (void)uniqueDeviceName;
    if (SUCCEEDED(hrStatus)) {
        // Initialize the Kinect sensor identified by the instanceName parameter.
        std::cout << hrStatus << std::endl;
        FUKinectTool *kinectTool = static_cast<FUKinectTool*>(pUserData);
        kinectTool->createFirstConnected();
    }
    else {
        // Uninitialize the Kinect sensor identified by the instanceName parameter.
        FUKinectTool *kinectTool = static_cast<FUKinectTool*>(pUserData);
        kinectTool->safeReleaseSensor();
        HRESULT kinectStatus = hrStatus;
        if (kinectStatus == E_NUI_NOTCONNECTED) {
            std::cout << "The device is not connected." << std::endl;
            kinectTool->mKinectErrorMessage = NOT_CONNECTED;
        }
        else if (kinectStatus == E_NUI_NOTGENUINE) {
            std::cout << "The device is not a valid Kinect." << std::endl;
            kinectTool->mKinectErrorMessage = NOT_GENUINE;
        }
        else if (kinectStatus == E_NUI_NOTSUPPORTED) {
            std::cout << "The device is an unsupported model." << std::endl;
            kinectTool->mKinectErrorMessage = NOT_SUPPORTED;
        }
        else if (kinectStatus == E_NUI_INSUFFICIENTBANDWIDTH) {
            std::cout << "The device is connected to a hub without the necessary bandwidth requirements." << std::endl;
            kinectTool->mKinectErrorMessage = INSUFFICENT_BANDWITH;
        }
        else if (kinectStatus == E_NUI_NOTPOWERED) {
            std::cout << "The device is connected, but unpowered." << std::endl;
            kinectTool->mKinectErrorMessage = NOT_POWERED;
        }
        else if (kinectStatus == E_NUI_NOTREADY) {
            std::cout << "There was some other unspecified error. Device not ready. Try reconnecting the device." << std::endl;
            kinectTool->mKinectErrorMessage = NOT_READY;
        }
        else if (kinectStatus == E_NUI_DEVICE_IN_USE) {
            std::cout << "The device is in use." << std::endl;
            kinectTool->mKinectErrorMessage = DEVICE_IN_USE;
        }
    }
}

HRESULT FUKinectTool::createFirstConnected()
{
    INuiSensor *nuiSensor;
    HRESULT hr;
    int sensorCount = 0;
    hr = NuiGetSensorCount(&sensorCount);
    if (sensorCount == 0) {
        std::cout << "No connected device!" << std::endl;
        mKinectErrorMessage = KINECT_STATUS::NOT_CONNECTED;
        return E_FAIL;
    }
    if (FAILED(hr))
        return hr;
    // Look at each Kinect sensor
    for (int i = 0; i < sensorCount; ++i) {
        // Create the sensor so we can check status, if we can't create it, move on to the next
        hr = NuiCreateSensorByIndex(i, &nuiSensor);
        if (FAILED(hr))
            continue;
        // Get the status of the sensor, and if connected, then we can initialize it
        hr = nuiSensor->NuiStatus();
        if (hr == S_OK) {
            if (mNuiSensor != nullptr) {
                mNuiSensor->NuiShutdown();
                mNuiSensor->Release();
                mNuiSensor = nullptr;
            }
            mNuiSensor = std::move(nuiSensor);
            break;
        }
        // This sensor wasn't OK, so release it since we're not using it
        nuiSensor->Release();
    }
    if (mNuiSensor != nullptr) {
        // Initialize the Kinect and specify that we'll be using color and skeleton tracking
        hr = mNuiSensor->NuiInitialize(mDWFlags);
        if (SUCCEEDED(hr)) {
            /**************************** Create Color ****************************************/
            // Create an event that will be signaled when color data is available
            mHandleNextColorFrameEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
            // Open a color image stream to receive color frames
            hr = mNuiSensor->NuiImageStreamOpen(
                        NUI_IMAGE_TYPE_COLOR,
                        NUI_IMAGE_RESOLUTION_1280x960,
                        0,
                        2,
                        mHandleNextColorFrameEvent,
                        &mHandleColorStream);
            /**************************** Create Skeleton ****************************************/
            // Create an event that will be signaled when skeleton data is available
            mHandleNextSkeletonEvent = CreateEventW(NULL, TRUE, FALSE, NULL);
            // Open a skeleton stream to receive skeleton data
            hr = mNuiSensor->NuiSkeletonTrackingEnable(mHandleNextSkeletonEvent, 0);

            /**************************** Create Interaction Stream ****************************************/
            // Create an event that will be signaled when skeleton data is available
            mHandleNextDepthFrameEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
            // Open a skeleton stream to receive skeleton data
            hr = mNuiSensor->NuiImageStreamOpen(
                        NUI_IMAGE_TYPE_DEPTH,
                        NUI_IMAGE_RESOLUTION_640x480,
                        0,
                        2,
                        mHandleNextDepthFrameEvent,
                        &mHandleDepthStream);

            hr = NuiCreateInteractionStream(mNuiSensor, mNuiInteractionClient, &mNuiInteractionStream);
            mHandleNextHandEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
            mNuiInteractionStream->Enable(mHandleNextHandEvent);

            mKinectErrorMessage = KINECT_STATUS::NO_PROBLEM;
        }
    }
    if (mNuiSensor == NULL || FAILED(hr)) {
        //No Kinect found
        return E_FAIL;
    }
    return hr;
}

void FUKinectTool::updateSensor()
{
    // Wait for 0ms, just quickly test if it is time to process a skeleton
    if (WaitForSingleObject(mHandleNextSkeletonEvent, 0) == WAIT_OBJECT_0)
        processSkeleton();
    // Wait for 0ms, just quickly test if it is time to process color
    if (WaitForSingleObject(mHandleNextColorFrameEvent, 0) == WAIT_OBJECT_0)
        processColor();
    // Wait for 0ms, just quickly test if it is time to process color
    if (WaitForSingleObject(mHandleNextHandEvent, 0) == WAIT_OBJECT_0)
        processInteraction();
    // Wait for 0ms, just quickly test if it is time to process color
    if (WaitForSingleObject(mHandleNextDepthFrameEvent, 0) == WAIT_OBJECT_0)
        processDepth();
}

void FUKinectTool::processInteraction()
{
    NUI_INTERACTION_FRAME interactionFrame = { 0 };
    HRESULT results = mNuiInteractionStream->GetNextFrame(0, &interactionFrame );

    if(FAILED(results))
        return;
    for(int i = 0; i < NUI_SKELETON_COUNT; i++) {
        NUI_USER_INFO user = interactionFrame.UserInfos[i];
        if (user.SkeletonTrackingId != 0) {
            for(int j = 0; j < NUI_USER_HANDPOINTER_COUNT; j++) {
                NUI_HANDPOINTER_INFO hand = user.HandPointerInfos[j];
                NUI_HANDPOINTER_STATE state  = (NUI_HANDPOINTER_STATE)hand.State;
                if(state & NUI_HANDPOINTER_STATE_PRIMARY_FOR_USER) {
                    if (mSkeletonDataOne != nullptr && user.SkeletonTrackingId == mSkeletonDataOne->dwTrackingID) {
                        mVectorSkeletonOneHandPosition.setX(hand.X);
                        mVectorSkeletonOneHandPosition.setY(hand.Y);
                    }
                    else if(mSkeletonDataTwo != nullptr && user.SkeletonTrackingId == mSkeletonDataTwo->dwTrackingID) {
                        mVectorSkeletonTwoHandPosition.setX(hand.X);
                        mVectorSkeletonTwoHandPosition.setY(hand.Y);
                    }
                }
                else {
                    if (mSkeletonDataOne != nullptr && user.SkeletonTrackingId == mSkeletonDataOne->dwTrackingID) {
                        mVectorSkeletonOneHandPosition.setX(0);
                        mVectorSkeletonOneHandPosition.setY(0);
                    }
                    else if(mSkeletonDataTwo != nullptr && user.SkeletonTrackingId == mSkeletonDataTwo->dwTrackingID) {
                        mVectorSkeletonTwoHandPosition.setX(0);
                        mVectorSkeletonTwoHandPosition.setY(0);
                    }
                }
                if(state & NUI_HANDPOINTER_STATE_PRESSED) {
                    if (mSkeletonDataOne != nullptr && user.SkeletonTrackingId == mSkeletonDataOne->dwTrackingID)
                        mIsPressedPlayerOne = true;
                    else if(mSkeletonDataTwo != nullptr && user.SkeletonTrackingId == mSkeletonDataTwo->dwTrackingID)
                        mIsPressedPlayerTwo = true;
                }
                else {
                    if (mSkeletonDataOne != nullptr && user.SkeletonTrackingId == mSkeletonDataOne->dwTrackingID)
                        mIsPressedPlayerOne = false;
                    else if(mSkeletonDataTwo != nullptr && user.SkeletonTrackingId == mSkeletonDataTwo->dwTrackingID)
                        mIsPressedPlayerTwo = false;
                }

                if(hand.State != NUI_HANDPOINTER_STATE_NOT_TRACKED) {
                    switch(hand.HandEventType) {
                    case NUI_HAND_EVENT_TYPE_NONE:
                        //                        std::cout << hand.HandType << " NUI_HAND_EVENT_TYPE_NONE" << std::endl; //This never output
                        break;
                    case NUI_HAND_EVENT_TYPE_GRIP:
                        std::cout << hand.HandType << " NUI_HAND_EVENT_TYPE_GRIP" << std::endl; //This never output
                        break;
                    case NUI_HAND_EVENT_TYPE_GRIPRELEASE:
                        //                        std::cout << hand.HandType << " NUI_HAND_EVENT_TYPE_GRIPRELEASE" << std::endl; //TODO: This never outputs
                        break;
                    }
                }
            }
        }
    }
}

void FUKinectTool::processDepth()
{
    HRESULT hr;
    NUI_IMAGE_FRAME imageFrame;
    // Attempt to get the depth frame
    hr = mNuiSensor->NuiImageStreamGetNextFrame(mHandleDepthStream, 0, &imageFrame);
    if (FAILED(hr))
        return;

    BOOL nearMode;
    INuiFrameTexture* pTexture;
    // Get the depth image pixel texture
    hr = mNuiSensor->NuiImageFrameGetDepthImagePixelFrameTexture(mHandleDepthStream, &imageFrame, &nearMode, &pTexture);
    if (FAILED(hr)) {
        mNuiSensor->NuiImageStreamReleaseFrame(mHandleDepthStream, &imageFrame);
        return;
    }
    NUI_LOCKED_RECT LockedRect;
    // Lock the frame data so the Kinect knows not to modify it while we're reading it
    pTexture->LockRect(0, &LockedRect, NULL, 0);
    // Make sure we've received valid data
    if (LockedRect.Pitch != 0) {
        mNuiInteractionStream->ProcessDepth(LockedRect.size,LockedRect.pBits, imageFrame.liTimeStamp);
    }

    // We're done with the texture so unlock it
    pTexture->UnlockRect(0);
    pTexture->Release();
    // Release the frame
    mNuiSensor->NuiImageStreamReleaseFrame(mHandleDepthStream, &imageFrame);
}

void FUKinectTool::processSkeleton()
{
    //TODO: Test this!
    std::vector<NUI_SKELETON_DATA*> skeletonVector;
    HRESULT hr = mNuiSensor->NuiSkeletonGetNextFrame(0, &mSkeletonFrame);
    if (FAILED(hr))
        return;
    // smooth out the skeleton data
    mNuiSensor->NuiTransformSmooth(&mSkeletonFrame, NULL);
    for (int i = 0 ; i < NUI_SKELETON_COUNT; ++i) {
        NUI_SKELETON_TRACKING_STATE trackingState = mSkeletonFrame.SkeletonData[i].eTrackingState;
        if (trackingState == NUI_SKELETON_TRACKED) {
            // We're tracking the skeleton
            skeletonVector.push_back(std::move(&mSkeletonFrame.SkeletonData[i]));
        }
        else if (trackingState == NUI_SKELETON_POSITION_ONLY) {
            // we've only received the center point of the skeleton
        }
    }
    mSkeletonDataOne = nullptr;
    mSkeletonDataTwo = nullptr;
    if (skeletonVector.size() == 1) {
        mSkeletonDataOne = std::move(skeletonVector.at(0));
        if (mSkeletonOneTrackingID != -1) {
            if (mSkeletonDataOne->dwTrackingID == mSkeletonOneTrackingID) {
                mSkeletonLeftScene = SKELETONS::SKELETON_TWO;
                mSkeletonTwoTrackingID = -1;//Indicating that there's no skeleton two
            }
            else {
                mSkeletonLeftScene = SKELETONS::SKELETON_ONE;
                mSkeletonOneTrackingID = -1;//Indicating that there's no skeleton two
            }
        }
        mSkeletonOneTrackingID = mSkeletonDataOne->dwTrackingID;
    }
    else if (skeletonVector.size() == 2) {
        mSkeletonLeftScene = SKELETONS::NONE;//Both skeletons are visible
        if (isSkeletonOnRight(*skeletonVector.at(0), *skeletonVector.at(1))) {
            mSkeletonDataOne = std::move(skeletonVector.at(0));
            mSkeletonDataTwo = std::move(skeletonVector.at(1));
            mSkeletonOneTrackingID = mSkeletonDataOne->dwTrackingID;
            mSkeletonTwoTrackingID = mSkeletonDataTwo->dwTrackingID;
        }
        else {
            mSkeletonDataOne = std::move(skeletonVector.at(1));
            mSkeletonOneTrackingID = mSkeletonDataOne->dwTrackingID;
            mSkeletonDataTwo = std::move(skeletonVector.at(0));
            mSkeletonTwoTrackingID = mSkeletonDataTwo->dwTrackingID;
        }
    }
    if (mSkeletonDataOne == nullptr && mSkeletonDataTwo == nullptr)
        mSkeletonLeftScene = SKELETONS::BOTH_SKELETONS;
    Vector4 tempVec = {0};
    mNuiSensor->NuiAccelerometerGetCurrentReading(&tempVec);
    mNuiInteractionStream->ProcessSkeleton(NUI_SKELETON_COUNT, mSkeletonFrame.SkeletonData,&tempVec, mSkeletonFrame.liTimeStamp);
}

HRESULT FUKinectTool::getScreenshotFileName(wchar_t *screenshotName, UINT screenshotNameSize)
{
    wchar_t *knownPath = NULL;
    HRESULT hr = SHGetKnownFolderPath(FOLDERID_Pictures, 0, NULL, &knownPath);
    if (SUCCEEDED(hr))
    {
        // Get the time
        wchar_t timeString[MAX_PATH];
        GetTimeFormatEx(NULL, 0, NULL, L"hh'-'mm'-'ss", timeString, _countof(timeString));

        // File name will be KinectSnapshot-HH-MM-SS.wav
        StringCchPrintfW(screenshotName, screenshotNameSize, L"%s\\KinectSnapshot-%s.bmp", knownPath, timeString);
        std::cout << screenshotName << std::endl;
    }
    CoTaskMemFree(knownPath);
    return hr;
}

HRESULT FUKinectTool::saveBitmapToFile(BYTE* pBitmapBits, LONG lWidth, LONG lHeight, WORD wBitsPerPixel, LPCWSTR lpszFilePath)
{
    DWORD dwByteCount = lWidth * lHeight * (wBitsPerPixel / 8);

    BITMAPINFOHEADER bmpInfoHeader = {0};

    bmpInfoHeader.biSize        = sizeof(BITMAPINFOHEADER);  // Size of the header
    bmpInfoHeader.biBitCount    = wBitsPerPixel;             // Bit count
    bmpInfoHeader.biCompression = BI_RGB;                    // Standard RGB, no compression
    bmpInfoHeader.biWidth       = lWidth;                    // Width in pixels
    bmpInfoHeader.biHeight      = -lHeight;                  // Height in pixels, negative indicates it's stored right-side-up
    bmpInfoHeader.biPlanes      = 1;                         // Default
    bmpInfoHeader.biSizeImage   = dwByteCount;               // Image size in bytes

    BITMAPFILEHEADER bfh = {0};

    bfh.bfType    = 0x4D42;                                           // 'M''B', indicates bitmap
    bfh.bfOffBits = bmpInfoHeader.biSize + sizeof(BITMAPFILEHEADER);  // Offset to the start of pixel data
    bfh.bfSize    = bfh.bfOffBits + bmpInfoHeader.biSizeImage;        // Size of image + headers

    // Create the file on disk to write to
    HANDLE hFile = CreateFileW(lpszFilePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    // Return if error opening file
    if (NULL == hFile)
    {
        return E_ACCESSDENIED;
    }

    DWORD dwBytesWritten = 0;

    // Write the bitmap file header
    if ( !WriteFile(hFile, &bfh, sizeof(bfh), &dwBytesWritten, NULL) )
    {
        CloseHandle(hFile);
        return E_FAIL;
    }

    // Write the bitmap info header
    if ( !WriteFile(hFile, &bmpInfoHeader, sizeof(bmpInfoHeader), &dwBytesWritten, NULL) )
    {
        CloseHandle(hFile);
        return E_FAIL;
    }

    // Write the RGB Data
    if ( !WriteFile(hFile, pBitmapBits, bmpInfoHeader.biSizeImage, &dwBytesWritten, NULL) )
    {
        CloseHandle(hFile);
        return E_FAIL;
    }

    // Close the file
    CloseHandle(hFile);
    return S_OK;
}

void FUKinectTool::processColor()
{
    HRESULT hr;
    NUI_IMAGE_FRAME imageFrame;
    // Attempt to get the color frame
    hr = mNuiSensor->NuiImageStreamGetNextFrame(mHandleColorStream, 0, &imageFrame);
    if (FAILED(hr))
        return;

    INuiFrameTexture *frameTexture = imageFrame.pFrameTexture;
    NUI_LOCKED_RECT lockedRect;

    // Lock the frame data so the Kinect knows not to modify it while we're reading it
    frameTexture->LockRect(0, &lockedRect, NULL, 0);

    // Make sure we've received valid data
    if (lockedRect.Pitch != 0) {
        // Draw the data with Direct2D
        //        m_pDrawColor->Draw(static_cast<BYTE *>(lockedRect.pBits), lockedRect.size);

        // If the user pressed the screenshot button, save a screenshot
        if (mSaveScreenshot) {
            // Retrieve the path to My Photos
            WCHAR screenshotPath[MAX_PATH];
            getScreenshotFileName(screenshotPath, _countof(screenshotPath));
            // Write out the bitmap to disk
            hr = saveBitmapToFile(static_cast<BYTE *>(lockedRect.pBits), mColorWidth, mColorHeight, 32, screenshotPath);
            if (SUCCEEDED(hr))
                printf("SCREENSHOT SAVED!"); // Success!
            else
                printf("SCREENSHOT FAIL!");//FAIL!

            // toggle off so we don't save a screenshot again next frame
            mSaveScreenshot = false;
        }
    }

    // We're done with the texture so unlock it
    frameTexture->UnlockRect(0);
    // Release the frame
    mNuiSensor->NuiImageStreamReleaseFrame(mHandleColorStream, &imageFrame);
}

bool FUKinectTool::checkForSkeletonVisibility(NUI_SKELETON_DATA &skeletonData, NUI_SKELETON_FRAME &outFrameSkeleton)
{
    for (int  i = 0;i < 6;i++) {
        if (skeletonData.dwUserIndex == outFrameSkeleton.SkeletonData[i].dwUserIndex)
            return true;
    }
    return false;
}

bool FUKinectTool::isSkeletonTracked(NUI_SKELETON_DATA &skeletonData)
{
    return skeletonData.eTrackingState == SKELETON_TRACKED;
}

bool FUKinectTool::isSkeletonOnRight(NUI_SKELETON_DATA &skeletonDataOne, NUI_SKELETON_DATA &skeletonDataTwo)
{
    bool isUserOnRight = false;
    if (isSkeletonTracked(skeletonDataOne) == false && isSkeletonTracked(skeletonDataTwo) == false)
        return false;
    else {
        if (skeletonDataOne.SkeletonPositions[HIP_CENTER].x > skeletonDataTwo.SkeletonPositions[HIP_CENTER].x)
            isUserOnRight = true;
    }
    return isUserOnRight;
}

bool FUKinectTool::detectRightHandUpPosture(NUI_SKELETON_DATA &skeletonData)
{
    bool isRightHandUp = false;
    if (skeletonData.eTrackingState == SKELETON_TRACKED)
    {
        //Elbow should be under hand
        if (skeletonData.SkeletonPositions[ELBOW_RIGHT].y > skeletonData.SkeletonPositions[HAND_RIGHT].y)
            return false;
        if (skeletonData.SkeletonPositions[HAND_RIGHT].y - skeletonData.SkeletonPositions[HEAD].y > 200)
            isRightHandUp = true;
        else
            isRightHandUp = false;
    }
    return isRightHandUp;
}

bool FUKinectTool::detectLeftHandUpPosture(NUI_SKELETON_DATA &skeletonData)
{
    bool isLeftHandUp = false;
    if (skeletonData.eTrackingState == SKELETON_TRACKED)
    {
        //Elbow should be under hand
        if (skeletonData.SkeletonPositions[ELBOW_LEFT].y > skeletonData.SkeletonPositions[HAND_LEFT].y)
            return false;
        if (skeletonData.SkeletonPositions[HAND_LEFT].y - skeletonData.SkeletonPositions[HEAD].y > 200)
            isLeftHandUp = true;
        else
            isLeftHandUp = false;
    }
    return isLeftHandUp;
}

bool FUKinectTool::detectBothHandsUp(NUI_SKELETON_DATA &skeletonData)
{
    return detectRightHandUpPosture(skeletonData) && detectLeftHandUpPosture(skeletonData);
}

bool FUKinectTool::detectLeftHandDownPosture(NUI_SKELETON_DATA &skeletonData)
{
    bool isLeftHandDown = false;
    if (skeletonData.eTrackingState == SKELETON_TRACKED)
    {
        //Elbow should be under hand
        if (skeletonData.SkeletonPositions[ELBOW_LEFT].y < skeletonData.SkeletonPositions[HAND_LEFT].y)
            return false;
        if (skeletonData.SkeletonPositions[HAND_LEFT].y < skeletonData.SkeletonPositions[HIP_CENTER].y)
            isLeftHandDown = true;
        else
            isLeftHandDown = false;
    }
    return isLeftHandDown;
}

bool FUKinectTool::detectRightHandDownPosture(NUI_SKELETON_DATA &skeletonData)
{
    bool isRightHandDown = false;
    if (skeletonData.eTrackingState == SKELETON_TRACKED)
    {
        //Elbow should be under hand
        if (skeletonData.SkeletonPositions[ELBOW_RIGHT].y < skeletonData.SkeletonPositions[HAND_RIGHT].y)
            return false;
        if (skeletonData.SkeletonPositions[HAND_RIGHT].y < skeletonData.SkeletonPositions[HIP_CENTER].y)
            isRightHandDown = true;
        else
            isRightHandDown = false;
    }
    return isRightHandDown;
}

bool FUKinectTool::detectOpenRightArm(NUI_SKELETON_DATA &skeletonData)
{
    if (!isSkeletonTracked(skeletonData))
        return false;
    bool armOpen = true;
    const Vector4 rightHandPoint = skeletonData.SkeletonPositions[HAND_RIGHT];
    const Vector4 rightElbowPoint = skeletonData.SkeletonPositions[ELBOW_RIGHT];
    const Vector4 rightShoulderPoint = skeletonData.SkeletonPositions[SHOULDER_RIGHT];
    //If elbow is below the shoulder, return false
    if (rightShoulderPoint.y - rightElbowPoint.y > 150)
        return false;
    //If elbow is over shoulder, return false
    if (rightShoulderPoint.y - rightElbowPoint.y < -150)
        return false;
    //If hand is below elbow, return false
    if (rightHandPoint.y - rightElbowPoint.y > 150)
        return false;
    //If the hand is over elbow, return false
    if (rightHandPoint.y - rightElbowPoint.y < -150)
        return false;
    return armOpen;
}

bool FUKinectTool::detectOpenLeftArm(NUI_SKELETON_DATA &skeletonData)
{
    if (!isSkeletonTracked(skeletonData))
        return false;
    bool armOpen = true;
    const Vector4 leftHandPoint = skeletonData.SkeletonPositions[HAND_RIGHT];
    const Vector4 leftElbowPoint = skeletonData.SkeletonPositions[ELBOW_RIGHT];
    const Vector4 leftShoulderPoint = skeletonData.SkeletonPositions[SHOULDER_RIGHT];
    //If elbow is below the shoulder, return false
    if (leftShoulderPoint.y - leftElbowPoint.y > 150)
        return false;
    //If elbow is over shoulder, return false
    if (leftShoulderPoint.y - leftElbowPoint.y < -150)
        return false;
    //If hand is below elbow, return false
    if (leftHandPoint.y - leftElbowPoint.y > 150)
        return false;
    //If the hand is over elbow, return false
    if (leftHandPoint.y - leftElbowPoint.y < -150)
        return false;
    return armOpen;
}

bool FUKinectTool::detectOpenArms(NUI_SKELETON_DATA &skeletonData)
{
    return detectOpenLeftArm(skeletonData) && detectOpenRightArm(skeletonData);
}

bool FUKinectTool::detectPush(DWORD skeletonTrackingID)
{
    bool isPressed = false;
    if (mSkeletonDataOne != nullptr && skeletonTrackingID == mSkeletonDataOne->dwTrackingID)
        isPressed = mIsPressedPlayerOne;
    else if (mSkeletonDataTwo != nullptr && skeletonTrackingID == mSkeletonDataTwo->dwTrackingID)
        isPressed = mIsPressedPlayerTwo;
    return isPressed;
}

bool FUKinectTool::detectGrip(DWORD skeletonTrackingID)
{
    bool isPressed = false;
    if (mSkeletonDataOne != nullptr && skeletonTrackingID == mSkeletonDataOne->dwTrackingID)
        isPressed = mIsGrippingPlayerOne;
    else if (mSkeletonDataTwo != nullptr && skeletonTrackingID == mSkeletonDataTwo->dwTrackingID)
        isPressed = mIsGrippingPlayerTwo;
    return isPressed;
}

bool FUKinectTool::detectLeanRight(NUI_SKELETON_DATA &skeletonData)
{
    if (isSkeletonTracked(skeletonData))
        return false;
    bool isLeaningRight = true;
    const Vector4 headPosition = skeletonData.SkeletonPositions[HEAD];
    const Vector4 hipCenterPosition = skeletonData.SkeletonPositions[HIP_CENTER];
    if (headPosition.x <= hipCenterPosition.x || headPosition.x - hipCenterPosition.x < 100)
        return false;
    return isLeaningRight;
}

bool FUKinectTool::detectLeanLeft(NUI_SKELETON_DATA &skeletonData)
{
    if (isSkeletonTracked(skeletonData))
        return false;
    bool isLeaningLeft = true;
    const Vector4 headPosition = skeletonData.SkeletonPositions[HEAD];
    const Vector4 hipCenterPosition = skeletonData.SkeletonPositions[HIP_CENTER];
    if (headPosition.x >= hipCenterPosition.x || hipCenterPosition.x - headPosition.x < 100)
        return false;
    return isLeaningLeft;
}

float FUKinectTool::getRightHandAngle(NUI_SKELETON_DATA &skeletonData)
{
    float angle = 0;
    const Vector4 righthandPosition = skeletonData.SkeletonPositions[HAND_RIGHT];
    const Vector4 rightShoulderPosition = skeletonData.SkeletonPositions[SHOULDER_RIGHT];
    float x = righthandPosition.x - rightShoulderPosition.x;
    float y = righthandPosition.y - rightShoulderPosition.y;
    FUMath::FUVector2<float> vector1(x, y);
    FUMath::FUVector2<float> vector2(x, 0);
    float vectorInnerProduct = vector1.getX() * vector2.getX() + vector1.getY() * vector2.getY();
    float lengthProduct = vector1.length() * vector2.length();
    angle = std::acos(vectorInnerProduct / lengthProduct) * 180 / 3.14159f;
    if (righthandPosition.y >= rightShoulderPosition.y && righthandPosition.x >= rightShoulderPosition.x)
        angle = 270 - angle;
    else if (righthandPosition.y <= rightShoulderPosition.y && righthandPosition.x >= rightShoulderPosition.x)
        angle += 270;
    else if (righthandPosition.y <= rightShoulderPosition.y && righthandPosition.x <= rightShoulderPosition.x)
        angle = 90 - angle;
    else if (righthandPosition.y >= rightShoulderPosition.y && righthandPosition.x <= rightShoulderPosition.x)
        angle = 90 + angle;
    return angle;
}

float FUKinectTool::getLeftHandAngle(NUI_SKELETON_DATA &skeletonData)
{
    float angle = 0;
    const Vector4 lefthandPosition = skeletonData.SkeletonPositions[HAND_LEFT];
    const Vector4 leftShoulderPosition = skeletonData.SkeletonPositions[SHOULDER_LEFT];
    float x = lefthandPosition.x - leftShoulderPosition.x;
    float y = lefthandPosition.y - leftShoulderPosition.y;
    FUMath::FUVector2<float> vector1(x, y);
    FUMath::FUVector2<float> vector2(x, 0);
    float vectorInnerProduct = vector1.getX() * vector2.getX() + vector1.getY() * vector2.getY();
    float lengthProduct = vector1.length() * vector2.length();
    angle = std::acos(vectorInnerProduct / lengthProduct) * 180 / 3.14159f;
    if (lefthandPosition.y >= leftShoulderPosition.y && lefthandPosition.x >= leftShoulderPosition.x)
        angle = 270 - angle;
    else if (lefthandPosition.y <= leftShoulderPosition.y && lefthandPosition.x >= leftShoulderPosition.x)
        angle += 270;
    else if (lefthandPosition.y <= leftShoulderPosition.y && lefthandPosition.x <= leftShoulderPosition.x)
        angle = 90 - angle;
    else if (lefthandPosition.y >= leftShoulderPosition.y && lefthandPosition.x <= leftShoulderPosition.x)
        angle = 90 + angle;
    return angle;
}

int FUKinectTool::getSkeletonCount(NUI_SKELETON_FRAME &sFrame)
{
    NUI_SKELETON_FRAME &skeletonFrame = sFrame;
    int count = 0;
    // smooth out the skeleton data
    mNuiSensor->NuiTransformSmooth(&skeletonFrame, NULL);
    for (int i = 0 ; i < NUI_SKELETON_COUNT; ++i) {
        NUI_SKELETON_TRACKING_STATE trackingState = skeletonFrame.SkeletonData[i].eTrackingState;
        if (trackingState == NUI_SKELETON_TRACKED) {
            count++;
        }
    }
    printf("COUNT: %d\n");
    return count;
}

bool FUKinectTool::isFloorVisible()
{
    // smooth out the skeleton data
    mNuiSensor->NuiTransformSmooth(&mSkeletonFrame, NULL);
    bool floor = false;
    if (mSkeletonFrame.vFloorClipPlane.x != 0 && mSkeletonFrame.vFloorClipPlane.y != 0
            && mSkeletonFrame.vFloorClipPlane.z != 0 && mSkeletonFrame.vFloorClipPlane.w != 0)
        floor = true;
    return floor;
}

double FUKinectTool::getDistanceFromFloor(Vector4 jointPosition)
{
    // smooth out the skeleton data
    mNuiSensor->NuiTransformSmooth(&mSkeletonFrame, NULL);
    //If floor isn't visible, can't get the distance
    if (!isFloorVisible()) {
        printf("FLOOR NOT VISIBLE!!!!\n");
        return -1;
    }
    double distanceFromFloor = 0;
    double floorClipPlaneX = mSkeletonFrame.vFloorClipPlane.x;
    double floorClipPlaneY = mSkeletonFrame.vFloorClipPlane.y;
    double floorClipPlaneZ = mSkeletonFrame.vFloorClipPlane.z;
    double floorClipPlaneW = mSkeletonFrame.vFloorClipPlane.w;
    distanceFromFloor = floorClipPlaneX * jointPosition.x + floorClipPlaneY * jointPosition.y + floorClipPlaneZ * jointPosition.z + floorClipPlaneW;
    return distanceFromFloor;
}

//TODO: Don't count it as jumping while getting close to Kinect
bool FUKinectTool::detectJumping(NUI_SKELETON_DATA &skeletonData)
{
    bool playerJump = false;
    if (skeletonData.dwTrackingID == mSkeletonDataOne->dwTrackingID)
        playerJump = mJumpedPlayerOne;
    else if (skeletonData.dwTrackingID == mSkeletonDataTwo->dwTrackingID)
        playerJump = mJumpedPlayerTwo;
    bool didJump = false;
    //If floor isn't visible, can't do anything
    if (!isFloorVisible())
        return false;
    if (playerJump == true && getDistanceFromFloor(skeletonData.SkeletonPositions[FOOT_LEFT]) < 0.02
            && getDistanceFromFloor(skeletonData.SkeletonPositions[FOOT_RIGHT]) < 0.02)
    {
        if (skeletonData.dwTrackingID == mSkeletonDataOne->dwTrackingID)
            mJumpedPlayerOne = false;
        else if (skeletonData.dwTrackingID == mSkeletonDataTwo->dwTrackingID)
            mJumpedPlayerTwo = false;
        return false;
    }
    if (playerJump == true)
        return false;
    if (playerJump == false && getDistanceFromFloor(skeletonData.SkeletonPositions[FOOT_LEFT]) > 0.06
            && getDistanceFromFloor(skeletonData.SkeletonPositions[FOOT_RIGHT]) > 0.06)
    {
        if (skeletonData.dwTrackingID == mSkeletonDataOne->dwTrackingID)
            mJumpedPlayerOne = true;
        else if (skeletonData.dwTrackingID == mSkeletonDataTwo->dwTrackingID)
            mJumpedPlayerTwo = true;
        didJump = true;
    }
    return didJump;
}

FUMath::FUVector2<float> FUKinectTool::getHandPosition(DWORD skeletonTrackingID)
{
    FUMath::FUVector2<float> position;
    if (mSkeletonDataOne != nullptr && skeletonTrackingID == mSkeletonDataOne->dwTrackingID)
        position = mVectorSkeletonOneHandPosition;
    else if (mSkeletonDataTwo != nullptr && skeletonTrackingID == mSkeletonDataTwo->dwTrackingID)
        position = mVectorSkeletonTwoHandPosition;
    return position;
}

void FUKinectTool::safeReleaseSensor()
{
    mNuiSensor->NuiShutdown();
    mNuiSensor->Release();
    mNuiSensor = nullptr;
}
