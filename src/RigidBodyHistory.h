//
//  RigidBodyHistory.h
//  MOCAP_CSV2OSC
//
//  Created by Machiel Veltkamp on 01/03/18.
//
//

#include "ofMain.h"

#ifndef RigidBodyHistory_h
#define RigidBodyHistory_h

//for velocity, defines how many layers to apply (2 * layers + 1 frames)
#define SMOOTHING 0


class RigidBodyHistory {
public:
    int                 rigidBodyId;
    ofVec3f             velocities[2 * SMOOTHING + 1];
    ofVec3f             angularVelocities[2 * SMOOTHING + 1];
    int                 currentDataPoint;
    ofVec3f             previousPosition;
    ofQuaternion        previousOrientation;
    bool                firstRun;
    int                 framesInactive;
    
    RigidBodyHistory( int rigidBodyId, ofVec3f position, ofQuaternion eulerOrientation ){}
};



#endif /* RigidBodyHistory_h */
