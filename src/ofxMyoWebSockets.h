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

        int             id;
        int             rssi;

        string          arm;
        string          direction;
        string          pose, lastPose;

        ofVec3f         accel, gyro;
        ofQuaternion    quat;
        float           roll, pitch, yaw;


        float           poseStartTime;
        bool            poseConfirmed;
        bool            unlocked;
        float           unlockStartTime;

    };


    class Connection {

    public:

        Connection();
        void connect(string hostname = "localhost", int port = 10138, bool autoReconnect = false);

		void setLockingPolicy(string type);
        void setRequiresUnlock(bool require = false);
        void setUnlockTimeout(float time = 3.0f);
        void setMinimumGestureDuration(float time = 0.0f);
        void setLockAfterPose(bool lock = true);
        void setUseDegrees(bool degrees = true);

        float getMinimumGestureDuration() { return minimumGestureDuration; }

        void update();

        void sendCommand(string command, string parameter);
        void sendCommand(int myoID, string command);
        void sendCommand(int myoID, string command, string type);
        void sendCommand(Armband* armband, string command);
        void sendCommand(Armband* armband, string command, string type);

        // type: short, medium, or long as per the Myo WebSocket API
        // also added double for convenience which sends two shorts
        void vibrate(int myoID, string type);
        void vibrate(Armband* armband, string type);

        void requestSignalStrength(int myoID);
        void requestSignalStrength(Armband* armband);

		void lock(int myoID);
		void lock(Armband* armband);

		void unlock(int myoID, string type);
		void unlock(Armband* armband, string type);

        vector<Armband*>    armbands;
        Armband*            getArmband(int myoID);
        Armband*            createArmband(int myoID);
        int                 numConnectedArmbands();

        ofEvent<Armband>    pairedEvent;
        ofEvent<Armband>    unpairedEvent;

        ofEvent<Armband>    connectedEvent;
        ofEvent<Armband>    disconnectedEvent;

        // API v1 compatibility
        ofEvent<Armband>    armRecognizedEvent;
        ofEvent<Armband>    armLostEvent;

        // API v2
        ofEvent<Armband>    armSyncedEvent;
        ofEvent<Armband>    armUnsyncedEvent;

        ofEvent<Armband>    unlockedEvent;
        ofEvent<Armband>    lockedEvent;

        ofEvent<Armband>    poseStartedEvent;
        ofEvent<Armband>    poseConfirmedEvent;

        ofEvent<Armband>    orientationEvent;
        ofEvent<Armband>    rssiReceivedEvent;


        // ofxLibwebsockets callbacks & such
        void onConnect( ofxLibwebsockets::Event& args );
        void onOpen( ofxLibwebsockets::Event& args );
        void onClose( ofxLibwebsockets::Event& args );
        void onIdle( ofxLibwebsockets::Event& args );
        void onMessage( ofxLibwebsockets::Event& args );
        void onBroadcast( ofxLibwebsockets::Event& args );
        
        ofxLibwebsockets::Client    client;
        
    private:
        
        bool    connected;
        bool    reconnect;
        float   reconnectTime;
        float   reconnectLastAttempt;

        string  hostname;
        int     port;

        bool    requiresUnlock;
        float   unlockTimeout;
        float   minimumGestureDuration;
        bool    lockAfterPose;
        
        bool    convertToDegrees;
        
    };
    
}