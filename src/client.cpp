//
//  client.cpp
//  NatNet2OSCbridge
//
//  Created by Enrico Becker on 12.10.15.
//
//

#include "client.h"
#include "fontawesome5.h"

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

void client::doGui()
{
    ImGui::Text(ICON_FA_BROADCAST_TOWER);
    ImGui::SameLine();
    //ImGui::Text(name.c_str());
    ImGui::Text("ip:");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.7f,1.0f,1.0f,1.0f), ip.c_str());
    ImGui::SameLine();
    ImGui::Text("port:");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.5f,0.3f,1.0f,1.0f), "%d", port);
    char rigidstr[6+ip.length()+6];
    sprintf(rigidstr, "Rigid##%s%i", ip.c_str(), port);
    ImGui::Checkbox(rigidstr, &isRigid);
    ImGui::SameLine();
    char markstr[6+ip.length()+6];
    sprintf(markstr, "Mark##%s%i", ip.c_str(), port);
    ImGui::Checkbox(markstr, &isMarker);
    ImGui::SameLine();
    char skelstr[6+ip.length()+6];
    sprintf(skelstr, "Skel##%s%i", ip.c_str(), port);
    ImGui::Checkbox(skelstr, &isSkeleton);
    ImGui::SameLine();
    char hierstr[6+ip.length()+6];
    sprintf(hierstr, "Hierarchy##%s%i", ip.c_str(), port);
    ImGui::Checkbox(hierstr, &deepHierarchy);
}

string &client::getName()
{
    return name;
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

