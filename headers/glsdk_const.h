/*

Copyright (c) 2014, GlassLab, Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer. 
2. Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies, 
either expressed or implied, of the FreeBSD Project.

*/


//
//  glsdk_const.h
//  GlasslabSDK
//
//  Authors:
//      Joseph Sutton
//  Copyright (c) 2014 Glasslab. All rights reserved.
//

/* ---------------------------- */
#ifndef GLASSLABSDK_CONST
#define GLASSLABSDK_CONST
/* ---------------------------- */


namespace nsGlasslabSDK {
    
    typedef struct _glConfig {
        int eventsPeriodSecs;
        int eventsMinSize;
        int eventsMaxSize;
        int eventsDetailLevel;
    } glConfig;

    typedef struct _glUserInfo {
        std::string username;
        std::string firstName;
        std::string lastInitial;
        std::string email;
    } glUserInfo;

    class Const {
    public:
        enum Status {
            Status_Ok = 0,
            Status_Error
        };
        
        enum Message {
            Message_None = 0,
            Message_Connect,
			Message_ConnectFail,
            Message_DeviceUpdate,
            Message_AuthStatus,
            Message_Register,
            Message_Login,
            Message_Logout,
            Message_Enroll,
            Message_UnEnroll,
            Message_GetCourses,
            Message_StartSession,
            Message_EndSession,
            Message_GameSave,
            Message_GetGameSave,
            Message_DeleteGameSave,
            Message_GetUserInfo,
            Message_SaveAchievement,
            Message_SavePlayerInfo,
            Message_GetPlayerInfo,
            Message_SendTotalTimePlayed,
            Message_Event,
            Message_Error
        };

        struct Response {
            Message m_message;
            std::string m_data;
        };
    };
    
}; // end nsGlasslabSDK

/* ---------------------------- */
#endif /* defined(GLASSLABSDK_CONST) */
/* ---------------------------- */