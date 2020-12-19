#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

    parameters.add(backGround.set("BackGround",
                   ofVec4f(255, 0, 0, 0),
                   ofVec4f(0, 0, 0, 0),
                   ofVec4f(255, 255, 255, 255)));

    //video.setup();
    camera.setup(1);
    //kinect.setup(); // default to "RGB" instead of infrared

    //parameters.add(video.parameters);
    parameters.add(camera.parameters);
    //parameters.add(kinect.parameters);

#ifdef NET
    clintListn = socket(AF_INET, SOCK_STREAM, 0); // creating socket
    memset(&ipOfServer, '0', sizeof(ipOfServer));
    memset(dataSending, '0', sizeof(dataSending));
    ipOfServer.sin_family = AF_INET;
    ipOfServer.sin_addr.s_addr = htonl(INADDR_ANY);
    ipOfServer.sin_port = htons(2017); 		// this is the port number of running server
    bind(clintListn, (struct sockaddr*)&ipOfServer, sizeof(ipOfServer));
    listen(clintListn, 20);
    clintConnt = accept(clintListn, (struct sockaddr*)NULL, NULL);

    NetMesh.setName("net_mesh");
    NetMesh.add(drawNetMesh.set("draw", false));
    NetMesh.add(pointSize.set("point_size", 3, 1, 100));
    parameters.add(NetMesh);
#endif

    oscQuery.setup(parameters, 1234, 5678, "ossiaKinects");

    oscQuery[backGround].setClipMode("both").setUnit("color.argb8");

    //video.setAtributes(oscQuery);
    camera.setAtributes(oscQuery);
    //kinect.setAtributes(oscQuery);
}

//--------------------------------------------------------------
#ifdef NET
void ofApp::writeInt(int fd, int x)
{
    unsigned int hx{htonl(x)};
    int nbW = write(fd, (char *) &hx, 4);

    if (nbW < 4) cout << "probleme!";
}

void ofApp::writeFloat(int fd, float x)
{
    float * gee = &x;
    int * buz = (int *) gee;
    * buz = htonl(* buz);

    write(fd, (char *) &x, 4);
}

void ofApp::writeContour(int fd, ofxCvBlob blob, const unsigned int* wh)
{
    writeInt(fd, blob.nPts);

    for (const glm::vec3& p : blob.pts)
    {
        float x = ((p.x / wh[0]) * 2) -1;
        float y = ((p.y / wh[1]) * 2) -1;

        writeFloat(fd, x);
        writeFloat(fd, y);
    }

    writeInt(fd, 0);
}

void ofApp::writeBlobs(int fd, ofxCvContourFinder &cntr, const unsigned int* wh)
{
    writeInt(fd, cntr.nBlobs);

    for (const ofxCvBlob& b : cntr.blobs)
    {
        writeContour(fd, b, wh);
    }
}

void readBuffer(int fd, char* buf, int n)
{
    while (n != 0)
    {
        int rcv = recv(fd, buf, n, 0);

        if (rcv == 0)
        {
            cout << "wrong buf" << '\n';
            exit(0);
        }

        buf += rcv;
        n -= rcv;
    }
}

int ofApp::readInt(int fd)
{
    int hx{0};
    readBuffer(fd, (char *) &hx, 4);

    return ntohl(hx);
}

float ofApp::readFloat(int fd)
{
    float hx{0};
    readBuffer(fd, (char *) &hx, 4);

    int gee = (ntohl(*((int *) &hx)));

    float buz = *((float *) &gee);

    return buz;
}

vector<ofPoint> ofApp::readContour(int fd, const unsigned int* wh)
{
    vector<ofPoint> p;
    int n{readInt(fd)};

    for (int i = 0; i < n; i++)
    {
        float x = readFloat(fd);
        float y = readFloat(fd);
        p.push_back(ofPoint(x,y));
    }

    readBlobs(fd, wh);

    return p;
}

ofMesh ofApp::readBlobs(int fd, const unsigned int* wh)
{
    ofMesh m;
    m.setMode(OF_PRIMITIVE_POINTS);

    int n{readInt(fd)};

    for (int i = 0; i < n; i++)
    {
        vector<ofPoint> vert{readContour(fd, wh)};

        for (const ofPoint p : vert)
        {
            ofPoint pn = (p + 1) / 2;
            pn.x = pn.x * wh[0];
            pn.y = pn.y * wh[1];
            m.addVertex(pn);
            m.addColor(ofColor(255, 255, 255, 255));
        }
    }

    return m;
}
#endif

//--------------------------------------------------------------
void ofApp::update(){

    //video.update();
    camera.update();
    //kinect.update();

#ifdef NET
    if (kinect.vids[0]->vid.isFrameNew() && kinect.vids[0]->getContours)
    {
        writeBlobs(clintConnt, kinect.vids[0]->contourFinder, kinect.vids[0]->vidWandH);
        if (recv(clintConnt, &dataSending, 0, MSG_DONTWAIT) == 0)
            meshToDraw = readBlobs(clintConnt, kinect.vids[0]->vidWandH);
    }
#endif

}

//--------------------------------------------------------------
void ofApp::draw(){

    ofBackground(backGround->y,
                 backGround->z,
                 backGround->w,
                 backGround->x);

#ifdef NET
    if (drawNetMesh)
    {
        glPointSize(pointSize);
        ofPushMatrix();
        ofTranslate(kinect.vids[0]->canv.x, kinect.vids[0]->canv.y, 0); // center the points a bit
        ofScale(kinect.vids[0]->size);
        meshToDraw.drawVertices();
        ofPopMatrix();
    }
#endif

    //video.draw();
    camera.draw();
    //kinect.draw();
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
