#include "ofApp.h"
#include "fontawesome5.h"

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
static const ImWchar icon_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };

//--------------------------------------------------------------
void ofApp::setup(){
    
    // set logging level determing which message will be showed
    ofSetLogLevel(OF_LOG_VERBOSE);
       
    // when startign we do not yet have any data
    dataLoaded = false;
    
    // setup a OSC sender
    sender.setup(HOST, PORT);
    
    // SETUP GUI
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontDefault();
    ImFontConfig config;
    config.MergeMode = true;
    //config.GlyphMinAdvanceX = 13.0f; // Use if you want to make the icon monospaced
    config.PixelSnapH = true;
    //io.Fonts->AddFontFromFileTTF("fonts/fontawesome-webfont.ttf", 13.0f, &config, icon_ranges);
    io.Fonts->AddFontFromFileTTF(FONT_ICON_FILE_NAME_FAS, 12.0f, &config, icon_ranges);

    gui.setup(nullptr, false);              // default theme, no autoDraw!
    io.IniFilename = NULL;                  // no imgui.ini
    guiVisible = true;
    //gui.setTheme(new ThemeTest());

    ImGuiStyle& style = ImGui::GetStyle();  //style tweaks
    //style.FrameBorderSize = 1.0f;
    //style.WindowBorderSize = 1.f;
    style.ChildBorderSize = 1.0f;
    //style.ChildRounding = 8.f;
    style.WindowPadding = ImVec2(5.0f, 5.0f);
    style.ItemInnerSpacing = ImVec2(8.0f, 8.0f);
    style.ItemSpacing = ImVec2(6.0f, 6.0f);

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
    frameRate = data.getValue("fps", 30);
    //string interface = data.getValue("interface", "en0");
    //string natnetip = data.getValue("ip", "10.200.200.13");
    //interfaceName.setText(interface);
    //interfaceIP.setText(natnetip);

    frameTime = 1.0 / frameRate;
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
    doGui();
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

    if ( this->guiVisible ) { gui.draw(); }
    //gui.draw();
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
    save.addValue("fps", frameRate);
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
	int indexToRemove = index;
	delete clients[indexToRemove];
	clients.erase(clients.begin() + indexToRemove); 
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

void ofApp::doGui() {
    this->mouseOverGui = false;
    if (this->guiVisible)
    {
        auto mainSettings = ofxImGui::Settings();
        //ui stuff
        gui.begin();
        // Create a main menu bar
        float mainmenu_height = 0;
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Open CSV..", "Ctrl+O")) { loadAFile(); }
                if (ImGui::MenuItem("Save Setup", "Ctrl+S"))   {saveData(); }
                if (ImGui::MenuItem("Exit", "Ctrl+W"))  { ofExit(0); }
                ImGui::EndMenu();
            }
            mainmenu_height = ImGui::GetWindowSize().y;
            ImGui::EndMainMenuBar();
        }

        // clients window
        ImGui::SetNextWindowPos(ImVec2( 0, mainmenu_height ));
        ImGui::SetNextWindowSize(ImVec2( ofGetWidth()-351, ofGetHeight()-mainmenu_height));
        ImGui::Begin("clientspanel", NULL,  ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoBringToFrontOnFocus);
        // DRAW CLIENTS
        for (int i = 0; i < clients.size(); i++)
        {
            bool enabled = true;
            char buf[256];
            snprintf(buf, sizeof(buf),"%s %s", ICON_FA_DESKTOP, clients[i]->getName().data());
            if ( ImGui::CollapsingHeader(buf, &enabled, ImGuiTreeNodeFlags_DefaultOpen) )
            {
                clients[i]->draw();
            }
            if ( ! enabled )
            {
                ofNotifyEvent(clients[i]->deleteClient,i);
            }
        }
        ImGui::End();

        // right dock
        ImGui::SetNextWindowPos(ImVec2( ofGetWidth()-350, mainmenu_height ));
        ImGui::SetNextWindowSize(ImVec2( 350, ofGetHeight()-mainmenu_height));
        ImGui::Begin("rightpanel", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_HorizontalScrollbar);
        ImGui::Columns(2, "csvstats");
        ImGui::Text("Name"); ImGui::NextColumn();
        ImGui::Text(csvloader.take_name.data()); ImGui::NextColumn();
        ImGui::Text("Capture Start Time"); ImGui::NextColumn();
        ImGui::Text(csvloader.cap_time.data()); ImGui::NextColumn();
        ImGui::Text("Total Frames"); ImGui::NextColumn();
        ImGui::Text("%d", totalFrames); ImGui::NextColumn();
        ImGui::Columns(1);
        ImGui::Separator();

        char fnumber[6];
        sprintf(fnumber, "framenumber %d/%d", frameNum, totalFrames );
        ImGui::ProgressBar(frameNum/float(totalFrames), ImVec2(-1,0), fnumber );

        ImGui::Separator();

        //playpause buttons
        if ( ImGui::Button(ICON_FA_BACKWARD) )
        {
            if ( dataLoaded == true )
            {
                frameNum = 0;
                fFrameNum = 0;
            }
            else
            {
                ofLogNotice() << "No data loaded yet?";
            }
        }
        ImGui::SameLine();
        if ( ImGui::Button(ICON_FA_PLAY) )
        {
            if ( dataLoaded == true )
            {
                playData = true;
            }
            else
            {
                ofLogNotice() << "No data loaded yet?";
            }
        }
        ImGui::SameLine();
        if ( ImGui::Button(ICON_FA_PAUSE) )
        {
            if ( dataLoaded == true )
            {
                playData = false;
            }
            else
            {
                ofLogNotice() << "No data loaded yet?";
            }
        }



        // Framerate setter
        if ( ImGui::DragInt("framerate", &frameRate, 1, 0, 300, "%.0f%") ) {
             //frameRate = ofToInt(fps.getText());
             frameTime = 1.0f / frameRate;
        }
        //ImGui::SameLine();
        //ShowHelpMarker("You can apply arithmetic operators +,*,/ on numerical values.\n  e.g. [ 100 ], input \'*2\', result becomes [ 200 ]\nUse +- to subtract.\n");

        // client entry
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20.f, 20.f));
        ImGui::Separator();
        ImGui::BeginChild(" ClientEntry", ImVec2(-1,140));
        //ImGui::Columns(2);
        static char client_name[128] = "localhost";
        ImGui::InputText("client name", client_name, IM_ARRAYSIZE(client_name));
        static char client_ip[15] = "127.0.0.1";
        ImGui::InputText("client ip", client_ip, IM_ARRAYSIZE(client_ip));
        static int client_port = 1234;
        ImGui::InputInt("client port", &client_port);
        if ( ImGui::Button(ICON_FA_DESKTOP " add client") )
        {
            addClient(clients.size(), ofToString(client_ip), client_port, ofToString(client_name), false, false, false, true, false );
        }
        ImGui::EndChild();
        ImGui::PopStyleVar(2);
        ImGui::End();

        gui.end();
        this->mouseOverGui = mainSettings.mouseOverGui;
    }
}
