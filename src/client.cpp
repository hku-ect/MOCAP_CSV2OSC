//
//  client.cpp
//  NatNet2OSCbridge
//
//  Created by Enrico Becker on 12.10.15.
//
//

#include "client.h"

client::client(int ind,string i,int p,string n,bool r,bool m,bool s, bool hier)
{
    //arange them gridwise
    index = ind;
    ip = i;
    port = p;
    name = n;
    isRigid = r;
    isMarker = m;
    isSkeleton = s;
    deepHierarchy = hier;
    notWholeScreen = true;
    
    setupSender();
}

client::~client(){}

void client::setupSender()
{
    sender.setup(ip, port);
}

void client::sendData(ofxOscMessage &m)
{
    sender.sendMessage(m);
}

void client::sendBundle(ofxOscBundle &b)
{
    sender.sendBundle(b);
}

void client::draw()
{
    ImGui::Text("ip:");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.7f,1.0f,1.0f,1.0f), ip.data());
    ImGui::SameLine();
    ImGui::Text("port:");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.5f,0.3f,1.0f,1.0f), "%d", port);
    ImGui::Checkbox("Rigid", &isRigid);
    ImGui::SameLine();
    ImGui::Checkbox("Mark", &isMarker);
    ImGui::SameLine();
    ImGui::Checkbox("Skel", &isSkeleton);
    ImGui::SameLine();
    ImGui::Checkbox("Hierarchy", &deepHierarchy);
}

string &client::getName()
{
    return name;
}


ofRectangle &client::getArea()
{
    return area;
}

int &client::getID()
{
    return index;
}


string &client::getIP()
{
    return ip;
}

int &client::getPort()
{
    return port;
}

void client::setRigid(bool value)
{
    isRigid = value;
}

void client::setMarker(bool value)
{
    isMarker = value;
}

void client::setSkeleton(bool value)
{
    isSkeleton = value;
}

bool &client::getRigid()
{
    return isRigid;
}

bool &client::getMarker()
{
    return isMarker;
}

bool &client::getSkeleton()
{
    return isSkeleton;
}

bool &client::getHierarchy()
{
    return deepHierarchy;
}

