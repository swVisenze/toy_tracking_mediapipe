using System;
using System.Collections;
using System.Collections.Generic;
using System.Threading;
using System.Globalization;
using UnityEngine;
using Unity.Collections;
using UnityEngine.UI;
using UnityEngine.SceneManagement;

using System.Diagnostics;
using Debug = UnityEngine.Debug;
#if UNITY_ANDROID
using UnityEngine.Android;
#elif UNITY_IOS
using UnityEngine.iOS;
#endif

public class StartStopApp : MonoBehaviour
{
    [Tooltip("Image to tint with color depending on status of app, start/stop")]
    public Image indicator = null;
    [Tooltip("Text display for status of app, start/stop")]
    public Text toggleText = null;
    [Tooltip("Text to display result of the last received scan")]
    public Text result = null;
    [Tooltip("Buffer frame input")]
    public InputField bufferFrameInput = null;
    [Tooltip("Text to display accumulated time so far since start of scan")]
    public Text latency = null;
    [Tooltip("Dependency script to process rendering of camera texture with dot")]
    public StartStopCamera startStopCamera = null;
    [Tooltip("Set camera to use front facing first if available")]
    public bool tryUseFrontCam = false;
    [Tooltip("The close button to disable when scanning")]
    public Button closeButton = null;
    // camera capture
    // private CameraCapture cameraCapture = null;
    // camera permission
    private CameraPermission cameraPermission = null;
    // multithreaded variable to image data
    private volatile byte[] sdkImage = null;
    // multithreaded variable to response from sdk

    private byte[] imageRawData = null;
    private volatile string sdkResult = string.Empty;
    // multithreaded variables regarding scanning image
    private volatile int sdkImageDataSize, sdkImageWidth, sdkImageHeight;

    // time accumulator
    private float timer = 0.0f;
    // if recognition is running
    private bool isRunning = false;

    private bool initFrame = false;

    private bool isInitialized = false;



    private void OnEnable()
    {
        // bind callbacks
        cameraPermission.onPermissionGranted += OnPermissionGranted;
    }

    private void OnDisable()
    {
        // unbind callbacks
        cameraPermission.onPermissionGranted -= OnPermissionGranted;
    }

    private void Awake()
    {
        // get dependency scripts references
        // cameraCapture = startStopCamera.GetComponent<CameraCapture>();
        cameraPermission = startStopCamera.GetComponent<CameraPermission>();
    }

    private void Start()
    {
        // initialize sdk (should not need to init framework if transitioned
        // from 'MainMenu' scene, as it also inits before loading us)

        ToyFrameworkBridge.framework_init(startStopCamera.cameraWidth, startStopCamera.cameraHeight);

        isRunning = false;
        sdkImage = null;
        sdkImageDataSize = 0;
        sdkImageWidth = 0;
        sdkImageHeight = 0;
        sdkResult = string.Empty;
        timer = 0.0f;

        // black to indicate camera not ready
        if (indicator != null)
            indicator.color = Color.black;
        if (bufferFrameInput != null) 
            bufferFrameInput.text = "20";
    }

    private void Update()
    {
        // early out on any of this cases
        if (!cameraPermission.HasPermission || !isRunning) {
            startStopCamera.SetCoordinatesPoints(null);            
            return;            
        }


        timer += Time.deltaTime;
        // Texture2D tex = cameraCapture.GetImageTexure(); 
        Texture2D tex = startStopCamera.GetScreenShotTexure();

        // sdkImageWidth = cameraCapture.CameraTexture.width; 
        // sdkImageHeight = cameraCapture.CameraTexture.height; 
        // Debug.Log("imageRawData[255554]: "+imageRawData[155554]);
        #if UNITY_EDITOR
            sdkImageWidth = tex.width;  // 256
            sdkImageHeight = tex.height; // 256
            Thread.Sleep(20); 
            // sdkResult="{\"landmark_2d\": \"364,145;295,129;313,100;377,113;372,77;299,63;316,38;384,50;\"}";
            sdkResult = "{\"debug_message\":\"3_prev_id_1\",\"status\":\"tracking\",\"track_box\":\"0.452927,0.310548;0.626055,0.580231\"}";
#else
            
            sdkImageWidth = tex.width;  
            sdkImageHeight = tex.height; 

            Debug.Log("TrackingUnityDemo: sdkImageWidth : "+ sdkImageWidth);
            Debug.Log("TrackingUnityDemo: sdkImageHeight: "+ sdkImageHeight);

            if (imageRawData == null) {
                sdkImageDataSize = sdkImageWidth*sdkImageHeight*3; // rgb24 format
                imageRawData = new byte[sdkImageDataSize];  
            }

            NativeArray<byte> rawTextureData = tex.GetRawTextureData<byte>();
            rawTextureData.CopyTo(imageRawData);
            sdkResult = ToyFrameworkBridge.framework_recognize(imageRawData, sdkImageDataSize, sdkImageWidth, sdkImageHeight);                
#endif
        Debug.Log("TrackingUnityDemo: SDK Result: " + sdkResult);        
        ParseSdkResult();
        
    }

    private void OnDestroy()
    {
        ToyFrameworkBridge.framework_terminate();
    }

    /// <summary>
    /// Callback when permission is granted to us to use the device camera
    /// </summary>
    private void OnPermissionGranted()
    {
        if (indicator != null) {
            indicator.color = Color.green;
        }

        startStopCamera.startCamera(tryUseFrontCam); 
        // if (tryUseFrontCam)
        //     startStopCamera.SwitchCamera();
    }


    /// <summary>
    /// Try to create a ToyInfo from the latest sdkResult string returned by the
    /// SDK (Json formatted).
    /// </summary>
    private void ParseSdkResult()
    {
        ToyInfo info = ToyInfo.CreateFromJson(sdkResult);
        Debug.Log("TrackingUnityDemo status: " + info.status);
        if(info.status == "tracking") {
            string[] points = info.track_box.Split(';');
            List<float> xycoords = new List<float>();
            for(int i=0; i<points.Length; i++) {
                string[] xy = points[i].Split(',');
                float xCoord = float.Parse(xy[0], CultureInfo.InvariantCulture); 
                // y in unity is flipped. 
                float yCoord = 1f - float.Parse(xy[1], CultureInfo.InvariantCulture);
                // float yCoord = float.Parse(xy[1]);
                xycoords.Add(xCoord);
                xycoords.Add(yCoord);    
            }
            startStopCamera.SetCoordinatesPoints(xycoords.ToArray());                        
        } else {
            startStopCamera.SetCoordinatesPoints(null);
            // user logic, in this demo stop sending images to sdk. the game may want to show messages when this happen
            if(info.status == "lost") {
                OnToggleRecognition();
            }            
            result.text = "status:";            
        } 
        if(result != null) {
            result.text = "status: " + info.status;
        }
        Debug.Log("TrackingUnityDemo: Finish parsing results");
    }

    /// <summary>
    /// Starts the recognition process if is not started yet, or stop it if it
    /// is currently running.
    /// </summary>
    public void OnToggleRecognition()
    {
        isRunning = !isRunning;
        if (toggleText != null)
            toggleText.text = isRunning ? "Stop" : "Start";
        if (indicator != null)
            indicator.color = isRunning ? Color.yellow : Color.green;

        if (isRunning) {
            timer = 0.0f;
            initFrame = true;
            isInitialized = true;
            // cameraCapture.CaptureTexture();            
            // refresh reset.            
            string input = bufferFrameInput.text;
            int inputNumber = 20; 
            bool isNumeric = Int32.TryParse(input, out inputNumber); 
            // if(isNumeric) {
            //     Debug.Log("bufferFrameInput: " + inputNumber);
            // }
            ToyFrameworkBridge.framework_refresh(inputNumber);                        
        }

        if (bufferFrameInput != null) {
            bufferFrameInput.gameObject.SetActive(!isRunning);
        }        
        if (latency != null) {
            latency.gameObject.SetActive(!isRunning); 
        }

        if (closeButton != null)
            closeButton.gameObject.SetActive(!isRunning);
    }

    /// <summary>
    /// When this scene should close via a buton/event, loads back 'MainMenu'
    /// </summary>
    public void OnClose()
    {
        Application.Quit();
    }
}
