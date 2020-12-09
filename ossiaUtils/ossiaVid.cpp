#include "ossiaVid.h"

//--------------------------------------------------------------
void ossiaVid::canvas::corner2center(const unsigned int* wAndH, const float& reSize, const ofVec3f& center)
{
    float width{wAndH[0] * reSize};
    float height{wAndH[1] * reSize};

    float wOfset{width / 2};
    float hOfset{height / 2};

    int ofHalfW{ofGetWidth() / 2};
    int ofHalfH{ofGetHeight() / 2};

    float xOfset{ofHalfW + (ofHalfW * center[0])};
    float yOfset{ofHalfH + (ofHalfH * center[1])};

    x = xOfset - wOfset;
    y = yOfset - hOfset;
    z = center.z;
    w = width;
    h = height;
}

//--------------------------------------------------------------
ossiaVid::ossiaVid()
{
    params.add(drawVid.set("draw_video", false));

    params.add(size.set("size", 1., 0., 10.));
    prevSize = size;

    params.add(placement.set("placement",
                             ofVec2f(0, 0),
                             ofVec2f(-1, -1),
                             ofVec2f(1, 1)));
    prevPlace = placement;

    params.add(color.set("draw_color",
                         ofVec4f(255, 255, 255, 255),
                         ofVec4f(0, 0, 0, 0),
                         ofVec4f(255, 255, 255, 255)));
}

void ossiaVid::checkResize()
{
    if (prevSize != size || prevPlace != placement)
    {
        canv.corner2center(vidWandH, size, placement);
        prevSize = size;
        prevPlace = placement;
    }
}

void ossiaVid::setMatrix(ofParameterGroup& params)
{
    pixMatrix.setName("matrix");

    int i{1};
    int j{1};

    for (ofParameter<float>& p: pixVal)
    {
        p.set("columms_" + to_string(i) + "/row_" + to_string(j), 0, 0, 255);
        pixMatrix.add(p);

        j++;

        if (j > MATRIX_SIZE)
        {
            i++;
            j = 1;
        }
    }

    pixControl.setName("pixels");
    pixControl.add(getPixels.set("get", false));
    pixControl.add(invert.set("invert", false));
    pixControl.add(hPoints.set("horizontal_points", MATRIX_SIZE / 2, 1, MATRIX_SIZE));
    pixControl.add(vPoints.set("vertical_points", MATRIX_SIZE / 2, 1, MATRIX_SIZE));
    pixControl.add(threshold.set("threshold", 64, 1, 255));

    pixControl.add(pixMatrix);
    pixControl.add(averageColor.set("average_color",
                                    ofVec4f(255, 0, 0, 0),
                                    ofVec4f(0, 0, 0, 0),
                                    ofVec4f(255, 255, 255, 255)));

    pixControl.add(centroid.set("barycenter",
                                ofVec2f(0, 0),
                                ofVec2f(-1, -1),
                                ofVec2f(1, 1)));

    pixControl.add(drawCircles.set("draw_matrix", false));
    pixControl.add(drawCenter.set("draw_barycenter", false));
    pixControl.add(circleSize.set("circles_size", 0.1, 0, 1));
    pixControl.add(circleResolution.set("circle_resolution", 22, 3, 100));
    pixControl.add(circleColor.set("circles_color",
                                   ofVec4f(255, 255, 255, 255),
                                   ofVec4f(0, 0, 0, 0),
                                   ofVec4f(255, 255, 255, 255)));
    params.add(pixControl);

    widthSpread = vidWandH[0] / hPoints; // horizontal points
    verticalStep = MATRIX_SIZE - vPoints; // vertical points
    heightSpread = vidWandH[1] / vPoints; // neumber of pixels between each points
    widthMargin = widthSpread / 2; // minimum number of pixels from the left
    heightMargin = heightSpread / 2; // minimum number of pixels from the top
    // number of skiped pixels before starting a new line
    widthRemainder = vidWandH[0] - (vidWandH[0] % hPoints) + widthMargin;
    heightRemainder = vidWandH[1] - (vidWandH[1] % vPoints) + heightMargin;
    halfWandH[0] = vidWandH[0] / 2;
    halfWandH[1] = vidWandH[1] / 2;
}

void ossiaVid::processPix(const ofPixels& px, ofParameter<float> *pv
                          #ifdef CV
                          , const ofPixels& cvPx
                          #endif
                              )
{
    widthSpread = vidWandH[0] / hPoints; // horizontal points
    verticalStep = MATRIX_SIZE - vPoints; // vertical points
    heightSpread = vidWandH[1] / vPoints; // neumber of pixels between each points
    widthMargin = widthSpread / 2; // minimum number of pixels from the left
    heightMargin = heightSpread / 2; // minimum number of pixels from the top
    // number of skiped pixels before starting a new line
    widthRemainder = vidWandH[0] - (vidWandH[0] % hPoints) + widthMargin;
    heightRemainder = vidWandH[1] - (vidWandH[1] % vPoints) + heightMargin;

    ofVec4f midColor{0, 0, 0, 0};
    float lightSum{0};
    ofVec2f baryCenter{0, 0};
    unsigned int iter{0};

    for (size_t i = widthMargin; i < widthRemainder; i+= widthSpread)
    {
        for (size_t j = heightMargin; j < heightRemainder; j+= heightSpread)
        {
            ofColor pxColor = px.getColor(i, j);
            midColor[0] += pxColor.a;
            midColor[1] += pxColor.r;
            midColor[2] += pxColor.g;
            midColor[3] += pxColor.b;
            iter++;

            float focus{0};

#ifdef CV
            pxColor = cvPx.getColor(i, j);
#endif
            if (invert) focus = 255 - pxColor.getLightness();
            else focus = pxColor.getLightness();

            if (focus >= threshold)
            {
                pv->set(focus);

                baryCenter += ofVec2f(i, j) * focus;
                lightSum += focus;

            } else {
                pv->set(0.f);
            }

            pv++;
        }
        pv+= verticalStep;
    }

    midColor /= iter;
    averageColor.set(midColor); // average_color

    if (lightSum != 0)
    {
        baryCenter /= lightSum;
        // to be drawn
        bary[0] = baryCenter[0];
        bary[1] = baryCenter[1];
        // to send as values
        baryCenter[0] /= halfWandH[0];
        baryCenter[0] -= 1;
        baryCenter[1] /= halfWandH[1];
        baryCenter[1] -= 1;

        centroid.set(ofVec3f(baryCenter[0], baryCenter[1], lightSum / iter)); // centroid
    }
}

void ossiaVid::drawPix(ofParameter<float>* pv)
{
    for (size_t i = widthMargin; i < widthRemainder; i+= widthSpread)
    {
        for (size_t j = heightMargin; j < heightRemainder; j+= heightSpread)
        {
            if (drawCircles && (*pv >= threshold)) // draw_matrix
            {
                ofDrawCircle(canv.x + i * size,
                             canv.y + j * size,
                             canv.z,
                             circleSize * *pv); // cicle size
            }
            pv++;
        }
        pv+= verticalStep;
    }
}

//--------------------------------------------------------------
#ifdef CV
void ossiaCv::cvSetup(const unsigned int* wAndH, ofParameterGroup& group)
{
    cvControl.setName("cv");
    cvControl.add(maxThreshold.set("threshold", 64));
    cvControl.add(holdBackGround.set("hold_background", false));
    holdBackGround.addListener(this, &ossiaCv::setBackGround);
    cvControl.add(drawCvImage.set("draw_grayscale", false));

    // blobs
    blobs.setName("blobs");
    blobs.add(getContours.set("get_contours", false));
    blobs.add(minArea.set("min_area", 20));
    blobs.add(maxArea.set("max_area", (wAndH[0] * wAndH[1]) / 3));
    blobs.add(drawContours.set("draw_contours", false));

    for (int i = 0; i < 5; i++)
    {
        blobs.add(position[i].set("position_" + to_string(i+1),
                                  ofVec3f(0, 0, 0),
                                  ofVec3f(-1, -1, -1),
                                  ofVec3f(1, 1, 1)));
        blobs.add(area[i].set("size_" + to_string(i+1), 0));
    }

    cvControl.add(blobs);

    colorImg.allocate(wAndH[0], wAndH[1]);
    grayImage.allocate(wAndH[0], wAndH[1]);

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
    cvControl.add(drawNetMesh.set("draw_mesh", false));
#endif

    group.add(cvControl);
}

void ossiaCv::cvUpdate(ofPixels& pixels, const unsigned int* wAndH, const unsigned int& wArea)
{
#ifdef KINECT
    grayImage.setFromPixels(pixels);
#else
    colorImg.setFromPixels(pixels);
    grayImage = colorImg;
#endif

    // take the abs value of the difference between background and incoming and then threshold:
    if (holdBackGround) grayImage.absDiff(grayBg, grayImage);
    grayImage.threshold(maxThreshold);

    if (getContours)
    {
        contourFinder.findContours(grayImage, minArea, maxArea, 5, false);

        int i{0};

#ifdef NET
        writeBlobs(clintConnt, contourFinder);
        if (recv(clintConnt, &dataSending, 0, MSG_DONTWAIT) == 0)
            meshToDraw = readBlobs(clintConnt, wAndH);
#endif

        for (const ofxCvBlob& b : contourFinder.blobs)
        {
            position[i].set(ofVec3f{(b.centroid.x / wAndH[0]) - 1,
                                      (b.centroid.y / wAndH[1]) - 1,
                                  b.centroid.z});
            area[i].set(b.area / wArea);

            i++;
        }       
    }
}

#ifdef NET
void ossiaCv::writeInt(int fd, int x)
{
    unsigned int hx{htonl(x)};
    int nbW = write(fd, (char *) &hx, 4);

    if (nbW < 4) cout << "probleme!";
}

void ossiaCv::writeFloat(int fd, float x)
{
    float * gee = &x;
    int * buz = (int *) gee;
    * buz = htonl(* buz);

    write(fd, (char *) &x, 4);
}

int writeN = 0;

void ossiaCv::writeContour(int fd, ofxCvBlob blob)
{
    writeInt(fd, blob.nPts);

    for (const glm::vec3& p : blob.pts)
    {
        float x = (p.x / (ofGetWidth() * 0.5)) -1;
        float y = (p.y / (ofGetHeight() * 0.5)) -1;

        writeFloat(fd, x);
        writeFloat(fd, y);
    }

    writeInt(fd, 0);
}

void ossiaCv::writeBlobs(int fd, ofxCvContourFinder &cntr)
{
    writeInt(fd, cntr.nBlobs);

    for (const ofxCvBlob& b : cntr.blobs)
    {
        writeContour(fd, b);
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

int ossiaCv::readInt(int fd)
{
    int hx{0};
    readBuffer(fd, (char *) &hx, 4);

    return ntohl(hx);
}

float ossiaCv::readFloat(int fd)
{
    float hx{0};
    readBuffer(fd, (char *) &hx, 4);

    int gee = (ntohl(*((int *) &hx)));

    float buz = *((float *) &gee);

    return buz;
}

vector<ofPoint> ossiaCv::readContour(int fd, const unsigned int* wh)
{
    vector<ofPoint> p;
    int n{readInt(fd)};

    //cout << "contour " << n << '\n';

    for (int i = 0; i < n; i++)
    {
        float x = readFloat(fd);
        float y = readFloat(fd);
        p.push_back(ofPoint(x,y));
    }

    readBlobs(fd, wh);

    return p;
}

ofMesh ossiaCv::readBlobs(int fd, const unsigned int* wh)
{
    ofMesh m;
    m.setMode(OF_PRIMITIVE_POINTS);

    int n{readInt(fd)};

    unsigned int halfW = wh[0] / 2;
    unsigned int halfH = wh[1] / 2;

    for (int i = 0; i < n; i++)
    {
        vector<ofPoint> vert{readContour(fd, wh)};

        for (const ofPoint p : vert)
        {
            ofPoint pn = (p + 1);
            pn.x = pn.x * wh[0] + halfW;
            pn.y = pn.y * wh[1] + halfH;
            //cout << pn.x << '\n';
            m.addVertex(pn);
            m.addColor(ofColor(255, 255, 255, 255));
        }
    }

    return m;
}
#endif

void ossiaCv::setBackGround(bool& hold)
{
    if (hold) grayBg = grayImage;
    else grayBg.clear();
}

void ossiaCv::cvdraw(const ossiaVid::canvas& cnv)
{
    if (drawCvImage)
        grayImage.draw(cnv.x,
                       cnv.y,
                       cnv.w,
                       cnv.h);

    if (drawContours)
    contourFinder.draw(cnv.x,
                       cnv.y,
                       cnv.w,
                       cnv.h);

    if (drawNetMesh)
    {
        glPointSize(3);
        ofPushMatrix();
        // the projected points are 'upside down' and 'backwards'
        //ofScale(1, -1, -1);
        //ofTranslate(0, 0, -1000); // center the points a bit
        //ofEnableDepthTest();
        meshToDraw.drawVertices();
        //ofDisableDepthTest();
        ofPopMatrix();
    }
}
#endif

//--------------------------------------------------------------
ossiaPlayer::ossiaPlayer(string file)
    :path{file}
{
}

void ossiaPlayer::setup()
{
    vid.load(path);           // initialize video path
    vid.setLoopState(OF_LOOP_NONE);
    vid.play();
    vid.setPaused(true);

    params.setName(ofFile(path).getBaseName());    // set parameters

    play.addListener(this, &ossiaPlayer::setPlay);
    params.add(play.set("play", false));

    loop.addListener(this, &ossiaPlayer::setLoop);
    params.add(loop.set("loop", false));

    params.add(seek.set("seek", 0., 0., 1.));
    seek.addListener(this, &ossiaPlayer::setFrame);

    params.add(volume.set("volume", 1., 0., 1.));
    volume.addListener(this, &ossiaPlayer::setVolume);

    vidWandH[0] = vid.getWidth(); // get the video's original size
    vidWandH[1] = vid.getHeight();

#ifdef CV
    vidArea = vidWandH[0] * vidWandH[1];
    cvSetup(vidWandH, params);
#endif

    setMatrix(params);

    canv.corner2center(vidWandH, size, placement);
}

void ossiaPlayer::update()
{
    if (play) vid.update();
    if (getPixels) processPix(vid.getPixels(), pixVal
                          #ifdef CV
                          , grayImage.getPixels()
                          #endif
                              );

#ifdef CV
    if (vid.isFrameNew()) cvUpdate(vid.getPixels(), halfWandH, vidArea);
#endif
}

void ossiaPlayer::setPlay(bool &toPlay)
{
    vid.setPaused(!toPlay);
}

void ossiaPlayer::setLoop(bool &toLoop)
{
    if (toLoop)
    {
        vid.setLoopState(OF_LOOP_NORMAL);
        if (vid.getIsMovieDone()) vid.setPosition(0);
    } else
    {
        vid.setLoopState(OF_LOOP_NONE);
    }
}

void ossiaPlayer::setFrame(float &toSeek)
{
    vid.setPosition(toSeek);
}

void ossiaPlayer::setVolume(float &toAmp)
{
    vid.setVolume(toAmp);
}

void ossiaPlayer::draw()
{
    if (drawVid || getPixels) checkResize();

    if (drawVid)
    {
        ofSetColor(color->y,
                   color->z,
                   color->w,
                   color->x);

        vid.getTexture().draw(canv.x, // getTexture allows the use of the Z axis
                canv.y,
                canv.z,
                canv.w,
                canv.h);
    }

    ofSetCircleResolution(circleResolution);

    ofSetColor(circleColor->y,
               circleColor->z,
               circleColor->w,
               circleColor->x);

    if (drawCircles) drawPix(pixVal);

    if (drawCenter) // draw_barycenter
    {
        ofDrawCircle((canv.x + bary[0]) * size,
                     (canv.y + bary[1]) * size,
                     canv.z,
                     circleSize * size * centroid->z); // cicle size
    }
}

void ossiaPlayer::close()
{
    vid.stop();
    vid.closeMovie();
}

//--------------------------------------------------------------
ossiaGrabber::ossiaGrabber(ofVideoDevice dev)
    :device{dev}
{
}

void ossiaGrabber::setup(unsigned int width, unsigned int height)
{
    vid.setDeviceID(device.id);
    vid.initGrabber(width, height);

    params.setName(device.deviceName);    // set parameters
    params.add(freeze.set("freeze", false));

    vidWandH[0] = vid.getWidth();
    vidWandH[1] = vid.getHeight();

#ifdef CV // only needed for cvUpdate
    vidArea = vidWandH[0] * vidWandH[1];
    cvSetup(vidWandH, params);
#endif

    setMatrix(params);

    canv.corner2center(vidWandH, size, placement);
}

void ossiaGrabber::update()
{
    if (!freeze) vid.update();
    if (getPixels) processPix(vid.getPixels(), pixVal
                          #ifdef CV
                          , grayImage.getPixels()
                          #endif
                              );

#ifdef CV
    if (vid.isFrameNew()) cvUpdate(vid.getPixels(), halfWandH, vidArea);
#endif
}

void ossiaGrabber::draw()
{
#ifdef CV
    cvdraw(canv);
#endif

    if (drawVid || getPixels) checkResize();

    if (drawVid)
    {
        ofSetColor(color->y,
                   color->z,
                   color->w,
                   color->x);

        vid.getTexture().draw(canv.x, // getTexture allows the use of the Z axis
                canv.y,
                canv.z,
                canv.w,
                canv.h);
    }

    ofSetCircleResolution(circleResolution);

    ofSetColor(circleColor->y,
               circleColor->z,
               circleColor->w,
               circleColor->x);

    if (drawCircles) drawPix(pixVal);

    if (drawCenter) // draw_barycenter
    {
        ofDrawCircle((canv.x + bary[0]) * size,
                     (canv.y + bary[1]) * size,
                     canv.z,
                     circleSize * size * centroid->z); // cicle size
    }
}

void ossiaGrabber::close()
{
    vid.close();
}

//--------------------------------------------------------------
#ifdef KINECT
ossiaKinect::ossiaKinect(int dev)
    :device{dev}
{
}

void ossiaKinect::setup(bool infrared)
{
    // enable depth->video image calibration
    vid.setRegistration(true);

    vid.init(infrared);
    vid.open(device);

    params.setName(to_string(device));    // set parameters
    params.add(freeze.set("freeze", false));
    params.add(angle.set("angle", 0, -30, 30));
    angle.addListener(this, &ossiaKinect::tilt);

    vidWandH[0] = vid.getWidth();
    vidWandH[1] = vid.getHeight();

#ifdef CV // only needed for cvUpdate
    vidArea = vidWandH[0] * vidWandH[1];
    cvSetup(vidWandH, params);
#endif

    setMatrix(params);

    canv.corner2center(vidWandH, size, placement);

    // print the intrinsic IR sensor values
    if(vid.isConnected()) {
        cout << "kinect " << device <<" sensor-emitter dist: " << vid.getSensorEmitterDistance() << "cm\n"
        << "kinect " << device <<" sensor-camera dist:  " << vid.getSensorCameraDistance() << "cm\n"
        << "kinect " << device <<" zero plane pixel size: " << vid.getZeroPlanePixelSize() << "mm\n"
        << "kinect " << device <<" zero plane dist: " << vid.getZeroPlaneDistance() << "mm\n";
    }
}

void ossiaKinect::tilt(float &angle)
{
    vid.setCameraTiltAngle(angle);
}

void ossiaKinect::update()
{
    if (!freeze) vid.update();
    if (getPixels) processPix(vid.getPixels(), pixVal
                          #ifdef CV
                          , grayImage.getPixels()
                          #endif
                              );

#ifdef CV
    if (vid.isFrameNew()) cvUpdate(vid.getDepthPixels(), halfWandH, vidArea);
#endif

}

void ossiaKinect::draw()
{
#ifdef CV
    cvdraw(canv);
#endif

    if (drawVid || getPixels) checkResize();

    if (drawVid)
    {
        ofSetColor(color->y,
                   color->z,
                   color->w,
                   color->x);

        vid.getTexture().draw(canv.x, // getTexture allows the use of the Z axis
                canv.y,
                canv.z,
                canv.w,
                canv.h);
    }

    ofSetCircleResolution(circleResolution);

    ofSetColor(circleColor->y,
               circleColor->z,
               circleColor->w,
               circleColor->x);

    if (drawCircles) drawPix(pixVal);

    if (drawCenter) // draw_barycenter
    {
        ofDrawCircle((canv.x + bary[0]) * size,
                     (canv.y + bary[1]) * size,
                     canv.z,
                     circleSize * size * centroid->z); // cicle size
    }
}

void ossiaKinect::close()
{
    vid.close();
}
#endif
