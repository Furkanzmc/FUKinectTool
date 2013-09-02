FFUKinectTool
=============

FUKinectTool - A class to detect gestures and postures and it has some helper functions for Kinect camera functions
It's not perfect, it may even fall below average good but it gets the job done. I haven't updated it for a long time,
although it workd with Microsoft Kinect SDK v1.7.


Classes
=============
- FUIInteractionClient - Provides a dummy client for InteractionStream
- FUKinectHelper - Some Regularly Used Functions in Kinect Programming. Use only one instance for all functions.
- FUHandGestureDetector - Used for detecting gestures involving only hands. Such as swipe gestures, grip gestures, pinch gesture
- FUPostureDetector - Used for detecting postures, like hand up, foot up etc...

FUKinectHelper
=============
- bool TakeColorPicture()
- bool TakeDepthPicture()
- BitmapSource GetKinectColorCameraFrame( ColorImageFrame colorImageFrame )
- WriteableBitmap GetKinectDepthCameraFrame( DepthImageFrame depthImageFrame )
- byte[] GenerateColoredPixels( DepthImageFrame depthFrame )
- Skeleton[] GetTrackedSkeletons(SkeletonFrame skeletonFrame)
- bool IsFloorVisible(SkeletonFrameReadyEventArgs e)
- double GetDistanceFromFloor(SkeletonFrameReadyEventArgs e, Joint joint)

FUHandGestureDetector
=============
- bool DetectSwipeLeftGesture(Skeleton skeleton)
- bool DetectSwipeRightGesture(Skeleton skeleton)
- Dictionary<int, Dictionary<InteractionHandType, InteractionHandEventType>> DetectGripping(InteractionFrame interactionFrame)
- Dictionary<int, Dictionary<InteractionHandType, bool>> DetectPressing(InteractionFrame interactionFrame)

FUPostureDetector
=============
- bool DetectGreaterPosture(Skeleton skeleton)
- bool DetectLesserPosture(Skeleton skeleton)
- bool DetectJumping( SkeletonFrameReadyEventArgs e, Skeleton skeleton )
- bool DetectRightFootUpPosture( SkeletonFrameReadyEventArgs e, Skeleton skeleton )
- bool DetectLeftFootUpPosture( SkeletonFrameReadyEventArgs e, Skeleton skeleton )
- bool DetectLeftHandUpPosture( Skeleton skeleton )
- bool DetectRightHandUpPosture( Skeleton skeleton )
- bool DetectLeftHandDownPosture(Skeleton skeleton)
- bool DetectRightHandDownPosture(Skeleton skeleton)
- bool DetectBothHandsUpPosture(Skeleton skeleton)
- double GetDistanceFromFloor( SkeletonFrameReadyEventArgs e, Joint joint )
- bool DetectOpenRightArm( Skeleton skeleton )
- bool DetectOpenLeftArm( Skeleton skeleton )
- bool DetectOpenArms( Skeleton skeleton )
- bool DetectCrouching( SkeletonFrameReadyEventArgs e, Skeleton skeleton )
- bool DetectStanding( SkeletonFrameReadyEventArgs e, Skeleton skeleton )
- bool DetectBendOver( Skeleton skeleton )
- double[] DetectRightHandAngle( Skeleton skeleton )
- double[] DetectLeftHandAngle( Skeleton skeleton )
- bool DetectCrossPosture( Skeleton skeleton )
- bool DetectSlashPosture( Skeleton skeleton )
- bool DetectSlashPostureForRightArm( Skeleton skeleton )
- bool DetectSlashPostureForLeftArm( Skeleton skeleton )
- bool DetectMinusPostureForRightArm( Skeleton skeleton )
- bool DetectMinusPostureForLeftArm(Skeleton skeleton)
- bool DetectMinusPosture( Skeleton skeleton )
- bool DetectPlusPosture( Skeleton skeleton )
