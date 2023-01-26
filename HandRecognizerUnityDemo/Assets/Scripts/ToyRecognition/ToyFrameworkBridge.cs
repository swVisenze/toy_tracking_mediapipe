using System.Runtime.InteropServices;
using UnityEngine;
using System.Diagnostics;
using Debug = UnityEngine.Debug;

public class ToyFrameworkBridge : MonoBehaviour
{
#if UNITY_IOS
    [DllImport("__Internal")]
    private static extern bool native_toy_tracking_reset(int buffer_frames);

    [DllImport("__Internal")]
    private static extern void native_toy_tracking_init(int width, int height);

    [DllImport("__Internal")]
    private static extern void native_toy_tracking_destroy();

    [DllImport("__Internal")]
    private static extern string native_toy_tracking_tracking(byte[] buffer, int size, int width, int height);
#endif 

#if UNITY_ANDROID
    internal const string MediaPipeLibrary = "mediapipe_jni";

    [DllImport(MediaPipeLibrary, ExactSpelling = true)]
    private static extern bool toy_tracking_reset(int buffer_frames); 

    [DllImport(MediaPipeLibrary, CharSet = CharSet.Ansi)]
    private static extern System.IntPtr toy_tracking_tracking(byte[] imageData, int size, int width, int height); 

    private static AndroidJavaClass toyTrackingClass = new AndroidJavaClass("com.visenze.ToyTracking");

    public static void toy_tracking_init(int width, int height) {
        using (AndroidJavaClass unityPlayer = new AndroidJavaClass("com.unity3d.player.UnityPlayer")) {
            using (AndroidJavaObject unityActivity = unityPlayer.GetStatic<AndroidJavaObject>("currentActivity")) {
                toyTrackingClass.CallStatic("init", unityActivity, width, height);
            }
        }
    }

    public static void toy_tracking_destroy() {
        toyTrackingClass.CallStatic("destroy");
    }
#endif 


    public static void framework_refresh(int buffer_frames)
    {
        Debug.Log("TrackingUnityDemo: Refresh Tracking Framework");
#if UNITY_IOS
        if (Application.platform == RuntimePlatform.IPhonePlayer)
        {
            Debug.Log("TrackingUnityDemo: Calling refresh from iOS SDK");
            native_toy_tracking_reset(buffer_frames);
        }
#endif     

#if UNITY_ANDROID
        if (Application.platform == RuntimePlatform.Android)
        {
            Debug.Log("TrackingUnityDemo: Calling refresh from Android SDK");
            toy_tracking_reset(buffer_frames);
        }
#endif

    }

    public static void framework_init(int width, int height)
    {
        Debug.Log("TrackingUnityDemo: Initialize Tracking Framework");
#if UNITY_IOS
        if (Application.platform == RuntimePlatform.IPhonePlayer)
        {
            Debug.Log("TrackingUnityDemo: Calling init from iOS SDK");
            native_toy_tracking_init(width, height);
        }
#endif

#if UNITY_ANDROID
        if (Application.platform == RuntimePlatform.Android)
        {
            Debug.Log("TrackingUnityDemo: Calling init from Android SDK");
            toy_tracking_init(width, height);
        }
#endif
    }

    public static void framework_terminate()
    {
#if UNITY_IOS
        if (Application.platform == RuntimePlatform.IPhonePlayer)
        {
            Debug.Log("TrackingUnityDemo: Calling desktroy from iOS SDK");
            native_toy_tracking_destroy();
        }
#endif

#if UNITY_ANDROID
        if (Application.platform == RuntimePlatform.Android)
        {
            Debug.Log("TrackingUnityDemo: Calling desktroy from Android SDK");
            toy_tracking_destroy();
        }
#endif
    }

    public static string framework_recognize(byte[] buffer, int size, int width, int height)
    {
#if UNITY_IOS
        if (Application.platform == RuntimePlatform.IPhonePlayer)
        {
            Stopwatch st = new Stopwatch();
            st.Start();
            Debug.Log("TrackingUnityDemo: Calling tracking from iOS SDK");
            string result = native_toy_tracking_tracking(buffer, size, width, height);
            st.Stop();
            Debug.Log(string.Format("Recognize API E2E Time Taken {0} seconds", st.ElapsedMilliseconds/1000.0));
            return result;
        }
#endif

#if UNITY_ANDROID
        if (Application.platform == RuntimePlatform.Android)
        {
            Debug.Log("TrackingUnityDemo: Calling tracking from Android SDK");
            Stopwatch st = new Stopwatch();
            st.Start();
            System.IntPtr output_pointer = toy_tracking_tracking(buffer, size, width, height);
            string result = Marshal.PtrToStringAnsi(output_pointer);
            st.Stop();
            Debug.Log(string.Format("Recognize API E2E Time Taken {0} seconds", st.ElapsedMilliseconds/1000.0));            
            return result; 
        }
#endif

        Debug.LogError("TrackingUnityDemo: No valid platform is found");
        return null;
    }
}