#pragma once

#include "ofMain.h"
#include "../../ossiaUtils/ossiaUtils.h"

#ifdef NET
#include <unistd.h> //contains various constants
#include <sys/types.h> //contains a number of basic derived types that should be used whenever appropriate
#include <arpa/inet.h> // defines in_addr structure
#include <sys/socket.h> // for socket creation
#include <netinet/in.h> //contains constants and structures needed for internet domain addresses
#endif

class ofApp : public ofBaseApp{

public:
    void setup();
    void update();
    void draw();
    void exit();

    void windowResized(int w, int h);

    ofParameterGroup parameters;
    //ossiaUtils::players video;
    //ossiaUtils::grabbers camera;
    ossiaUtils::kinects kinect;
    ofParameter<ofVec4f> backGround;

    ofxOscQueryServer oscQuery;

#ifdef NET
    char dataSending[4];
    // Actually this is called packet in Network Communication, which contain data and send through.
    int clintListn = 0, clintConnt = 0;
    struct sockaddr_in ipOfServer;

    void writeInt(int fd, int x);
    void writeFloat(int fd, float x);
    void writeContour(int fd, ofxCvBlob blob,const unsigned int* wh);
    void writeBlobs(int fd, ofxCvContourFinder &cntr, const unsigned int* wh);

    int readInt(int fd);
    float readFloat(int fd);
    vector<ofPoint> readContour(int fd, const unsigned int* wh);
    ofMesh readBlobs(int fd, const unsigned int* wh);

    ofMesh meshToDraw;
    ofParameterGroup NetMesh;
    ofParameter<bool> drawNetMesh;
    ofParameter<float> pointSize;
#endif
};
