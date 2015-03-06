# ofxMyoWebSockets

An openFrameworks addon for working with [Myo armbands from Thalmic Labs](https://www.thalmic.com/en/myo/) over the built-in [WebSocket interface](https://developer.thalmic.com/forums/topic/534/) that is part of the Myo Connect background application from Thalmic.

## Example
See the included example project. In brief:

	class ofApp : public ofBaseApp{
	public:
		ofxMyo::Hub myo;
	};

	void ofApp::setup(){
		// pass in a bool for auto-reconnect
		myo.connect(true);
	}
	
	void ofApp::update(){
		myo.update();
	}
	
	void ofApp::draw(){
	
		ofDrawBitmapStringHighlight("Connected bands: " + ofToString(myo.numConnectedArmbands()), 20, 20);
	
		float angle; ofVec3f axis;
		for (int i = 0; i < myo.numConnectedArmbands(); i++) {
	
			ofxMyo::Armband* armband = myo.armbands[i];
	
			cam.begin();
			ofDrawGrid(250, 5);
	
			ofVec3f pos = armband->quat * ofVec3f(300, 0, 0);
			ofDrawArrow( ofVec3f(0, 0, 0), pos, 8 );
	
			cam.end();
	
		}
	}

## Features

### Multiple Myo Armbands
This addon supports multiple armbands. The current limitation is the Myo Connect background application, which only supports two. If Myo Connect is updated, this addon should automatically support additional armbands.

You only need to create one instance of `ofxMyoWebSockets::Connection`. Each armband should be referenced by its ID assigned from Myo Connect, which can be accessed at `ofxMyoWebSockets::Armband::id`.

### Event Notification
Data and events from the websocket stream are relayed using the openFrameworks notification system. Register for events you care about in your app's setup method:

	ofAddListener(myo.connectedEvent, this, &ofApp::myoConnected);
	ofAddListener(myo.poseConfirmedEvent, this, &ofApp::poseConfirmed);
	ofAddListener(myo.orientationEvent, this, &ofApp::orientation);

Make sure to declare your callbacks in your app's header file:

	void myoConnected(ofxMyoWebSockets::Armband& armband);
	void poseConfirmed(ofxMyoWebSockets::Armband& armband);
	void orientation(ofxMyoWebSockets::Armband& armband);

And handle them like so:

	void ofApp::poseConfirmed(ofxMyoWebSockets::Armband& armband){
		if (armband.pose == "wave_out") {
			ofLogNotice() << "Hi!";
		}
	}

See the [WebSocket interface specification](https://developer.thalmic.com/forums/topic/534/) and look in the `ofxMyoWebSockets.h` header file for a full list of events.

### Pinky-to-Thumb Unlock
Enable this if you'd like to require this hand pose to be performed before other hand poses are recognized.

	myo.setRequiresUnlock(true);

### Unlock Timeout
You can set how long the unlocked state will last by calling the following method. The current default is 3 seconds.

	myo.setUnlockTimeout(5.0);

### Lock After Pose
Armbands can be set back to being locked immediately following a recognized hand pose, requiring the pinky-to-thumb pose again in order to prevent accidental input.

	myo.setLockAfterPose(true);

### Pose Confirmatin/Minimum Gesture Duration
You can set a minimum time that a pose most be held before it is recognized. This is useful if you want to wave out for 1 second before triggering a pose to prevent accidental input. The current default is 0.5 seconds.

	myo.setMinimumGestureDuration(1.0);

To disable, simply pass in a duration of 0:

	myo.setMinimumGestureDuration(0.0);

### Use Degrees or Radians
Roll, pitch, and yaw values are automatically calculated based on the stored quaternion value. By default, these values are represented in radians. If you'd prefer to work in degrees, just call the following method:

	myo.setUseDegrees(true);

## Dependencies

- [ofxLibwebsockets](https://github.com/labatrockwell/ofxLibwebsockets) from the [LAB at Rockwell](https://github.com/labatrockwell)
- [ofxJSON](https://github.com/jefftimesten/ofxJSON) from [Jeff Crouse](https://github.com/jefftimesten)