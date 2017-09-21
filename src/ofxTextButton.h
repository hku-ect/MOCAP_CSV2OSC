//
//  ofxTextButton.h
//  NatNet2OSCbridge
//
//  Created by Enrico Becker on 03.12.15.
//
//

#ifndef __NatNet2OSCbridge__ofxTextButton__
#define __NatNet2OSCbridge__ofxTextButton__

#include <stdio.h>
#include "ofMain.h"

class ofxTextButton
{
public:
    ofxTextButton();
    ~ofxTextButton();
    
    void setup(ofRectangle area,string description,int tSize,ofColor tC,ofColor bC);
    bool isInside(int &x,int &y);
    void draw();
    
    ofVec2f textPosition();
    
private:
    ofRectangle     area;
    string          text;
    ofTrueTypeFont  font;
    ofColor         tCol;
    ofColor         bCol;
    ofVec2f         tPos;
    int             tSize;
    bool            clicked;
    int             clickCount;
};

#endif /* defined(__NatNet2OSCbridge__ofxTextButton__) */
