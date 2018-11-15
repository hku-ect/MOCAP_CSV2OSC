#pragma once

#include "ofMain.h"
#include "ofxImGui.h"
#include "ofxOsc.h"
#include "CSVloader.h" // include our ThreadedObject class.
#include "MOCAP_Marker.h"
#include "MOCAP_Skeleton.h"
#include "ofxXmlSettings.h"
#include "client.h"
#include "uiWidgets.h"

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
        void addClient(int i,string ip,int p,string n,bool r,bool m,bool s,bool live, bool hierarchy);
        void deleteClient(int &index);
        void saveData();
        void setupData();
        void setFPS();
        void doFrame();
        void printOSCmessage(ofxOscMessage m);
    
        CSVloader csvloader;
        /// \brief A local count that is only accessed in the main thread
        int mainAppsCount;
    
        vector<client*>             clients;
        
    
        bool dataLoaded;
        int frameNum;
        float fFrameNum;
        int totalFrames = 1;
        int frameRate;
        float frameTime;
        float fTimeCounter;
    
        std::map<string,MOCAP_Marker> rigidbodies;
        std::map<string,MOCAP_Skeleton> skeletons;
        ofxOscSender sender;
    
        //GUI
        ofxImGui::Gui gui;
        bool guiVisible;
        bool mouseOverGui;
        void doGui();
        uiLogger uiLogWidget;
};
