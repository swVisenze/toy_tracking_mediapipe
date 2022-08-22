#import "ToyTracking.h"
#import "UnityBridge.h"
#import <Foundation/Foundation.h>

ToyTracking* toyTracking;
void native_toy_tracking_init(int width, int height) {
    if(toyTracking != nil) {
        [toyTracking destroy];
    }
    toyTracking = [[ToyTracking alloc] init];
    [toyTracking loadGraph:width height:height];
}

void native_toy_tracking_reset(const char* code) {
    [toyTracking reset: code];
}

void native_toy_tracking_destroy() {
    [toyTracking destroy];
    toyTracking = nil;
}

char* native_toy_tracking_tracking(const unsigned char* image, int size, int width, int height) {
//    return [toyTracking tracking:(image) size:(size) width:(width) height:(height)];
    NSString* resultJsonStr = [toyTracking tracking:(image) size:(size) width:(width) height:(height)];
//    NSLog(@"native_toy_tracking_tracking: %@",resultJsonStr);
    char* res = (char*)malloc(strlen([resultJsonStr UTF8String]) + 1);
    strcpy(res, [resultJsonStr UTF8String]);
    return res;
}