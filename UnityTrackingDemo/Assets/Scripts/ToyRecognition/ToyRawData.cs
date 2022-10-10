using UnityEngine;

public static class ToyRawData
{
    public static volatile Texture2D captureTexture = null;

    public static void Destroy(bool force = false)
    {
        //if (force)
        //{
           // UnityEngine.Object.Destroy(captureTexture);
        //}
        captureTexture = null;
    }
}