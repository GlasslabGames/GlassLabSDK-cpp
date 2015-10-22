#import <UIKit/UIKit.h>
#import <string>
#import <map>

using namespace std;

static __strong NSURL *launchURL = nil;

map<string, string> GetLaunchParameters()
{
    map<string, string> params;
    
    if (launchURL != nil) {
        for (NSString *qs in [launchURL.query componentsSeparatedByString:@"&"]) {
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

extern "C" void Init_iOS_GLSDK(void *_launchOptions)
{
    NSDictionary *launchOptions = (__bridge_transfer id)_launchOptions;

    if ([launchOptions objectForKey:UIApplicationLaunchOptionsURLKey]) {
        launchURL = [launchOptions objectForKey:UIApplicationLaunchOptionsURLKey];
    }
}

