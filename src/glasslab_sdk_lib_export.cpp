//
//  glasslab_sdk_lib_export.cpp
//  GlasslabSDK
//
//  Authors:
//      Joseph Sutton
//      Ben Dapkiewicz
//
//  Copyright (c) 2014 Glasslab. All rights reserved.
//

#if WIN32
#define APIEXPORT __declspec(dllexport) 
#else
#define APIEXPORT 
#endif

#include "glasslab_sdk.h"

extern "C"
{
    APIEXPORT void* GlasslabSDK_CreateInstance( const char* clientId, const char* deviceId = NULL, const char* dataPath = NULL, const char* uri = NULL )
    {
        return (void *)( new GlasslabSDK( clientId, deviceId, dataPath, uri ) );
    }
    
    APIEXPORT void GlasslabSDK_FreeInstance( void* inst ) {
        if( inst != NULL ) {
            GlasslabSDK* tmp = static_cast<GlasslabSDK *>( inst );
            delete tmp;
        }
    }


    APIEXPORT nsGlasslabSDK::Const::Status GlasslabSDK_GetLastStatus( void* inst ) {
        if( inst != NULL ) {
            return static_cast<GlasslabSDK *>( inst )->getLastStatus();
        }
        else {
            return nsGlasslabSDK::Const::Status_Error;
        }
    }
    
    APIEXPORT void GlasslabSDK_PopMessageStack( void* inst ) {
        if( inst != NULL ) {
            static_cast<GlasslabSDK *>( inst )->popMessageStack();
        }
    }
    
    APIEXPORT int GlasslabSDK_ReadTopMessageCode( void* inst ) {
        if( inst != NULL ) {
            return ( static_cast<GlasslabSDK *>( inst )->readTopMessageCode() );
        } else {
            return nsGlasslabSDK::Const::Message_Error;
        }
    }
    
    APIEXPORT const char * GlasslabSDK_ReadTopMessageString( void* inst ) {
        if( inst != NULL ) {
            return static_cast<GlasslabSDK *>( inst )->readTopMessageString();
        } else {
            return NULL;
        }
    }


    APIEXPORT void GlasslabSDK_Connect( void* inst, const char* gameId, const char* uri ) {
        if( inst != NULL ) {
            static_cast<GlasslabSDK *>( inst )->connect( gameId, uri );
        }
    }
    
    APIEXPORT void GlasslabSDK_DeviceUpdate( void* inst ) {
        if( inst != NULL ) {
            static_cast<GlasslabSDK *>( inst )->deviceUpdate();
        }
    }

    APIEXPORT void GlasslabSDK_AuthStatus( void* inst ) {
        if( inst != NULL ) {
            static_cast<GlasslabSDK *>( inst )->authStatus();
        }
    }

    APIEXPORT void GlasslabSDK_RegisterStudent( void* inst, const char* username, const char* password, const char* firstName, const char* lastInitial ) {
        if( inst != NULL ) {
            static_cast<GlasslabSDK *>( inst )->registerStudent( username, password, firstName, lastInitial );
        }
    }

    APIEXPORT void GlasslabSDK_RegisterInstructor( void* inst, const char* name, const char* email, const char* password, bool newsletter ) {
        if( inst != NULL ) {
            static_cast<GlasslabSDK *>( inst )->registerInstructor( name, email, password, newsletter );
        }
    }

    APIEXPORT void GlasslabSDK_GetUserInfo( void* inst ) {
        if( inst != NULL ) {
            static_cast<GlasslabSDK *>( inst )->getUserInfo();
        }
    }

    APIEXPORT void GlasslabSDK_Login( void* inst, const char* username, const char* password, const char* type ) {
        if( inst != NULL ) {
            static_cast<GlasslabSDK *>( inst )->login( username, password, type );
        }
    }

    APIEXPORT void GlasslabSDK_Enroll( void* inst, const char* courseCode ) {
        if( inst != NULL ) {
            static_cast<GlasslabSDK *>( inst )->enroll( courseCode );
        }
    }
    
    APIEXPORT void GlasslabSDK_UnEnroll( void* inst, const char* courseId ) {
        if( inst != NULL ) {
            static_cast<GlasslabSDK *>( inst )->unenroll( courseId );
        }
    }

    APIEXPORT void GlasslabSDK_GetCourses( void* inst ) {
        if( inst != NULL ) {
            static_cast<GlasslabSDK *>( inst )->getCourses();
        }
    }

    APIEXPORT void GlasslabSDK_Logout( void* inst, const char* username, const char* password ) {
        if( inst != NULL ) {
            static_cast<GlasslabSDK *>( inst )->logout();
        }
    }
    
    APIEXPORT void GlasslabSDK_StartSession( void* inst ) {
        if( inst != NULL ) {
            static_cast<GlasslabSDK *>( inst )->startSession();
        }
    }
    
    APIEXPORT void GlasslabSDK_EndSession( void* inst ) {
        if( inst != NULL ) {
            static_cast<GlasslabSDK *>( inst )->endSession();
        }
    }
    
    APIEXPORT void GlasslabSDK_SaveGame( void* inst, const char* gameData ) {
        if( inst != NULL ) {
            static_cast<GlasslabSDK *>( inst )->saveGame( gameData );
        }
    }

    APIEXPORT void GlasslabSDK_GetSaveGame( void* inst ) {
        if( inst != NULL ) {
            static_cast<GlasslabSDK *>( inst )->getSaveGame();
        }
    }

    APIEXPORT void GlasslabSDK_DeleteSaveGame( void* inst ) {
        if( inst != NULL ) {
            static_cast<GlasslabSDK *>( inst )->deleteSaveGame();
        }
    }

    APIEXPORT void GlasslabSDK_SaveAchievement( void* inst, const char* item, const char* group, const char* subGroup ) {
        if( inst != NULL ) {
            static_cast<GlasslabSDK *>( inst )->saveAchievement( item, group, subGroup );
        }
    }

    APIEXPORT void GlasslabSDK_SendTelemEvents( void* inst ) {
        if( inst != NULL ) {
            static_cast<GlasslabSDK *>( inst )->sendTelemEvents();
        }
    }

    APIEXPORT void GlasslabSDK_CancelRequest( void* inst, const char* key ) {
        if( inst != NULL ) {
            static_cast<GlasslabSDK *>( inst )->cancelRequest( key );
        }
    }

    
    APIEXPORT void GlasslabSDK_AddTelemEventValue_ccp   ( void* inst, const char* key, const char* value )    { if( inst != NULL ) static_cast<GlasslabSDK *>( inst )->addTelemEventValue( key, value ); }
    APIEXPORT void GlasslabSDK_AddTelemEventValue_int8  ( void* inst, const char* key, int8_t value )         { if( inst != NULL ) static_cast<GlasslabSDK *>( inst )->addTelemEventValue( key, value ); }
    APIEXPORT void GlasslabSDK_AddTelemEventValue_int16 ( void* inst, const char* key, int16_t value )        { if( inst != NULL ) static_cast<GlasslabSDK *>( inst )->addTelemEventValue( key, value ); }
    APIEXPORT void GlasslabSDK_AddTelemEventValue_int32 ( void* inst, const char* key, int32_t value )        { if( inst != NULL ) static_cast<GlasslabSDK *>( inst )->addTelemEventValue( key, value ); }
    APIEXPORT void GlasslabSDK_AddTelemEventValue_uint8 ( void* inst, const char* key, uint8_t value )        { if( inst != NULL ) static_cast<GlasslabSDK *>( inst )->addTelemEventValue( key, value ); }
    APIEXPORT void GlasslabSDK_AddTelemEventValue_uint16( void* inst, const char* key, uint16_t value )       { if( inst != NULL ) static_cast<GlasslabSDK *>( inst )->addTelemEventValue( key, value ); }
    APIEXPORT void GlasslabSDK_AddTelemEventValue_uint32( void* inst, const char* key, uint32_t value )       { if( inst != NULL ) static_cast<GlasslabSDK *>( inst )->addTelemEventValue( key, value ); }
    APIEXPORT void GlasslabSDK_AddTelemEventValue_float ( void* inst, const char* key, float value )          { if( inst != NULL ) static_cast<GlasslabSDK *>( inst )->addTelemEventValue( key, value ); }
    APIEXPORT void GlasslabSDK_AddTelemEventValue_double( void* inst, const char* key, double value )         { if( inst != NULL ) static_cast<GlasslabSDK *>( inst )->addTelemEventValue( key, value ); }
    APIEXPORT void GlasslabSDK_AddTelemEventValue_bool  ( void* inst, const char* key, bool value )           { if( inst != NULL ) static_cast<GlasslabSDK *>( inst )->addTelemEventValue( key, value ); }

    APIEXPORT void GlasslabSDK_ClearTelemEventValues( void* inst ) {
        if( inst != NULL ) {
            static_cast<GlasslabSDK *>( inst )->clearTelemEventValues();
        }
    }

    APIEXPORT void GlasslabSDK_SaveTelemEvent( void* inst, const char* name ) {
        if( inst != NULL ) {
            static_cast<GlasslabSDK *>( inst )->saveTelemEvent( name );
        }
    }


    APIEXPORT void GlasslabSDK_UpdatePlayerInfoKey_ccp   ( void* inst, const char* key, const char* value )    { if( inst != NULL ) static_cast<GlasslabSDK *>( inst )->updatePlayerInfoKey( key, value ); }
    APIEXPORT void GlasslabSDK_UpdatePlayerInfoKey_int8  ( void* inst, const char* key, int8_t value )         { if( inst != NULL ) static_cast<GlasslabSDK *>( inst )->updatePlayerInfoKey( key, value ); }
    APIEXPORT void GlasslabSDK_UpdatePlayerInfoKey_int16 ( void* inst, const char* key, int16_t value )        { if( inst != NULL ) static_cast<GlasslabSDK *>( inst )->updatePlayerInfoKey( key, value ); }
    APIEXPORT void GlasslabSDK_UpdatePlayerInfoKey_int32 ( void* inst, const char* key, int32_t value )        { if( inst != NULL ) static_cast<GlasslabSDK *>( inst )->updatePlayerInfoKey( key, value ); }
    APIEXPORT void GlasslabSDK_UpdatePlayerInfoKey_uint8 ( void* inst, const char* key, uint8_t value )        { if( inst != NULL ) static_cast<GlasslabSDK *>( inst )->updatePlayerInfoKey( key, value ); }
    APIEXPORT void GlasslabSDK_UpdatePlayerInfoKey_uint16( void* inst, const char* key, uint16_t value )       { if( inst != NULL ) static_cast<GlasslabSDK *>( inst )->updatePlayerInfoKey( key, value ); }
    APIEXPORT void GlasslabSDK_UpdatePlayerInfoKey_uint32( void* inst, const char* key, uint32_t value )       { if( inst != NULL ) static_cast<GlasslabSDK *>( inst )->updatePlayerInfoKey( key, value ); }
    APIEXPORT void GlasslabSDK_UpdatePlayerInfoKey_float ( void* inst, const char* key, float value )          { if( inst != NULL ) static_cast<GlasslabSDK *>( inst )->updatePlayerInfoKey( key, value ); }
    APIEXPORT void GlasslabSDK_UpdatePlayerInfoKey_double( void* inst, const char* key, double value )         { if( inst != NULL ) static_cast<GlasslabSDK *>( inst )->updatePlayerInfoKey( key, value ); }
    APIEXPORT void GlasslabSDK_UpdatePlayerInfoKey_bool  ( void* inst, const char* key, bool value )           { if( inst != NULL ) static_cast<GlasslabSDK *>( inst )->updatePlayerInfoKey( key, value ); }
    APIEXPORT void GlasslabSDK_RemovePlayerInfoKey( void* inst, const char* key ) { if( inst != NULL ) static_cast<GlasslabSDK *>( inst )->removePlayerInfoKey( key ); }


    APIEXPORT void GlasslabSDK_SetName( void* inst, const char* name ) {
        if( inst != NULL ) {
            static_cast<GlasslabSDK *>( inst )->setName( name );
        }
    }
    
    APIEXPORT void GlasslabSDK_SetVersion(void* inst, const char* version ) {
        if( inst != NULL ) {
            static_cast<GlasslabSDK *>( inst )->setVersion( version );
        }
    }
    
    APIEXPORT void GlasslabSDK_SetGameLevel( void* inst, const char* gameLevel ) {
        if( inst != NULL ) {
            static_cast<GlasslabSDK *>( inst )->setGameLevel( gameLevel );
        }
    }

    APIEXPORT void GlasslabSDK_SetUserId( void* inst, int userId ) {
        if( inst != NULL ) {
            static_cast<GlasslabSDK *>( inst )->setUserId( userId );
        }
    }

    APIEXPORT void GlasslabSDK_SetPlayerHandle( void* inst, const char* handle ) {
        if( inst != NULL ) {
            static_cast<GlasslabSDK *>( inst )->setPlayerHandle( handle );
        }
    }

    APIEXPORT void GlasslabSDK_RemovePlayerHandle( void* inst, const char* handle ) {
        if( inst != NULL ) {
            static_cast<GlasslabSDK *>( inst )->removePlayerHandle( handle );
        }
    }

    APIEXPORT void GlasslabSDK_SetCookie( void* inst, const char* cookie ) {
        if( inst != NULL ) {
            static_cast<GlasslabSDK *>( inst )->setCookie( cookie );
        }
    }

    APIEXPORT void GlasslabSDK_SetAutoSessionManagement( void* inst, bool state ) {
        if( inst != NULL ) {
            static_cast<GlasslabSDK *>( inst )->setAutoSessionManagement( state );
        }
    }


    APIEXPORT void GlasslabSDK_StartGameTimer( void* inst ) {
        if( inst != NULL ) {
            static_cast<GlasslabSDK *>( inst )->startGameTimer();
        }
    }

    APIEXPORT void GlasslabSDK_StopGameTimer( void* inst ) {
        if( inst != NULL ) {
            static_cast<GlasslabSDK *>( inst )->stopGameTimer();
        }
    }


    APIEXPORT void GlasslabSDK_ResetDatabase( void* inst ) {
        if( inst != NULL ) {
            static_cast<GlasslabSDK *>( inst )->resetDatabase();
        }
    }


    APIEXPORT  const char * GlasslabSDK_GetConnectUri( void* inst) {
        if( inst != NULL ) {
            return static_cast<GlasslabSDK *>( inst )->getConnectUri();
        } else {
            return NULL;
        }
    }


    APIEXPORT int GlasslabSDK_GetUserId( void* inst ) {
        if( inst != NULL ) {
            return static_cast<GlasslabSDK *>( inst )->getUserId();
        } else {
            return -1;
        }
    }
    

    APIEXPORT  const char* GlasslabSDK_GetCookie( void* inst ) {
        if( inst != NULL ) {
            return static_cast<GlasslabSDK *>( inst )->getCookie();
        }
        else {
            return NULL;
        }
    }


    APIEXPORT const char* GlasslabSDK_PopLogQueue( void* inst ) {
        if( inst != NULL ) {
            return static_cast<GlasslabSDK *>( inst )->popLogQueue();
        }
        else {
            return NULL;
        }
    }
}