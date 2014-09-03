//
//  glasslab_sdk.cpp
//  GlasslabSDK
//
//  Authors:
//      Joseph Sutton
//      Ben Dapkiewicz
//
//  Copyright (c) 2014 Glasslab. All rights reserved.
//

#include "glasslab_sdk.h"


GlasslabSDK::GlasslabSDK( const char* clientId, const char* deviceId, const char* dataPath, const char* uri ) {
    m_core = new nsGlasslabSDK::Core( this, clientId, deviceId, dataPath, uri );
}


nsGlasslabSDK::Const::Status GlasslabSDK::getLastStatus() {
    if( m_core != NULL ) return m_core->getLastStatus();
    else return nsGlasslabSDK::Const::Status_Error;
}

void GlasslabSDK::popMessageStack() {
    if( m_core != NULL ) m_core->popMessageStack();
}

nsGlasslabSDK::Const::Message GlasslabSDK::readTopMessageCode() {
    if( m_core != NULL ) return m_core->readTopMessageCode();
    else                 return nsGlasslabSDK::Const::Message_Error;
}

const char * GlasslabSDK::readTopMessageString() {
    if( m_core != NULL ) return (m_core->readTopMessageString());
    else                 return NULL;
}


void GlasslabSDK::connect( const char* gameId, const char* uri ) {
    if( m_core != NULL ) m_core->connect( gameId, uri );
}

void GlasslabSDK::deviceUpdate() {
    if( m_core != NULL ) m_core->deviceUpdate();
}

void GlasslabSDK::authStatus() {
    if( m_core != NULL ) m_core->authStatus();
}

void GlasslabSDK::registerStudent( const char* username, const char* password, const char* firstName, const char* lastInitial ) {
    if( m_core != NULL ) m_core->registerStudent( username, password, firstName, lastInitial );
}

void GlasslabSDK::registerInstructor( const char* name, const char* email, const char* password, bool newsletter ) {
    if( m_core != NULL ) m_core->registerInstructor( name, email, password, newsletter );
}

void GlasslabSDK::getUserInfo() {
    if( m_core != NULL ) m_core->getUserInfo();
}

void GlasslabSDK::login( const char* username, const char* password, const char* type ) {
    if( m_core != NULL ) m_core->login( username, password, type );
}

void GlasslabSDK::login( const char* username, const char* password ) {
    this->login( username, password, NULL );
}

void GlasslabSDK::enroll( const char* courseCode ) {
    if( m_core != NULL ) m_core->enroll( courseCode );
}

void GlasslabSDK::unenroll( const char* courseId ) {
    if( m_core != NULL ) m_core->unenroll( courseId );
}

void GlasslabSDK::getCourses() {
    if( m_core != NULL ) m_core->getCourses();
}

void GlasslabSDK::logout() {
    if( m_core != NULL ) m_core->logout();
}

void GlasslabSDK::startSession() {
    if( m_core != NULL ) m_core->startSession();
}

void GlasslabSDK::endSession() {
    if( m_core != NULL ) m_core->endSession();
}

void GlasslabSDK::saveGame( const char* gameData ) {
    if( m_core != NULL ) m_core->saveGame( gameData );
}

void GlasslabSDK::getSaveGame() {
    if( m_core != NULL ) m_core->getSaveGame();
}

void GlasslabSDK::deleteSaveGame() {
    if( m_core != NULL ) m_core->deleteSaveGame();
}

void GlasslabSDK::saveAchievement( const char* item, const char* group, const char* subGroup ) {
    if( m_core != NULL ) m_core->saveAchievement( item, group, subGroup );
}

void GlasslabSDK::sendTelemEvents() {
    if( m_core != NULL ) m_core->sendTelemEvents();
}

void GlasslabSDK::forceFlushTelemEvents() {
    if( m_core != NULL ) m_core->forceFlushTelemEvents();
}

void GlasslabSDK::cancelRequest( const char* key ) {
    if( m_core != NULL ) m_core->cancelRequest( key );
}


void GlasslabSDK::addTelemEventValue( const char* key, const char* value ) { if( m_core != NULL ) m_core->addTelemEventValue( key, value ); }
void GlasslabSDK::addTelemEventValue( const char* key, int8_t value )      { if( m_core != NULL ) m_core->addTelemEventValue( key, value ); }
void GlasslabSDK::addTelemEventValue( const char* key, int16_t value )     { if( m_core != NULL ) m_core->addTelemEventValue( key, value ); }
void GlasslabSDK::addTelemEventValue( const char* key, int32_t value )     { if( m_core != NULL ) m_core->addTelemEventValue( key, value ); }
void GlasslabSDK::addTelemEventValue( const char* key, uint8_t value )     { if( m_core != NULL ) m_core->addTelemEventValue( key, value ); }
void GlasslabSDK::addTelemEventValue( const char* key, uint16_t value )    { if( m_core != NULL ) m_core->addTelemEventValue( key, value ); }
void GlasslabSDK::addTelemEventValue( const char* key, uint32_t value )    { if( m_core != NULL ) m_core->addTelemEventValue( key, value ); }
void GlasslabSDK::addTelemEventValue( const char* key, float value )       { if( m_core != NULL ) m_core->addTelemEventValue( key, value ); }
void GlasslabSDK::addTelemEventValue( const char* key, double value )      { if( m_core != NULL ) m_core->addTelemEventValue( key, value ); }
void GlasslabSDK::addTelemEventValue( const char* key, bool value )        { if( m_core != NULL ) m_core->addTelemEventValue( key, value ); }

void GlasslabSDK::clearTelemEventValues( ) {
    if( m_core != NULL ) m_core->clearTelemEventValues( );
}

void GlasslabSDK::saveTelemEvent( const char* name ) {
    if( m_core != NULL ) m_core->saveTelemEvent( name );
}


void GlasslabSDK::updatePlayerInfoKey( const char* key, const char* value ) { if( m_core != NULL ) m_core->updatePlayerInfoKey( key, value ); }
void GlasslabSDK::updatePlayerInfoKey( const char* key, int8_t value )      { if( m_core != NULL ) m_core->updatePlayerInfoKey( key, value ); }
void GlasslabSDK::updatePlayerInfoKey( const char* key, int16_t value )     { if( m_core != NULL ) m_core->updatePlayerInfoKey( key, value ); }
void GlasslabSDK::updatePlayerInfoKey( const char* key, int32_t value )     { if( m_core != NULL ) m_core->updatePlayerInfoKey( key, value ); }
void GlasslabSDK::updatePlayerInfoKey( const char* key, uint8_t value )     { if( m_core != NULL ) m_core->updatePlayerInfoKey( key, value ); }
void GlasslabSDK::updatePlayerInfoKey( const char* key, uint16_t value )    { if( m_core != NULL ) m_core->updatePlayerInfoKey( key, value ); }
void GlasslabSDK::updatePlayerInfoKey( const char* key, uint32_t value )    { if( m_core != NULL ) m_core->updatePlayerInfoKey( key, value ); }
void GlasslabSDK::updatePlayerInfoKey( const char* key, float value )       { if( m_core != NULL ) m_core->updatePlayerInfoKey( key, value ); }
void GlasslabSDK::updatePlayerInfoKey( const char* key, double value )      { if( m_core != NULL ) m_core->updatePlayerInfoKey( key, value ); }
void GlasslabSDK::updatePlayerInfoKey( const char* key, bool value )        { if( m_core != NULL ) m_core->updatePlayerInfoKey( key, value ); }
void GlasslabSDK::removePlayerInfoKey( const char* key )                    { if( m_core != NULL ) m_core->removePlayerInfoKey( key ); }


void GlasslabSDK::setName( const char* name ) {
    if( m_core != NULL ) m_core->setName( name );
}

void GlasslabSDK::setVersion( const char* version ) {
    if( m_core != NULL ) m_core->setVersion( version );
}

void GlasslabSDK::setGameLevel( const char* gameLevel ) {
    if( m_core != NULL ) m_core->setGameLevel( gameLevel );
}

void GlasslabSDK::setUserId( int userId ) {
    if( m_core != NULL ) m_core->setUserId( userId );
}

void GlasslabSDK::setConfig( nsGlasslabSDK::glConfig config ) {
    if( m_core != NULL ) m_core->setConfig( config );
}

void GlasslabSDK::setTime( time_t time ) {
    if( m_core != NULL ) m_core->setTime( time );
}

void GlasslabSDK::setPlayerHandle( const char* handle ) {
    if( m_core != NULL ) m_core->setPlayerHandle( handle );
}

void GlasslabSDK::removePlayerHandle( const char* handle ) {
    if( m_core != NULL ) m_core->removePlayerHandle( handle );
}

void GlasslabSDK::setCookie( const char* cookie ) {
    if( m_core != NULL ) m_core->setCookie( cookie );
}

void GlasslabSDK::setAutoSessionManagement( bool state ) {
    if( m_core != NULL ) m_core->setAutoSessionManagement( state );
}


void GlasslabSDK::startGameTimer() {
    if( m_core != NULL ) m_core->startGameTimer();
}

void GlasslabSDK::stopGameTimer() {
    if( m_core != NULL ) m_core->stopGameTimer();
}

void GlasslabSDK::resetDatabase() {
    if( m_core != NULL ) m_core->resetDatabase();
}


const char* GlasslabSDK::getConnectUri() {
    if( m_core != NULL ) {
        return m_core->getConnectUri();
    }
    else {
        return "";
    }
}

int GlasslabSDK::getUserId() {
    if( m_core != NULL ) {
        return m_core->getUserId();
    }
    else {
        return -1;
    }
}

const char* GlasslabSDK::getCookie() {
    if( m_core != NULL ) {
        return m_core->getCookie();
    }
    else {
        return "";
    }
}


const char* GlasslabSDK::popLogQueue() {
    if( m_core != NULL ) {
        return m_core->popLogQueue();
    }
    else {
        return "";
    }
}