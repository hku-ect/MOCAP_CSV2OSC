#include "uiWidgets.h"

uiLogger::uiLogger()
{
    channel = std::shared_ptr<ofxImGui::LoggerChannel>(new ofxImGui::LoggerChannel());
}

void uiLogger::doGui()
{
    // based on Log Example App from imgui demo
    if (ImGui::Button("Clear")) Clear();
    ImGui::SameLine();
    bool copy = ImGui::Button("Copy");
    ImGui::SameLine();
    Filter.Draw("Filter", -100.0f);
    ImGui::Separator();
    ImGui::BeginChild("scrolling", ImVec2(0,0), false, ImGuiWindowFlags_HorizontalScrollbar);
    if (copy) ImGui::LogToClipboard();
    if (Filter.IsActive())
    {
        const char* buf_begin = channel->getBuffer().begin();
        const char* line = buf_begin;
        for (int line_no = 0; line != NULL; line_no++)
        {
            const char* line_end = (line_no < LineOffsets.Size) ? buf_begin + LineOffsets[line_no] : NULL;
            if (Filter.PassFilter(line, line_end))
                ImGui::TextUnformatted(line, line_end);
            line = line_end && line_end[1] ? line_end + 1 : NULL;
        }
    }
    else
    {
        ImGui::TextUnformatted(channel->getBuffer().begin());
    }
    ImGuiTextBuffer test = channel->getBuffer();

    if (ScrollToBottom)
        ImGui::SetScrollHere(1.0f);
    ScrollToBottom = false;
    ImGui::EndChild();
}
