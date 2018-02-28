#include "ofApp.h"


/*
 Name: CSVtoOSC
 Description: 
 Read CSV file with MOCAP data and play back over OSC
*/


// TODO: Set right ID's for rigidbodies and skeletons
// TODO: set looping to be an option
// TODO: use OSC bundle


const float MOTIVE_MOCAP_FPS = 120.0f;

// boolean to check if we play the data or not
bool playData = false;

//--------------------------------------------------------------
void ofApp::setup(){
    
    // set logging level determing which message will be showed
    ofSetLogLevel(OF_LOG_VERBOSE);
    
    // get a font
    font.load("verdana.ttf", 12);
    
    // when startign we do not yet have any data
    dataLoaded = false;
    
    // setup a OSC sender
    sender.setup(HOST, PORT);
    
    // SETUP GUI
    InterfaceX = ofGetWidth() - 250;
    InterfaceY = 20;
    
    loadFileBTN.setup(ofRectangle(InterfaceX, InterfaceY, 120, 20), "Load csv file", 12,ofColor(0,0,0), ofColor(255,255,255));
    
    
    // Add Client GUI
    int addClientY = 300;
    int stepY = 40;
    newName.setup(ofRectangle(InterfaceX, InterfaceY+addClientY, 140, 20), 10, "New Client","Client Name");
    newIP.setup(ofRectangle(InterfaceX, InterfaceY+addClientY+(1*stepY), 140, 20), 10, "127.0.0.1","Client IP");
    newPort.setup(ofRectangle(InterfaceX, InterfaceY+addClientY+(2*stepY), 140, 20), 10, "6200","Client Port");
    addBTN.setup(ofRectangle(InterfaceX, InterfaceY+addClientY+(3*stepY), 140, 20), "Add Client", 12,ofColor(0,0,0), ofColor(255,255,255));
    saveBTN.setup(ofRectangle(InterfaceX, InterfaceY+addClientY+(4*stepY), 120, 20), "Save Setup", 12, ofColor(0,0,0), ofColor(255,255,255));
    
    // other buttons
    playpauseBTN.setup(ofRectangle(InterfaceX, InterfaceY+160, 120, 20), "play/pause", 12, ofColor(0,0,0), ofColor(255,255,255));
    rewindBTN.setup(ofRectangle(InterfaceX+130, InterfaceY+160, 60, 20), "rewind", 12, ofColor(0,0,0), ofColor(255,255,255));
    setFPSBTN.setup(ofRectangle(InterfaceX+100, InterfaceY+220, 70, 20), "set FPS", 12, ofColor(0,0,0), ofColor(255,255,255));
    fps.setup(ofRectangle(InterfaceX, InterfaceY+220, 60, 20), 10, "30","FPS");
    
    // get data
    setupData();
    


    //default framerate
    ofSetFrameRate(60);
}

//--------------------------------------------------------------
void ofApp::setupData()
{
    ofxXmlSettings data("setup.xml");
    data.pushTag("setup",0);
    int fRate = data.getValue("fps", 30);
    //string interface = data.getValue("interface", "en0");
    //string natnetip = data.getValue("ip", "10.200.200.13");
    //interfaceName.setText(interface);
    //interfaceIP.setText(natnetip);
    fps.setText(ofToString(fRate));
    
    frameRate = fRate;
    frameTime = 1.0 / fRate;
    //ofSetFrameRate(fRate);
    data.popTag();
    
    int numClients = data.getNumTags("client");
    for (int i = 0; i < numClients; i++)
    {
        data.pushTag("client",i);
        string ip = data.getValue("ip", "127.0.0.1");
        int port = data.getValue("port", 1234);
        string name = data.getValue("name", "unknown");
        bool r = data.getValue("rigid", 0);
        bool m = data.getValue("marker", 0);
        bool s = data.getValue("skeleton", 0);
        bool live = data.getValue("live", 0);
        bool hier = data.getValue("hierarchy", 0);
        addClient(i,ip,port,name,r,m,s,live,hier);
        data.popTag();
    }
}

//--------------------------------------------------------------
void ofApp::update(){
    mainAppsCount++;
    
    // This is fired when the CSV file is loaded
    if(csvloader.checkFileLoaded() == true && dataLoaded == false){
        ofLogVerbose("DONE -----------------------------");
        // get rigidbody data
        rigidbodies = csvloader.getRigidBodies();
        // get skeleton data
        skeletons = csvloader.getSkeletons();
        // get number of frame sin recording
        totalFrames = csvloader.getNumFrames();
        // (re)set starting frame to zero
        frameNum = 0;
        fFrameNum = 0;
        // data is loaded
        dataLoaded = true;
        // we can start playing
        playData = true;
    }
    
    //TODO: offload this to an ofThread, so we can run it completely independently from the ofFrameRate
    // run frameloop based on time (semi-independent from ofFrameRate)
    fTimeCounter += ofGetLastFrameTime();
    while ( fTimeCounter >= frameTime ) {
        doFrame();
        fTimeCounter -= frameTime;
    }
}

void ofApp::doFrame() {
    // SEND OSC
    if(dataLoaded == true && playData == true){
        //ofLogVerbose("frame: "+ofToString(frameNum));
        
        for( int i = 0; i < clients.size(); ++i )
        {
            ofxOscBundle bundle;
            
            for (auto & rb : rigidbodies)
            {
                ofxOscMessage m;
                rb.second.getOSCData(frameNum, &m, clients[i]->getHierarchy(), true);
                bundle.addMessage(m);
            }
            
            // loop through skeletons
            for (auto & sk : skeletons)
            {
                //ofxOscMessage m = sk.second.getOSCData(frameNum);
                std::vector<ofxOscMessage> ms;
                if ( clients[i]->getHierarchy() ) {
                    ms = sk.second.getOSCDataHierarchy(frameNum);
                }
                else {
                    ms.push_back(sk.second.getOSCData(frameNum));
                }
                for( ofxOscMessage m : ms){
                    bundle.addMessage(m);
                }
            }
            
            //check if not empty & send
            if ( bundle.getMessageCount() > 0 )
            {
                clients[i]->sendBundle(bundle);
            }
        }
        
        //UPDATE FRAMES
        // advance to the next frame
        // scales the amount of frames to step based on framerate
        // stores this as a float to allow for more accurate frame-stepping
        if ( frameRate != 0 ) {
            float frameStep = MOTIVE_MOCAP_FPS / frameRate;
            fFrameNum += frameStep;
            if ( fFrameNum >= totalFrames ) {
                fFrameNum -= totalFrames;
            }
            frameNum = (int)fFrameNum;
        }
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    //ofDrawBitmapString("Press 'l' to load file", 20, 15);
    ofSetColor(255);
    ofSetBackgroundColor(40);
    
    // for visual feedback of file loading
    csvloader.draw();
    
    // GUI
    loadFileBTN.draw();
    addBTN.draw();
    newName.draw();
    newIP.draw();
    newPort.draw();
    fps.draw();
    saveBTN.draw();
    setFPSBTN.draw();
    if(dataLoaded == true){
        playpauseBTN.draw();
        rewindBTN.draw();
    }
    
    
    // show frames
    ofSetColor(255);
    
    font.drawString("New User", InterfaceX, InterfaceY+280);
    
    // DRAW CLIENTS
    for (int i = 0; i < clients.size(); i++)
    {
        clients[i]->draw();
    }
    
    
    int w = ofMap(frameNum, 0, totalFrames, 0, 200);
    int barY = 100;
    
    ofSetColor(255,100,100);
    ofNoFill();
    ofDrawRectangle(ofRectangle(InterfaceX, InterfaceY+barY, 202, 22));
    ofFill();
    ofSetColor(255);
    ofDrawRectangle(ofRectangle(InterfaceX, InterfaceY+barY, w, 20));
    ofDrawBitmapString("FrameNum: "+ofToString(frameNum), InterfaceX, InterfaceY+barY+40);
    
    ofDrawBitmapString("FrameRate: "+ofToString(ofGetFrameRate()), InterfaceX, InterfaceY+barY+110);
    
    
   
    
    // Playback teh recorded data
    /*
    if(dataLoaded == true && playData == true){
        ofLogVerbose("SENT OSC -----------------------------");
        
        ofLogVerbose("frame: "+frameNum);
        
        // loop through rigidbodies..
     
        for (auto & rb : rigidbodies)
        {
            ofxOscMessage m;
            rb.second.getOSCData(frameNum, &m, true);
            sender.sendMessage(m);
        }

        // loop through skeletons
        for (auto & sk : skeletons)
        {
            //ofxOscMessage m = sk.second.getOSCData(frameNum);
            std::vector<ofxOscMessage> ms = sk.second.getOSCDataHierarchy(frameNum);
            for( ofxOscMessage m : ms){
                sender.sendMessage(m);
            }
        }

        // show frames
        ofDrawBitmapString("FrameNum: "+ofToString(frameNum), 20, 130);
       
        // advance to the next frame
        if(frameNum < totalFrames){
            frameNum++;
        }
        else if(frameNum >= totalFrames){
            frameNum = 0;
        }
    }
     */
}

//--------------------------------------------------------------
void ofApp::saveData()
{
    ofxXmlSettings save;
    save.addTag("setup");
    save.pushTag("setup",0);
    save.addValue("fps", ofToInt(fps.getText()));
    //save.addValue("interface", interfaceName.getText());
    //save.addValue("ip", interfaceIP.getText());
    save.popTag();
    for (int i = 0; i < clients.size(); i++)
    {
        save.addTag("client");
        save.pushTag("client",i);
        save.addValue("ip", clients[i]->getIP());
        save.addValue("port", clients[i]->getPort());
        save.addValue("name", clients[i]->getName());
        save.addValue("rigid", clients[i]->getRigid());
        save.addValue("marker", clients[i]->getMarker());
        save.addValue("skeleton", clients[i]->getSkeleton());
        save.addValue("hierarchy", clients[i]->getHierarchy());
        save.popTag();
    }
    save.save("setup.xml");
}


//--------------------------------------------------------------
void ofApp::addClient(int i,string ip,int p,string n,bool r,bool m,bool s, bool live, bool hierarchy)
{
    client *c = new client(i,ip,p,n,r,m,s,hierarchy);
    ofAddListener(c->deleteClient, this, &ofApp::deleteClient);
    clients.push_back(c);
}

//--------------------------------------------------------------
void ofApp::deleteClient(int &index)
{
    ofRemoveListener(clients[index]->deleteClient, this, &ofApp::deleteClient);
    delete clients[index];
    clients.erase(clients.begin() + index);
    for (int i = 0; i < clients.size(); i++)
    {
        clients[i]->rearangePosition(i,true);
    }
}

//--------------------------------------------------------------
void ofApp::deactivateInputs()
{
    //deactivate all inputfields
    //interfaceName.deactivate();
    //interfaceIP.deactivate();
    fps.deactivate();
    newName.deactivate();
    newIP.deactivate();
    newPort.deactivate();
}


//--------------------------------------------------------------
void ofApp::loadAFile(){
    
    //Open the Open File Dialog
    ofFileDialogResult openFileResult= ofSystemLoadDialog("Select a .csv file");
    
    //Check if the user opened a file
    if (openFileResult.bSuccess){
        
        ofLogVerbose("User selected a file");
        
        // Check if th efile exists
        ofFile file (openFileResult.getPath());
        if (file.exists()){
            
            ofLogVerbose("The file exists - now checking the type via file extension");
            string fileExtension = ofToUpper(file.getExtension());
            
            //We only want CSV
            if (fileExtension == "CSV" ) {
                ofLogVerbose("the file is going to load!");
                
                // send file to csvloader who will proces the file in it;s own thread
                csvloader.setFile(file);
                
                
            }
            else{
                ofLogVerbose("Chosen file is not a csv file");
            }
        }
        else{
            ofLogVerbose("File doe not exist");
        }
        
    }else {
        ofLogVerbose("User hit cancel");
    }
}


//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    
    // start/stop playback of fata
    if (key == 's')
    {
        //playData = ! playData;
    }
    
    // GUI
    if (newName.getState())
    {
        newName.addKey(key);
        return;
    }
    if (newIP.getState())
    {
        newIP.addKey(key);
        return;
    }
    if (newPort.getState())
    {
        newPort.addKey(key);
        return;
    }
    if (fps.getState())
    {
        fps.addKey(key);
        return;
    }

    
   }

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    
    deactivateInputs();
    for (int i = 0; i < clients.size(); i++)
    {
        bool isInside = clients[i]->getArea().inside(x, y);
        if (isInside)
        {
            clients[i]->isInside(x, y);
            return;
        }
    }
    
    if (loadFileBTN.isInside(x, y)) loadAFile();
    if(newName.isInside(x, y)) return;
    if(newIP.isInside(x, y)) return;
    if(newPort.isInside(x, y)) return;
    if(fps.isInside(x, y)) return;
    if(addBTN.isInside(x, y))
    {
        addClient(clients.size(), newIP.getText(), ofToInt(newPort.getText()), newName.getText(), false, false, false, true, false);
        return;
    }
    if(saveBTN.isInside(x, y)) saveData();
    if(setFPSBTN.isInside(x,y)){
        frameRate = ofToInt(fps.getText());
        frameTime = 1.0f / frameRate;
        //ofSetFrameRate(frameRate);
    }
    if(playpauseBTN.isInside(x,y)){
        ofLogVerbose("hit play/pause button");
        if(dataLoaded == true ){
            playData = ! playData;
            ofLogVerbose("whithin dataloaded true");
        }
    }
    
    if(rewindBTN.isInside(x,y)){
        frameNum = 0;
        fFrameNum = 0;
    }

    
    

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
