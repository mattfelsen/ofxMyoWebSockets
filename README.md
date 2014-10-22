# ofxMyoWebSockets

An openFrameworks addon for working with [Myo armbands from Thalmic Labs](https://www.thalmic.com/en/myo/) over the built-in [WebSocket interface](https://developer.thalmic.com/forums/topic/534/) that is part of the Myo Connect background application from Thalmic.

## Example

	class ofApp : public ofBaseApp{
	public:
		ofxMyoWebSockets::Connection myo;
	};

	void ofApp::setup(){
		// pass in a bool for auto-reconnect
		myo.connect(true);
	}
	
	void ofApp::update(){
		myo.update();
	}
	
	void ofApp::draw(){
	
		ofDrawBitmapStringHighlight("Connected bands: " + ofToString(myo.numConnectedArmbands()), 20, 20 * y);
	
		float angle; ofVec3f axis;
		for (int i = 0; i < myo.numConnectedArmbands(); i++) {
	
			ofxMyoWebSockets::Armband* armband = myo.armbands[i];
	
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

### Pinky-to-Thumb Unlock
Enable this if you'd like to require this hand pose to be performed before other hand poses are recognized.

	myo.setRequiresUnlock(true)

### Lock After Pose
Armbands can be set back to being locked immediately following a recognized hand pose, requiring the pinky-to-thumb pose again in order to prevent accidental input.

	myo.setLockAfterPose(true)

## Dependencies

- [ofxLibwebsockets](https://github.com/labatrockwell/ofxLibwebsockets) from the [LAB at Rockwell](https://github.com/labatrockwell)
- [ofxJSON](https://github.com/jefftimesten/ofxJSON) from [Jeff Crouse](https://github.com/jefftimesten)