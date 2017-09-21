//
//  CSVloader.h
//  CSVtoOSC
//
//  Created by Machiel Veltkamp on 18/05/17.
//
//

#ifndef CSVloader_h
#define CSVloader_h


#endif /* CSVloader_h */

#pragma once


#include "ofThread.h"
#include "MOCAP_Marker.h"
#include "MOCAP_Skeleton.h"


/*
 
 Name: CSVloader
 Description:
 This class loads in it's own thread the CSV file and returns the laoded data 
 to the main thread to be used.
 
 */
class CSVloader: public ofThread
{
public:
    /// Create a ThreadedObject and initialize the member
    CSVloader(): count(0), shouldThrowTestException(false)
    {
    }
    
    /// Start the thread.
    void start()
    {
        // Mutex blocking is set to true by default
        // It is rare that one would want to use startThread(false).
        startThread();
    }
    
    /// Signal the thread to stop.  After calling this method,
    /// isThreadRunning() will return false and the while loop will stop
    /// next time it has the chance to.
    void stop()
    {
        stopThread();
    }
    
    /// Our implementation of threadedFunction.
    void threadedFunction()
    {
        while(isThreadRunning())
        {
            
            // Read File
            string currentLine = ""; // empty for the moment
            
            
            // While we have not yet reached the end of the file
            if (! csvBuffer.isLastLine()) {
                
                if(lock()){ // get lock
                    
                    // get next line
                    currentLine = csvBuffer.getNextLine();
                    // split line in chunks (use the ,
                    vector<string> data = ofSplitString(currentLine, ",");

                    ofLogVerbose(ofToString(count));
                    
                    // get the data from the file
                    // starting form line 7 the data starts
                    if(count > 6 && currentLine != ""){
                        ofLogVerbose("Frame: "+ofToString(data[0])+" time: "+data[1]);
                        
                        // loop through rigid bodies
                        for (auto & rb : rigidbodies)
                        {
                            rb.second.addMarkerEntry(data);
                        }
                        
                        
                        // loop through skeletons
                        for (auto & sk : skeletons)
                        {
                            sk.second.addSkeletonEntry(data);
                        }

                        
                    }
                    
                    // clear the lock
                    unlock();
                    
                    // increase the count
                    count++;

                }
                else
                {
                    // If we reach this else statement, it means that we could not
                    // lock our mutex, and so we do not need to call unlock().
                    // Calling unlock without locking will lead to problems.
                    ofLogWarning("threadedFunction()") << "Unable to lock mutex.";
                }

            }
            // We have reached th eend of the file
            // stop the thread and set the fiel laoded flag to true
            else{
                ofLogVerbose("stopping thread");
                fileLoaded = true;
                stopThread();
            }
        }
    }
    
    /// This drawing function cannot be called from the thread itself because
    /// it includes OpenGL calls (ofDrawBitmapString).
    void draw()
    {
        
        // show meta info of the recording (when set)
        // TODO: fix absolute numbers
        ofDrawBitmapString(info, ofGetWidth() - 250, 80);
        
        // show status of loading (when loading)
        if(fileLoaded == false){
            string s = "loading "+ofToString(count)+"/"+ofToString(numFrames);
            ofDrawBitmapString(s, ofGetWidth() - 250, 55);
        }
        
    }
    
    void setFile(ofFile file){
        ofLogVerbose("file set!!");
        fileLoaded = false;
        csvBuffer = file.readToBuffer();
        
        // --> Figure out what is in the file and what we need to do ..
        
        // Get first Line (with all the basic information)
        ofBuffer::Line it = csvBuffer.getLines().begin();
        string line = *it;
        // split into array
        vector<string> words = ofSplitString(line, ",");
        
         if(lock()){
            // set info for feedback text
            numFrames = ofToInt(words[11]);
            info = "Name: "+ words[3]+"\n";
            info += "Capture Start Time: "+ words[5]+"\n";
            info += "Total Frames: "+ words[11];
             
             unlock();
        }
        
        ofLogVerbose("---> "+info);
        
        // get line with bonses/rigidbodies
        // collumn 2 is start info
        // row 3 = type marker, row 4 = name, row 5 = id, row 6 = name element
        ++it; // move to line 2 --> Empty
        ++it; // move to line 3 --> typeMarkers
        line = *it;
        vector<string> typeMarkers = ofSplitString(line, ",");
        ++it; // move to line 4 --> names
        line = *it;
        vector<string> names = ofSplitString(line, ",");
        ++it; // move to line 5 --> id
        ++it; // move to line 6 --> name element
        line = *it;
        vector<string> nameElement = ofSplitString(line, ",");
        ++it; // move to line 7 --> x,y,z
        line = *it;
        vector<string> nameAxis = ofSplitString(line, ",");
        
        // debug log
        for(int i =0;i<names.size();i++){
            ofLogVerbose(ofToString(i-2)+" - "+names[i]);
            
        }
        
        
        // Loop through the collumns
        for(int i =0;i<typeMarkers.size();i++)
        {
            // Part of rigid Body
            if(typeMarkers[i] == "Rigid Body"){
                
                // We did not find the rigid body -> Create rigidbody
                if (rigidbodies.find(names[i]) == rigidbodies.end()){
                    ofLogVerbose(ofToString(i)+" rigid body: "+names[i]+" CREATED");
                    MOCAP_Marker m;
                    m.setMarker(0, names[i], i); // FIXME: move to constructor?
                    rigidbodies.insert(std::pair<string,MOCAP_Marker>(names[i],m));
                }
            }
            // Part of bone
            else if(typeMarkers[i] == "Bone"){
                
                // get the name of the skeleton which is the part for the _
                std::size_t found = names[i].find("_");
                string skeletonName =  names[i].substr (0,found);
                
                // We did not find the skeleton -> Create skeleton
                if(skeletons.find(skeletonName) == skeletons.end()){
                    ofLogVerbose(ofToString(i)+" SKELETOM: "+skeletonName+" CREATED");
                    MOCAP_Skeleton ms;
                    ms.setSkeleton(0,skeletonName,i);
                    skeletons.insert(std::pair<string,MOCAP_Skeleton>(skeletonName,ms));
                }
            }
            // else --> dont want to do anything with it
        }
        
        // All the foun rigidbodies and skeletons are found now
        // we can start the thread that wil extract the content
        startThread();
    }
    
    // return if the fil is already loaded
    Boolean checkFileLoaded()
    {
        unique_lock<std::mutex> lock(mutex);
        return fileLoaded;
    }
    
    // Get Rigidbody data
    std::map<string,MOCAP_Marker> getRigidBodies(){
        unique_lock<std::mutex> lock(mutex);
        return rigidbodies;
    }

    // Get skeleton data
    std::map<string,MOCAP_Skeleton> getSkeletons(){
        unique_lock<std::mutex> lock(mutex);
        return skeletons;
    }
    
    // return the number of frames (length) of the recording
    int getNumFrames(){
        unique_lock<std::mutex> lock(mutex);
        return numFrames;
    }

    
    // Use unique_lock to protect a copy of count while getting a copy.
    int getCount()
    {
        unique_lock<std::mutex> lock(mutex);
        return count;
    }
    
    
protected:
    // A flag to check and see if we should throw a test exception.
    Poco::AtomicCounter shouldThrowTestException;
    
    // This is a simple variable that we aim to always access from both the
    // main thread AND this threaded object.  Therefore, we need to protect it
    // with the mutex.  In the case of simple numerical variables, some
    // garuntee thread safety for small integral types, but for the sake of
    // illustration, we use an int.  This int could represent ANY complex data
    // type that needs to be protected.
    //
    // Note, if we simply want to count in a thread-safe manner without worrying
    // about mutexes, we might use Poco::AtomicCounter instead.
    int count;              // use as frame counter for loading feedabck
    int numFrames;          // use to store number of frames in recording
    ofFile file;            // the filw that is being loaded
    Boolean fileLoaded;     // store if file is loaded or not
    ofBuffer csvBuffer;     // buffer to store the data
    // map to store the found rigidboies and their data
    std::map<string,MOCAP_Marker> rigidbodies;
    // map to store the found skeletons and their data
    std::map<string,MOCAP_Skeleton> skeletons;
    // info feedback string
    string info = "";

    
};

