#include "ofApp.h"


/*
 Name: CSVtoOSC
 Description: 
 Read CSV file with MOCAP data and play back over OSC
*/


// TODO: Set right ID's for rigidbodies and skeletons
// TODO: set looping to be an option


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
    
    UserFeedback = "";
    
    
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
        
        ofxOscBundle RigidBodiesWithHiearchy,RigidBodiesNoHiearchy;
        ofxOscBundle SkeletonsWithHiearchy,SkeletonsNoHiearchy;
        
        // Loop through rigidbodies and calculate speed
        // Do this once every frame..
        for (auto & rb : rigidbodies)
        {
            rb.second.calculateSpeed(frameNum);
        }
        
        
        // Loop through rigidbodies to create OSC messages
        for (auto & rb : rigidbodies)
        {
            ofxOscMessage ms1;
            rb.second.getOSCData(frameNum, &ms1, true, true);
            RigidBodiesWithHiearchy.addMessage(ms1);
            
            ofxOscMessage ms2;
            rb.second.getOSCData(frameNum, &ms2, false, true);
            RigidBodiesNoHiearchy.addMessage(ms2);
        }
        
        
        // loop through skeletons to create OSC messages
        for (auto & sk : skeletons)
        {
            // WITH HIEARCHY
            std::vector<ofxOscMessage> messagesWithHierarchy;
            messagesWithHierarchy = sk.second.getOSCDataHierarchy(frameNum);
            
            // Add OSCmessages to the bundle
            for( ofxOscMessage m : messagesWithHierarchy){
                SkeletonsWithHiearchy.addMessage(m);
            }
            
            // WITHOUT HIEARCHY
            ofxOscMessage ms = sk.second.getOSCDataForBones(frameNum);
            SkeletonsNoHiearchy.addMessage(ms);
        }


        //LOOP THROUGH ALL THE CLIENTS
        for (std::vector<client*>::iterator it = clients.begin() ; it != clients.end(); ++it)
        {
            
            // RIGID BODIES
            if( (*it)->getRigid() ){
                // send bundle with hiearchy
                if( (*it)->getHierarchy() ){
                    (*it)->sendBundle(RigidBodiesWithHiearchy);
                }
                // send bundle without hiearchy
                else{
                    (*it)->sendBundle(RigidBodiesNoHiearchy);
                }
            }
            
            // SKELETONS
            if( (*it)->getSkeleton() ){
                if( (*it)->getHierarchy() ){
                    (*it)->sendBundle(SkeletonsWithHiearchy);
                }
                // send bundle without hiearchy
                else{
                    (*it)->sendBundle(SkeletonsNoHiearchy);
                }
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
    
    
    if(UserFeedback != ""){
        ofDrawBitmapStringHighlight(UserFeedback, ofGetWindowWidth()/2-UserFeedbackCanvas.width/2,ofGetWindowHeight()/2-UserFeedbackCanvas.height/2);
    }
    
    
   
    
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
    // Check if we do not add a cleint with the same properties twice
    Boolean uniqueClient = true;
    for (int i = 0; i < clients.size(); i++)
    {
        if(clients[i]->getIP() == ip && clients[i]->getPort() == p){
            uniqueClient = false;
            break;
        }
    }
    
    if(uniqueClient){
        client *c = new client(i,ip,p,n,r,m,s,hierarchy);
        ofAddListener(c->deleteClient, this, &ofApp::deleteClient);
        clients.push_back(c);
        if(UserFeedback != "") UserFeedback = "";
    }else{
        // give feedback client already exists
        ofLogError("NOOOOOOOO....." );
        UserFeedback = "\n A client with the same settings already exists. \n Please change IP address and or port! \n";
        UserFeedbackCanvas = UserFeedbackFont.getBoundingBox(UserFeedback,0,0);
        UserFeedbackCanvas.setPosition(ofGetWindowWidth()/2-UserFeedbackCanvas.width/2,ofGetWindowHeight()/2-UserFeedbackCanvas.height/2);
    }
}

//--------------------------------------------------------------
void ofApp::deleteClient(int &index)
{
    ofRemoveListener(clients[index]->deleteClient, this, &ofApp::deleteClient);
	int indexToRemove = index;
	delete clients[indexToRemove];
	clients.erase(clients.begin() + indexToRemove); 
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
        
        // stop playing file
        playData = false;
        // set data loaded to false
        dataLoaded = false;
        
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
void ofApp::printOSCmessage(ofxOscMessage m){
    
    string msg_string;
    msg_string = m.getAddress();
    msg_string += ": ";
    for(int i = 0; i < m.getNumArgs(); i++){
        // get the argument type
        //msg_string += m.getArgTypeName(i);
        msg_string += " : ";
        // display the argument - make sure we get the right type
        if(m.getArgType(i) == OFXOSC_TYPE_INT32){
            msg_string += ofToString(m.getArgAsInt32(i));
        }
        else if(m.getArgType(i) == OFXOSC_TYPE_FLOAT){
            msg_string += ofToString(m.getArgAsFloat(i));
        }
        else if(m.getArgType(i) == OFXOSC_TYPE_STRING){
            msg_string += m.getArgAsString(i);
        }
        else{
            msg_string += "unknown";
        }
    }
    
    ofLogWarning(ofToString(msg_string));
    
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
    
    if (UserFeedbackCanvas.inside(x,y)){
        UserFeedback = "";
        
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
