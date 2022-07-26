//
//  ToyTracking.h
//  Mediapipe
//
//  Created by li shiwei on 23/7/22.
//

#ifndef ToyTracking_h
#define ToyTracking_h

//#define CEXPORT __attribute__ ((visibility ("default")))

extern "C" {
    extern void native_toy_tracking_init(int width, int height);
    extern void native_toy_tracking_reset(const char* code);
    extern void native_toy_tracking_destroy();
    extern const char* native_toy_tracking_tracking(const char* image, int size, int width, int height);
}





#endif /* ToyTracking_h */
