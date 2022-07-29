//
//  ToyTracking.h
//  Mediapipe
//
//  Created by li shiwei on 23/7/22.
//

#ifndef ToyTracking_h
#define ToyTracking_h

#import <Foundation/Foundation.h>



@interface ToyTracking : NSObject
- (void)loadGraph:(int) width
      height: (int) height;

- (void)destroy;

- (bool)reset:(const char*) code;

- (NSString*)tracking:(const unsigned char*)image
        size:(int)size
        width:(int)width
        height:(int)height;

@end
#endif /* ToyTracking_h */
