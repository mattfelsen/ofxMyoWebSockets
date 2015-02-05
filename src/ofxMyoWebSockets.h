//
//  ofxMyoWebSockets
//
//  Created by Matt Felsen on 10/21/14.
//
//

#pragma once

#include "ofxJSON.h"
#include "ofxLibwebsockets.h"

namespace ofxMyo {

    class Hub;
    class Armband;

    class Armband {

    public:

        Hub*            hub;

        int             id;
        int             rssi;
        bool            isPaired;
        bool            isConnected;

        string          arm;
        string          direction;
        string          pose, lastPose;

        ofVec3f         accel, gyro;
        ofQuaternion    quat, quatRaw, quatOffset;
        float           roll, pitch, yaw;
        float           rollRaw, pitchRaw, yawRaw;

        float           poseStartTime;
        bool            poseConfirmed;
        float           unlockStartTime;

        void    setLockedState();
        void    setUnlockedState();
        bool    isLocked();
        bool    isUnlocked();

        void    lock();
        void    unlock(string type = "");
        void    vibrate(string type = "short");             // type: short, medium, or long
        void    notifyUserAction(string type = "single");   // type: single
        void    requestSignalStrength();

        void    resetCoordinateSystem();
        bool    rollIsNear(float degrees, float threshold);
        bool    pitchIsNear(float degrees, float threshold);
        bool    yawIsNear(float degrees, float threshold);

    private:

        bool    unlocked;

    };


    class Hub {

    public:

        Hub();
        
        void connect(bool autoReconnect = false);
        void connect(string hostname = "localhost", int port = 10138, bool autoReconnect = false);

		void setLockingPolicy(string type);
        void setRequiresUnlock(bool require = false);
        void setUnlockTimeout(float time = 3.0f);
        void setMinimumGestureDuration(float time = 0.0f);
        void setLockAfterPose(bool lock = true);
        void setUseDoubleTapUnlock(bool lock = true);
        void setUseDegrees(bool degrees = true);

        float getMinimumGestureDuration() { return minimumGestureDuration; }
        bool  getUsingDegrees() { return convertToDegrees; }

        void update();

        void sendCommand(string command);
        void sendCommand(string command, string parameter);
        void sendCommand(int myoID, string command);
        void sendCommand(int myoID, string command, string type);

        vector<Armband*>    armbands;
        Armband*            getArmband(int myoID);
        Armband*            createArmband(int myoID);
        int                 numConnectedArmbands();

        ofEvent<Armband>    pairedEvent;
        ofEvent<Armband>    unpairedEvent;

        ofEvent<Armband>    connectedEvent;
        ofEvent<Armband>    disconnectedEvent;

        ofEvent<Armband>    armRecognizedEvent; // API v1 compatibility
        ofEvent<Armband>    armLostEvent;

        ofEvent<Armband>    armSyncedEvent;     // API v2
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

        bool    useBuiltInUnlocking;
        bool    requiresUnlock;
        float   unlockTimeout;
        float   minimumGestureDuration;
        bool    lockAfterPose;
        bool    useDoubleTapUnlock;
        
        bool    convertToDegrees;
        
    };
    
}