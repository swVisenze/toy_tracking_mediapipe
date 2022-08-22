#ifndef COM_VISENZE_TOY_TRACKING_H_
#define COM_VISENZE_TOY_TRACKING_H_

#include <jni.h>
#include <stdio.h>
#include<string.h>

extern "C" {

#define TOY_TRACKING_OUTPUT_METHOD(METHOD_NAME) \
  Java_com_visenze_ToyTracking_##METHOD_NAME

JNIEXPORT void JNICALL TOY_TRACKING_OUTPUT_METHOD(nativeInit)(
        JNIEnv* env, jobject thiz, jint jwidth, jint jheight);

JNIEXPORT void JNICALL TOY_TRACKING_OUTPUT_METHOD(nativeDestroy)(
    JNIEnv* env, jobject thiz);

//JNIEXPORT void toy_tracking_init();

//JNIEXPORT void toy_tracking_destroy();

JNIEXPORT bool toy_tracking_reset(int frames);

JNIEXPORT const char* toy_tracking_tracking(const unsigned char* image_buffer, int size, int image_width, int image_height);


}

#endif  // COM_VISENZE_TOY_TRACKING_H_