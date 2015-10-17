//
//  AppDelegate.m
//  GlasslabSDKTestBed
//
//  Created by Joe Pearce on 10/16/15.
//  Copyright © 2015 Glasslab Games. All rights reserved.
//

#import "AppDelegate.h"
#import <string>
#import <iostream>
#import "glasslab_sdk.h"

GlasslabSDK *glsdk;

char host[]         =   "https://developer.playfully.org";

char gameId[]       =   "TEST";
char gameName[]     =   "Test Game";
char gameVersion[]  =   "1.0";
char gameLevel[]    =   "Level 1";

char deviceId[]     =   "test_mac";

@interface AppDelegate ()

@property (weak) IBOutlet NSWindow *window;

@property () bool reportDebugLog;

@end

@implementation AppDelegate

- (void)applicationWillFinishLaunching:(NSNotification *)aNotification {
    GlasslabSDK::initSDK();
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    self.reportDebugLog = NO;
    
    NSArray *args = [NSProcessInfo processInfo].arguments;
    NSMutableString *s = [NSMutableString stringWithString:@""];
    for (NSString *arg in args) {
        [s appendString:arg];
        [s appendString:@"\n"];
    }
    
    [self.messageTextView replaceCharactersInRange:NSMakeRange(0, self.messageTextView.string.length) withString:s];
}

- (void)applicationWillTerminate:(NSNotification *)aNotification {
    // Insert code here to tear down your application
}

- (void)reportMessages {
    if (glsdk != NULL) {
        NSMutableString *s = [NSMutableString stringWithString:@""];
        const char *msg;
        
        while ((msg = glsdk->readTopMessageString()) != NULL) {
            [s appendString:[NSString stringWithCString:msg encoding:NSUTF8StringEncoding]];
            [s appendString:@"\n"];
            glsdk->popMessageStack();
        }
        
        if (self.reportDebugLog) {
            while (strlen(msg = glsdk->popLogQueue()) > 0) {
                [s appendString:@"LOG: "];
                [s appendString:[NSString stringWithCString:msg encoding:NSUTF8StringEncoding]];
                [s appendString:@"\n"];
            }
        }
        
        [s appendString:@"Status: "];
        [s appendString:(glsdk->getLastStatus() == nsGlasslabSDK::Const::Status_Ok ? @"OK" : @"Error")];
    
        [self.messageTextView replaceCharactersInRange:NSMakeRange(0, self.messageTextView.string.length) withString:s];
    }
}

- (void)reportPendingLogs {
    if (glsdk != NULL) {
        NSMutableString *s = [NSMutableString stringWithString:@""];
        const char *msg;
        
        if (self.reportDebugLog) {
            while (strlen(msg = glsdk->popLogQueue()) > 0) {
                [s appendString:@"LOG: "];
                [s appendString:[NSString stringWithCString:msg encoding:NSUTF8StringEncoding]];
                [s appendString:@"\n"];
            }
        }

        [self.messageTextView replaceCharactersInRange:NSMakeRange(0, self.messageTextView.string.length) withString:s];
    }
}

- (IBAction)connect:(id)sender {
    if (glsdk == NULL) {
        glsdk = new GlasslabSDK(gameId, deviceId, NULL, host);
    } else {
        glsdk->connect(gameId, host);
    }
    
    [self reportMessages];
}

- (IBAction)local:(id)sender {
    if (glsdk == NULL) {
        glsdk = new GlasslabSDK(gameId, deviceId, NULL, "https://127.0.0.1");
    } else {
        glsdk->connect(gameId, "https://127.0.0.1");
    }
    
    [self reportMessages];
}

- (IBAction)login:(id)sender {
    if (glsdk == NULL) return;
    
    glsdk->login([self.loginUserNameField.stringValue cStringUsingEncoding:NSUTF8StringEncoding], [self.loginPasswordField.stringValue cStringUsingEncoding:NSUTF8StringEncoding], NULL);
    
    [self reportMessages];
}

- (IBAction)logout:(id)sender {
    if (glsdk == NULL) return;
    
    glsdk->logout();
    
    [self reportMessages];
}

- (IBAction)getAuthStatus:(id)sender {
    if (glsdk == NULL) return;
    
    glsdk->authStatus();
    
    [self reportMessages];
}

- (IBAction)getUserInfo:(id)sender {
    if (glsdk == NULL) return;
    
    glsdk->getUserInfo();
    
    [self reportMessages];
}

- (IBAction)getPlayerInfo:(id)sender {
    if (glsdk == NULL) return;
    
    glsdk->getPlayerInfo();
    
    [self reportMessages];
}

- (IBAction)enroll:(id)sender {
    if (glsdk == NULL) return;
    
    glsdk->enroll([self.enrollField.stringValue cStringUsingEncoding:NSUTF8StringEncoding]);
    
    [self reportMessages];
}

- (IBAction)getCourses:(id)sender {
    if (glsdk == NULL) return;
    
    glsdk->getCourses();
    
    [self reportMessages];
}

- (IBAction)startSession:(id)sender {
    if (glsdk == NULL) return;
    
    glsdk->startSession();
    
    [self reportMessages];
}

- (IBAction)endSession:(id)sender {
    if (glsdk == NULL) return;
    
    glsdk->endSession();
    
    [self reportMessages];
}

- (IBAction)saveTelemEvent:(id)sender {
    if (glsdk == NULL) return;
    
    glsdk->addTelemEventValue("fakedata", "data");
    glsdk->saveTelemEvent("telem name");
    
    [self reportMessages];
}

- (IBAction)saveAchievement:(id)sender {
    if (glsdk == NULL) return;
    
    glsdk->saveAchievement("item", "group", "subGroup");
    
    [self reportMessages];
}

- (IBAction)postSaveGame:(id)sender {
    if (glsdk == NULL) return;

    glsdk->saveGame("{test: \"test val\"}");
    
    [self reportMessages];
}

- (IBAction)getSaveGame:(id)sender {
    if (glsdk == NULL) return;
    
    glsdk->getSaveGame();
    
    [self reportMessages];
}

- (IBAction)showDebug:(id)sender {
    bool oldState = self.reportDebugLog;
    self.reportDebugLog = ((NSButton *)sender).state == NSOnState;
    if (!oldState && self.reportDebugLog) {
        [self reportPendingLogs];
    }
}
@end
