using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

#if UNITY_ANDROID
using UnityEngine.Android;
#endif

public class CameraPermission : MonoBehaviour
{
    // callback signature when permission has been grated
    public delegate void OnPermissionGranted();
    // callback registration
    public event OnPermissionGranted onPermissionGranted;
    // if permission is given
    private bool hasPermission = false;
    // Getter for permission
    public bool HasPermission
    {
        get { return hasPermission; } 
    }

#if UNITY_IOS
    IEnumerator Start()
    {
        hasPermission = false;
        // request device for permission to use camera, wait for async function
        // to finish
        yield return Application.RequestUserAuthorization(UserAuthorization.WebCam);
        // query whether the device allows it
        hasPermission = Application.HasUserAuthorization(UserAuthorization.WebCam);
        if (hasPermission)
            onPermissionGranted();
    }
#elif UNITY_ANDROID
    IEnumerator Start()
    {
        yield return new WaitForEndOfFrame();
        // query whether the device has permission to use camera
        hasPermission = Permission.HasUserAuthorizedPermission(Permission.Camera);
        if (!hasPermission)
        {
            // request permission from async function with callback on granted
            PermissionCallbacks callbacks = new PermissionCallbacks();
            callbacks.PermissionGranted += PermissionGranted;
            Permission.RequestUserPermission(Permission.Camera, callbacks);
        }
        else
        {
            onPermissionGranted();
        }
    }

    internal void PermissionGranted(string permissionName)
    {
        // permission received
        hasPermission = true;
        onPermissionGranted();
    }
#else
    void Start()
    {
        hasPermission = false;
    }
#endif

}
