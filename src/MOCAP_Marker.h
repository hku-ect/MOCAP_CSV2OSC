//
//  marker.h
//  CSVtoOSC
//
//  Created by Machiel Veltkamp on 19/05/17.
//  A class to describe a a marker (either a rigid body or as part of a skeleton)
//

#include "ofxOsc.h"
#include "RigidBodyHistory.h"
#include "ofMain.h"

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
        invFPS = 1.0f / ofGetFrameRate();
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
    void getOSCData(int frame, ofxOscMessage *m, bool hierarchy, bool notPartSkeleton, string prefix = "" ){
        
        // RIGID BODY
        if(notPartSkeleton){
            if(hierarchy==true){ // USE HIEARCHY MODE (for example for Isadora)
                m->setAddress("/rigidbody/"+name);
            }
            else{ // USE NON-HIEARCHY MODE (for example for Processing or OF)
                m->setAddress("/rigidBody");
            }
            m->addIntArg(id);
            m->addStringArg(name);
        }
        // SKELETON BONE
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
        
        // With rigidbodie we also sent position speed and rotation speed
        
        if ( notPartSkeleton ) {
            
            //velocity over SMOOTHING * 2 + 1 frames
            m->addFloatArg(currentVelocity.x);
            m->addFloatArg(currentVelocity.y);
            m->addFloatArg(currentVelocity.z);
            //angular velocity (euler), also smoothed
            m->addFloatArg(currentAngluarVelocity.x);
            m->addFloatArg(currentAngluarVelocity.y);
            m->addFloatArg(currentAngluarVelocity.z);
            
            // add is active
            // TODO: add is active (currently always active)
            m->addIntArg(1);
        }
    }
    
    
    void calculateSpeed(int frame){
        
        ofVec3f position = positions[frame];
        ofQuaternion rotation = rotations[frame];
        
        //we're going to fetch or create this
        RigidBodyHistory *rb;
        
        //Get or create rigidbodyhistory
        bool found = false;
        // Retrieve stored rigidbody
        for( int r = 0; r < rbHistory.size(); ++r )
        {
            if ( rbHistory[r].rigidBodyId == id )
            {
                rb = &rbHistory[r];
                found = true;
            }
        }
        
        // Add new rigidbody to history
        if ( !found )
        {
            rb = new RigidBodyHistory( id, position, rotation );
            rbHistory.push_back(*rb);
            
        }
        
        ofVec3f velocity;
        ofVec3f angularVelocity;
        
        // First run for
        if ( rb->firstRun == TRUE )
        {
            rb->currentDataPoint = 0;
            rb->firstRun = FALSE;
        }
        else
        {
            // SAVE data point
            if ( rb->currentDataPoint < 2 * SMOOTHING + 1 )
            {
                rb->velocities[rb->currentDataPoint] = ( position - rb->previousPosition ) * invFPS;
                
                ofVec3f diff = ( rb->previousOrientation * rotation.inverse() ).getEuler();
                rb->angularVelocities[rb->currentDataPoint] = ( diff * invFPS );
                
                rb->currentDataPoint++;
            }
            else
            {
                int count = 0;
                int maxDist = SMOOTHING;
                ofVec3f totalVelocity;
                ofVec3f totalAngularVelocity;
                //calculate smoothed velocity
                for( int x = 0; x < SMOOTHING * 2 + 1; ++x )
                {
                    //calculate integer distance from "center"
                    //above - maxDist = influence of data point
                    int dist = abs( x - SMOOTHING );
                    int infl = ( maxDist - dist ) + 1;
                    
                    //add all
                    totalVelocity += rb->velocities[x] * infl;
                    totalAngularVelocity += rb->angularVelocities[x] * infl;
                    //count "influences"
                    count += infl;
                }
                
                //divide by total data point influences
                velocity = totalVelocity / count;
                angularVelocity = totalAngularVelocity / count;
                
                for( int x = 0; x < rb->currentDataPoint - 1; ++x )
                {
                    rb->velocities[x] = rb->velocities[x+1];
                    rb->angularVelocities[x] = rb->angularVelocities[x+1];
                }
                rb->velocities[rb->currentDataPoint-1] = ( position - rb->previousPosition ) * invFPS;
                
                ofVec3f diff = ( rb->previousOrientation * rotation.inverse() ).getEuler();
                rb->angularVelocities[rb->currentDataPoint-1] = ( diff * invFPS );
            }
            
            rb->previousPosition = position;
            rb->previousOrientation = rotation;
            
            float scaleFactor = 1000.0;
            
            currentVelocity         = velocity * scaleFactor;
            currentAngluarVelocity  = angularVelocity * scaleFactor;
            
            //Check for NAN
            if(isnan(currentAngluarVelocity.x)) currentAngluarVelocity.x = 0;
            if(isnan(currentAngluarVelocity.y)) currentAngluarVelocity.y = 0;
            if(isnan(currentAngluarVelocity.z)) currentAngluarVelocity.z = 0;

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
    
    // For speed calculation
    vector<RigidBodyHistory>    rbHistory;
    float               invFPS;
    ofVec3f currentVelocity;
    ofVec3f currentAngluarVelocity;

    
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
