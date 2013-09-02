using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Media.Imaging;
using System.Windows.Media.Media3D;
using System.IO;
using Microsoft.Kinect;
using Microsoft.Kinect.Toolkit;
using Microsoft.Kinect.Toolkit.Interaction;

namespace FUKinectTool
{
    /// <summary>
    /// Some Regularly Used Functions in Kinect Programming. Use only one instance for all functions.
    /// TakeColorPicture()
    /// TakeDepthPicture()
    /// GetKinectColorCameraFrame()
    /// GetKinectColorCameraFrame( ColorImageFrame colorImageFrame )
    /// GetKinectDepthCameraFrame( DepthImageFrame depthImageFrame )
    /// </summary>
    public class FUKinectHelper
    {
        #region Kinect Camera Methods
        /// <summary>
        /// Bitmap that will hold color information
        /// </summary>
        private WriteableBitmap _colorBitmap;
        /// <summary>
        /// Bitmap that will hold depth information
        /// </summary>
        private WriteableBitmap _depthBitmap;

        /// <summary>
        /// Takes image from the color camera. Only available If you're getting the color image frame from FUKinectHelper
        /// </summary>
        public bool TakeColorPicture()
        {
            if (_colorBitmap == null)
                return false;
            // create a png bitmap encoder which knows how to save a .png file
            BitmapEncoder encoder = new PngBitmapEncoder();
            // create frame from the writable bitmap and add to encoder
            encoder.Frames.Add( BitmapFrame.Create( _colorBitmap ) );
            string time = System.DateTime.Now.ToString( "hh'-'mm'-'ss", System.Globalization.CultureInfo.CurrentUICulture.DateTimeFormat );
            string myPhotos = Environment.GetFolderPath( Environment.SpecialFolder.MyPictures );
            string path = Path.Combine( myPhotos, "KinectColorSnapshot-" + time + ".png" );
            // write the new file to disk
            try
            {
                using (FileStream fs = new FileStream( path, FileMode.Create ))
                {
                    encoder.Save( fs );
                }
            }
            catch
            {
                //TODO: Find a way to show error message
                return false;
            }
            return true;
        }

        /// <summary>
        /// Takes image from the depth camera. Only available If you're getting the depth image frame from FUKinectHelper
        /// </summary>
        public bool TakeDepthPicture()
        {
            if (_depthBitmap == null)
                return false;
            // create a png bitmap encoder which knows how to save a .png file
            BitmapEncoder encoder = new PngBitmapEncoder();
            // create frame from the writable bitmap and add to encoder
            encoder.Frames.Add( BitmapFrame.Create( _depthBitmap ) );
            string time = System.DateTime.Now.ToString( "hh'-'mm'-'ss", System.Globalization.CultureInfo.CurrentUICulture.DateTimeFormat );
            string myPhotos = Environment.GetFolderPath( Environment.SpecialFolder.MyPictures );
            string path = Path.Combine( myPhotos, "KinectDepthSnapshot-" + time + ".png" );
            // write the new file to disk
            try
            {
                using (FileStream fs = new FileStream( path, FileMode.Create ))
                {
                    encoder.Save( fs );
                }
            }
            catch
            {
                //TODO: Find a way to show error message
                return false;
            }
            return true;
        }

        /// <summary>
        /// Takes ColorImageFrame and returns a BitmapSource containing Kinect Color Camera Frame
        /// </summary>
        /// <param name="e">ColorImageFrame</param>
        byte[] _colorPixels = null;
        public BitmapSource GetKinectColorCameraFrame( ColorImageFrame colorImageFrame )
        {
            if (colorImageFrame == null)
                return null;
            if (_colorBitmap == null)
                _colorBitmap = new WriteableBitmap( colorImageFrame.Width, colorImageFrame.Height, 96.0, 96.0, System.Windows.Media.PixelFormats.Bgr32, null );
            if (_colorPixels == null)
                _colorPixels = new byte[colorImageFrame.PixelDataLength];
            colorImageFrame.CopyPixelDataTo( _colorPixels );
            // Write the pixel data into our bitmap
            _colorBitmap.WritePixels( new System.Windows.Int32Rect( 0, 0, _colorBitmap.PixelWidth, _colorBitmap.PixelHeight ), _colorPixels,
                _colorBitmap.PixelWidth * sizeof( int ), 0 );
            return _colorBitmap;
        }

        /// <summary>
        /// Takes DepthImageFrame and returns a BitmapSource containing Kinect Depth Camera Frame
        /// </summary>
        /// <param name="e">DepthImageFrame</param>
        byte[] _depthPixels;
        public WriteableBitmap GetKinectDepthCameraFrame( DepthImageFrame depthImageFrame )
        {
            if (depthImageFrame == null)
                return null;
            if (_depthBitmap == null)
                _depthBitmap = new WriteableBitmap( depthImageFrame.Width, depthImageFrame.Height, 96, 96, System.Windows.Media.PixelFormats.Bgr32, null );
            _depthPixels = GenerateColoredPixels( depthImageFrame );
            _depthBitmap.WritePixels( new System.Windows.Int32Rect( 0, 0, _depthBitmap.PixelWidth, _depthBitmap.PixelHeight ), _depthPixels,
                _depthBitmap.PixelWidth * sizeof( int ), 0 );
            return _depthBitmap;
        }

        /// <summary>
        /// Generate different colored pixels for different players
        /// </summary>
        /// <param name="depthFrame">DepthImageFrame</param>
        short[] _rawDepthFrameData;
        Byte[] _coloredPixels;
        public byte[] GenerateColoredPixels( DepthImageFrame depthFrame )
        {
            using (depthFrame)
            {
                if (_rawDepthFrameData == null)
                    _rawDepthFrameData = new short[depthFrame.PixelDataLength];
                depthFrame.CopyPixelDataTo( _rawDepthFrameData );
                if (_coloredPixels == null)
                    _coloredPixels = new Byte[depthFrame.Width * depthFrame.Height * 4];
                const int BlueIndex = 0;
                const int GreenIndex = 1;
                const int RedIndex = 2;
                for (int depthIndex = 0, colorIndex = 0; depthIndex < _rawDepthFrameData.Length && colorIndex < _coloredPixels.Length; depthIndex++, colorIndex += 4)
                {
                    int player = _rawDepthFrameData[depthIndex] & DepthImageFrame.PlayerIndexBitmask;
                    if (player > 0)
                    {
                        int blueColor = Convert.ToInt32( Math.Pow( player, 3 ) ) + 100;
                        int greenColor = Convert.ToInt32( Math.Pow( player, 3 ) ) + 180;
                        int redColor = Convert.ToInt32( Math.Pow( player, 3 ) ) + 200;
                        if (blueColor > 255)
                            blueColor = 255;
                        if (greenColor > 255)
                            greenColor = 255;
                        if (redColor > 255)
                            redColor = 255;
                        _coloredPixels[colorIndex + BlueIndex] = Convert.ToByte( blueColor );
                        _coloredPixels[colorIndex + GreenIndex] = Convert.ToByte( greenColor );
                        _coloredPixels[colorIndex + RedIndex] = Convert.ToByte( redColor );
                    }
                    else
                    {
                        _coloredPixels[colorIndex + BlueIndex] = 0;
                        _coloredPixels[colorIndex + GreenIndex] = 0;
                        _coloredPixels[colorIndex + RedIndex] = 0;
                    }
                }
                return _coloredPixels;
            }
        }
        #endregion

        /// <summary>
        /// Returns available skeletons
        /// </summary>
        /// <param name="e">SkeletonFrameReadyEventArgs</param>
        public Skeleton[] GetTrackedSkeletons(SkeletonFrame skeletonFrame)
        {
            using (skeletonFrame)
            {
                if (skeletonFrame == null)
                    return null;
                Skeleton[] allSkeletons = new Skeleton[6];
                skeletonFrame.CopySkeletonDataTo(allSkeletons);
                int playerCount = (from s in allSkeletons where s.TrackingState == SkeletonTrackingState.Tracked select s).Count();
                if (playerCount == 0)
                    return null;
                Skeleton[] trackedSkeletons = new Skeleton[playerCount];
                for (int i = 0; i < playerCount; i++)
                    trackedSkeletons[i] = (from s in allSkeletons where s.TrackingState == SkeletonTrackingState.Tracked select s).ElementAt(i);
                return trackedSkeletons;
            }
        }

        /// <summary>
        /// Returns tru if the floor is visible, otherwise returns false
        /// </summary>
        /// <param name="e">SkeletonFrameReadyEventArgs</param>
        public bool IsFloorVisible(SkeletonFrameReadyEventArgs e)
        {
            using (SkeletonFrame skeletonFrame = e.OpenSkeletonFrame())
            {
                if (skeletonFrame == null || e == null)
                    return false;
                bool floorVisibility = false;
                if (skeletonFrame.FloorClipPlane.Item1 == 0 && skeletonFrame.FloorClipPlane.Item2 == 0 && skeletonFrame.FloorClipPlane.Item3 == 0 && skeletonFrame.FloorClipPlane.Item4 == 0)
                    floorVisibility = false;
                else
                    floorVisibility = true;
                return floorVisibility;
            }
        }

        /// <summary>
        /// Takes a SkeletonFrameReadyEventArgs and a joint and return the distance of that joint to floor if the floor is visible, otherwise returns -1
        /// </summary>
        /// <param name="skeleton">Tracked skeleton.</param>
        public double GetDistanceFromFloor(SkeletonFrameReadyEventArgs e, Joint joint)
        {
            using (SkeletonFrame skeletonData = e.OpenSkeletonFrame())
            {
                if (skeletonData == null || joint == null)
                    return -1;
                //If floor isn't visible, can't get the distance
                if (IsFloorVisible(e) == false)
                    return -1;
                double distanceFromFloor = 0.0d;
                double floorClipPlaneX = skeletonData.FloorClipPlane.Item1;
                double floorClipPlaneY = skeletonData.FloorClipPlane.Item2;
                double floorClipPlaneZ = skeletonData.FloorClipPlane.Item3;
                double floorClipPlaneW = skeletonData.FloorClipPlane.Item4;

                double hipCenterX = joint.Position.X;
                double hipCenterY = joint.Position.Y;
                double hipCenterZ = joint.Position.Z;
                distanceFromFloor = floorClipPlaneX * hipCenterX + floorClipPlaneY * hipCenterY + floorClipPlaneZ * hipCenterZ + floorClipPlaneW;
                return distanceFromFloor;
            }
        }
    }

    /// <summary>
    /// Provides a dummy client for InteractionStream
    /// </summary>
    public abstract class FUIInteractionClient
    {
        public class DummyInteractionClient : Microsoft.Kinect.Toolkit.Interaction.IInteractionClient
        {
            Microsoft.Kinect.Toolkit.Interaction.InteractionInfo Microsoft.Kinect.Toolkit.Interaction.IInteractionClient.GetInteractionInfoAtLocation(int skeletonTrackingId, InteractionHandType handType, double x, double y)
            {
                Microsoft.Kinect.Toolkit.Interaction.InteractionInfo result = new Microsoft.Kinect.Toolkit.Interaction.InteractionInfo();
                result.IsGripTarget = true;
                result.IsPressTarget = true;
                result.PressAttractionPointX = 0.5;
                result.PressAttractionPointY = 0.5;
                result.PressTargetControlId = 1;

                return result;
            }
        }

        public sealed class InteractionInfo
        {
            public bool IsPressTarget { get; set; }
            public int PressTargetControlId { get; set; }
            public double PressAttractionPointX { get; set; }
            public double PressAttractionPointY { get; set; }
            public bool IsGripTarget { get; set; }
        }

        public interface IInteractionClient
        {
            InteractionInfo GetInteractionInfoAtLocation(int skeletonTrackingId, InteractionHandType handType, double x, double y);
        }
    }

    /// <summary>
    /// Used for detecting gestures involving only hands. Such as swipe gestures, grip gestures, pinch gesture
    /// </summary>
    public class FUHandGestureDetector
    {
        #region Swipe Detection
        const int _SwipeMinimalDuration = 100;
        const int _SwipeMaximalDuration = 1000;
        DateTime _startTimeSwipeRight = DateTime.Today, _startTimeSwipeLeft = DateTime.Today;
        bool _isOnRight = false;
        bool _isOnLeft = false;

        /// <summary>
        /// Takes a skeleton hand returns true when swipe left gesture is performed
        /// </summary>
        /// <param name="skeleton">Tracked skeleton.</param>
        public bool DetectSwipeLeftGesture(Skeleton skeleton)
        {
            if (skeleton == null)
                return false;
            bool swipeDetected = false;

            #region Swipe Availability Check
            //If the hand is below the elbow, no swipe gesture...
            if (skeleton.Joints[JointType.HandRight].Position.Y < skeleton.Joints[JointType.ElbowRight].Position.Y)
            {
                _startTimeSwipeRight = DateTime.Today;
                _isOnRight = false;
                return false;
            }
            //If the hand is over the head, no swipe gesture...
            if (skeleton.Joints[JointType.HandRight].Position.Y > skeleton.Joints[JointType.Head].Position.Y)
            {
                _startTimeSwipeRight = DateTime.Today;
                _isOnRight = false;
                return false;
            }
            //If the hand is below hip, no swipe gesture...
            if (skeleton.Joints[JointType.HandRight].Position.Y < skeleton.Joints[JointType.HipCenter].Position.Y)
            {
                _startTimeSwipeRight = DateTime.Today;
                _isOnRight = false;
                return false;
            }
            SkeletonPoint rightHandPosition = skeleton.Joints[JointType.HandRight].Position;
            SkeletonPoint rightShoulderPosition = skeleton.Joints[JointType.ShoulderRight].Position;
            //If hand is on the right of the right shoulder, start the gesture scanning when the right hand is on the soulder
            if (rightHandPosition.X >= rightShoulderPosition.X)
            {
                //If the hand is too far on the right, no swipe gesture...
                if (rightHandPosition.X - rightShoulderPosition.X > rightShoulderPosition.X - skeleton.Joints[JointType.ShoulderCenter].Position.X)
                {
                    _startTimeSwipeRight = DateTime.Today;
                    _isOnRight = false;
                    return false;
                }
                _isOnRight = true;
            }
            #endregion

            if (_isOnRight)
            {
                if (_startTimeSwipeRight == DateTime.Today)
                    _startTimeSwipeRight = DateTime.Now;
                DateTime endTime = DateTime.Now;
                if (rightHandPosition.X <= skeleton.Joints[JointType.ShoulderCenter].Position.X && (endTime - _startTimeSwipeRight).TotalMilliseconds < _SwipeMaximalDuration
                    && (endTime - _startTimeSwipeRight).TotalMilliseconds > _SwipeMinimalDuration)
                {
                    swipeDetected = true;
                    _startTimeSwipeRight = DateTime.Today;
                    _isOnRight = false;
                }
            }
            return swipeDetected;
        }

        /// <summary>
        /// Takes a skeleton hand returns true when swipe right gesture is performed
        /// </summary>
        /// <param name="skeleton">Tracked skeleton.</param>
        public bool DetectSwipeRightGesture(Skeleton skeleton)
        {
            if (skeleton == null)
                return false;
            bool swipeDetected = false;

            #region Swipe Availability Check
            //If the hand is below the elbow, no swipe gesture...
            if (skeleton.Joints[JointType.HandLeft].Position.Y < skeleton.Joints[JointType.ElbowLeft].Position.Y)
            {
                _startTimeSwipeLeft = DateTime.Today;
                _isOnLeft = false;
                return false;
            }
            //If the hand is over the head, no swipe gesture...
            if (skeleton.Joints[JointType.HandLeft].Position.Y > skeleton.Joints[JointType.Head].Position.Y)
            {
                _startTimeSwipeLeft = DateTime.Today;
                _isOnLeft = false;
                return false;
            }
            //If the hand is below hip, no swipe gesture...
            if (skeleton.Joints[JointType.HandLeft].Position.Y < skeleton.Joints[JointType.HipCenter].Position.Y)
            {
                _startTimeSwipeLeft = DateTime.Today;
                _isOnLeft = false;
                return false;
            }
            SkeletonPoint leftHandPosition = skeleton.Joints[JointType.HandLeft].Position;
            SkeletonPoint leftShoulderPosition = skeleton.Joints[JointType.ShoulderLeft].Position;
            //If hand is on the left of the left shoulder, start the gesture scanning when the right hand is on the soulder
            if (leftHandPosition.X <= leftShoulderPosition.X)
            {
                //If the hand is too far on the right, no swipe gesture...
                if (leftHandPosition.X - leftShoulderPosition.X < leftShoulderPosition.X - skeleton.Joints[JointType.ShoulderCenter].Position.X)
                {
                    _startTimeSwipeLeft = DateTime.Today;
                    _isOnLeft = false;
                    return false;
                }
                _isOnLeft = true;
            }
            #endregion

            if (_isOnLeft)
            {
                if (_startTimeSwipeLeft == DateTime.Today)
                    _startTimeSwipeLeft = DateTime.Now;
                DateTime endTime = DateTime.Now;
                if (leftHandPosition.X >= skeleton.Joints[JointType.ShoulderCenter].Position.X && (endTime - _startTimeSwipeLeft).TotalMilliseconds < _SwipeMaximalDuration
                    && (endTime - _startTimeSwipeLeft).TotalMilliseconds > _SwipeMinimalDuration)
                {
                    swipeDetected = true;
                    _startTimeSwipeLeft = DateTime.Today;
                    _isOnLeft = false;
                }
            }
            return swipeDetected;
        }
        #endregion

        #region Grip Detection
        private UserInfo[] _userInfosGripping; //the information about the interactive users
        private Dictionary<int, InteractionHandEventType> _lastLeftHandEvents = new Dictionary<int, InteractionHandEventType>();
        private Dictionary<int, InteractionHandEventType> _lastRightHandEvents = new Dictionary<int, InteractionHandEventType>();
        /// <summary>
        /// Takes InteractionFrame and returns the gripping hands in a Dictionary.
        /// Integer value is the tracking ID of the skeleton and the nested Dictionary returns the InteractionHandEventType variable for the given InteractionHandType
        /// Returns null if the InteractionFrame is null
        /// </summary>
        /// <param name="interactionFrame">InteractionFrame</param>
        public Dictionary<int, Dictionary<InteractionHandType, InteractionHandEventType>> DetectGripping(InteractionFrame interactionFrame)
        {
            if (_userInfosGripping == null)
                _userInfosGripping = new UserInfo[InteractionFrame.UserInfoArrayLength];
            Dictionary<int, Dictionary<InteractionHandType, InteractionHandEventType>> gripDictionary = new Dictionary<int, Dictionary<InteractionHandType, InteractionHandEventType>>();
            using (interactionFrame)
            {
                if (interactionFrame == null)
                    return null;
                interactionFrame.CopyInteractionDataTo(_userInfosGripping);
            }
            foreach (UserInfo userInfo in _userInfosGripping)
            {
                int userID = userInfo.SkeletonTrackingId;
                if (userID == 0)
                    continue;
                Dictionary<InteractionHandType, InteractionHandEventType> handDictionary = new Dictionary<InteractionHandType, InteractionHandEventType>();
                var handPointers = userInfo.HandPointers;
                if (handPointers.Count() == 0)
                    continue;
                else
                {
                    if (handPointers.Count() == 2 && handPointers.ElementAt(0).HandType == InteractionHandType.None &&
                        handPointers.ElementAt(1).HandType == InteractionHandType.None)
                        continue;
                    foreach (var hand in handPointers)
                    {
                        var lastHandEvents = hand.HandType == InteractionHandType.Left ? _lastLeftHandEvents : _lastRightHandEvents;
                        if (hand.HandEventType != InteractionHandEventType.None)
                            lastHandEvents[userID] = hand.HandEventType;
                        var lastHandEvent = lastHandEvents.ContainsKey(userID) ? lastHandEvents[userID] : InteractionHandEventType.None;
                        handDictionary.Add(hand.HandType, lastHandEvent);
                    }
                }
                gripDictionary.Add(userID, handDictionary);
            }
            return gripDictionary;
        }
        #endregion

        #region Press Detection
        private UserInfo[] _userInfosPressing; //the information about the interactive users
        /// <summary>
        /// Takes InteractionFrame and returns the pressing hands in a Dictionary.
        /// Integer value is the tracking ID of the skeleton and the nested Dictionary returns the boolean variable for the given InteractionHandType
        /// Returns null if the InteractionFrame is null
        /// </summary>
        /// <param name="interactionFrame">InteractionFrame</param>
        public Dictionary<int, Dictionary<InteractionHandType, bool>> DetectPressing(InteractionFrame interactionFrame)
        {
            if (_userInfosPressing == null)
                _userInfosPressing = new UserInfo[InteractionFrame.UserInfoArrayLength];
            Dictionary<int, Dictionary<InteractionHandType, bool>> pressDictionary = new Dictionary<int, Dictionary<InteractionHandType, bool>>();
            using (interactionFrame)
            {
                if (interactionFrame == null)
                    return null;
                interactionFrame.CopyInteractionDataTo(_userInfosPressing);
            }
            foreach (UserInfo userInfo in _userInfosPressing)
            {
                int userID = userInfo.SkeletonTrackingId;
                if (userID == 0)
                    continue;
                Dictionary<InteractionHandType, bool> handDictionary = new Dictionary<InteractionHandType, bool>();
                var handPointers = userInfo.HandPointers;
                if (handPointers.Count() == 0)
                    continue;
                else
                {
                    if (handPointers.Count() == 2 && handPointers.ElementAt(0).HandType == InteractionHandType.None &&
                        handPointers.ElementAt(1).HandType == InteractionHandType.None)
                        continue;
                    foreach (var hand in handPointers)
                    {
                        handDictionary.Add(hand.HandType, hand.IsPressed);
                    }
                }
                pressDictionary.Add(userID, handDictionary);
            }
            return pressDictionary;
        }
        #endregion
    }

    /// <summary>
    /// Used for detecting postures, like hand up, foot up...
    /// </summary>
    public class FUPostureDetector
    {
        /// <summary>
        /// Detects the greater sign
        /// </summary>
        /// <param name="skeleton">Tracked skeleton</param>
        public bool DetectGreaterPosture(Skeleton skeleton)
        {
            if (skeleton == null)
                return false;
            SkeletonPoint leftHandPosition = skeleton.Joints[JointType.HandLeft].Position;
            SkeletonPoint rightHandPosition = skeleton.Joints[JointType.HandRight].Position;
            SkeletonPoint leftHipPosition = skeleton.Joints[JointType.HipLeft].Position;
            SkeletonPoint rightHipPosition = skeleton.Joints[JointType.HipRight].Position;
            SkeletonPoint rightShoulderPosition = skeleton.Joints[JointType.ShoulderRight].Position;
            SkeletonPoint leftShoulderPosition = skeleton.Joints[JointType.ShoulderLeft].Position;
            if ((leftHandPosition.X > leftShoulderPosition.X || leftHandPosition.Y - leftShoulderPosition.Y < 0.2))
                return false;
            if (Math.Abs(rightHandPosition.Y - leftHipPosition.Y) > 0.1 || rightHandPosition.X > leftHipPosition.X)
                return false;
            return true;
        }

        /// <summary>
        /// Detects the lesser sign
        /// </summary>
        /// <param name="skeleton">Tracked skeleton</param>
        public bool DetectLesserPosture(Skeleton skeleton)
        {
            if (skeleton == null)
                return false;
            SkeletonPoint leftHandPosition = skeleton.Joints[JointType.HandLeft].Position;
            SkeletonPoint rightHandPosition = skeleton.Joints[JointType.HandRight].Position;
            SkeletonPoint leftHipPosition = skeleton.Joints[JointType.HipLeft].Position;
            SkeletonPoint rightHipPosition = skeleton.Joints[JointType.HipRight].Position;
            SkeletonPoint rightShoulderPosition = skeleton.Joints[JointType.ShoulderRight].Position;
            if (rightHandPosition.X - rightHipPosition.X < 0.2)
                return false;
            if (leftHandPosition.X < rightShoulderPosition.X || leftHandPosition.Y - rightShoulderPosition.Y < 0.2)
                return false;
            return true;
        }

        //TODO: Don't count it as jumping while getting close to Kinect
        bool _jumped = false;

        /// <summary>
        /// Detects jumping
        /// </summary>
        /// <param name="e">SkeletonFrameReadyEventArgs</param>
        /// <param name="skeleton">Tracked skeleton</param>
        public bool DetectJumping( SkeletonFrameReadyEventArgs e, Skeleton skeleton )
        {
            bool didJump = false;
            if (skeleton == null)
                return didJump;
            //If floor isn't visible, can't do anything
            if (IsFloorVisible( e ) == false)
                return false;
            if (_jumped == true && GetDistanceFromFloor( e, skeleton.Joints[JointType.FootLeft] ) < 0.02d && GetDistanceFromFloor( e, skeleton.Joints[JointType.FootRight] ) < 0.02d)
            {
                _jumped = false;
                return false;
            }
            if (_jumped == true)
                return false;
            if (_jumped == false && GetDistanceFromFloor( e, skeleton.Joints[JointType.FootLeft] ) > 0.06d && GetDistanceFromFloor( e, skeleton.Joints[JointType.FootRight] ) > 0.06d)
            {
                _jumped = true;
                didJump = true;
            }
            return didJump;
        }

        /// <summary>
        /// Takes a skeleton and a SkeletonFrameReadyEventArgs and returns true when user performs right foot up posture
        /// </summary>
        /// <param name="skeleton">Tracked skeleton.</param>
        /// <param name="e">SkeletonFrameReadyEventArgs</param>
        public bool DetectRightFootUpPosture( SkeletonFrameReadyEventArgs e, Skeleton skeleton )
        {
            if (skeleton == null || e == null)
                return false;
            //If floor isn't visible, can't do anything
            if (IsFloorVisible( e ) == false)
                return false;
            double rightKneeHeight = GetDistanceFromFloor( e, skeleton.Joints[JointType.KneeRight] );
            bool isRightFootUp = false;
            if (GetDistanceFromFloor( e, skeleton.Joints[JointType.FootRight] ) > rightKneeHeight / 2.4)
                isRightFootUp = true;
            else
                isRightFootUp = false;
            return isRightFootUp;
        }

        /// <summary>
        /// Takes a skeleton and a SkeletonFrameReadyEventArgs and returns true when user performs left foot up posture
        /// </summary>
        /// <param name="skeleton">Tracked skeleton.</param>
        /// <param name="e">SkeletonFrameReadyEventArgs</param>
        public bool DetectLeftFootUpPosture( SkeletonFrameReadyEventArgs e, Skeleton skeleton )
        {
            if (skeleton == null || e == null)
                return false;
            //If floor isn't visible, can't do anything
            if (IsFloorVisible( e ) == false)
                return false;
            double leftKneeHeight = GetDistanceFromFloor( e, skeleton.Joints[JointType.KneeLeft] );
            bool isLeftFootUp = false;
            if (GetDistanceFromFloor( e, skeleton.Joints[JointType.FootLeft] ) > leftKneeHeight / 2.4)
                isLeftFootUp = true;
            else
                isLeftFootUp = false;
            return isLeftFootUp;
        }

        /// <summary>
        /// Takes a skeleton and returns true when user performs left hand up posture
        /// </summary>
        /// <param name="skeleton">Tracked skeleton.</param>
        public bool DetectLeftHandUpPosture( Skeleton skeleton )
        {
            bool isLeftHandUp = false;
            if (skeleton == null)
                return false;
            //Elbow should be under hand
            if (skeleton.Joints[JointType.ElbowLeft].Position.Y > skeleton.Joints[JointType.HandLeft].Position.Y)
                return false;
            if (skeleton.Joints[JointType.HandLeft].Position.Y >= skeleton.Joints[JointType.Head].Position.Y)
                isLeftHandUp = true;
            else
                isLeftHandUp = false;
            return isLeftHandUp;
        }

        /// <summary>
        /// Takes a skeleton and returns true when user performs right hand up posture
        /// </summary>
        /// <param name="skeleton">Tracked skeleton.</param>
        public bool DetectRightHandUpPosture( Skeleton skeleton )
        {
            bool isRightHandUp = false;
            if (skeleton == null)
                return false;
            //Elbow should be under hand
            if (skeleton.Joints[JointType.ElbowRight].Position.Y > skeleton.Joints[JointType.HandRight].Position.Y)
                return false;
            if (skeleton.Joints[JointType.HandRight].Position.Y >= skeleton.Joints[JointType.Head].Position.Y)
                isRightHandUp = true;
            else
                isRightHandUp = false;
            return isRightHandUp;
        }

        /// <summary>
        /// Takes a skeleton and returns true when user's left hand is down
        /// </summary>
        /// <param name="skeleton">Tracked skeleton.</param>
        public bool DetectLeftHandDownPosture(Skeleton skeleton)
        {
            bool isLeftHandDown = false;
            if (skeleton == null)
                return false;
            //Elbow should be over hand
            if (skeleton.Joints[JointType.ElbowLeft].Position.Y < skeleton.Joints[JointType.HandLeft].Position.Y)
                return false;
            if (skeleton.Joints[JointType.HandLeft].Position.Y < skeleton.Joints[JointType.Head].Position.Y)
                isLeftHandDown = true;
            else
                isLeftHandDown = false;
            return isLeftHandDown;
        }

        /// <summary>
        /// Takes a skeleton and returns true when user's right hand is down
        /// </summary>
        /// <param name="skeleton">Tracked skeleton.</param>
        public bool DetectRightHandDownPosture(Skeleton skeleton)
        {
            bool isRightHandDown = false;
            if (skeleton == null)
                return false;
            //Elbow should be over hand
            if (skeleton.Joints[JointType.ElbowRight].Position.Y < skeleton.Joints[JointType.HandRight].Position.Y)
                return false;
            if (skeleton.Joints[JointType.HandRight].Position.Y < skeleton.Joints[JointType.Head].Position.Y)
                isRightHandDown = true;
            else
                isRightHandDown = false;
            return isRightHandDown;
        }

        /// <summary>
        /// Takes a skeleton and returns true when user performs both hands up posture
        /// </summary>
        /// <param name="skeleton">Tracked skeleton.</param>
        public bool DetectBothHandsUpPosture(Skeleton skeleton)
        {
            return DetectRightHandUpPosture(skeleton) && DetectLeftHandUpPosture(skeleton);
        }

        /// <summary>
        /// Returns tru if the floor is visible, otherwise returns false
        /// </summary>
        /// <param name="e">SkeletonFrameReadyEventArgs</param>
        public bool IsFloorVisible(SkeletonFrameReadyEventArgs e)
        {
            using (SkeletonFrame skeletonFrame = e.OpenSkeletonFrame())
            {
                if (skeletonFrame == null || e == null)
                    return false;
                bool floorVisibility = false;
                if (skeletonFrame.FloorClipPlane.Item1 == 0 && skeletonFrame.FloorClipPlane.Item2 == 0 && skeletonFrame.FloorClipPlane.Item3 == 0 && skeletonFrame.FloorClipPlane.Item4 == 0)
                    floorVisibility = false;
                else
                    floorVisibility = true;
                return floorVisibility;
            }
        }

        /// <summary>
        /// Takes a SkeletonFrameReadyEventArgs and a joint and return the distance of that joint to floor if the floor is visible, otherwise returns -1
        /// </summary>
        /// <param name="skeleton">Tracked skeleton.</param>
        public double GetDistanceFromFloor( SkeletonFrameReadyEventArgs e, Joint joint )
        {
            using (SkeletonFrame skeletonData = e.OpenSkeletonFrame())
            {
                if (skeletonData == null || joint == null)
                    return -1;
                //If floor isn't visible, can't get the distance
                if (IsFloorVisible( e ) == false)
                    return -1;
                double distanceFromFloor = 0.0d;
                double floorClipPlaneX = skeletonData.FloorClipPlane.Item1;
                double floorClipPlaneY = skeletonData.FloorClipPlane.Item2;
                double floorClipPlaneZ = skeletonData.FloorClipPlane.Item3;
                double floorClipPlaneW = skeletonData.FloorClipPlane.Item4;

                double hipCenterX = joint.Position.X;
                double hipCenterY = joint.Position.Y;
                double hipCenterZ = joint.Position.Z;
                distanceFromFloor = floorClipPlaneX * hipCenterX + floorClipPlaneY * hipCenterY + floorClipPlaneZ * hipCenterZ + floorClipPlaneW;
                return distanceFromFloor;
            }
        }

        /// <summary>
        /// Checks if the right arm is open as a straight line
        /// </summary>
        /// <param name="skeleton">Tracked Skeleton</param>
        public bool DetectOpenRightArm( Skeleton skeleton )
        {
            if (skeleton == null)
                return false;
            bool armOpen = true;
            SkeletonPoint rightHandPoint = skeleton.Joints[JointType.HandRight].Position;
            SkeletonPoint rightElbowPoint = skeleton.Joints[JointType.ElbowRight].Position;
            SkeletonPoint rightShoulderPoint = skeleton.Joints[JointType.ShoulderRight].Position;
            //If elbow is below the shoulder, return false
            if (rightShoulderPoint.Y - rightElbowPoint.Y > 0.1)
                return false;
            //If elbow is over shoulder, return false
            if (rightShoulderPoint.Y - rightElbowPoint.Y < -0.1)
                return false;
            //If hand is below elbow, return false
            if (rightHandPoint.Y - rightElbowPoint.Y > 0.1)
                return false;
            //If the hand is over elbow, return false
            if (rightHandPoint.Y - rightElbowPoint.Y < -0.1)
                return false;
            return armOpen;
        }

        /// <summary>
        /// Checks if the left arm is open as a straight line
        /// </summary>
        /// <param name="skeleton">Tracked Skeleton</param>
        public bool DetectOpenLeftArm( Skeleton skeleton )
        {
            if (skeleton == null)
                return false;
            bool armOpen = true;
            SkeletonPoint leftHandPoint = skeleton.Joints[JointType.HandLeft].Position;
            SkeletonPoint leftElbowPoint = skeleton.Joints[JointType.ElbowLeft].Position;
            SkeletonPoint leftShoulderPoint = skeleton.Joints[JointType.ShoulderLeft].Position;
            //If elbow is below the shoulder, return false
            if (leftShoulderPoint.Y - leftElbowPoint.Y > 0.15)
                return false;
            //If elbow is over shoulder, return false
            if (leftShoulderPoint.Y - leftElbowPoint.Y < -0.15)
                return false;
            //If hand is below elbow, return false
            if (leftHandPoint.Y - leftElbowPoint.Y > 0.15)
                return false;
            //If the hand is over elbow, return false
            if (leftHandPoint.Y - leftElbowPoint.Y < -0.15)
                return false;
            return armOpen;
        }

        /// <summary>
        /// Returns true if both arms open
        /// </summary>
        /// <param name="skeleton">Tracked skeleton</param>
        public bool DetectOpenArms( Skeleton skeleton )
        {
            if (skeleton == null)
                return false;
            bool isArmsOpen = false;
            if (DetectOpenLeftArm( skeleton ) && DetectOpenRightArm( skeleton ))
                return true;
            return isArmsOpen;
        }

        /// <summary>
        /// Checks if the user is crouching
        /// </summary>
        /// <param name="e">SkeletonFrameReadyEventArgs: cannot be null</param>
        /// <param name="skeleton">Detected Skeleton: Cannot be null</param>
        public bool DetectCrouching( SkeletonFrameReadyEventArgs e, Skeleton skeleton )
        {
            if (e == null || skeleton == null)
                return false;
            bool isCrouching = true;
            if (GetDistanceFromFloor( e, skeleton.Joints[JointType.HipRight] ) <= 0.3 || GetDistanceFromFloor( e, skeleton.Joints[JointType.KneeLeft] ) <= 0.3)
                isCrouching = true;
            else
                isCrouching = false;
            return isCrouching;
        }

        /// <summary>
        /// Returns tru if the user is standing or else false, floor must be visible
        /// </summary>
        /// <param name="e">SkeletonFrameReadyEventArgs: cannot be null</param>
        /// <param name="skeleton">Detected Skeleton: Cannot be null</param>
        public bool DetectStanding( SkeletonFrameReadyEventArgs e, Skeleton skeleton )
        {
            if (e == null || skeleton == null)
                return false;
            if (!IsFloorVisible( e ))
                return false;
            bool isStanding = true;
            SkeletonPoint kneeRightPoint = skeleton.Joints[JointType.KneeRight].Position;
            SkeletonPoint kneeLeftPoint = skeleton.Joints[JointType.KneeLeft].Position;
            SkeletonPoint footLeftPoint = skeleton.Joints[JointType.FootLeft].Position;
            SkeletonPoint footRightPoint = skeleton.Joints[JointType.FootRight].Position;
            SkeletonPoint hipCenterPoint = skeleton.Joints[JointType.HipCenter].Position;
            SkeletonPoint headPoint = skeleton.Joints[JointType.Head].Position;
            //Feet must be on the floor
            if (GetDistanceFromFloor( e, skeleton.Joints[JointType.FootLeft] ) > 0.03 || GetDistanceFromFloor( e, skeleton.Joints[JointType.FootRight] ) > 0.03)
                return false;
            //Knees must be over feet
            if (kneeLeftPoint.Y < footLeftPoint.Y || kneeRightPoint.Y < footRightPoint.Y)
                return false;
            //Head must be over hip center
            if (headPoint.Y < hipCenterPoint.Y)
                return false;
            //Hip center must be at least as high as the distance between knees and the hip center
            if (hipCenterPoint.Z > headPoint.Z)
                return false;
            return isStanding;
        }

        /// <summary>
        /// Returns true if the user is bending over
        /// </summary>
        /// <param name="skeleton">Tracked Skeleton</param>
        public bool DetectBendOver( Skeleton skeleton )
        {
            if (skeleton == null)
                return false;
            bool isBendingOver = false;
            double shoulderCenterToHipCenterLength = skeleton.Joints[JointType.ShoulderCenter].Position.Y - skeleton.Joints[JointType.HipCenter].Position.Y;
            double skeletonPositionZ = skeleton.Position.Z;
            //If head isn't over the hip, return false
            if (shoulderCenterToHipCenterLength < 0)
                return false;
            if (skeletonPositionZ - skeleton.Joints[JointType.Head].Position.Z > shoulderCenterToHipCenterLength / 2)
                isBendingOver = true;
            return isBendingOver;
        }

        /// <summary>
        /// Returns the right arm angle to the right shoulder in an array. First element is the angle between right arm and Y axis, 
        /// second element is the angle between the right arm and Z axis. Values below zero indicate the direction, below zero is left.
        /// </summary>
        /// <param name="skeleton">Tracked Skeleton</param>
        public double[] DetectRightHandAngle( Skeleton skeleton )
        {
            if (skeleton == null)
                return null;
            double[] angles = new double[2];
            double angleY, angleZ;
            SkeletonPoint rightHandPoint = skeleton.Joints[JointType.HandRight].Position;
            SkeletonPoint rightShoulderPoint = skeleton.Joints[JointType.ShoulderRight].Position;
            double x = rightHandPoint.X - rightShoulderPoint.X;
            double y = rightHandPoint.Y - rightShoulderPoint.Y;
            double z = rightHandPoint.Z - rightShoulderPoint.Z;
            Vector3D vector1 = new Vector3D( x, y, z );
            Vector3D vector2 = new Vector3D( x, 0, z );
            angleY = y < 0 ? Vector3D.AngleBetween( vector1, vector2 ) * -1 : Vector3D.AngleBetween( vector1, vector2 );
            vector2 = new Vector3D( 0, y, z );
            angleZ = x < 0 ? Vector3D.AngleBetween( vector1, vector2 ) * -1 : Vector3D.AngleBetween( vector1, vector2 );
            angles[0] = angleY;
            angles[1] = angleZ;
            return angles;
        }

        /// <summary>
        /// Returns the left arm angle to the left shoulder in an array. First element is the angle between left arm and Y axis, 
        /// second element is the angle between the left arm and Z axis. Values below zero indicate the direction, below zero is left
        /// </summary>
        /// <param name="skeleton">Tracked Skeleton</param>
        public double[] DetectLeftHandAngle( Skeleton skeleton )
        {
            if (skeleton == null)
                return null;
            double[] angles = new double[2];
            double angleY, angleZ;
            SkeletonPoint leftHandPoint = skeleton.Joints[JointType.HandLeft].Position;
            SkeletonPoint leftShoulderPoint = skeleton.Joints[JointType.ShoulderLeft].Position;
            //Take the shoulder as the origin for the angle calculation
            double x = leftHandPoint.X - leftShoulderPoint.X;
            double y = leftHandPoint.Y - leftShoulderPoint.Y;
            double z = leftHandPoint.Z - leftShoulderPoint.Z;
            Vector3D vector1 = new Vector3D( x, y, z );
            Vector3D vector2 = new Vector3D( x, 0, z );
            angleY = y < 0 ? Vector3D.AngleBetween( vector1, vector2 ) * -1 : Vector3D.AngleBetween( vector1, vector2 );
            vector2 = new Vector3D( 0, y, z );
            angleZ = x < 0 ? Vector3D.AngleBetween( vector1, vector2 ) * -1 : Vector3D.AngleBetween( vector1, vector2 );
            angles[0] = angleY;
            angles[1] = angleZ;
            return angles;
        }

        /// <summary>
        /// Returns true if the arms are crossed in a cross gesture. NEEDS IMPROVEMENT
        /// </summary>
        /// <param name="skeleton">Tracked skeleton</param>
        public bool DetectCrossPosture( Skeleton skeleton )
        {
            //TODO: Check if Z angles matter
            if (skeleton == null)
                return false;
            return DetectSlashPostureForRightArm( skeleton ) && DetectSlashPostureForLeftArm( skeleton );
        }

        /// <summary>
        /// Returns true if any of the arms are in a slash posture
        /// </summary>
        /// <param name="skeleton">Tracked skeleton</param>
        public bool DetectSlashPosture( Skeleton skeleton )
        {
            if (skeleton == null)
                return false;
            bool isSlashPosture = false;
            if (DetectSlashPostureForRightArm(skeleton))
            {
                //left hand should be down
                if (skeleton.Joints[JointType.HandLeft].Position.Y > skeleton.Joints[JointType.HipCenter].Position.Y)
                    return false;
                isSlashPosture = true;
            }
            else if (DetectSlashPostureForLeftArm(skeleton))
            {
                //Right hand should be down
                if (skeleton.Joints[JointType.HandRight].Position.Y > skeleton.Joints[JointType.HipCenter].Position.Y)
                    return false;
                isSlashPosture = true;
            }
            return isSlashPosture;
        }

        /// <summary>
        /// Returns true if right arm is in a slash posture
        /// </summary>
        /// <param name="skeleton">Tracked skeleton</param>
        public bool DetectSlashPostureForRightArm( Skeleton skeleton )
        {
            //TODO: Check if Z angles matter
            if (skeleton == null)
                return false;
            SkeletonPoint rightHandPoint = skeleton.Joints[JointType.HandRight].Position;
            SkeletonPoint rightElbowPoint = skeleton.Joints[JointType.ElbowRight].Position;
            //If the hand is on the left side of the elbow return false
            if (rightHandPoint.X > rightElbowPoint.X)
                return false;
            //If the hand is below elbow return false
            if (rightHandPoint.Y < rightElbowPoint.Y)
                return false;
            if (rightHandPoint.Y - rightElbowPoint.Y < 0.15 || rightHandPoint.Y - rightElbowPoint.Y > 0.45)
                return false;
            return true;
        }

        /// <summary>
        /// Returns true if let arm is in a slash posture
        /// </summary>
        /// <param name="skeleton">Tracked skeleton</param>
        public bool DetectSlashPostureForLeftArm( Skeleton skeleton )
        {
            //TODO: Check if Z angles matter
            if (skeleton == null)
                return false;
            SkeletonPoint leftHandPoint = skeleton.Joints[JointType.HandLeft].Position;
            SkeletonPoint leftElbowPoint = skeleton.Joints[JointType.ElbowLeft].Position;
            //If the hand is on the left side of the elbow return false
            if (leftHandPoint.X < leftElbowPoint.X)
                return false;
            //If the hand is below elbow return false
            if (leftHandPoint.Y < leftElbowPoint.Y)
                return false;
            if (leftHandPoint.Y - leftElbowPoint.Y < 0.15 || leftHandPoint.Y - leftElbowPoint.Y > 0.45)
                return false;
            return true;
        }

        /// <summary>
        /// Returns true if right arm is in a minus posture
        /// </summary>
        /// <param name="skeleton">Tracked skeleton</param>
        public bool DetectMinusPostureForRightArm( Skeleton skeleton )
        {
            if (skeleton == null)
                return false;
            SkeletonPoint rightHandPoint = skeleton.Joints[JointType.HandRight].Position;
            SkeletonPoint rightElbowPoint = skeleton.Joints[JointType.ElbowRight].Position;
            //Left hand should be down
            if (skeleton.Joints[JointType.HandLeft].Position.Y > skeleton.Joints[JointType.HipCenter].Position.Y)
                return false;
            //If hand is below hip center return false
            if (rightHandPoint.Y < skeleton.Joints[JointType.HipCenter].Position.Y)
                return false;
            //If the hand is too much over elbow or too much below elbow return false
            if (Math.Abs( rightHandPoint.Y - rightElbowPoint.Y ) > 0.1)
                return false;
            //If the hand is on the right side of the elbow return false
            if (rightHandPoint.X > rightElbowPoint.X)
                return false;
            return true;
        }

        /// <summary>
        /// Returns true left arm is in a minus posture
        /// </summary>
        /// <param name="skeleton">Tracked skeleton</param>
        public bool DetectMinusPostureForLeftArm(Skeleton skeleton)
        {
            if (skeleton == null)
                return false;
            SkeletonPoint leftHandPoint = skeleton.Joints[JointType.HandLeft].Position;
            SkeletonPoint leftElbowPoint = skeleton.Joints[JointType.ElbowLeft].Position;
            //Right hand should be down
            if (skeleton.Joints[JointType.HandRight].Position.Y > skeleton.Joints[JointType.HipCenter].Position.Y)
                return false;
            //If hand is below hip center return false
            if (leftHandPoint.Y < skeleton.Joints[JointType.HipCenter].Position.Y)
                return false;
            //If the hand is too much over elbow or too much below elbow return false
            if (Math.Abs(leftHandPoint.Y - leftElbowPoint.Y) > 0.1)
                return false;
            //If the hand is on the left side of the elbow return false
            if (leftHandPoint.X < leftElbowPoint.X)
                return false;
            return true;
        }

        /// <summary>
        /// Returns true if any of the arms are in a minux posture
        /// </summary>
        /// <param name="skeleton">Tracked Skeleton</param>
        public bool DetectMinusPosture( Skeleton skeleton )
        {
            if (skeleton == null)
                return false;
            bool isMinusPosture = false;
            if (DetectMinusPostureForLeftArm( skeleton ) || DetectMinusPostureForRightArm( skeleton ))
                isMinusPosture = true;
            else
                isMinusPosture = false;
            return isMinusPosture;
        }

        /// <summary>
        /// Returns true if the arms are crossed in a plus posture
        /// </summary>
        /// <param name="skeleton">Tracked skeleton</param>
        public bool DetectPlusPosture( Skeleton skeleton )
        {
            if (skeleton == null)
                return false;
            SkeletonPoint leftHandPoint = skeleton.Joints[JointType.HandLeft].Position;
            SkeletonPoint leftElbowPoint = skeleton.Joints[JointType.ElbowLeft].Position;
            SkeletonPoint rightHandPoint = skeleton.Joints[JointType.HandRight].Position;
            SkeletonPoint rightElbowPoint = skeleton.Joints[JointType.ElbowRight].Position;
            //If hand is below elbow return false
            if (leftHandPoint.Y < leftElbowPoint.Y || rightHandPoint.Y < rightElbowPoint.Y)
                return false;
            //If hands are below hip center return false
            if (rightHandPoint.Y < skeleton.Joints[JointType.HipCenter].Position.Y || leftHandPoint.Y < skeleton.Joints[JointType.HipCenter].Position.Y)
                return false;
            if (rightHandPoint.X > leftHandPoint.X || leftElbowPoint.X > rightElbowPoint.X)
                return false;
            if (rightHandPoint.Y > leftHandPoint.Y && (Math.Abs( rightHandPoint.X - rightElbowPoint.X ) > 0.1
                && Math.Abs( leftHandPoint.Y - leftElbowPoint.Y ) > 0.1))
                return false;
            if (rightHandPoint.Y < leftHandPoint.Y && (Math.Abs( leftHandPoint.X - leftElbowPoint.X ) > 0.1
                && Math.Abs( rightHandPoint.Y - rightElbowPoint.Y ) > 0.1))
                return false;
            return true;
        }
    }
}
