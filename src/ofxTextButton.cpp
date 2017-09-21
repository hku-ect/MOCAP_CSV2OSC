//
//  ofxTextButton.cpp
//  NatNet2OSCbridge
//
//  Created by Enrico Becker on 03.12.15.
//
//

#include "ofxTextButton.h"

ofxTextButton::ofxTextButton(){}

ofxTextButton::~ofxTextButton(){}

void ofxTextButton::setup(ofRectangle a,string description,int tS,ofColor tC,ofColor bC)
{
    area = a;
    text = description;
    tSize = tS;
    font.load("verdana.ttf", tSize);
    tCol = tC;
    bCol = bC;
    tPos = textPosition();
    clicked = false;
}

bool ofxTextButton::isInside(int &x,int &y)
{
    if (area.inside(x, y))
    {
        clicked = true;
        clickCount = 0;
        return true;
    }
    return false;
}

void ofxTextButton::draw()
{
    ofFill();
    ofSetColor(bCol);
    if (clicked)
    {
        ofSetColor(255,0,0);
        clickCount++;
        if (clickCount > 5) clicked = false;
    }
    ofDrawRectangle(area);
    ofSetColor(tCol);
    font.drawString(text, tPos.x,tPos.y);
}


ofVec2f ofxTextButton::textPosition()
{
    ofRectangle bbox = font.getStringBoundingBox(text, 0, 0);
    float x = ((area.width - bbox.width) / 2) + area.x;
    float y = (area.y + area.height) - ((area.height - tSize) / 2);
    return ofVec2f(x,y);
}
