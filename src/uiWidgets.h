#ifndef UIWIDGETS_H
#define UIWIDGETS_H
#include "ofxImGuiLoggerChannel.h"

class uiLogger
{                       // based on ExampleAppLog from ImGui demo
public:
    uiLogger();
    ImGuiTextFilter     Filter;
    ImVector<int>       LineOffsets;        // Index to lines offset
    bool                ScrollToBottom;
    std::shared_ptr<ofxImGui::LoggerChannel> channel;

    void    Clear()     { channel->getBuffer().clear(); LineOffsets.clear(); }
    void    doGui();
};

#endif // UIWIDGETS_H
