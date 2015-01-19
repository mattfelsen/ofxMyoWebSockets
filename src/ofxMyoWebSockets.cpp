//--------------------------------------------------------------//
//  ofxMyoWebSockets
//
//  Created by Matt Felsen on 10/21/14.
//
//

#include "ofxMyoWebSockets.h"

using namespace ofxMyo;

//--------------------------------------------------------------
void Armband::setLockedState(){
    unlocked = false;
    ofNotifyEvent(hub->lockedEvent, *this);
}

//--------------------------------------------------------------
void Armband::setUnlockedState(){
    unlocked = true;
    unlockStartTime = ofGetElapsedTimef();
    poseStartTime = ofGetElapsedTimef();
    ofNotifyEvent(hub->unlockedEvent, *this);
}

//--------------------------------------------------------------
bool Armband::isLocked(){
    return !unlocked;
}

//--------------------------------------------------------------
bool Armband::isUnlocked(){
    return unlocked;
}

//--------------------------------------------------------------
void Armband::lock(){
//    hub->sendCommand(id, "lock");
    setLockedState();
}

//--------------------------------------------------------------
void Armband::unlock(string type){

    if (type != "")
        hub->sendCommand(id, "unlock", type);

    setUnlockedState();
}

//--------------------------------------------------------------
void Armband::vibrate(string type){
    hub->sendCommand(id, "vibrate", type);
}

//--------------------------------------------------------------
void Armband::notifyUserAction(string type){
    hub->sendCommand(id, "notify_user_action", type);
}

//--------------------------------------------------------------
void Armband::requestSignalStrength(){
    hub->sendCommand(id, "request_rssi");
}

//--------------------------------------------------------------
void Armband::resetCoordinateSystem(){
    quatOffset = ofQuaternion(-yawRaw, ofVec3f(0, 0, 1) );
}

//--------------------------------------------------------------
bool Armband::rollIsNear(float degrees, float threshold){

    if ( (roll >= degrees - threshold) && (roll <= degrees + threshold) )
        return true;
    else
        return false;

}

//--------------------------------------------------------------
bool Armband::pitchIsNear(float degrees, float threshold){

    if ( (pitch >= degrees - threshold) && (pitch <= degrees + threshold) )
        return true;
    else
        return false;

}

//--------------------------------------------------------------
bool Armband::yawIsNear(float degrees, float threshold){

    if ( (yaw >= degrees - threshold) && (yaw <= degrees + threshold) )
        return true;
    else
        return false;

}

//--------------------------------------------------------------
Hub::Hub(){

    requiresUnlock = false;
    unlockTimeout = 3.0f;

    minimumGestureDuration = 0.33f;
    lockAfterPose = true;

    convertToDegrees = false;

}

//--------------------------------------------------------------
void Hub::connect(bool autoReconnect){
    if (!connected) connect(hostname, port, autoReconnect);
}

//--------------------------------------------------------------
void Hub::connect(string hostname, int port, bool autoReconnect){

    this->hostname = hostname;
    this->port = port;

    ofxLibwebsockets::ClientOptions options = ofxLibwebsockets::defaultClientOptions();
    options.host = hostname;
    options.port = port;
    options.channel = "/myo/3";

    connected = false;
    reconnect = autoReconnect;
    reconnectTime = 3.0f;

    client.connect(options);
    client.addListener(this);
    
}

//--------------------------------------------------------------
void Hub::setLockingPolicy(string type){
    sendCommand("set_locking_policy", type);
}

//--------------------------------------------------------------
void Hub::setRequiresUnlock(bool require){
    requiresUnlock = require;
}

//--------------------------------------------------------------
void Hub::setUnlockTimeout(float time){
    unlockTimeout = time;
}

//--------------------------------------------------------------
void Hub::setMinimumGestureDuration(float time){
    minimumGestureDuration = time;
}

//--------------------------------------------------------------
void Hub::setLockAfterPose(bool lock){
    lockAfterPose = lock;
}

//--------------------------------------------------------------
void Hub::setUseDoubleTapUnlock(bool lock){
    useDoubleTapUnlock = lock;
}

//--------------------------------------------------------------
void Hub::setUseDegrees(bool degrees){
    convertToDegrees = degrees;
}

//--------------------------------------------------------------
void Hub::update(){

    // Check if we need to reconnect
    if (!connected && reconnect) {
        if (ofGetElapsedTimef() - reconnectLastAttempt > reconnectTime) {
            connect(reconnect);
            reconnectLastAttempt = ofGetElapsedTimef();
        }
    }

    // Check if poses have been held long enough that they're "confirmed"
    for (int i = 0; i < armbands.size(); i++) {

        Armband* armband = armbands[i];
        if (armband->poseConfirmed) continue;

        // Compare against the minimum hold time for hand poses
        if (ofGetElapsedTimef() - armband->poseStartTime > minimumGestureDuration) {

            if (armband->isUnlocked()) {
                armband->poseConfirmed = true;
                ofNotifyEvent(poseConfirmedEvent, *armband, this);
            }

            // unlock gesture
            if (armband->pose == "thumb_to_pinky" || armband->pose == "double_tap") {

                if (armband->isLocked()) {
                    armband->vibrate("short");
                    armband->notifyUserAction("single");
                }

                armband->setUnlockedState();

            } else {

                // re-lock after a confirmed pose
                if (lockAfterPose && armband->isUnlocked() && armband->pose != "thumb_to_pinky" && armband->pose != "double_tap") {
                    armband->setLockedState();
//                    armband->pose = "rest";
                    armband->poseConfirmed = false;
                }
            }
        }

        // The exception is the rest pose. This gets confirmed immediately.
        // I.e. hold a fist for half a second to get it confirmed, but immediately
        // go back to rest when you're done
        if (armband->pose == "rest") {
            if (armband->isUnlocked()) {
                armband->poseConfirmed = true;
                ofNotifyEvent(poseConfirmedEvent, *armband, this);
            }
        }
    }

    // Check if the bands need to be re-locked since they were last unlocked
    for (int i = 0; i < armbands.size(); i++) {

        Armband* armband = armbands[i];

        if (!requiresUnlock && armband->isLocked())
            armband->setUnlockedState();

        else if (requiresUnlock) {
            if (armband->isLocked()) continue;

            if (ofGetElapsedTimef() - armband->unlockStartTime > unlockTimeout) {
                armband->setLockedState();
//                armband->pose = "rest";
                armband->poseConfirmed = false;
                armband->notifyUserAction("single");
            }
        }
    }

}

//--------------------------------------------------------------
Armband* Hub::getArmband(int myoID){

    for (int i = 0; i < armbands.size(); i++) {
        if (armbands[i]->id == myoID) {
            return armbands[i];
        }
    }

    // If no armband is found, create and return an empty one
	return createArmband(myoID);

}

//--------------------------------------------------------------
Armband* Hub::createArmband(int myoID){

    ofLogNotice() << "ofxMyo::Armband Creating ID: " << myoID;

    Armband *armband = new Armband();

    armband->hub = this;

    armband->id = myoID;
    armband->rssi = -999;

    armband->arm = "unknown";
    armband->direction = "unknown";
    armband->pose = "unknown";
    armband->lastPose = "unknown";

    armband->quat = ofQuaternion(0, 0, 0, 1);
    armband->quatRaw = ofQuaternion(0, 0, 0, 1);
    armband->quatOffset = ofQuaternion(0, 0, 0, 1);
    armband->roll = 0;
    armband->pitch = 0;
    armband->yaw = 0;

    armband->poseStartTime = 0;
    armband->poseConfirmed = false;
    armband->unlockStartTime = 0;

    requiresUnlock ? armband->setLockedState() : armband->setUnlockedState();

    armbands.push_back(armband);

    return armband;

}

//--------------------------------------------------------------
int Hub::numConnectedArmbands(){
    return armbands.size();
}

//--------------------------------------------------------------
void Hub::sendCommand(string command, string type){
    sendCommand(-1, command, type);
}

//--------------------------------------------------------------
void Hub::sendCommand(int myoID, string command){
    sendCommand(myoID, command, "");
}

//--------------------------------------------------------------
void Hub::sendCommand(int myoID, string commandString, string type){

    // build the json message
    ofxJSONElement message;
    message[0] = "command";

    ofxJSONElement command;
    command["command"] = commandString;
    command["type"] = type;

    if (myoID != -1)
        command["myo"] = myoID;

    message[1] = command;

    // send it off
    client.send(message.getRawString());

    ofLogNotice() << "sendCommand(): " << myoID << ", " << commandString << ", " << type;

}

//--------------------------------------------------------------
void Hub::onMessage( ofxLibwebsockets::Event& args ){

    try {

        ofxJSONElement data = args.json[1];
        if (data.isNull()) return;

        int id = data["myo"].asInt();
        string event = data["type"].asString();

        Armband *armband = getArmband(id);
        if (!armband) armband = createArmband(id);

        //
        // PAIRED
        //
        if (event == "paired") {
            ofNotifyEvent(pairedEvent, *armband, this);
        }

        //
        // UNPAIRED
        //
        if (event == "unpaired") {

            for (int i = 0; i < armbands.size(); i++) {
                if (armbands[i]->id == id) {
                    armbands.erase(armbands.begin() + i);
                    break;
                }
            }

            ofNotifyEvent(unpairedEvent, *armband, this);
        }

        //
        // CONNECTED
        //
        if (event == "connected") {
            armband->requestSignalStrength();
            ofNotifyEvent(connectedEvent, *armband, this);
        }

        //
        // DISCONNECTED
        //
        if (event == "disconnected") {

            for (int i = 0; i < armbands.size(); i++) {
                if (armbands[i]->id == id) {
                    armbands.erase(armbands.begin() + i);
                    break;
                }
            }

            ofNotifyEvent(disconnectedEvent, *armband, this);
        }

        //
        // ARM RECOGNIZED
        //
        if (event == "arm_recognized" || event == "arm_synced") {

            armband->arm = data["arm"].asString();
            armband->direction = data["x_direction"].asString();

            ofNotifyEvent(armRecognizedEvent, *armband, this);
        }

        //
        // ARM LOST
        //
        if (event == "arm_lost" || event == "arm_unsynced") {

            armband->arm = "unknown";
            armband->direction = "unknown";

            ofNotifyEvent(armLostEvent, *armband, this);
        }

        //
        // ARM SYNCED
        //
        if (event == "arm_synced") {

            armband->arm = data["arm"].asString();
            armband->direction = data["x_direction"].asString();

            ofNotifyEvent(armSyncedEvent, *armband, this);
        }

        //
        // ARM UNSYNCED
        //
        if (event == "arm_unsynced") {

            armband->arm = "unknown";
            armband->direction = "unknown";

            ofNotifyEvent(armUnsyncedEvent, *armband, this);
        }

        //
        // ORIENTATION
        //
        if (event == "orientation") {

            // store accelerometer data
            ofxJSONElement accelerometer = data["accelerometer"];

            armband->accel.x = accelerometer[0].asFloat();
            armband->accel.y = accelerometer[1].asFloat();
            armband->accel.z = accelerometer[2].asFloat();

            // store gyroscope data
            ofxJSONElement gyroscope = data["gyroscope"];

            armband->gyro.x = gyroscope[0].asFloat();
            armband->gyro.y = gyroscope[1].asFloat();
            armband->gyro.z = gyroscope[2].asFloat();

            // store quaternion data
            ofxJSONElement quat = data["orientation"];

            float x = quat["x"].asFloat();
            float y = quat["y"].asFloat();
            float z = quat["z"].asFloat();
            float w = quat["w"].asFloat();

            armband->quat.set(x, y, z, w);
            armband->quatRaw.set(x, y, z, w);

            // calculate roll, pitch, and yaw value and store in raw versions
            // before offsetting the quaternion for the current coordinate system
            armband->rollRaw = atan2(2.0f * (w * x + y * z), 1.0f - 2.0f * (x * x + y * y));
            armband->pitchRaw = asin(2.0f * (w * y - z * x));
            armband->yawRaw = atan2(2.0f * (w * z + x * y), 1.0f - 2.0f * (y * y + z * z));

            // apply the offset to reset the coordinate system/zero out yaw
            armband->quat *= armband->quatOffset;

            x = armband->quat.x();
            y = armband->quat.y();
            z = armband->quat.z();
            w = armband->quat.w();

            // now re-calculate roll, pitch, and yaw values after quat has been offset
            armband->roll = atan2(2.0f * (w * x + y * z), 1.0f - 2.0f * (x * x + y * y));
            armband->pitch = asin(2.0f * (w * y - z * x));
            armband->yaw = atan2(2.0f * (w * z + x * y), 1.0f - 2.0f * (y * y + z * z));

            // convert to degrees if the setting is on
            if (convertToDegrees) {
                armband->roll = ofRadToDeg(armband->roll);
                armband->pitch = ofRadToDeg(armband->pitch);
                armband->yaw = ofRadToDeg(armband->yaw);

                armband->rollRaw = ofRadToDeg(armband->rollRaw);
                armband->pitchRaw = ofRadToDeg(armband->pitchRaw);
                armband->yawRaw = ofRadToDeg(armband->yawRaw);
            }
            
            // flip pitch so that...
            // - up is positive
            // - down is negative
            if (armband->direction == "toward_wrist") {
                armband->pitch *= -1;
                armband->pitchRaw *= -1;
            }
            
            // flip roll so that...
            // - rolling right is positive roll
            // - rolling left is negative
            if (armband->direction == "toward_elbow") {
                armband->roll *= -1;
                armband->rollRaw *= -1;
            }
            
            ofNotifyEvent(orientationEvent, *armband, this);
        }

        //
        // UNLOCK
        //
        if (event == "unlocked") {

            armband->setUnlockedState();

        }

        //
        // LOCK
        //
        if (event == "locked") {

            armband->setLockedState();
//            armband->pose = "rest";

        }

        //
        // POSE
        //
        if (event == "pose") {

            armband->lastPose = armband->pose;
            
            armband->pose = data["pose"].asString();
            armband->poseConfirmed = false;
            armband->poseStartTime = ofGetElapsedTimef();

            ofNotifyEvent(poseStartedEvent, *armband, this);

            if (armband->pose == "double_tap" && useDoubleTapUnlock) {

                armband->vibrate("short");
                armband->notifyUserAction("single");
                armband->setUnlockedState();

            }

        }
        
        //
        // RSSI
        //
        if (data["type"].asString() == "rssi") {
            armband->rssi = data["rssi"].asInt();
            ofNotifyEvent(rssiReceivedEvent, *armband, this);
        }
        
    }
    catch(exception& e){
        ofLogError() << e.what();
    }
    
}

//--------------------------------------------------------------
void Hub::onConnect( ofxLibwebsockets::Event& args ){
    ofLogNotice("Socket Connected");
}

//--------------------------------------------------------------
void Hub::onOpen( ofxLibwebsockets::Event& args ){
    connected = true;
    ofLogNotice("Socket Open");
}

//--------------------------------------------------------------
void Hub::onClose( ofxLibwebsockets::Event& args ){
    armbands.clear();
    connected = false;
    reconnectLastAttempt = ofGetElapsedTimef();
    ofLogNotice("Socket Closed");
}

//--------------------------------------------------------------
void Hub::onIdle( ofxLibwebsockets::Event& args ){
    ofLogVerbose("Socket Idle");
}

//--------------------------------------------------------------
void Hub::onBroadcast( ofxLibwebsockets::Event& args ){
    ofLogVerbose("Socket Broadcast");
}
