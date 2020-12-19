#ifndef OSSIAVID_H
#define OSSIAVID_H

#include "ofMain.h"

#define MATRIX_SIZE 32

#ifdef KINECT
#include "ofxKinect.h"
#endif

#ifdef CV
#include "ofxOpenCv.h"
#endif

//--------------------------------------------------------------
class ossiaVid
{
public:
    ossiaVid();
    ofParameterGroup params;

    void checkResize();

    struct canvas
    {
    void corner2center(const unsigned int* wAndH, const float& reSize, const ofVec3f& center);
    float x, y, z;
    unsigned int h, w;
    ofVec2f middle;
    };

    unsigned int vidWandH[2];
    canvas canv;
    ofParameter<float> size;

protected:
    float prevSize;
    ofVec3f prevPlace;
    ofParameter<ofVec3f> placement;

#ifdef CV // only needed for cvUpdate
    unsigned int vidArea;
#endif

    ofParameter<ofVec4f> color;

    ofParameter<bool> drawVid;
    ofParameter<bool> flipVid;
    ofTexture videoTexture;

    // Pixel process;
    size_t widthSpread;
    unsigned int verticalStep;
    size_t heightSpread;
    size_t widthMargin;
    size_t heightMargin;
    unsigned int widthRemainder;
    unsigned int heightRemainder;
    unsigned int halfWandH[2];
    unsigned int bary[2]{0, 0};

    ofParameterGroup pixControl;
    ofParameter<bool> getPixels;
    ofParameter<bool> invert;
    ofParameter<int> hPoints;
    ofParameter<int> vPoints;
    ofParameter<float> threshold;
    ofParameter<float> pixVal[MATRIX_SIZE * MATRIX_SIZE];
    ofParameter<ofVec4f> averageColor;
    ofParameter<ofVec3f> centroid;
    ofParameter<bool> drawCircles;
    ofParameter<bool> drawCenter;
    ofParameter<float> circleSize;
    ofParameter<int> circleResolution;
    ofParameter<ofVec4f> circleColor;
    ofParameterGroup pixMatrix;

    void setMatrix(ofParameterGroup& params);
    void processPix(const ofPixels& px, ofParameter<float>* pv
                #ifdef CV
                , const ofPixels& cvPx
                #endif
                    );
    void drawPix(ofParameter<float>* pv);
};

//--------------------------------------------------------------
#ifdef CV
class ossiaCv
{
public:
    ofxCvContourFinder contourFinder;
    ofParameter<bool> getContours;

protected:
    ofParameterGroup cvControl;

    ofParameter<int> maxThreshold;

    ofxCvColorImage colorImg;
    ofxCvGrayscaleImage grayImage; // grayscale depth image

    ofxCvGrayscaleImage grayBg;
    ofParameter<bool> holdBackGround;
    void setBackGround(bool& hold);

    ofParameter<bool> drawCvImage;

    ofParameterGroup blobs;
    ofParameter<int> minArea;
    ofParameter<int> maxArea;
    ofParameter<bool> drawContours;
    // blob position and size
    ofParameter<ofVec3f> position[5];
    ofParameter<float> area[5];

    void cvSetup(const unsigned int* wAndH, ofParameterGroup& group);
    void cvUpdate(ofPixels& pixels, const unsigned int* wAndH, const unsigned int& wArea);
    void cvdraw(const ossiaVid::canvas& cnv);
};
#endif

//--------------------------------------------------------------
class ossiaPlayer: public ossiaVid
        #ifdef CV
        , protected ossiaCv
        #endif
{
public:
    ossiaPlayer(string path);
    void setup();
    void update();
    void draw();
    void close();
    ofVideoPlayer vid;

private:
    string path;

    ofParameter<bool> play;
    void setPlay(bool &toPlay);

    ofParameter<bool> loop;
    void setLoop(bool &toLoop);

    ofParameter<float> seek;
    void setFrame(float &toSeek);

    ofParameter<float> volume;
    void setVolume(float &toAmp);
};

//--------------------------------------------------------------
class ossiaGrabber: public ossiaVid
        #ifdef CV
        , protected ossiaCv
        #endif
{
public:
    ossiaGrabber(ofVideoDevice dev);
    void setup(unsigned int width, unsigned int height);
    void update();
    void draw();
    void close();
    ofVideoGrabber vid;

private:
    ofVideoDevice device;

    ofParameter<bool> freeze;
};

//--------------------------------------------------------------
#ifdef KINECT
class ossiaKinect: public ossiaVid
        #ifdef CV
        , public ossiaCv
        #endif
{
public:
    ossiaKinect(int dev);
    void setup(bool infrared);
    void update();
    void draw();
    void close();
    ofxKinect vid;

private:    
    int device;

    ofParameter<bool> freeze;

    ofParameter<float> angle;
    void tilt(float &angle);

    ofMesh mesh;
    ofParameterGroup depthMesh;
    ofParameter<bool> drawMesh;
    ofParameter<ofVec4f> mColor;
    ofParameter<float> mThreshold, vSize, yRotate, xRotate;
    ofParameter<ofVec3f> mPosition;
    ofMesh getMesh();
};
#endif

#endif // OSSIAVID_H
