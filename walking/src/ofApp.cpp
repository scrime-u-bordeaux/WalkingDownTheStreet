#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

    parameters.add(backGround.set("BackGround",
                   ofVec4f(255, 0, 0, 0),
                   ofVec4f(0, 0, 0, 0),
                   ofVec4f(255, 255, 255, 255)));

    video.setup();
    //camera.setup(1);
    kinect.setup(); // default to "RGB" instead of infrared

    parameters.add(video.parameters);
    //parameters.add(camera.parameters);
    parameters.add(kinect.parameters);

    oscQuery.setup(parameters, 1234, 5678, "ossiaKinects");

    oscQuery[backGround].setClipMode("both").setUnit("color.argb8");

    video.setAtributes(oscQuery);
    //camera.setAtributes(oscQuery);
    kinect.setAtributes(oscQuery);
}

//--------------------------------------------------------------
void ofApp::update(){

    video.update();
    //camera.update();
    kinect.update();
}

//--------------------------------------------------------------
void ofApp::draw(){

    ofBackground(backGround->y,
                 backGround->z,
                 backGround->w,
                 backGround->x);

    video.draw();
    //camera.draw();
    kinect.draw();
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

    kinect.resize();
    //camera.resize();
}

void ofApp::exit(){

    kinect.close();
    //camera.close();
}
