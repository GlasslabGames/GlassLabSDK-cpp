using UnityEngine;
#if UNITY_EDITOR
using UnityEditor;
#endif
using System.Collections;
using System.Threading;
using System.Runtime.InteropServices;

public class Main : MonoBehaviour {

	private GlasslabSDK glsdk;

	void Start() {
		glsdk = GlasslabSDK.Instance;

		// Connect to the server and set basic app information
		// The first parameter indicates the location of the internal SDK database
		// for storing telemetry information. Leave this blank for iOS deployment.
		glsdk.Connect( "", "TEST", "http://developer.playfully.org", ConnectDone );
		glsdk.SetName( "My Awesome Game" );
		glsdk.SetVersion( "1.0" );
	}

	private void ConnectDone( string response = "" ) {
		Debug.Log( "Connect complete: " + response );

		// Attempt to start a session.
		Debug.Log( "Start a session..." );
		glsdk.StartSession( StartSessionDone );
		
		// Send some basic events.
		Debug.Log( "Saving Events..." );
		for( int i = 1; i < 30; i++ ){
			glsdk.AddTelemEventValue( "string key", "asd" );
			glsdk.AddTelemEventValue( "int key", i );
			glsdk.AddTelemEventValue( "float key", i * 1.23 );
			
			glsdk.SaveTelemEvent( "SampleEvent" );
			
			Thread.Sleep( 1 );
		}
		Debug.Log( "Done Saving Events" );
		
		// Attempt to end the session.
		Debug.Log( "End Session..." );
		glsdk.EndSession( EndSessionDone );
	}
	private void StartSessionDone( string response = "" ) {
		Debug.Log( "Start Session complete: " + response );
	}
	private void EndSessionDone( string response = "" ) {
		Debug.Log( "End Session complete: " + response );
	}

	/*
	 * Unity Editor keeps SDK thread alive even when not running, causing a huge server load.
	 * This rectifies that issue.
	 */
	public void OnApplicationQuit()
	{
		glsdk.StopThread();
		glsdk = null;
	}
}