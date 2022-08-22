#ifndef UnityBridge_h
#define UnityBridge_h

extern "C" {
    extern void native_toy_tracking_init(int width, int height);
    extern void native_toy_tracking_reset(const char* code);
    extern void native_toy_tracking_destroy();
    extern char* native_toy_tracking_tracking(const unsigned char* image, int size, int width, int height);
}

#endif /* UnityBridge_h */
