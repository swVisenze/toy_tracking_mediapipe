using System.Collections;
using System.Collections.Generic;
using Unity.Collections;
using UnityEngine;
using UnityEngine.UI;

public class CameraCapture : MonoBehaviour
{
    private int IMAGE_SIZE = 256;

    // callback signature when a capture is done
    public delegate void OnCaptureTexture(Texture2D tex);
    // callback registration 
    public event OnCaptureTexture onCaptureTexture;
    // webcam texture to use as reference to capture
    private Texture2D cameraTexture = null;
    // cached copy of the webcam texture we capture
    private Texture2D cachedTex2D = null;
    // temporary render texture to perform graphics blit from
    private RenderTexture cachedRenderTex = null;

    // Getter/setter for which WebCamTexture we are referencing to 
    public Texture2D CameraTexture
    {
        get { return cameraTexture; }
        set { cameraTexture = value; }
    }


    private void OnDestroy()
    {
        // deallocate all allocated resources 
        if (cachedTex2D)
            Object.Destroy(cachedTex2D);
        if (cachedRenderTex)
            RenderTexture.ReleaseTemporary(cachedRenderTex);
    }

    public Texture2D GetImageTexure() {
        Texture texToCapture = cameraTexture;
        // int adjustedWidth = IMAGE_SIZE;
        // int adjustedHeight = IMAGE_SIZE; 
        int adjustedWidth = cameraTexture.width / 2;
        int adjustedHeight = cameraTexture.height / 2; 
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

        Graphics.Blit(texToCapture, cachedRenderTex);
#endif        
        // set the temp buffer as the active render texture
        RenderTexture.active = cachedRenderTex;

        // get the cached camera texture to read from the current active render texture
        cachedTex2D.ReadPixels(new Rect(0, 0, adjustedWidth, adjustedHeight), 0, 0);
        cachedTex2D.Apply();
        RenderTexture.active = currRt;
        return cachedTex2D;
    }

}
