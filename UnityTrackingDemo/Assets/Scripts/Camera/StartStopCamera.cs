using System.Collections;
using System.Collections.Generic;
using System;
using UnityEngine;

using UnityEngine.UI;

#if UNITY_ANDROID
using UnityEngine.Android;
#endif

[RequireComponent(typeof(CameraPermission), typeof(CameraCapture))]
public class StartStopCamera : MonoBehaviour
{
    enum UsingCamera
    {
        None = 0,
        Front,
        Back
    }

    [Tooltip("The RawImage component that we will render the camera's texture onto")]
    public RawImage background = null;

    [Tooltip("The crosshair to re-position onto the detected box's center")]
    public Image crosshair = null;



    // camera permission
    private CameraPermission camPerm = null;
    // camera capture
    // private CameraCapture camCap = null;
    // front cam
    private WebCamTexture frontCam = null;
    // back cam
    private WebCamTexture backCam = null;
    // which camera are we using
    private UsingCamera usingCamera = UsingCamera.None;

    // last known width
    private int lastScreenWidth = 0;
    // last known height
    private int lastScreenHeight = 0;
    // width of the capture texture
    // private int capturedWidth = 0;
    // // height of the capture texture
    // private int capturedHeight = 0;
    // aspect ratio fitter to resize the RawImage
    private AspectRatioFitter fit = null;
    // our final rendered texture for presentation
    private Texture2D webcamTex = null;

    private Texture2D displayTex = null; 

    // cached copy of the webcam texture we capture
    private Texture2D cachedTex2D = null;
    // temporary render texture to perform graphics blit from
    private RenderTexture cachedRenderTex = null;

    // last known detected box in texture space (captureWidth, capturedHeight)
    public float[] coordPoints = null; 

    public bool IsFrontCamera { get { return usingCamera == UsingCamera.Front; } }
    public bool IsBackCamera { get { return usingCamera == UsingCamera.Back; } }


    public int cameraWidth = 1280;
    public int cameraHeight = 720;

    private Quaternion baseRotation; 

    #region MonoBehaviour 

    private void OnEnable()
    {
        // bind callbacks
        // camPerm.onPermissionGranted += OnPermissionGranted;
        // camCap.onCaptureTexture += OnCaptureTexture;
    }

    private void OnDisable()
    {
        // unbind callbacks
        // camPerm.onPermissionGranted -= OnPermissionGranted;
        // camCap.onCaptureTexture -= OnCaptureTexture;
    }

    private void Awake()
    {
        // acquire dependency references
        camPerm = GetComponent<CameraPermission>();
        // camCap = GetComponent<CameraCapture>();
        if (background != null)
        {
            fit = background.GetComponent<AspectRatioFitter>();
        }
    }

    private void Start() {
        baseRotation = background.gameObject.transform.rotation;
    }

    private void Update()
    {
        if (!camPerm.HasPermission)
            return;

        // update camera cosmetics
        // UpdateResolution();
        UpdateAspectRatio();
    }

    private void LateUpdate()
    {
        if (!camPerm.HasPermission)
            return;

        StartCoroutine(ProcessFrame());
    }

    IEnumerator ProcessFrame()
    {
        yield return new WaitForEndOfFrame();

        if (background != null)
        {
            // post processing for drawing detected bounding box's center point

            webcamTex = AllocateWebcamTex();
            webcamTex.SetPixels32(GetWebcamPixels(false), 0);            
            webcamTex.Apply();            

            displayTex = AllocateDisplayTex();
            Graphics.CopyTexture(webcamTex, displayTex); 
        

            if(coordPoints != null && coordPoints.Length == 4) {
                int targetWidth = webcamTex.width;
                int targetHeight = webcamTex.height; 
                // Debug.Log("ProcessFrame targetWidth: " + targetWidth);
                // Debug.Log("ProcessFrame targetHeight: " + targetHeight);
                int[] boxPoints = cal2dBoxPoints(coordPoints, targetWidth, targetHeight);
                Draw2DBox(displayTex, boxPoints);
            }

            displayTex.Apply();            
            background.texture = displayTex;
            // int rotateAngle = -1 * backCam.videoRotationAngle;
            // Debug.Log("rotateAngle: " + rotateAngle);
            // background.gameObject.transform.rotation = baseRotation * Quaternion.AngleAxis(rotateAngle, Vector3.forward);
        }
    }

#endregion

#region Public Functions

    public Texture2D GetScreenShotTexure() {
        Texture texToCapture = webcamTex;
        // int adjustedWidth = IMAGE_SIZE;
        // int adjustedHeight = IMAGE_SIZE; 
        int adjustedWidth = texToCapture.width / 2;
        int adjustedHeight = texToCapture.height / 2; 
        Debug.Log("camera texture adjustedWidth: "+ adjustedWidth);
        Debug.Log("camera texture adjustedHeight: "+ adjustedHeight);

#if UNITY_ANDROID
        if (!cachedTex2D)
        {
            cachedTex2D = new Texture2D(adjustedWidth, adjustedHeight, TextureFormat.RGB24, false);
            cachedRenderTex = RenderTexture.GetTemporary(adjustedWidth, adjustedHeight);
        }

        cachedRenderTex.filterMode = FilterMode.Point;
        // back-up the original active render texture
        RenderTexture currRt = RenderTexture.active;

        Graphics.Blit(texToCapture, cachedRenderTex, new Vector2(1.0f, -1.0f), new Vector2(0.0f, 0.0f));

        // if (cameraTexture.videoVerticallyMirrored) {
        //     // Debug.Log("cameraTexture.videoVerticallyMirrored");
        //     Graphics.Blit(texToCapture, cachedRenderTex, new Vector2(1.0f, -1.0f), new Vector2(0.0f, 0.0f));
        // } else {
        //     // Debug.Log("cameraTexture. not flipped");
        //     Graphics.Blit(texToCapture, cachedRenderTex);
        // }        
#else 
        if (!cachedTex2D)
        {
            cachedTex2D = new Texture2D(adjustedWidth, adjustedHeight, TextureFormat.RGB24, false);
            cachedRenderTex = RenderTexture.GetTemporary(adjustedWidth, adjustedHeight);
        }

        cachedRenderTex.filterMode = FilterMode.Point;
        // back-up the original active render texture
        RenderTexture currRt = RenderTexture.active;

        Graphics.Blit(texToCapture, cachedRenderTex, new Vector2(1.0f, -1.0f), new Vector2(0.0f, 0.0f));
        // Graphics.Blit(texToCapture, cachedRenderTex);
#endif        
        // set the temp buffer as the active render texture
        RenderTexture.active = cachedRenderTex;

        // get the cached camera texture to read from the current active render texture
        cachedTex2D.ReadPixels(new Rect(0, 0, adjustedWidth, adjustedHeight), 0, 0);
        cachedTex2D.Apply();
        RenderTexture.active = currRt;
        return cachedTex2D;
    }


    /// <summary>
    /// Attempts to toggle from front camera to back camera, and vice-versa. 
    /// Nothing will happen if the camera you are trying to toggle to does not
    /// exist.
    /// </summary>
    public void SwitchCamera()
    {
        if (IsFrontCamera)
            UseBackCamera();
        else if (IsBackCamera)
            UseFrontCamera();
    }

    /// <summary>
    /// Tells this script where the bounding box is at. During the LateUpdate
    /// function, a Dot will be rendered as "post-processing" depending on the
    /// detected box. Note: This value is stored untill replaced, meaning we
    /// will continuously render the Dot at the last registered detected box
    /// location.
    /// </summary>
    public void SetCoordinatesPoints(float[] points)
    {
        // debug box is with coordinates respective to the original size of the 
        // image sent to the sdk. We might not be rendering to the right size
        // if we just use it as-is. Need to convert from original screenshot 
        // sizes to our rendered sizes. i.e. Screenshot resolution might not be
        // equal to render resolution on the camTex we hold.
        // if (capturedWidth > 0 && capturedHeight > 0)
        // {
        //     coordPoints = points; 
        // }
        coordPoints = points;
    }

    public int[] calAxisPoints(float[] points, int targetWidth, int targetHeight) {

        int c_x = (int) (targetWidth * (points[0] + points[4]) / 2);
        int c_y = (int) (targetHeight * (points[1] + points[5]) / 2);

        int xa_x = (int) (targetWidth * (points[0] + points[6]) / 2);
        int xa_y = (int) (targetHeight * (points[1] + points[7]) / 2);   
            
        int ya_x = (int) (targetWidth * (points[0] + points[2]) / 2);
        int ya_y = (int) (targetHeight * (points[1] + points[3]) / 2);

        int za_x = (int) (targetWidth * (points[8] + points[12]) / 2); 
        int za_y = (int) (targetHeight * (points[9] + points[13]) / 2);
        int[] axisPoints = new int[] {
            c_x, c_y, xa_x, xa_y, ya_x, ya_y, za_x, za_y
        };
        // Debug.Log("targetWidth: " + targetWidth);
        // Debug.Log("targetHeight: " + targetHeight);
        Debug.Log("axisPoints: " + string.Join(",", axisPoints));
        return axisPoints;
    }

    public int[] cal2dBoxPoints(float[] points, int targetWidth, int targetHeight) {
        int length = points.Length;
        int[] boxPoints = new int[length]; 
        for(int i=0; i<length/2 ; i++) {
            boxPoints[i*2] = (int) (targetWidth * points[i*2]); 
            boxPoints[i*2 + 1] = (int) (targetHeight * points[i*2 + 1]);
        }
        return boxPoints;
    }

    public int[] calBoxPoints(float[] points, int targetWidth, int targetHeight) {
        int[] boxPoints = new int[16]; 
        for(int i=0; i<8; i++) {
            boxPoints[i*2] = (int) (targetWidth * points[i*2]); 
            boxPoints[i*2 + 1] = (int) (targetHeight * points[i*2 + 1]);
        }
        return boxPoints;
    }

    public void startCamera(bool useFrontCamera) {

        
        WebCamDevice[] devices = WebCamTexture.devices;
        if (devices.Length == 0)
        {
            Debug.Log("No cameras found on this device!");
            return;
        }


        for (int i = 0; i < devices.Length; i++)
        {
            if (devices[i].isFrontFacing)
            {
                if (frontCam == null) {
                    frontCam = new WebCamTexture(devices[i].name, cameraWidth, cameraHeight, 30);
                }
            }
            else
            {
                if (backCam == null) {
                    backCam = new WebCamTexture(devices[i].name, cameraWidth, cameraHeight, 30);
                }
            }
        }

        if(useFrontCamera) {
            if (frontCam != null) {
                UseFrontCamera();            
            } else {
                Debug.Log("No front camera available");
            } 
        } else {
            if (backCam != null) {
                UseBackCamera();
            } else {
                Debug.Log("No back camera available");
            }
        }   

    }

#endregion

#region Private Helper Functions

    /// <summary>
    /// Attempt to allocate a local Texture2D for performing pixel manipulation
    /// </summary>
    /// <returns>
    /// Either a new Texture2D recently allocated, or the previously allocated
    /// once if no changes where made.
    /// </returns>
    private Texture2D AllocateWebcamTex()
    {
        int w = IsFrontCamera ?
                frontCam.width :
                IsBackCamera ?
                backCam.width :
                0;

        int h = IsFrontCamera ?
                frontCam.height :
                IsBackCamera ?
                backCam.height :
                0;

        // if (webcamTex != null) {
        //     return webcamTex; 
        // }
        // return new Texture2D(h, w, TextureFormat.RGB24, false);
        // return new Texture2D(w, h, TextureFormat.RGBA32, false);

        // early out on no texture size changes
        if (webcamTex != null)
        {
            if (webcamTex.width == w && webcamTex.height == h)
                return webcamTex;
            // deallocate
            else
                Debug.Log("Allocate new webcam texture");
                UnityEngine.Object.Destroy(webcamTex);
        }
        // allocate
        return new Texture2D(w, h, TextureFormat.RGB24, false);
    }

    private Texture2D AllocateDisplayTex () {
        int w = IsFrontCamera ?
                frontCam.width :
                IsBackCamera ?
                backCam.width :
                0;

        int h = IsFrontCamera ?
                frontCam.height :
                IsBackCamera ?
                backCam.height :
                0;

        // if (displayTex != null) {
        //     return displayTex; 
        // }
        // return new Texture2D(h, w, TextureFormat.RGB24, false);

        if (displayTex != null)
        {
            if (displayTex.width == w && displayTex.height == h)
                return displayTex;
            // deallocate
            else
                Debug.Log("Allocate new display texture");            
                UnityEngine.Object.Destroy(displayTex);
        }
        // allocate
        return new Texture2D(w, h, TextureFormat.RGB24, false);

    }

    /// <summary>
    /// Get the webcam's texture as pixel array
    /// </summary>
    /// <returns>
    /// An array of Color32 values representing the entire color buffer
    /// </returns>
    private Color32[] GetWebcamPixels(bool flipped)
    {
        int w = IsFrontCamera ?
                frontCam.width :
                IsBackCamera ?
                backCam.width :
                0;

        int h = IsFrontCamera ?
                frontCam.height :
                IsBackCamera ?
                backCam.height :
                0;               

        Color32[] color32Pixels = IsFrontCamera ?
               frontCam.GetPixels32() :
               IsBackCamera ?
               backCam.GetPixels32() :
               null;

        return color32Pixels;
    }

    /// <summary>
    /// Update aspect ratio fitter according to webcam texture aspect resolution
    /// </summary>
    private void UpdateAspectRatio()
    {
        if (fit == null)
            return;

        switch (usingCamera)
        {
            case UsingCamera.Front:
                fit.aspectRatio = (float)frontCam.width / (float)frontCam.height;
                break;
            case UsingCamera.Back:
                fit.aspectRatio = (float)backCam.width / (float)backCam.height;
                break;
        }

    }

    private void UseFrontCamera()
    {
        if (frontCam != null)
        {
            if (backCam != null && backCam.isPlaying)
                backCam.Stop();
            usingCamera = UsingCamera.Front;
            if (!frontCam.isPlaying)
                frontCam.Play();
        }
    }

    /// <summary>
    /// Try to use back camera if possible 
    /// </summary>
    private void UseBackCamera()
    {
        if (backCam != null)
        {
            if (frontCam != null && frontCam.isPlaying)
                frontCam.Stop();
            usingCamera = UsingCamera.Back;
            if (!backCam.isPlaying)
                backCam.Play();
        }
    }

    /// <summary>
    /// Checks that a given point (x,y) is within the texture space
    /// </summary>
    /// <param name="tex"> Texture reference </param>
    /// <param name="x"> x position </param>
    /// <param name="y"> y position </param>
    /// <returns> If position (x,y) is within coordinates (0,0), (width,height) </returns>
    private bool IsOutOfBounds(Texture2D tex, int x, int y)
    {
        return y < 0 || x < 0 || y > tex.height || x > tex.width;
    }
    private void drawPoint(Texture2D tex, int x, int y, Color col) {
        if(!IsOutOfBounds(tex, x, y)) {
            tex.SetPixel(x, y, col);
        }
    }
    private void drawPointsNearby(Texture2D tex, int x, int y, Color col) {
        int xm = x - 1;
        int xp = x + 1; 
        int ym = y - 1; 
        int yp = y + 1; 
        drawPoint(tex, xm, y, col);
        drawPoint(tex, x, y, col);
        drawPoint(tex, xp, y, col);
        drawPoint(tex, x, ym, col); 
        drawPoint(tex, x, yp, col);         
    }


    private void DrawLine(Texture2D tex, int x1, int y1, int x2, int y2, Color col) {
        int xDiff = Math.Abs(x1 - x2); 
        int yDiff = Math.Abs(y1 - y2); 
        if(xDiff > yDiff) {
            float a = (float) (y2 - y1) / (x2 - x1);
            float b = (float) (y1*x2 - x1*y2) / (x2 - x1); 
            
            int xStart = x1 < x2 ? x1: x2; 
            int xEnd = x1 > x2 ? x1: x2;
            for (int x = xStart; x<=xEnd; x++) {
                int y =(int) (a*x + b); 
                // drawPointsNearby(tex, x, y, col);
                drawPoint(tex, x, y, col);
            }                
        } else if (yDiff !=0) {
            float a = (float) (x2-x1) / (y2-y1); 
            float b = (float) (x1*y2 - x2*y1) / (y2-y1); 
            int yStart = y1 < y2 ? y1 : y2; 
            int yEnd = y1 > y2 ? y1 : y2; 
            for (int y=yStart; y<=yEnd; y++) {
                int x = (int) (a*y + b); 
                // drawPointsNearby(tex, x, y, col);
                drawPoint(tex, x, y, col);                
            }
        } 
    }

    private void DrawAxis(Texture2D tex, int[] coords) {
        // x axis
        DrawLine(tex, coords[0], coords[1], coords[2], coords[3], Color.red);
        // y axis
        DrawLine(tex, coords[0], coords[1], coords[4], coords[5], Color.green); 
        // z axis
        DrawLine(tex, coords[0], coords[1], coords[6], coords[7], Color.blue);
    }

    private void Draw2DBox(Texture2D tex, int[] coords) {
        DrawLine(tex, coords[0], coords[1], coords[0], coords[3], Color.red); 
        DrawLine(tex, coords[0], coords[1], coords[2], coords[1], Color.red); 
        DrawLine(tex, coords[2], coords[3], coords[0], coords[3], Color.red);
        DrawLine(tex, coords[2], coords[3], coords[2], coords[1], Color.red);
    }

    private void DrawBox(Texture2D tex, int[] coords) {
        //front face
        DrawLine(tex, coords[0], coords[1], coords[2], coords[3], Color.red);
        DrawLine(tex, coords[2], coords[3], coords[10], coords[11], Color.red);
        DrawLine(tex, coords[10], coords[11], coords[8], coords[9], Color.red);
        DrawLine(tex, coords[0], coords[1], coords[8], coords[9], Color.red);        

        //back face
        DrawLine(tex, coords[4], coords[5], coords[6], coords[7], Color.blue);
        DrawLine(tex, coords[4], coords[5], coords[12], coords[13], Color.blue);
        DrawLine(tex, coords[12], coords[13], coords[14], coords[15], Color.blue);
        DrawLine(tex, coords[6], coords[7], coords[14], coords[15], Color.blue);        
        
        // middle line
        DrawLine(tex, coords[4], coords[5], coords[2], coords[3], Color.green);
        DrawLine(tex, coords[0], coords[1], coords[6], coords[7], Color.green);
        DrawLine(tex, coords[12], coords[13], coords[10], coords[11], Color.green);
        DrawLine(tex, coords[8], coords[9], coords[14], coords[15], Color.green);        
    }

    private void DrawAxis(Texture2D tex, int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3) {
        // x axis
        DrawLine(tex, x0, y0, x1, y1, Color.red);
        // y axis
        DrawLine(tex, x0, y0, x2, y2, Color.green); 
        // z axis
        DrawLine(tex, x0, y0, x3, y3, Color.blue);
    }

#endregion

}
