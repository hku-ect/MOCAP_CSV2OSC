//
//  MOCAP_Skeleton.h
//  CSVtoOSC
//
//  Created by Machiel Veltkamp on 19/05/17.
//
//

#ifndef MOCAP_Skeleton_h
#define MOCAP_Skeleton_h

#include "client.h"




class MOCAP_Skeleton{
public:
    MOCAP_Skeleton(){
        // create skeleton
    }
    
    // set skeleton indentity
    void setSkeleton(int _id, string _name, int _startCollumn){
        id = _id;                       // set id of skeleotn
        name = _name;                   // set name of skeleton
        startCollumn = _startCollumn;   // set where in the fiel de data of the skeleton is
        
        // Create Bones based on predefined definition
        int i = 0;
        for(const string &bonename : boneNames){
            
            cout << "bonename: " << bonename << " bone num: "<< boneNum[i] << endl;
            
            // create markers
            MOCAP_Marker m;
            // set data for the markers
            m.setMarker(0, bonename, boneNum[i]);
            // ad to the bones
            bones.push_back(m);
            i++;
        }
    }
    
    // add marker entry
    void addSkeletonEntry(vector<string> data){
        
        // FIXME: does every bone needs the id and time ?
        int frame = ofToInt(data[0]);
        float time = ofToFloat(data[1]);
        
        for( MOCAP_Marker &bone : bones){
            
            // Rotation comes first then position in CVSV file
            int startRot = startCollumn + bone.getStartCollumn();
            int startPos = startCollumn + bone.getStartCollumn() + 4;
            
            //ofLogVerbose(bone.getName()+" --> startCollumn: "+ofToString(bone.getStartCollumn())+" startRot: "+ofToString(startRot)+" startPos: "+ofToString(startPos)+" length: "+ofToString(data.size()));
            
            ofVec4f rotation(ofToFloat(data[startRot]),ofToFloat(data[startRot+1]),ofToFloat(data[startRot+2]),ofToFloat(data[startRot+3]));
            ofVec3f position(ofToFloat(data[startPos]),ofToFloat(data[startPos+1]),ofToFloat(data[startPos+2]));

            // add data for the bone
            bone.addMarkerEntry(frame,time, position, rotation);
        }
        
    }
    
    // return the name of the skeleton
    void getData(int frame){
        ofLogVerbose("Skeleton INFO: name: "+name);
    }
    
    // get the data of the skeleton for a specific frame and return as osc message
    ofxOscMessage getOSCDataForBones(int frame){
        ofxOscMessage m;
        m.setAddress("/skeleton");
        m.addStringArg(name);
        m.addIntArg(id);
        
        // loop through the bones
        for( MOCAP_Marker &bone : bones){
            bone.getOSCData(frame,&m,false,false);
        }
        
        return m;
    }
    
    // get the data of the skeleton for a specific frame and return a vector of osc messages
    // per bone ONE OSC message per skeleton a VECTOR of OSC messages
    // /skeleton/skeletonname/bone
    std::vector<ofxOscMessage> getOSCDataHierarchy(int frame){
        
        
       std::vector<ofxOscMessage> messages;
        
        // loop through the bones
        for( MOCAP_Marker &bone : bones){
            
            ofxOscMessage m;
            m.setAddress("/skeleton/"+name+"/"+bone.getName());
            // add position
            ofVec3f position = bone.getPosition(frame); // FIXME: better to do with map and lookup?
            m.addStringArg(bone.getName());
            m.addFloatArg(position.x);
            m.addFloatArg(position.y);
            m.addFloatArg(position.z);
            
            // add rotation
            ofQuaternion rotation = bone.getRotation(frame); // FIXME: better to do with map and lookup?
            m.addFloatArg(rotation.x());
            m.addFloatArg(rotation.y());
            m.addFloatArg(rotation.z());
            m.addFloatArg(rotation.w());
            
            
            // --> ADD SPEED
            // num frames used to calculate speed
            int const numFrames = 10;
            
            ofVec3f positionSpeed(0,0,0);
            ofVec3f rotationSpeed(0,0,0);
            for(int i = 0; i < numFrames; i++){
                
                positionSpeed.x += abs(bone.getPosition(frame).x - bone.getPosition(frame-1).x);
                positionSpeed.y += abs(bone.getPosition(frame).y - bone.getPosition(frame-1).y);
                positionSpeed.z += abs(bone.getPosition(frame).z - bone.getPosition(frame-1).z);
                
                ofVec3f rotNow = bone.getRotation(frame).getEuler();
                ofVec3f rotPrev = bone.getRotation(frame-1).getEuler();
                
                rotationSpeed.x += abs(rotNow.x - rotPrev.x);
                rotationSpeed.y += abs(rotNow.y - rotPrev.y);
                rotationSpeed.z += abs(rotNow.z - rotPrev.z);
                
                
            }
            
            positionSpeed = positionSpeed/numFrames;
            rotationSpeed = rotationSpeed/numFrames;
            
            
            // position speed
            m.addFloatArg(positionSpeed.x);
            m.addFloatArg(positionSpeed.y);
            m.addFloatArg(positionSpeed.z);
            
            // rotation speed
            m.addFloatArg(rotationSpeed.x);
            m.addFloatArg(rotationSpeed.y);
            m.addFloatArg(rotationSpeed.z);
            
            messages.push_back(m);

        }
        
        return messages;
    }
    
protected:
    std::vector<MOCAP_Marker> bones;
    string name;
    int id;
    int startCollumn;
    
    // OPTITRACK MOCAP SKELETON DEFINITION
    std::string boneNames[21] = {"Hip","Ab","Chest","Neck","Head","LShoulder","LUArm" ,"LFArm","LHand" ,"RShoulder","RUArm" ,"RFArm" ,"RHand" ,"LThigh","LShin","LFoot","RThigh","RShin","RFoot","LToe","RToe"};
    
    int boneNum[21] = {0,19,26,45,52,68,81,94,101,117,130,143,150,166,179,192,205,218,231,244,251};
    
    //st::vector<int> boneNum()
};


#endif /* MOCAP_Skeleton_h */
