//
//  AppDelegate.h
//  GlasslabSDKTestBed
//
//  Created by Joe Pearce on 10/16/15.
//  Copyright © 2015 Glasslab Games. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface AppDelegate : NSObject <NSApplicationDelegate>

- (IBAction)connect:(id)sender;
- (IBAction)local:(id)sender;
- (IBAction)login:(id)sender;
- (IBAction)logout:(id)sender;
- (IBAction)getAuthStatus:(id)sender;
- (IBAction)getUserInfo:(id)sender;
- (IBAction)getPlayerInfo:(id)sender;
- (IBAction)enroll:(id)sender;
- (IBAction)getCourses:(id)sender;
- (IBAction)startSession:(id)sender;
- (IBAction)endSession:(id)sender;
- (IBAction)saveTelemEvent:(id)sender;
- (IBAction)saveAchievement:(id)sender;
- (IBAction)postSaveGame:(id)sender;
- (IBAction)getSaveGame:(id)sender;
- (IBAction)showDebug:(id)sender;

@property (unsafe_unretained) IBOutlet NSTextView *messageTextView;

@property (weak) IBOutlet NSTextField *loginUserNameField;
@property (weak) IBOutlet NSSecureTextField *loginPasswordField;
@property (weak) IBOutlet NSTextField *enrollField;


@end

