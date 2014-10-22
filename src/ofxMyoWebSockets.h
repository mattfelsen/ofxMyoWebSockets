//
//  ofxMyoWebSockets
//
//  Created by Matt Felsen on 10/21/14.
//
//

#pragma once

#include "ofxJSON.h"
#include "ofxLibwebsockets.h"

namespace ofxMyoWebSockets {

	struct Armband {

		int				id;
		int				rssi;

		string			arm;
		string			direction;
		string			pose, lastPose;

		ofQuaternion	quat;
		float			roll, pitch, yaw;

		float			poseStartTime;
		bool			poseConfirmed;
		bool			unlocked;
		float			unlockStartTime;

	};


	class Connection {

	public:

		Connection();

		void setRequiresUnlock(bool require = false);
		void setUnlockTimeout(float time = 3.0f);
		void setMinimumGestureDuration(float time = 0.0f);
		void setLockAfterPose(bool lock = true);

		void connect(bool autoReconnect = false);
		void update();

		// type: short, medium, or long as per the Myo WebSocket API
		// also added double for convenience which sends two shorts
		void vibrate(int myoID, string type);
		void vibrate(Armband* armband, string type);

		void requestSignalStrength(int myoID);
		void requestSignalStrength(Armband* armband);

		vector<Armband*>	armbands;
		Armband*			getArmband(int myoID);
		Armband*			createArmband(int myoID);
		int					numConnectedArmbands();

		bool				connected;
		bool				reconnect;

		ofEvent<Armband*>	unlockedEvent;
		ofEvent<Armband*>	lockedEvent;
		ofEvent<Armband*>	poseStartedEvent;
		ofEvent<Armband*>	poseConfirmedEvent;
		ofEvent<Armband*>	orientationEvent;


		// ofxLibwebsockets callbacks & such
		void onConnect( ofxLibwebsockets::Event& args );
		void onOpen( ofxLibwebsockets::Event& args );
		void onClose( ofxLibwebsockets::Event& args );
		void onIdle( ofxLibwebsockets::Event& args );
		void onMessage( ofxLibwebsockets::Event& args );
		void onBroadcast( ofxLibwebsockets::Event& args );

		ofxLibwebsockets::Client	client;

	private:

		float				reconnectTime;
		float				reconnectLastAttempt;

		bool				requiresUnlock;
		float				unlockTimeout;
		float				minimumGestureDuration;
		bool				lockAfterPose;

	};

}