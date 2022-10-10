using System;
using System.Collections;
using System.Runtime.InteropServices;
using AOT;
using UnityEngine;
using System.Diagnostics;
using Debug = UnityEngine.Debug;
using System.Collections.Generic;

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
                // trackingInstance = new AndroidJavaObject("com.visenze.ToyTracking", unityActivity);
                toyTrackingClass.CallStatic("init", unityActivity, width, height);
            }
        }
    }

    public static void toy_tracking_destroy() {
        // trackingInstance.Call("close");
        // toy_tracking_destroy();
        toyTrackingClass.CallStatic("destroy");
    }


#endif 
    public static void framework_refresh(int buffer_frames)
    {
        Debug.Log("framework_refresh");
#if UNITY_IOS
        if (Application.platform == RuntimePlatform.IPhonePlayer)
        {
            Debug.Log("IOS framework_refresh");
            native_toy_tracking_reset(buffer_frames);
        }
#endif     

#if UNITY_ANDROID
        if (Application.platform == RuntimePlatform.Android)
        {
            Debug.Log("ANDROID framework_refresh");
            toy_tracking_reset(buffer_frames);
        }
#endif

    }

    public static void framework_init(int width, int height)
    {
        Debug.Log("framework_init");
#if UNITY_IOS
        if (Application.platform == RuntimePlatform.IPhonePlayer)
        {
            Debug.Log("IOS framework_init");
            native_toy_tracking_init(width, height);
        }
#endif

#if UNITY_ANDROID
        if (Application.platform == RuntimePlatform.Android)
        {
            Debug.Log("ANDROID framework_init");
            toy_tracking_init(width, height);
        }
#endif
    }

    public static void framework_terminate()
    {
#if UNITY_IOS
        if (Application.platform == RuntimePlatform.IPhonePlayer)
        {
            Debug.Log("IOS framework_terminate");
            native_toy_tracking_destroy();
        }
#endif

#if UNITY_ANDROID
        if (Application.platform == RuntimePlatform.Android)
        {
            Debug.Log("ANDROID framework_terminate");
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
            Debug.Log("IOS framework_recognize");
            string result = native_toy_tracking_tracking(buffer, size, width, height);
            st.Stop();
            Debug.Log(string.Format("Recognize API E2E Time Taken {0} seconds", st.ElapsedMilliseconds/1000.0));
            return result;
        }
#endif

#if UNITY_ANDROID
        if (Application.platform == RuntimePlatform.Android)
        {
            Debug.Log("ANDROID framework_recognize");

            Stopwatch st = new Stopwatch();
            st.Start();

            System.IntPtr output_pointer = toy_tracking_tracking(buffer, size, width, height);
            string result = Marshal.PtrToStringAnsi(output_pointer);
            st.Stop();
            Debug.Log(string.Format("Recognize API E2E Time Taken {0} seconds", st.ElapsedMilliseconds/1000.0));            
            return result; 
        }
#endif

        Debug.Log("recognizing skipped - return");
        return null;
    }
}