//
//  client.cpp
//  NatNet2OSCbridge
//
//  Created by Enrico Becker on 12.10.15.
//
//

#include "client.h"

client::client(int ind,string i,int p,string n,bool r,bool m,bool s, bool hier)
{
    //arange them gridwise
    index = ind;
    ip = i;
    port = p;
    name = n;
    isRigid = r;
    isMarker = m;
    isSkeleton = s;
    deepHierarchy = hier;
    notWholeScreen = true;
    
    rearangePosition(ind,notWholeScreen);
    
    ofTrueTypeFont::setGlobalDpi(72);
    
    verdana14.load("verdana.ttf", 14, true, true);
    verdana14.setLineHeight(18.0f);
    verdana14.setLetterSpacing(1.037);
    setupSender();
}

client::~client(){}

void client::rearangePosition(int ind, bool _notWholeScreen)
{
    /*
     --> Get size window
     --> client width = 330
     */
    
    notWholeScreen = _notWholeScreen;
    
    int clientWidth = 330;
    int collumnSpace = ofGetWidth();
    
    // use whole screen or only screen left of interface
    if(notWholeScreen) collumnSpace = ofGetWidth() - 350; // 350 is width of connection interface
    
    int numCollumns = floor(collumnSpace/clientWidth);
    // I may never be 0 ..
    if(numCollumns < 1) numCollumns = 1;
    
    index = ind;
    int width = 30;
    int x = 20 + 340 * (index%numCollumns);
    int row = (index / numCollumns);
    int height = int(width * 2.1);
    area = ofRectangle(x, 20 + (row * height + (row * 10)), 330, height);
    rigButton = ofRectangle(0, width, width, width);
    markButton = ofRectangle(70, width, width, width);
    skelButton = ofRectangle(140, width, width, width);
    hierarchyButton = ofRectangle(300, width, width, width);
    delButton = ofRectangle(area.width - (width / 2), 0, width / 2, width / 2);
    
    //todo: add some kind of "mode" setup for gears, or which things we want to send or not send?
    modeButton = ofRectangle(0, width*2+5, 200, width-10);
}


void client::setupSender()
{
    sender.setup(ip, port);
}

void client::sendData(ofxOscMessage &m)
{
    sender.sendMessage(m);
}

void client::sendBundle(ofxOscBundle &b)
{
    sender.sendBundle(b);
}

void client::draw()
{
    ofSetLineWidth(1);
    ofPushMatrix();
    ofTranslate(area.getX(),area.getY());
    drawGUI();
    string msg;
    msg += name + " ip " + ofToString(ip);
    msg += " : " + ofToString(port);

    ofSetColor(255,255,255);
    verdana14.drawString(msg, 20, 20);
    //verdana14.drawString("Rigid", 30, 50);
    //verdana14.drawString("Mark", 100, 50);
    //verdana14.drawString("Skel", 170, 50);
    verdana14.drawString("Hierarchy", 230, 50);
    
    ofPopMatrix();
}

void client::drawGUI()
{
    ofNoFill();
    ofSetColor(255, 255, 255);
    ofDrawRectangle(0,0,area.getWidth(),area.getHeight());
    ofFill();
    
    /*
    if (!isRigid) ofSetColor(255, 0, 0);
    else ofSetColor(0,255,0);
    ofDrawRectangle(rigButton);
    
    if (!isMarker) ofSetColor(255, 0, 0);
    else ofSetColor(0,255,0);
    ofDrawRectangle(markButton);

    if (!isSkeleton) ofSetColor(255, 0, 0);
    else ofSetColor(0,255,0);
    ofDrawRectangle(skelButton);
    */
    
    if (!deepHierarchy) ofSetColor(255, 0, 0);
    else ofSetColor(0,255,0);
    ofDrawRectangle(hierarchyButton);
    
    ofSetColor(127,127,127);
    //ofDrawRectangle(modeButton);
    
    ofSetColor(255, 255, 255);
    ofDrawRectangle(delButton);
    ofSetColor(0, 0, 0);
    ofDrawLine(delButton.x, delButton.y, delButton.x + delButton.width, delButton.y + delButton.height);
    ofDrawLine(delButton.x + delButton.width, delButton.y, delButton.x, delButton.y + delButton.height);
}

void client::isInside(int & xp, int & yp)
{
    int x = xp - area.getX();
    int y = yp - area.getY();
    
    bool rig = rigButton.inside(x, y);
    bool mark = markButton.inside(x, y);
    bool skel = skelButton.inside(x, y);
    bool hierarchy = hierarchyButton.inside(x, y);
    bool del = delButton.inside(x, y);

    if (rig) isRigid = !isRigid;
    
    else if (mark) isMarker = !isMarker;
    
    else if (skel) isSkeleton = !isSkeleton;
    
    else if (hierarchy) deepHierarchy = !deepHierarchy;
    
    else if (del) ofNotifyEvent(deleteClient,index);
    
}

string &client::getName()
{
    return name;
}


ofRectangle &client::getArea()
{
    return area;
}

int &client::getID()
{
    return index;
}


string &client::getIP()
{
    return ip;
}

int &client::getPort()
{
    return port;
}

void client::setRigid(bool value)
{
    isRigid = value;
}

void client::setMarker(bool value)
{
    isMarker = value;
}

void client::setSkeleton(bool value)
{
    isSkeleton = value;
}

bool &client::getRigid()
{
    return isRigid;
}

bool &client::getMarker()
{
    return isMarker;
}

bool &client::getSkeleton()
{
    return isSkeleton;
}

bool &client::getHierarchy()
{
    return deepHierarchy;
}

