//
//  glsdk_data_sync.cpp
//  GlasslabSDK
//
//  Authors:
//      Joseph Sutton
//      Ben Dapkiewicz
//
//  Copyright (c) 2014 Glasslab. All rights reserved.
//

#include "glasslab_sdk.h"
#include "glsdk_config.h"


namespace nsGlasslabSDK {

    /**
     * DataSync constructor creates the SQLite database.
     */
    DataSync::DataSync( Core* core, const char* dbPath ) {
        // Set the Core SDK object
        m_core = core;
        
        m_dbName = "";
        if( dbPath ) {
            m_dbName += dbPath;
        } else {
			char cwd[1024];
#if __APPLE__
#if TARGET_OS_IPHONE
			if(getcwd(cwd, sizeof(cwd)) != NULL) {
				m_dbName += cwd;
			}
#else
			sprintf( cwd, "%s/Documents", getenv( "HOME" ) );
			m_dbName += cwd;
#endif
#endif
#if WIN32
			sprintf( cwd, "%s%s\\Documents", getenv( "HOMEDRIVE" ), getenv( "HOMEPATH" ) );
            m_dbName += cwd;
#endif
		}


        
        // If the database path exists, append the glasslabsdk.db, unless it is ":memory:"
        if( !dbPath ) {
            m_dbName += "/glasslabsdk.db";
        }
        
        m_core->logMessage( "Database file:", m_dbName.c_str() );
        //cout << "Database file: " << result << endl;

        // Open the database
        initDB();

        // Check the SDK version and do a comparison with the stored version
        validateSDKVersion();

        // Create the tables needed (tables that already exist will be ignored)
        createTables();
    }

    /**
     * DataSync deconstructor closes the SQLite database
     */
    DataSync::~DataSync() {
        cout << endl << endl << "Destructor has been called" << endl << endl;
        try {
            m_db.close();
        }
        catch( CppSQLite3Exception e ) {
            m_core->displayError( "DataSync::~DataSync()", e.errorMessage() );
            //cout << "Exception in ~DataSync() " << e.errorMessage() << " (" << e.errorCode() << ")" << endl;
        }
    }

    
    //--------------------------------------
    //--------------------------------------
    //--------------------------------------
    /**
     * Function opens the SQLite database.
     */
    void DataSync::initDB() {
        try {
            cout << "SQLite Version...: " << m_db.SQLiteVersion() << endl;
            cout << "------------------------------------" << endl;

            // Open the database
            m_db.open( m_dbName.c_str() );
            
            /*
            CppSQLite3Query q = m_db.execQuery("PRAGMA page_size;");
            if( !q.eof() ) {
                printf("\nPage size: %d\n", atoi(q.fieldValue(0)) );
            }
            q.finalize();
            */
            // page_size == 4096
            
            //m_db.execDML( "PRAGMA cache_size = 1024;" );
            
            /*
            CppSQLite3Query q = m_db.execQuery("PRAGMA cache_size;");
            if( !q.eof() ) {
                printf("\nCache Size: %d\n", atoi(q.fieldValue(0)) );
            }
            q.finalize();
            */
        }
        catch( CppSQLite3Exception e ) {
            cout << "error opening the database: " << e.errorMessage() << endl;
            m_core->displayError( "DataSync::initDB()", e.errorMessage() );
            //cout << "Exception in initDB() " << e.errorMessage() << " (" << e.errorCode() << ")" << endl;
        }
    }

    /**
     * Function validates the SDK version by searching for an entry in the CONFIG table.
     * If there is a version mismatch, the database is reset.
     */
    void DataSync::validateSDKVersion() {
        try {
            printf("Current SDK_VERSION: %s\n", SDK_VERSION);

            // Set a state for if we need to drop the tables and reset
            bool resetTables = false;
            // Set a state for if we need to migrate the contents of one table to another
            // This will occur when columns have either been added or deleted in a table
            bool performMigration = false;
            string s;

            // If the CONFIG table doesn't exist, be sure to drop it
            if( !m_db.tableExists( CONFIG_TABLE_NAME ) ) {
                resetTables = true;
                printf("%s does not exist!\n", CONFIG_TABLE_NAME);
            }
            // The table does exist, check the version field in there
            else {
                s = "";
                s += "select * from ";
                s += CONFIG_TABLE_NAME;
                s += ";";
                printf("SQL: %s\n", s.c_str());
                CppSQLite3Query q = m_db.execQuery( s.c_str() );

                // If the table is empty, reset
                if( q.eof() ) {
                    resetTables = true;
                    printf("No entry exists in %s\n", CONFIG_TABLE_NAME);
                }
                // There is an entry, grab it
                else {
                    float configSDKVersion = (float)atof( q.fieldValue( 0 ) );
                    printf("CONFIG entry: %s", q.fieldValue( 0 ));

                    // If the stored SDK version is less than current, update it
                    if( configSDKVersion < (float)atof( SDK_VERSION ) ) {
                        s = "";
                        s += "update ";
                        s += CONFIG_TABLE_NAME;
                        s += " set version='";
                        s += SDK_VERSION;
                        s += "'";
                        printf("SQL: %s\n", s.c_str());
                        int nRows = m_db.execDML( s.c_str() );
                        printf("%d rows updated in %s\n", nRows, CONFIG_TABLE_NAME);

                        // Indicate that a table migration is required
                        performMigration = true;
                    }
                }

                // Finalize the query
                q.finalize();
            }

            // Reset the tables if we need to
            if( resetTables ) {
                cout << "Need to reset all tables." << endl;
                dropTables();
            }
            // Or migrate the contents if we need to
            else if( performMigration ) {
                cout << "We need to perform a data migration, schemas may have changed." << endl;
                migrateTables();
            }
        }
        catch( CppSQLite3Exception e ) {
            m_core->displayError( "DataSync::validateSDKVersion()", e.errorMessage() );
            //cout << "Exception in validateSDKVersion() " << e.errorMessage() << " (" << e.errorCode() << ")" << endl;
        }
    }


    //--------------------------------------
    //--------------------------------------
    //--------------------------------------
    /**
     * MSG_QUEUE operation.
     *
     * Inserts a new entry into the MSG_QUEUE table.
     */
    void DataSync::addToMsgQ( string deviceId, string path, string requestType, string coreCB, string postdata, const char* contentType ) {
        if( m_messageTableSize > DB_MESSAGE_CAP ) {
            cout << "------------------------------------" << endl;
            cout << "Database has reached a message cap! No longer inserting events!" << endl;
            cout << "------------------------------------" << endl;
            return;
        }

        int nRows = 0;
        // string stream
        string s = "";
        
        try {
            //cout << "------------------------------------" << endl;
            s += "INSERT INTO ";
            s += MSG_QUEUE_TABLE_NAME;
            s += " (deviceId, path, requestType, coreCB, postdata, contentType, status) VALUES ('";
            s += deviceId;
            s += "', ";

            // Check the API path
            if( path.c_str() == NULL ) {
               s += "''";
            }
            else {
                s += "'";
                s += path;
                s += "'";
            }
            s += ", ";

            // Check the request type
            if( requestType.c_str() == NULL ) {
               s += "''";
            }
            else {
                s += "'";
                s += requestType;
                s += "'";
            }
            s += ", ";

            // Check the Core Callback key
            if( coreCB.c_str() == NULL ) {
               s += "''";
            }
            else {
                s += "'";
                s += coreCB;
                s += "'";
            }
            s += ", ";
            
            // Check the postdata
            if( postdata.c_str() == NULL ) {
                s += "''";
            }
            else {
                // Replace all ' characters with escapes: \'
                string characterToSearch = "'";
                string characterToReplace = "''";
                string::size_type n = 0;
                while( ( n = postdata.find( characterToSearch, n ) ) != string::npos ) {
                    postdata.replace( n, 1, characterToReplace );
                    n += 2;
                }

                s += "'";
                s += postdata;
                s += "'";
            }
            s += ", ";
            
            // Check the content type
            if( contentType == NULL ) {
                s += "''";
            }
            else {
                s += "'";
                s += contentType;
                s += "'";
            }
            s += ", 'ready'";
            s += ");";
        
            // Execute the insertion
            //printf("SQL: %s\n", s.c_str());
            nRows = m_db.execDML( s.c_str() );
            //printf("%d rows inserted\n", nRows);
            //printf("------------------------------------\n");

            // Set the message table size
            m_messageTableSize++;
            
            // Debug display
            //displayTable( MSG_QUEUE_TABLE_NAME );
        }
        catch( CppSQLite3Exception e ) {
            m_core->displayError( "DataSync::addToMsgQ()", e.errorMessage() );
            //cout << "Exception in addToMsgQ() " << e.errorMessage() << " (" << e.errorCode() << ")" << endl;
        }
    }

    /**
     * MSG_QUEUE operation.
     *
     * Removes an existing entry from MSG_QUEUE using the rowId.
     */
    void DataSync::removeFromMsgQ( int rowId ) {
        try {
            int r = 0;
            // string stream
            string s = "";
            char t[255];
            
            // Remove the entry at rowId
            s += "delete from ";
            s += MSG_QUEUE_TABLE_NAME;
            s += " where id=";
            sprintf(t, "%d", rowId);
            s += t;
            //cout << "delete SQL: " << m_sql << endl;
            r = m_db.execDML( s.c_str() );
            printf("Deleting result: %d\n", r);

            // Set the message table size
            m_messageTableSize--;
        }
        catch( CppSQLite3Exception e ) {
            m_core->displayError( "DataSync::removeFromMsgQ()", e.errorMessage() );
            printf("Exception in removeFromMsgQ() %s (%d)\n", e.errorMessage(), e.errorCode());
        }
    }

    /**
     * MSG_QUEUE operation.
     *
     * Updates the status of an existing entry in MSG_QUEUE using the rowId.
     */
    void DataSync::updateMessageStatus( int rowId, string status ) {
        try {
            // If the status is success, remove the entry from the db
            if( status == "success" ) {
                cout << "Successful request, removing entry from database." << endl;
                removeFromMsgQ( rowId );
            }
            // Else, update the entry's status field
            else {
                // string stream
                string s = "";
                char t[255];

                // Execute the update operation
                s += "UPDATE ";
                s += MSG_QUEUE_TABLE_NAME;
                s += " SET status='";
                s += status;
                s += "' WHERE id='";
                sprintf(t, "%d", rowId);
                s += t;
                s += "'";
                
                //printf("update SQL: %s\n", s.c_str());
                int r = m_db.execDML( s.c_str() );
                //cout << "Updating result: " << r << endl;
            }
        }
        catch( CppSQLite3Exception e ) {
            m_core->displayError( "DataSync::updateMessageStatus()", e.errorMessage() );
            //cout << "Exception in updateMessageStatus() " << e.errorMessage() << " (" << e.errorCode() << ")" << endl;
        }
    }

    /**
     * MSG_QUEUE operation.
     *
     * Returns the current size of the message queue table.
     */
    int DataSync::getMessageTableSize() {
        return m_messageTableSize;
    }


    //--------------------------------------
    //--------------------------------------
    //--------------------------------------
    /**
     * SESSION operation.
     *
     * Updates an existing session with a valid cookie, or inserts a new entry with
     * the cookie and deviceId.
     */
    void DataSync::updateSessionTableWithCookie( string deviceId, string cookie ) {
        // string stream
        string s = "";
        
        try {
            // Display the session table
            //displayTable( SESSION_TABLE_NAME );
            
            // Look for an existing entry with the device Id
            printf("------------------------------------\n");
            s = "";
            s += "select * from ";
            s += SESSION_TABLE_NAME;
            s += " where deviceId='";
            s += deviceId;
            s += "';";
            printf("session SQL: %s\n", s.c_str());
            CppSQLite3Query sessionQuery = m_db.execQuery( s.c_str() );

            // If the count is 0, insert a new entry
            if( sessionQuery.eof() ) {
                printf("EMPTY: need to insert\n");

                // Create a new session entry
                s = createNewSessionEntry( deviceId, cookie, "" );
            }
            // Otherwise, update an existing entry
            else {
                printf("FOUND:\n");
                for( int fld = 0; fld < sessionQuery.numFields(); fld++ ) {
                    printf("%s | ", sessionQuery.fieldValue( fld ));
                }
                printf("\n");

                // Update
                printf("UPDATING %s with cookie: %s\n", SESSION_TABLE_NAME, cookie.c_str());
                s = "";
                s += "UPDATE ";
                s += SESSION_TABLE_NAME;
                s += " SET cookie='";
                s += cookie;
                s += "' WHERE deviceId='";
                s += deviceId;
                s += "'";
            }

            // Finalize the query
            sessionQuery.finalize();
        
            printf("SQL: %s\n", s.c_str());
            int nRows = m_db.execDML( s.c_str() );
            printf("%d rows inserted\n", nRows);
            printf("------------------------------------\n");
            
            
            displayTable( SESSION_TABLE_NAME );
        }
        catch( CppSQLite3Exception e ) {
            m_core->displayError( "DataSync::updateSessionTableWithCookie()", e.errorMessage() );
            //cout << "Exception in updateSessionTableWithCookie() " << e.errorMessage() << " (" << e.errorCode() << ")" << endl;
        }
    }

    /**
     * SESSION operation.
     *
     * Updates an existing session with a valid gameSessionId, or inserts a new entry with
     * the gameSessionId and deviceId.
     */
    void DataSync::updateSessionTableWithGameSessionId( string deviceId, string gameSessionId ) {
        // string stream
        string s;
        
        try {
            // Display the session table
            //displayTable( SESSION_TABLE_NAME );
            
            // Look for an existing entry with the device Id
            printf("------------------------------------\n");
            s = "";
            s += "select * from ";
            s += SESSION_TABLE_NAME;
            s += " where deviceId='";
            s += deviceId;
            s += "';";
            printf("session SQL: %s\n", s.c_str());
            CppSQLite3Query sessionQuery = m_db.execQuery( s.c_str() );

            // If the count is 0, insert a new entry
            if( sessionQuery.eof() ) {
                cout << "EMPTY: need to insert\n";

                // Create a new session entry
                s = createNewSessionEntry( deviceId, "", gameSessionId );
            }
            // Otherwise, update an existing entry
            else {
                cout << "FOUND:\n";
                for ( int fld = 0; fld < sessionQuery.numFields(); fld++ ) {
                    cout << sessionQuery.fieldValue( fld ) << " | ";
                }
                cout << endl;

                // Update
                printf("UPDATING %s with gameSessionId: %s\n", SESSION_TABLE_NAME, gameSessionId.c_str());
                s = "";
                s += "UPDATE ";
                s += SESSION_TABLE_NAME;
                s += " SET gameSessionId='";
                s += gameSessionId;
                s += "', gameSessionEventOrder='1'";
                s += " WHERE deviceId='";
                s += deviceId;
                s += "'";
            }

            // Finalize the query
            sessionQuery.finalize();
        
            printf("SQL: %s\n", s.c_str());
            int nRows = m_db.execDML( s.c_str() );
            printf("%d rows inserted\n", nRows);
            printf("------------------------------------\n");
            
            
            displayTable( SESSION_TABLE_NAME );
        }
        catch( CppSQLite3Exception e ) {
            m_core->displayError( "DataSync::updateSessionTableWithGameSessionId()", e.errorMessage() );
            //cout << "Exception in updateSessionTableWithGameSessionId() " << e.errorMessage() << " (" << e.errorCode() << ")" << endl;
        }
    }

    /**
     * SESSION operation.
     *
     * Updates an existing session with a replacement deviceId, or inserts a new entry with
     * the deviceId. The new deviceId will include a player handle, in the form of "handle_deviceId".
     */
    void DataSync::updateSessionTableWithPlayerHandle( string deviceIdWithHandle ) {
        // string stream
        string s = "";
        
        try {
            // Display the session table
            //displayTable( SESSION_TABLE_NAME );
            
            // Look for an existing entry with the device Id
            printf("------------------------------------\n");
            s += "select * from ";
            s += SESSION_TABLE_NAME;
            s += " where deviceId='";
            s += deviceIdWithHandle;
            s += "';";
            printf("session SQL: %s\n", s.c_str());
            CppSQLite3Query sessionQuery = m_db.execQuery( s.c_str() );

            // If the count is 0, insert a new entry
            if( sessionQuery.eof() ) {
                printf("session table missing new device id: %s\n", deviceIdWithHandle.c_str());

                // Create a new session entry
                s = createNewSessionEntry( deviceIdWithHandle, "", "" );

                // Perform the operation
                printf("SQL: %s\n", s.c_str());
                int nRows = m_db.execDML( s.c_str() );
                printf("%d rows inserted\n", nRows);
                printf("------------------------------------\n");
            }
            // Otherwise, update an existing entry if we need to
            else {
                printf("FOUND entry with new device Id, we can ignore\n");
            }

            // Finalize the query
            sessionQuery.finalize();
            
            //displayTable( SESSION_TABLE_NAME );
        }
        catch( CppSQLite3Exception e ) {
            m_core->displayError( "DataSync::updateSessionTableWithPlayerHandle()", e.errorMessage() );
            //cout << "Exception in updateSessionTableWithPlayerHandle() " << e.errorMessage() << " (" << e.errorCode() << ")" << endl;
        }
    }

    /**
     * SESSION operation.
     *
     * Removes the session entry associated with the parameter deviceId.
     */
    void DataSync::removeSessionWithDeviceId( string deviceId ) {
        try {
            string s = "";
            // Remove the entry with the associated deviceId
            s += "delete from ";
            s += SESSION_TABLE_NAME;
            s += " where deviceId='";
            s += deviceId;
            s += "';";
            printf("delete SQL: %s\n", s.c_str());
            int r = m_db.execDML( s.c_str() );
            printf("Deleting result: %d\n", r);
        }
        catch( CppSQLite3Exception e ) {
            m_core->displayError( "DataSync::removeSessionWithDeviceId()", e.errorMessage() );
            //cout << "Exception in removeSessionWithDeviceId() " << e.errorMessage() << " (" << e.errorCode() << ")" << endl;
        }
    }

    /**
     * SESSION operation.
     *
     * Gets a cookie stored in the SESSION table using the deviceId.
     */
    const char* DataSync::getCookieFromDeviceId( string deviceId ) {
        // string stream
        string s = "";
        string cookie = "";
        
        try {
            // Display the session table
            displayTable( SESSION_TABLE_NAME );
            
            // Look for an existing entry with the device Id
            s += "select * from ";
            s += SESSION_TABLE_NAME;
            s += " where deviceId='";
            s += deviceId;
            s += "';";
            CppSQLite3Query sessionQuery = m_db.execQuery( s.c_str() );
            
            // If the count is 0, no entry exists with deviceId, return an empty string
            // The empty string will tell the next get request that we need one to store
            if( sessionQuery.eof() ) {
                cout << "no cookie exists for " << deviceId.c_str() << endl;
            }
            // An entry does exist, grab the cookie and return it
            else {
                cookie = sessionQuery.fieldValue( 0 );
                cout << "cookie exists for " << deviceId.c_str() << ": " << cookie.c_str() << endl;
            }
            
            // Finalize the query
            sessionQuery.finalize();
            
            return cookie.c_str();
        }
        catch( CppSQLite3Exception e ) {
            m_core->displayError( "DataSync::getCookieFromDeviceId()", e.errorMessage() );
            //cout << "Exception in getCookieFromDeviceId() " << e.errorMessage() << " (" << e.errorCode() << ")" << endl;
        }

        // Return empty string by default
        return "";
    }

    /**
     * SESSION operation.
     *
     * Updates an existing session with player info, including total time played and the current game session event order.
     */
    void DataSync::updatePlayerInfoFromDeviceId( string deviceId, float totalTimePlayed, int gameSessionEventOrder ) {
        // string stream
        string s;
        char t[255];
        
        try {
            // Look for an existing entry with the device Id
            //printf("------------------------------------\n");
            s = "";
            s += "select * from ";
            s += SESSION_TABLE_NAME;
            s += " where deviceId='";
            s += deviceId;
            s += "';";
            //cout << "session SQL: " << m_sql << endl;
            CppSQLite3Query sessionQuery = m_db.execQuery( s.c_str() );

            // Only continue if an entry exists
            if( !sessionQuery.eof() ) {
                // Update
                s = "";
                s += "UPDATE ";
                s += SESSION_TABLE_NAME;
                s += " SET totalTimePlayed='";
                sprintf(t, "%f", totalTimePlayed);
                s += t;
                s += "', gameSessionEventOrder='";
                sprintf(t, "%d", gameSessionEventOrder);
                s += t;
                s += "' WHERE deviceId='";
                s += deviceId;
                s += "'";

                //printf("SQL: %s\n", s.c_str());
                int nRows = m_db.execDML( s.c_str() );
                //printf("%d rows inserted\n", nRows);
                //printf("------------------------------------\n");
            }

            // Finalize the query
            sessionQuery.finalize();
        }
        catch( CppSQLite3Exception e ) {
            m_core->displayError( "DataSync::updatePlayerInfoFromDeviceId()", e.errorMessage() );
            //cout << "Exception in updatePlayerInfoFromDeviceId() " << e.errorMessage() << " (" << e.errorCode() << ")" << endl;
        }
    }

    /**
     * SESSION operation.
     *
     * Get the totalTimePlayed stored in the SESSION table using the deviceId.
     */
    float DataSync::getTotalTimePlayedFromDeviceId( string deviceId ) {
        try {
            float totalTimePlayed = 0;
            string s = "";
            // Look for an existing entry with the device Id
            s += "select * from ";
            s += SESSION_TABLE_NAME;
            s += " where deviceId='";
            s += deviceId;
            s += "';";
            CppSQLite3Query sessionQuery = m_db.execQuery( s.c_str() );

            // If the count is 0, no entry exists with deviceId, return a default value of 0.0
            if( sessionQuery.eof() ) {
                cout << "totalTimePlayed does not exist for " << deviceId.c_str() << endl;
            }
            // An entry does exist, grab the totalTimePlayed and return it
            else if( sessionQuery.fieldValue( 4 ) != NULL ) {
                totalTimePlayed = atof( sessionQuery.fieldValue( 4 ) );
                cout << "totalTimePlayed exists for " << deviceId.c_str() << ": " << totalTimePlayed << endl;
            }
            
            // Finalize the query
            sessionQuery.finalize();

            return totalTimePlayed;
        }
        catch( CppSQLite3Exception e ) {
            m_core->displayError( "DataSync::getTotalTimePlayedFromDeviceId()", e.errorMessage() );
            //cout << "Exception in getTotalTimePlayedFromDeviceId() " << e.errorMessage() << " (" << e.errorCode() << ")" << endl;
        }

        // Return 0.0 by default
        return 0.0;
    }

    /**
     * SESSION operation.
     *
     * Update the gameSessionEventOrder stored in the SESSION table using the deviceId.
     */
    void DataSync::updateGameSessionEventOrderWithDeviceId( string deviceId, int gameSessionEventOrder ) {
        // string stream
        string s = "";
        char t[255];
        
        try {
            // Update the SESSION table with the new gameSessionEventOrder value
            s += "UPDATE ";
            s += SESSION_TABLE_NAME;
            s += " SET gameSessionEventOrder='";
            sprintf(t, "%d", gameSessionEventOrder);
            s += t;
            s += "' WHERE deviceId='";
            s += deviceId;
            s += "';";

            printf("update SQL: %s\n", s.c_str());
            int r = m_db.execDML( s.c_str() );
            printf("Updating gameSessionEventOrder result: %d\n", r);
        }
        catch( CppSQLite3Exception e ) {
            m_core->displayError( "DataSync::updateGameSessionEventOrderWithDeviceId()", e.errorMessage() );
            //cout << "Exception in updateGameSessionEventOrderWithDeviceId() " << e.errorMessage() << " (" << e.errorCode() << ")" << endl;
        }
    }

    /**
     * SESSION operation.
     *
     * Get the gameSessionEventOrder stored in the SESSION table using the deviceId.
     */
    int DataSync::getGameSessionEventOrderFromDeviceId( string deviceId ) {
        try {
            int gameSessionEventOrder = 1;
            string s = "";
            // Look for an existing entry with the device Id
            s += "select * from ";
            s += SESSION_TABLE_NAME;
            s += " where deviceId='";
            s += deviceId;
            s += "';";
            CppSQLite3Query sessionQuery = m_db.execQuery( s.c_str() );

            // If the count is 0, no entry exists with deviceId, return a default value of 1
            if( sessionQuery.eof() ) {
                cout << "gameSessionEventOrder does not exist for " << deviceId.c_str() << endl;
            }
            // An entry does exist, grab the gameSessionEventOrder and return it
            else if( sessionQuery.fieldValue( 3 ) != NULL ) {
                gameSessionEventOrder = atoi( sessionQuery.fieldValue( 3 ) );
                cout << "gameSessionEventOrder exists for " << deviceId.c_str() << ": " << gameSessionEventOrder << endl;
            }
            
            // Finalize the query
            sessionQuery.finalize();
            
            return gameSessionEventOrder;
        }
        catch( CppSQLite3Exception e ) {
            m_core->displayError( "DataSync::getGameSessionEventOrderFromDeviceId()", e.errorMessage() );
            //cout << "Exception in getGameSessionEventOrderFromDeviceId() " << e.errorMessage() << " (" << e.errorCode() << ")" << endl;
        }

        // Return 1 by default
        return 1;
    }

    /**
     * SESSION operation.
     *
     * Inserts a new session object into SESSION.
     */
    string DataSync::createNewSessionEntry( string deviceId, string cookie, string gameSessionId ) {
        try {
            // Insert
            string s = "";
            s += "INSERT INTO ";
            s += SESSION_TABLE_NAME;
            s += " VALUES (";

            // Check the cookie
            if( cookie.c_str() == 0 ) {
               s += "''";
            }
            else {
               s += "'";
               s += cookie;
               s += "'";
            }
            s += ", ";

            // Check the deviceId
            if( deviceId.c_str() == 0 ) {
               s += "0";
            }
            else {
               s += "'";
               s += deviceId;
               s += "'";
            }
            s += ", ";
            
            // Check the gameSessionId
            if( gameSessionId.c_str() == 0 ) {
                s += "0";
            }
            else {
                s += "'";
                s += gameSessionId;
                s += "'";
            }
            s += ", ";

            // Include the gameSessionEventOrder (defaults to 1)
            s += "'";
            s += "1";
            s += "'";
            s += ", ";

            // Include the totalTimePlayed (defaults to 0.0)
            s += "'";
            s += "0.0";
            s += "'";

            // Close the SQL statement
            s += ");";

            // Set the SQLite string to execute
            return s;
        }
        catch( CppSQLite3Exception e ) {
            m_core->displayError( "DataSync::createNewSessionEntry()", e.errorMessage() );
            //cout << "Exception in createNewSessionEntry() " << e.errorMessage() << " (" << e.errorCode() << ")" << endl;
        }
        
        return "";
    }


    //--------------------------------------
    //--------------------------------------
    //--------------------------------------
    /**
     * Function flushes the MSG_QUEUE table, parsing the data in each entry to send
     * them to Core for an HTTP request to the server. All data must be valid, otherwise
     * it remains in the queue or is removed.
     */
    void DataSync::flushMsgQ() {
        try {
            char t[255];
            string s;
            // Begin display out
            //cout << "\n\n\n-----------------------------------" << endl;
            //printf("\tflushing MSG_QUEUE: %d\n", m_messageTableSize);
            m_core->logMessage( "flushing MSG_QUEUE" );

            // Select all entries in MSG_QUEUE
            s = "select * from " MSG_QUEUE_TABLE_NAME ";";
            //printf("msgQ SQL: %s\n", s.c_str());
            CppSQLite3Query msgQuery = m_db.execQuery( s.c_str() );

            // Keep a counter for the number of requests made so we can limit it
            int requestsMade = 0;

            // Iterate 
            while ( !msgQuery.eof() )
            {
                // Print the entry to consider
                /*cout << "Entry: ";
                for( int fld = 0; fld < msgQuery.numFields(); fld++ ) {
                    cout << msgQuery.fieldValue( fld ) << "|";
                }
                cout << endl;*/

                /*
                This message will contain the following information:
                - id
                - deviceId
                - path
                - requestType
                - coreCB
                - postdata
                - contentType
                - status (ready, pending, failed, success)

                A session will contain the following information:
                - cookie
                - deviceId
                - gameSessionId
                - gameSessionEventOrder
                - totalTimePlayed
                */

                // Check the deviceId listed in the message for an entry in the SESSION table
                // If the deviceId is empty, which should not open, we should skip this entry
                // or remove it from MSG_QUEUE (?)
                int rowId = atoi( msgQuery.fieldValue( 0 ) );
                string deviceId = msgQuery.fieldValue( 1 );
                if( deviceId.c_str() != NULL ) {
                    // Select all entries in SESSION with deviceId
                    displayTable( SESSION_TABLE_NAME );
                    s = "select * from " SESSION_TABLE_NAME " where deviceId='";
                    s += deviceId;
                    s += "';";
                    //printf("session SQL: %s\n", s.c_str());
                    CppSQLite3Query sessionQuery = m_db.execQuery( s.c_str() );

                    // Only continue if we received an entry from SESSION
                    if( !sessionQuery.eof() ) {
                        // Get the cookie field
                        string cookie = sessionQuery.fieldValue( 0 );

                        // Only continue if the cookie exists
                        if( cookie.c_str() != NULL ) {
                            //cout << "cookie is: " << cookie << endl;

                            // Get the path from MSG_QUEUE
                            string apiPath = msgQuery.fieldValue( 2 );
                            string requestType = msgQuery.fieldValue( 3 );
                            string coreCB = msgQuery.fieldValue( 4 );
                            //cout << "coreCB is: " << coreCB << endl;

                            // We only care about startsession, endsession, and sendtelemetry
                            // Anything else should be ignored (and not present in the queue)
                            // Only continue with endsession and sendtelemetry if gameSessionId
                            // exists in the SESSION entry
                            string gameSessionId = sessionQuery.fieldValue( 2 );
                            
                            // Finalize the query
                            sessionQuery.finalize();
                            
                            //cout << "game session Id is: " << gameSessionId << endl;
                            if( strstr( apiPath.c_str(), API_POST_SESSION_START ) ||
                                strstr( apiPath.c_str(), API_POST_SAVEGAME ) ||
                                strstr( apiPath.c_str(), API_POST_PLAYERINFO ) ||
                                strstr( apiPath.c_str(), API_POST_ACHIEVEMENT ) ||
                                strstr( apiPath.c_str(), API_POST_TOTAL_TIME_PLAYED ) ||
                                ( ( strstr( apiPath.c_str(), API_POST_SESSION_END ) || strstr( apiPath.c_str(), API_POST_EVENTS ) ) &&
                                    gameSessionId.c_str() != NULL &&
                                    gameSessionId.length() != 0
                                )
                              ) {

                                // Get the event information
                                //string coreCB = msgQuery.fieldValue( 3 );
                                string postdata = msgQuery.fieldValue( 5 );
                                const char* contentType = msgQuery.fieldValue( 6 );

                                // If this is a telemetry event or end session, update the postdata to include the correct gameSessionId
                                if( strstr( apiPath.c_str(), API_POST_SESSION_END ) || strstr( apiPath.c_str(), API_POST_EVENTS ) ) {
                                    string gameSessionIdTag = "$gameSessionId$";

                                    string::size_type n = 0;
                                    while( ( n = postdata.find( gameSessionIdTag, n ) ) != string::npos ) {
                                        postdata.replace( n, gameSessionIdTag.size(), gameSessionId );
                                        n += gameSessionId.size();
                                    }
                                }

                                // Update the entry's status field
                                s = "UPDATE " MSG_QUEUE_TABLE_NAME " SET status='pending' WHERE ";
                                sprintf(t, "id='%d'", rowId);
                                s += t;

                                //printf("update SQL: %s\n", s.c_str());
                                int r = m_db.execDML( s.c_str() );
                                //printf("Updating result: %d\n", r);

                                // Perform the get request using the message information
                                m_core->mf_httpGetRequest( apiPath, requestType, coreCB, postdata, contentType, rowId );
                                requestsMade++;
                            }
                            else {
                                m_core->displayWarning( "DataSync::flushMsgQ()", "The API path specified was invalid. Removing the entry from the queue." );
                                removeFromMsgQ( rowId );
                            }
                        }
                        else {
                            cout << "cookie for device Id: " << deviceId << " is NULL" << endl;
                        }
                    }
                    else {
                        cout << "no entry from SESSION was found for: " << deviceId << "..." << endl;
                    }
                }
                else {
                    m_core->displayWarning( "DataSync::flushMsgQ()", "The deviceId was NULL for the entry. Removing the entry from the queue." );
                    removeFromMsgQ( rowId );
                }

                // If we've exceeded the max number of requests we can make per flush, exit
                // The next batch of events will be picked up during the next flush
                if( requestsMade >= m_core->config.eventsMaxSize ) {
                    cout << "Exceeded max number of requests we can make, exit." << endl;
                    break;
                }

                // Get the next row in MSG_QUEUE
                //cout << "--- get the next entry in MSG_QUEUE ---" << endl;
                msgQuery.nextRow();
            }
            
            // Finalize the query
            msgQuery.finalize();
        }
        catch( CppSQLite3Exception e ) {
            m_core->displayError( "DataSync::flushMsgQ()", e.errorMessage() );
            //cout << "Exception in flushMsgQ() " << e.errorMessage() << " (" << e.errorCode() << ")" << endl;
        }
        
        // End display out
        //cout << "reached the end of MSG_QUEUE" << endl;
        //cout << "-----------------------------------\n\n\n" << endl;
        m_core->logMessage( "reached the end of MSG_QUEUE" );
        //displayTable( MSG_QUEUE_TABLE_NAME );
        //displayTable( SESSION_TABLE_NAME );
    }


    //--------------------------------------
    //--------------------------------------
    //--------------------------------------
    /**
     * Function resets all tables in the database and recreates them.
     */
    void DataSync::resetDatabase() {
        dropTables();
        createTables();
    }


    //--------------------------------------
    //--------------------------------------
    //--------------------------------------
    /**
     * Function creates all tables used in the SDK.
     */
    void DataSync::createTables() {
        int r;
        string s;

        try {
            // Create the CONFIG table
            if( !m_db.tableExists( CONFIG_TABLE_NAME ) ){
                printf("\nCreating %s table\n", CONFIG_TABLE_NAME);
   
                s = "";
                s += "create table ";
                s += CONFIG_TABLE_NAME;
                s += " (version char(256));";

                printf("SQL: %s\n", s.c_str());
                r = m_db.execDML( s.c_str() );
                
                printf("Creating table: %d\n", r);
                
                // Insert the SDK version
                s = "";
                s += "INSERT INTO ";
                s += CONFIG_TABLE_NAME;
                s += " VALUES ('";
                s += SDK_VERSION;
                s += "');";
                
                printf("insert version SQL: %s\n", s.c_str());
                int nRows = m_db.execDML( s.c_str() );
                printf("%d rows inserted\n", nRows);
                printf("------------------------------------\n");
            }

            // Create the MSG_QUEUE table
            if( !m_db.tableExists( MSG_QUEUE_TABLE_NAME ) ) {
                printf("\nCreating %s table\n", MSG_QUEUE_TABLE_NAME);
                
                s = "";
                s += "create table ";
                s += MSG_QUEUE_TABLE_NAME;
                s += " (";
                s += "id integer primary key autoincrement, ";
                s += "deviceId char(256), ";
                s += "path char(256), ";
                s += "requestType char(256), ";
                s += "coreCB char(256), ";
                s += "postdata text, ";
                s += "contentType char(256), ";
                s += "status char(256) ";
                s += ");";
                
                printf("SQL: %s\n", s.c_str());
                r = m_db.execDML( s.c_str() );
                printf("Created table: %d", r);
                printf("------------------------------------\n");

                // Initialize the table size to 0
                m_messageTableSize = 0;
            }
            else {
                // Set the message table size
                s = "";
                s += "select * from ";
                s += MSG_QUEUE_TABLE_NAME;
                CppSQLite3Table t = m_db.getTable( s.c_str() );
                m_messageTableSize = t.numRows();
            }
            
            // Create the SESSION table
            if( !m_db.tableExists( SESSION_TABLE_NAME ) ) {
                printf("\nCreating %s table\n", SESSION_TABLE_NAME);
                
                s = "";
                s += "create table ";
                s += SESSION_TABLE_NAME;
                s += " (";
                s += "cookie char(256), ";
                s += "deviceId char(256), ";
                s += "gameSessionId char(256), ";
                s += "gameSessionEventOrder integer, ";
                s += "totalTimePlayed real ";
                s += ");";

                printf("SQL: %s\n", s.c_str());
                r = m_db.execDML( s.c_str() );
                printf("Created table: %d", r);
                printf("------------------------------------\n");
            }

            // Display all tables
            displayTable( CONFIG_TABLE_NAME );
            displayTable( MSG_QUEUE_TABLE_NAME );
            displayTable( SESSION_TABLE_NAME );
        }
        catch( CppSQLite3Exception e ) {
            m_core->displayError( "DataSync::createTables()", e.errorMessage() );
            //cout << "Exception in createTables() " << e.errorMessage() << " (" << e.errorCode() << ") " << CppSQLite3Exception::errorCodeAsString( e.errorCode() ) << endl;
        }
    }

    /**
     * Function drops all existing tables in the database.
     */
    void DataSync::dropTables() {
        int r;
        string s;
        
        try {
            // Drop the CONFIG table
            if( m_db.tableExists( CONFIG_TABLE_NAME ) ) {
                printf("\nDropping %s table\n", CONFIG_TABLE_NAME);
                
                s = "drop table " CONFIG_TABLE_NAME ";";
                
                printf("SQL: %s\n", s.c_str());
                r = m_db.execDML( s.c_str() );
                printf("Dropped table: %d", r);
                printf("------------------------------------\n");
            }
            
            // Drop the MSG_QUEUE table
            if( m_db.tableExists( MSG_QUEUE_TABLE_NAME ) ) {
                printf("\nDropping %s table\n", MSG_QUEUE_TABLE_NAME);
                
                s = "drop table " MSG_QUEUE_TABLE_NAME ";";
                
                printf("SQL: %s\n", s.c_str());
                r = m_db.execDML( s.c_str() );
                printf("Dropped table: %d", r);
                printf("------------------------------------\n");
            }
            
            // Drop the SESSION table
            if( m_db.tableExists( SESSION_TABLE_NAME ) ) {
                printf("\nDropping %s table\n", SESSION_TABLE_NAME);
                
                s = "drop table " SESSION_TABLE_NAME ";";
                
                printf("SQL: %s\n", s.c_str());
                r = m_db.execDML( s.c_str() );
                
                printf("Dropped table: %d", r);
                printf("------------------------------------\n");
            }
        }
        catch( CppSQLite3Exception e ) {
            m_core->displayError( "DataSync::dropTables()", e.errorMessage() );
            //cout << "Exception in dropTables() " << e.errorMessage() << " (" << e.errorCode() << ")" << endl;
        }
    }

    /**
     * Function will perform migration for all tables, calling the migrateTable( table ) function.
     */
    void DataSync::migrateTables() {
        try {
            // Perform migration for the CONFIG table
            string config_schema = "create table " CONFIG_TABLE_NAME "_backup ("
                "version char(256) "
                ");";
            migrateTable( CONFIG_TABLE_NAME, config_schema );

            // Perform migration for the MSG_QUEUE table
            string message_schema = "create table " MSG_QUEUE_TABLE_NAME "_backup ("
                "id integer primary key autoincrement, "
                "deviceId char(256), "
                "path char(256), "
                "requestType char(256), "
                "coreCB char(256), "
                "postdata text, "
                "contentType char(256), "
                "status char(256) "
                ");";
            migrateTable( MSG_QUEUE_TABLE_NAME, message_schema );

            // Perform migration for the SESSION table
            string session_schema = "create table " SESSION_TABLE_NAME "_backup ("
                "cookie char(256), "
                "deviceId char(256), "
                "gameSessionId char(256), "
                "gameSessionEventOrder integer, "
                "totalTimePlayed real "
                ");";
            migrateTable( SESSION_TABLE_NAME, session_schema );

            // Display all tables
            displayTable( CONFIG_TABLE_NAME );
            displayTable( MSG_QUEUE_TABLE_NAME );
            displayTable( SESSION_TABLE_NAME );
        }
        catch( CppSQLite3Exception e ) {
            m_core->displayError( "DataSync::migrateTables()", e.errorMessage() );
            //cout << "Exception in migrateTables() " << e.errorMessage() << " (" << e.errorCode() << ")" << endl;
        }
    }

    /**
     * Function will handle migrating the data from an existing table into a new one sharing
     * the same name. This is to acount for potential changes in schema, including column add
     * and removal. The data in the previous schema will be preserved.
     * 
     * Migration accounting for column operations can be handled via these operations:
     * - CREATE TABLE current (a int, b int, c int);
     * - CREATE TABLE backup (a int, c int, d int);
     * - INSERT INTO backup SELECT a,c,0 FROM current;
     * - DROP TABLE current;
     * - ALTER TABLE backup RENAME TO current;
     */
    void DataSync::migrateTable( string table, string newSchema ) {
        int r;
        string m_sql;
        
        try {
            // Migrate the contents of the parameter table if it exists
            if( m_db.tableExists( table.c_str() ) ) {
                cout << endl << "Migrating " << table << " table" << endl;

                // Create the backup table with the desired schema
                m_sql = newSchema;
                // Execute.
                cout << "SQL: " << m_sql << endl;
                string t2 = m_sql;
                r = m_db.execDML( t2.c_str() );
                printf( "result: %i\n", r );


                // Get the schema for both the backup and current tables
                m_sql = "select * from " + table + "_backup;";
                CppSQLite3Query backup_q = m_db.execQuery( m_sql.c_str() );
                m_sql = "select * from " + table + ";";
                CppSQLite3Query current_q = m_db.execQuery( m_sql.c_str() );

                // Now we need to construct the insert string to migrate data from current to backup
                string insertString = "";
                int numBackupFields = backup_q.numFields();
                for( int b = 0; b < numBackupFields; b++ ) {
                    bool matchFound = false;

                    // We need to check this backup table field for existence in the current table
                    for( int c = 0; c < current_q.numFields(); c++ ) {
                        // If we have a match, append the value to the insert string
                        //printf( "comparing %s with %s\n", backup_q.fieldName( b ), current_q.fieldName( c ) );
                        if( strcmp( backup_q.fieldName( b ), current_q.fieldName( c ) ) == 0 ) {
                            //printf( "found!\n" );
                            insertString += backup_q.fieldName( b );
                            matchFound = true;
                            break;
                        }
                    }

                    // If there was no match found, set the default value for that data type
                    if( !matchFound ) {
                        insertString += "NULL";
                    }

                    // Append a comma if we haven't yet reached the end of backup fields
                    if( b + 1 < numBackupFields ) {
                        insertString += ",";
                    }
                }

                // Finalize the two select statements
                backup_q.finalize();
                current_q.finalize();

                // Insert shared values the from current into the backup
                m_sql = "insert into " + table + "_backup "
                    "select " + insertString + " from " + table + ";";
                // Execute.
                cout << "SQL: " << m_sql << endl;
                t2 = m_sql;
                r = m_db.execDML( t2.c_str() );
                printf( "result: %i\n", r );

                // Drop the current table
                m_sql = "drop table " + table + ";";
                // Execute.
                cout << "SQL: " << m_sql << endl;
                t2 = m_sql;
                r = m_db.execDML( t2.c_str() );
                printf( "result: %i\n", r );

                // Alter the backup table to rename it as current
                m_sql = "alter table " + table + "_backup "
                    "rename to " + table + ";";
                // Execute.
                cout << "SQL: " << m_sql << endl;
                t2 = m_sql;
                r = m_db.execDML( t2.c_str() );
                printf( "result: %i\n", r );

                // Print final results
                cout << "Migration results:" << r << endl;
                cout << "------------------------------------" << endl;
            }
        }
        catch( CppSQLite3Exception e ) {
            m_core->displayError( "DataSync::migrateTable()", e.errorMessage() );
            //cout << "Exception in migrateTable() " << e.errorMessage() << " (" << e.errorCode() << ")" << endl;
        }
    }

    /**
     * Functions displays the contents of a given table.
     */
    void DataSync::displayTable( string table ) {
        /*try {
            // display out
            cout << "------------------------------------" << endl;
            cout << "all rows in " << table << endl;
            int fld;
            
            string s = "select * from " + table + ";";
            //cout << "SQL: " << m_sql << endl;
            CppSQLite3Query q = m_db.execQuery( s.c_str() );
            
            for( fld = 0; fld < q.numFields(); fld++ )
            {
                cout << q.fieldName( fld ) << "(" << q.fieldDataType( fld ) << ")|";
            }
            cout << endl;
            
            while( !q.eof() )
            {
                for( int i = 0; i < q.numFields(); i++ ) {
                    cout << q.fieldValue( i ) << " | ";
                }
                cout << "END" << endl;
                q.nextRow();
            }
            q.finalize();
            cout << "------------------------------------" << endl;
        }
        catch( CppSQLite3Exception e ) {
            m_core->displayError( "DataSync::displayTable()", e.errorMessage() );
            //cout << "Exception in displayTable() " << e.errorMessage() << " (" << e.errorCode() << ")" << endl;
        }*/
    }
    
}; // end nsGlasslabSDK