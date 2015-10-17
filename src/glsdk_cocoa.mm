//
//  glsdk_cocoa.m
//  GlasslabSDK
//
//  Created by Joe Pearce on 10/15/15.
//  Copyright © 2015 Glasslab. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <string>
#import <map>

using namespace std;

static __strong NSString *launchURL = nil;

map<string, string> GetLaunchParameters()
{
    map<string, string> params;
    
    // get command line argument array
    /*
    NSArray *args = [NSProcessInfo processInfo].arguments;
    if (args.count > 1) {
        // only one real argument is expected, so no parameter value can include a space
        NSURL *url = [NSURL URLWithString:args[1]];
    */
    if (launchURL != nil) {
        NSURL *url = [NSURL URLWithString:launchURL];
     
        for (NSString *qs in [url.query componentsSeparatedByString:@"&"]) {
            NSArray *components = [qs componentsSeparatedByString:@"="];
            if (components.count == 2) {
                // Get the parameter name
                NSString *key = [components objectAtIndex:0];
                // Get the parameter value
                NSString *value = [components objectAtIndex:1];
                value = [value stringByReplacingOccurrencesOfString:@"+" withString:@" "];
                value = [value stringByReplacingPercentEscapesUsingEncoding:NSUTF8StringEncoding];
            
                const char *ckey = [key cStringUsingEncoding:NSUTF8StringEncoding];
                const char *cvalue = [value cStringUsingEncoding:NSUTF8StringEncoding];
            
                params[ckey] = cvalue;
            }
        }
    }
    
    return params;
}

@interface LaunchParameterHandler : NSObject

- (void)handleGetURLEvent:(NSAppleEventDescriptor *)event
           withReplyEvent:(NSAppleEventDescriptor *)replyEvent;
@end

@implementation LaunchParameterHandler

- (void)handleGetURLEvent:(NSAppleEventDescriptor *)event
           withReplyEvent:(NSAppleEventDescriptor *)replyEvent {

    // Extract the URL from the Apple event and handle it here.
    launchURL = [[event paramDescriptorForKeyword:keyDirectObject] stringValue];
}

@end

static __strong LaunchParameterHandler *handler = nil;

extern "C" void Init_Mac_GLSDK() {
    handler = [[LaunchParameterHandler alloc] init];
    
    NSAppleEventManager *appleEventManager = [NSAppleEventManager sharedAppleEventManager];
    [appleEventManager setEventHandler:handler andSelector:@selector(handleGetURLEvent:withReplyEvent:) forEventClass:kInternetEventClass andEventID:kAEGetURL];
}

