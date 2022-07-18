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

JNIEXPORT void toy_tracking_reset(const char* code);

JNIEXPORT const char* toy_tracking_tracking();


}

#endif  // COM_VISENZE_TOY_TRACKING_H_