//
//  ViewController.m
//  GLG-iOS-SDK-Test
//
//  Created by Joe Pearce on 10/21/15.
//  Copyright © 2015 Glasslab Games. All rights reserved.
//

#import "ViewController.h"

#define USE_SDK

#ifdef USE_SDK
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
#endif

@interface ViewController ()
@property (weak, nonatomic) IBOutlet UITextField *username;
@property (weak, nonatomic) IBOutlet UITextField *password;
@property (weak, nonatomic) IBOutlet UITextField *enrollCourseID;
@property (weak, nonatomic) IBOutlet UITextView *messages;

@property () bool reportDebugLog;

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view, typically from a nib.
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (void)reportMessages {
#ifdef USE_SDK
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
    
        self.messages.text = s;
    }
#endif
}

- (void)reportPendingLogs {
#ifdef USE_SDK
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

        self.messages.text = s;
    }
#endif
}

- (IBAction)connect:(id)sender {
#ifdef USE_SDK
    if (glsdk == NULL) {
        glsdk = new GlasslabSDK(gameId, deviceId, NULL, host);
    } else {
        glsdk->connect(gameId, host);
    }
    
    [self reportMessages];
#endif
}

- (IBAction)local:(id)sender {
#ifdef USE_SDK
    if (glsdk == NULL) {
        glsdk = new GlasslabSDK(gameId, deviceId, NULL, "https://127.0.0.1");
    } else {
        glsdk->connect(gameId, "https://127.0.0.1");
    }
    
    [self reportMessages];
#endif
}

- (IBAction)login:(id)sender {
#ifdef USE_SDK
    if (glsdk == NULL) return;
    
    glsdk->login([self.username.text cStringUsingEncoding:NSUTF8StringEncoding], [self.password.text cStringUsingEncoding:NSUTF8StringEncoding], NULL);
    
    [self reportMessages];
#endif
}

- (IBAction)logout:(id)sender {
#ifdef USE_SDK
    if (glsdk == NULL) return;
    
    glsdk->logout();
    
    [self reportMessages];
#endif
}

- (IBAction)getAuthStatus:(id)sender {
#ifdef USE_SDK
    if (glsdk == NULL) return;
    
    glsdk->authStatus();
    
    [self reportMessages];
#endif
}

- (IBAction)getUserInfo:(id)sender {
#ifdef USE_SDK
    if (glsdk == NULL) return;
    
    glsdk->getUserInfo();
    
    [self reportMessages];
#endif
}

- (IBAction)getPlayerInfo:(id)sender {
#ifdef USE_SDK
    if (glsdk == NULL) return;
    
    glsdk->getPlayerInfo();
    
    [self reportMessages];
#endif
}

- (IBAction)getCourses:(id)sender {
#ifdef USE_SDK
    if (glsdk == NULL) return;
    
    glsdk->getCourses();
    
    [self reportMessages];
#endif
}

- (IBAction)enroll:(id)sender {
#ifdef USE_SDK
    if (glsdk == NULL) return;
    
    glsdk->enroll([self.enrollCourseID.text cStringUsingEncoding:NSUTF8StringEncoding]);
    
    [self reportMessages];
#endif
}

- (IBAction)startSession:(id)sender {
#ifdef USE_SDK
    if (glsdk == NULL) return;
    
    glsdk->startSession();
    
    [self reportMessages];
#endif
}

- (IBAction)endSession:(id)sender {
#ifdef USE_SDK
    if (glsdk == NULL) return;
    
    glsdk->endSession();
    
    [self reportMessages];
#endif
}

- (IBAction)saveTelemEvent:(id)sender {
#ifdef USE_SDK
    if (glsdk == NULL) return;
    
    glsdk->addTelemEventValue("fakedata", "data");
    glsdk->saveTelemEvent("telem name");
    
    [self reportMessages];
#endif
}

- (IBAction)saveAchievement:(id)sender {
#ifdef USE_SDK
    if (glsdk == NULL) return;
    
    glsdk->saveAchievement("item", "group", "subGroup");
    
    [self reportMessages];
#endif
}

- (IBAction)postSaveGame:(id)sender {
#ifdef USE_SDK
    if (glsdk == NULL) return;

    glsdk->saveGame("{test: \"test val\"}");
    
    [self reportMessages];
#endif
}

- (IBAction)getSaveGame:(id)sender {
#ifdef USE_SDK
    if (glsdk == NULL) return;
    
    glsdk->getSaveGame();
    
    [self reportMessages];
#endif
}

- (IBAction)showDebug:(id)sender {
#ifdef USE_SDK
    bool oldState = self.reportDebugLog;
    self.reportDebugLog = ((UISwitch *)sender).on;
    if (!oldState && self.reportDebugLog) {
        [self reportPendingLogs];
    }
#endif
}

@end
