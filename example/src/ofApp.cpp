#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

	myo.connect(true);
	myo.setRequiresUnlock(true);
	myo.setLockAfterPose(true);
	cam.setOrientation( ofVec3f(70, 0, 25) );

}

//--------------------------------------------------------------
void ofApp::update(){

	myo.update();

}

//--------------------------------------------------------------
void ofApp::draw(){

	float y = 1;
	float angle; ofVec3f axis;

	ofDrawBitmapStringHighlight("Connected bands: " + ofToString(myo.numConnectedArmbands()), 20, 20 * y);

	for (int i = 0; i < myo.numConnectedArmbands(); i++) {

		ofxMyoWebSockets::Armband* armband = myo.armbands[i];

		cam.begin();
		ofDrawGrid(250, 5);

		ofVec3f pos = armband->quat * ofVec3f(300, 0, 0);
		ofSetColor( ofColor::fromHsb((float)i/myo.numConnectedArmbands()*255, 225, 225) );
		ofDrawArrow( ofVec3f(0, 0, 0), pos, 8 );

		cam.end();

		y++;

		ofDrawBitmapStringHighlight("Armband " + ofToString(i), 20, 20 * ++y);
		ofDrawBitmapStringHighlight("Myo ID: " + ofToString(armband->id), 20, 20 * ++y);
		ofDrawBitmapStringHighlight("Arm: " + armband->arm, 20, 20 * ++y);
		ofDrawBitmapStringHighlight("X Direction: " + armband->direction, 20, 20 * ++y);
		ofDrawBitmapStringHighlight("Pose: " + armband->pose, 20, 20 * ++y);
		ofDrawBitmapStringHighlight("Pose Confirmed: " + ofToString(armband->poseConfirmed), 20, 20 * ++y);
		ofDrawBitmapStringHighlight("Unlocked: " + ofToString(armband->unlocked), 20, 20 * ++y);
		
	}

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
