using UnityEngine;
using System.Collections;

public class SDKConnectionTest : MonoBehaviour {
	private const string GAME_ID = "AA-1";
	private const string CONNECT_URI = "http://developer.playfully.org";

	// When game starts and this component initializes, it automatically attempts
	// a connection with the given server using a game ID that the server recognizes.
	void Start () {
		GlasslabSDK.Instance.Connect (Application.persistentDataPath, GAME_ID, CONNECT_URI, connectCallback); // Attempt connection
	}

	void connectCallback( string response )
	{
		Debug.Log ("Connect response: "+response); // Print out response blob
	}
}
