#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

    parameters.add(backGround.set("BackGround",
                   ofVec4f(255, 0, 0, 0),
                   ofVec4f(0, 0, 0, 0),
                   ofVec4f(255, 255, 255, 255)));

    //kinect.setup(); // default to "RGB" instead of infrared
    camera.setup(1);

    //parameters.add(kinect.parameters);
    parameters.add(camera.parameters);

    oscQuery.setup(parameters, 1234, 5678, "ossiaKinects");

    oscQuery[backGround].setClipMode("both").setUnit("color.argb8");

    //kinect.setAtributes(oscQuery);
    camera.setAtributes(oscQuery);
}

//--------------------------------------------------------------
void ofApp::update(){

    //kinect.update();
    camera.update();
}

//--------------------------------------------------------------
void ofApp::draw(){

    ofBackground(backGround->y,
                 backGround->z,
                 backGround->w,
                 backGround->x);

    //kinect.draw();
    camera.draw();
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

    //kinect.resize();
    camera.resize();
}

void ofApp::exit(){

    //kinect.close();
    camera.close();
}
