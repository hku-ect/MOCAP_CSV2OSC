#pragma once

#include "ofMain.h"
#include "ofxOsc.h"
#include "CSVloader.h" // include our ThreadedObject class.
#include "MOCAP_Marker.h"
#include "MOCAP_Skeleton.h"
#include "ofxTextButton.h"
#include "ofxTextInputField.h"
#include "ofxXmlSettings.h"
#include "client.h"

#define HOST "192.168.0.105"
#define PORT 1234

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();
    
       void loadJSONfile(ofFileDialogResult openFileResult);

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
    
        void loadAFile();
        void addClient(int i,string ip,int p,string n,bool r,bool m,bool s,bool live, bool hierarchy, ClientMode mode);
        void deleteClient(int &index);
        void saveData();
        void setupData();
        void setFPS();
        void doFrame();
    
        void deactivateInputs();
    
        CSVloader csvloader;
        /// \brief A local count that is only accessed in the main thread
        int mainAppsCount;
    
        vector<client*>             clients;
    
        bool dataLoaded;
        int frameNum;
        float fFrameNum;
        int totalFrames;
        int frameRate;
        float frameTime;
        float fTimeCounter;
    
        std::map<string,MOCAP_Marker> rigidbodies;
        std::map<string,MOCAP_Skeleton> skeletons;
        ofxOscSender sender;
    
        // interface
        int                 InterfaceX;
        int                 InterfaceY;
    
        ofTrueTypeFont      font;
        ofxTextButton       loadFileBTN;
        ofxTextButton       addBTN;
        ofxTextButton       saveBTN;
        ofxTextButton       setFPSBTN;
        ofxTextButton       playpauseBTN;
        ofxTextButton       rewindBTN;

    
        ofxTextInputField   newName;
        ofxTextInputField   newIP;
        ofxTextInputField   newPort;
        ofxTextInputField   fps;

		
};
