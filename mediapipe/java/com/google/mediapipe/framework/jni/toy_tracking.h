#ifndef COM_VISENZE_TOY_TRACKING_H_
#define COM_VISENZE_TOY_TRACKING_H_

#include <jni.h>
#include <stdio.h>
#include<string.h>

extern "C" {

JNIEXPORT void toy_tracking_init();

JNIEXPORT void toy_tracking_reset(const char* code);

JNIEXPORT const char* toy_tracking_tracking();

JNIEXPORT void toy_tracking_destroy();
}

#endif  // COM_VISENZE_TOY_TRACKING_H_