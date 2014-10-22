//
//  ofxMyoWebSockets
//
//  Created by Matt Felsen on 10/21/14.
//
//

#include "ofxMyoWebSockets.h"

using namespace ofxMyoWebSockets;

//--------------------------------------------------------------
Connection::Connection(){

	requiresUnlock = false;
	unlockTimeout = 3.0f;

	minimumGestureDuration = 0.5f;
	lockAfterPose = true;

}

//--------------------------------------------------------------
void Connection::setRequiresUnlock(bool require){
	requiresUnlock = require;
}

//--------------------------------------------------------------
void Connection::setUnlockTimeout(float time){
	unlockTimeout = time;
}

//--------------------------------------------------------------
void Connection::setMinimumGestureDuration(float time){
	minimumGestureDuration = time;
}

//--------------------------------------------------------------
void Connection::setLockAfterPose(bool lock){
	lockAfterPose = lock;
}

//--------------------------------------------------------------
void Connection::connect(bool autoReconnect){

	ofxLibwebsockets::ClientOptions options = ofxLibwebsockets::defaultClientOptions();
	options.host = "localhost";
	options.port = 10138;
	options.channel = "/myo/1";

	connected = false;
	reconnect = autoReconnect;
	reconnectTime = 3.0f;

	client.connect(options);
	client.addListener(this);

}

//--------------------------------------------------------------
void Connection::update(){

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
		if (armband->pose == "rest" || armband->poseConfirmed) continue;

		if (ofGetElapsedTimef() - armband->poseStartTime > minimumGestureDuration) {
			armband->poseConfirmed = true;
			ofNotifyEvent(poseConfirmedEvent, armband, this);

			// unlock gesture
			if (armband->pose == "thumb_to_pinky") {

				if (!armband->unlocked) {
					vibrate(armband, "double");
					ofNotifyEvent(unlockedEvent, armband, this);
				}

				armband->unlocked = true;
				armband->unlockStartTime = ofGetElapsedTimef();
				
			} else {

				// re-lock after a confirmed pose
				if (lockAfterPose && armband->unlocked && armband->pose != "thumb_to_pinky") {
					armband->unlocked = false;
					vibrate(armband, "short");
				}
			}
		}
	}

	// Check if the bands need to be re-locked since they were last unlocked
	for (int i = 0; i < armbands.size(); i++) {

		Armband* armband = armbands[i];
		if (!armband->unlocked) continue;

		if (ofGetElapsedTimef() - armband->unlockStartTime > unlockTimeout) {
			armband->unlocked = false;
			ofNotifyEvent(lockedEvent, armband, this);
		}
	}

}

//--------------------------------------------------------------
Armband* Connection::getArmband(int myoID){

	for (int i = 0; i < armbands.size(); i++) {
		if (armbands[i]->id == myoID) {
			return armbands[i];
		}
	}

	return NULL;

}

//--------------------------------------------------------------
Armband* Connection::createArmband(int myoID){

	Armband *armband = new Armband();

	armband->id = myoID;
	armband->rssi = -999;

	armband->arm = "unknown";
	armband->direction = "unknown";
	armband->pose = "unknown";
	armband->lastPose = "unknown";

	armband->quat = ofQuaternion(0, 0, 0, 1);
	armband->roll = 0;
	armband->pitch = 0;
	armband->yaw = 0;

	armband->poseStartTime = 0;
	armband->poseConfirmed = false;
	armband->unlocked = !requiresUnlock;
	armband->unlockStartTime = 0;

	armbands.push_back(armband);

	return armband;
	
}

//--------------------------------------------------------------
int Connection::numConnectedArmbands(){
	return armbands.size();
}

//--------------------------------------------------------------
void Connection::vibrate(int myoID, string type){

	string vibrationString = type;
	if (type == "double") vibrationString = "short";

	// build the json message
	ofxJSONElement message;
	message[0] = "command";

	ofxJSONElement command;
	command["myo"] = myoID;
	command["command"] = "vibrate";
	command["type"] = vibrationString;
	message[1] = command;

	// send it off
	client.send(message.getRawString());

	// if it's a double, just send it again! no need for timers :)
	if (type == "double")
		client.send(message.getRawString());
}

//--------------------------------------------------------------
void Connection::vibrate(Armband* armband, string type){
	vibrate(armband->id, type);
}

//--------------------------------------------------------------
void Connection::requestSignalStrength(int myoID){

	// build the json message
	ofxJSONElement message;
	message[0] = "command";

	ofxJSONElement command;
	command["myo"] = myoID;
	command["command"] = "request_rssi";
	message[1] = command;

	// send it off
	client.send(message.getRawString());

}

//--------------------------------------------------------------
void Connection::requestSignalStrength(Armband* armband	){
	requestSignalStrength(armband->id);
}

//--------------------------------------------------------------
void Connection::onMessage( ofxLibwebsockets::Event& args ){

	try {

		ofxJSONElement data = args.json[1];
		if (data.isNull()) return;

		int id = data["myo"].asInt();
		string event = data["type"].asString();

		Armband *armband = getArmband(id);
		if (!armband) armband = createArmband(id);

		//
		// CONNECTED
		//
		if (event == "connected") {
			vibrate(armband, "short");
			requestSignalStrength(armband);
			ofLogNotice() << "Myo Connected: ID " << id;
		}

		//
		// PAIRED
		//
		if (event == "paired") {
			ofLogNotice() << "Myo Paired: ID " << id;
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

			ofLogNotice() << "Myo Disconnected: ID " << id;
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

			ofLogNotice() << "Myo Unpaired: ID " << id;
		}

		//
		// ARM RECOGNIZED
		//
		if (event == "arm_recognized") {

			armband->arm = data["arm"].asString();
			armband->direction = data["x_direction"].asString();

			ofLogNotice() << "Arm Recognized: ID " << id << ", " << armband->arm << ", " << armband->direction;
		}

		//
		// ARM LOST
		//
		if (event == "arm_lost") {

			armband->arm = "unknown";
			armband->direction = "unknown";

			ofLogNotice() << "Arm Lost: ID " << id;
		}

		//
		// ORIENTATION
		//
		if (event == "orientation") {

			ofxJSONElement quat = data["orientation"];

			float x = quat["x"].asFloat();
			float y = quat["y"].asFloat();
			float z = quat["z"].asFloat();
			float w = quat["w"].asFloat();

			armband->quat.set(x, y, z, w);

			armband->roll = atan2(2.0f * (w * x + y * z), 1.0f - 2.0f * (x * x + y * y));
			armband->pitch = asin(2.0f * (w * y - z * x));
			armband->yaw = atan2(2.0f * (w * z + x * y), 1.0f - 2.0f * (y * y + z * z));

			ofNotifyEvent(orientationEvent, armband, this);
		}

		//
		// POSE
		//
		if (event == "pose") {

			armband->lastPose = armband->pose;

			armband->pose = data["pose"].asString();
			armband->poseConfirmed = false;
			armband->poseStartTime = ofGetElapsedTimef();

			ofLogNotice("Myo Pose: " + armband->pose);
			ofNotifyEvent(poseStartedEvent, armband, this);

		}

		//
		// RSSI
		//
		if (data["type"].asString() == "rssi") {
			armband->rssi = data["rssi"].asInt();
			ofLogNotice() << "ID " << armband->id << " RSSI: " << armband->rssi;
		}

	}
	catch(exception& e){
		ofLogError() << e.what();
	}

}

//--------------------------------------------------------------
void Connection::onConnect( ofxLibwebsockets::Event& args ){
	ofLogNotice("Socket Connected");
}

//--------------------------------------------------------------
void Connection::onOpen( ofxLibwebsockets::Event& args ){
	connected = true;
	ofLogNotice("Socket Open");
}

//--------------------------------------------------------------
void Connection::onClose( ofxLibwebsockets::Event& args ){
	armbands.clear();
	connected = false;
	reconnectLastAttempt = ofGetElapsedTimef();
	ofLogNotice("Socket Closed");
}

//--------------------------------------------------------------
void Connection::onIdle( ofxLibwebsockets::Event& args ){
	ofLogVerbose("Socket Idle");
}

//--------------------------------------------------------------
void Connection::onBroadcast( ofxLibwebsockets::Event& args ){
	ofLogVerbose("Socket Broadcast");
}
