//
//  marker.h
//  CSVtoOSC
//
//  Created by Machiel Veltkamp on 19/05/17.
//  A class to describe a a marker (either a rigid body or as part of a skeleton)
//

#include "ofxOsc.h"

#ifndef marker_h
#define marker_h

class MOCAP_Marker{
    
public:
    MOCAP_Marker(){}
    
    // set marker indentity
    void setMarker(int _id, string _name, int _startCollumn){
        id = _id;                       // id of marker
        name = _name;                   // name of marker
        startCollumn = _startCollumn;   // where in the fiel does the marker start
    }
    
    // add marker entry
    // base don split or filterd data
    void addMarkerEntry(int frame, float theTime, ofVec3f position, ofVec4f rotation){
        // add frame number
        frames.push_back(frame);
        // add timestamp
        time.push_back(theTime);
        // add the position for this frame/timestamp
        positions.push_back(position);
        // add the rotation for this frame/timestamp
        rotations.push_back(rotation);
    }
    
    // add marker entry
    // based on the raw data from the CSV file
    void addMarkerEntry(vector<string> data){
        // add frame number
        frames.push_back(ofToInt(data[frameNumID]));
         // add timestamp
        time.push_back(ofToFloat(data[timeNumID]));
        
        // create quarternion for rotation
        ofQuaternion rotation(ofToFloat(data[startCollumn + startRot]),ofToFloat(data[ startCollumn + startRot+1]),ofToFloat(data[startCollumn + startRot + 2]),ofToFloat(data[startCollumn + startRot+3]));
        // create ofVec3f for position
        ofVec3f position(ofToFloat(data[startCollumn + startPOS]),ofToFloat(data[startCollumn + startPOS +1]),ofToFloat(data[startCollumn + startPOS + 2]));
        
        // add the position for this frame/timestamp
        positions.push_back(position);
        // add the rotation for this frame/timestamp
        rotations.push_back(rotation);
        
        isActive.push_back(1);
    }
    
    // send to the console the data for a specifick frame
    void displayData(int frame){
        ofLogVerbose("Marker INFO: name: "+name+" position: "+ofToString(positions[frame].x)+","+ofToString(positions[frame].y)+","+ofToString(positions[frame].z));
    }
    
    // send to the console general data of th emarker
    void displayInfo(){
        ofLogVerbose("Marker INFO: name: "+name+" id: "+ofToString(id)+" startCollumn: "+ofToString(startCollumn));
    }
    
    // Get data for specifick frame and create an OSC message for it
    void getOSCData(int frame, ofxOscMessage *m, bool notPartSkeleton, string prefix = "" ){
        
        if(notPartSkeleton){
            m->setAddress("/rigidBody");
            m->addIntArg(id);
            m->addStringArg(name);
        }
        else {
            m->addStringArg(prefix + name);
        }
        
        // add position
        ofVec3f position = positions[frame]; // FIXME: better to do with map and lookup?
        m->addFloatArg(position.x);
        m->addFloatArg(position.y);
        m->addFloatArg(position.z);
    
        // add rotation
        ofQuaternion rotation = rotations[frame]; // FIXME: better to do with map and lookup?
        m->addFloatArg(rotation.x());
        m->addFloatArg(rotation.y());
        m->addFloatArg(rotation.z());
        m->addFloatArg(rotation.w());
        
        ///skeleton specific
        if ( notPartSkeleton ) {
            // add position speed
            // TODO: position speed (currently 0)
            m->addFloatArg(0);
            m->addFloatArg(0);
            m->addFloatArg(0);
            
            // add rotation speed
            // TODO: add rotation speed (currently 0)
            m->addFloatArg(0);
            m->addFloatArg(0);
            m->addFloatArg(0);
            
            // add is active
            // TODO: add is active (currently always active)
            m->addIntArg(1);
        }
        else {
            // add parent bone index
            // TODO: parent id
            
            // add parent offset (xyz)
            // TODO: add parent offset
        }
    }
    
    // return the collumn in the file where the data starts
    int getStartCollumn(){
        return startCollumn;
    }

    // return the id of the marker
    int getID(){
        return id;
    }

    // return the name of the marker
    string getName(){
        return name;
    }
    
    ofVec3f getPosition(int frame) {
        ofVec3f position = positions[frame];
        return position;
        
    }
    
    ofQuaternion getRotation(int frame) {
        ofQuaternion rotation = rotations[frame];
        return rotation;
        
    }
    
    
    
protected:
    int id;
    string name;
    int startCollumn;
    std::vector<int> frames;
    std::vector<float> time;
    std::vector<ofVec3f> positions;
    std::vector<ofQuaternion> rotations;
    std::vector<int> isActive;
    
    // Definition of the OPTITRACK MOCAP RIGID BODIE Data string
    // defines how the data structure in the CSV file is written
    
    //global for all frames
    int frameNumID = 0;
    int timeNumID = 1;
    
    //relative to startColumn
    int startPOS = 4;
    int lenghtPOS = 3;
    int startRot = 0;
    int lengthRot = 4;
    
};


#endif /* marker_h */
