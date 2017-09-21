//
//  ofxTextInputField.h
//  textInput
//
//  Created by Enrico Becker on 30.11.15.
//
//

#ifndef __textInput__ofxTextInputField__
#define __textInput__ofxTextInputField__

#include <stdio.h>
#include "ofMain.h"

class ofxTextInputField
{
public:
    ofxTextInputField();
    ~ofxTextInputField();
    
    void setup(ofRectangle area, int fSize, string text, string description);
    void addKey(int key);
    void draw();
    bool isInside(int &x, int &y);
    void activate();
    void deactivate();
    void setBounds(ofRectangle area);
    void setText(string text);
    void setColor(ofColor c);
    int calCPos(int &x);
    
    void calculateDrawIndices();
    string getText();
    bool & getState();
    
private:
    ofRectangle     bounds;
    ofVec2f         tPos;
    int             fSize;
    ofColor         col;
    bool            active;
    string          text;
    string          description;
    ofTrueTypeFont  font;
    int             cIndex;           //cursor index
    int             firstDrawIndex;
    int             lastDrawIndex;
};
#endif /* defined(__textInput__ofxTextInputField__) */
