using System;
using System.Collections;
using System.Collections.Generic;
using System.Threading;
using UnityEngine;
using UnityEngine.UI;
using System.Diagnostics;
using Debug = UnityEngine.Debug;
#if UNITY_ANDROID
using UnityEngine.Android;
#elif UNITY_IOS
using UnityEngine.iOS;
#endif

[System.Serializable]
public class ToyInfo
{   
    public string status; 
    public string track_box;

    public string debug_message;

    public override string ToString()
    {
        return "status=" + status + " track_box=" + track_box + 
        " debug_message=" + debug_message;
    }

    public static ToyInfo CreateFromJson(string jsonString)
    {
        return JsonUtility.FromJson<ToyInfo>(jsonString);
    }
}