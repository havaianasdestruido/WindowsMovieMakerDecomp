#include "Project.h"
#include <fstream>
#include <sstream>
#include <regex>

using namespace tinyxml2;

bool ProjectManager::CreateNew(const std::wstring& title) {
    m_project.title = title;
    m_project.media.clear();
    m_project.timeline.clear();
    return true;
}

bool ProjectManager::Load(const std::wstring& filePath) {
    XMLDocument doc;
    XMLError err = doc.LoadFile(std::string(filePath.begin(), filePath.end()).c_str());
    if (err != XML_SUCCESS) return false;
    XMLElement* root = doc.FirstChildElement("Project");
    if (!root) return false;
    const char* t = root->Attribute("title");
    m_project.title = std::wstring(t, t + strlen(t));
    // Media
    XMLElement* mediaElem = root->FirstChildElement("Media");
    if (mediaElem) {
        for (XMLElement* item = mediaElem->FirstChildElement("Item"); item; item = item->NextSiblingElement("Item")) {
            MediaRef mr;
            const char* id = item->Attribute("id");
            const char* path = item->Attribute("path");
            if (id) mr.id = std::wstring(id, id + strlen(id));
            if (path) mr.path = std::wstring(path, path + strlen(path));
            m_project.media.push_back(mr);
        }
    }
    // Timeline
    XMLElement* tlElem = root->FirstChildElement("Timeline");
    if (tlElem) {
        for (XMLElement* clip = tlElem->FirstChildElement("Clip"); clip; clip = clip->NextSiblingElement("Clip")) {
            Clip c{};
            const char* mid = clip->Attribute("mediaId");
            if (mid) c.mediaId = std::wstring(mid, mid + strlen(mid));
            clip->QueryDoubleAttribute("start", &c.start);
            clip->QueryDoubleAttribute("duration", &c.duration);
            clip->QueryIntAttribute("track", &c.track);
            m_project.timeline.push_back(c);
        }
    }
    return true;
}

bool ProjectManager::Save(const std::wstring& filePath) const {
    XMLDocument doc;
    XMLElement* root = doc.NewElement("Project");
    root->SetAttribute("title", std::string(m_project.title.begin(), m_project.title.end()).c_str());
    // Media
    XMLElement* mediaElem = doc.NewElement("Media");
    for (const auto& mr : m_project.media) {
        XMLElement* item = doc.NewElement("Item");
        item->SetAttribute("id", std::string(mr.id.begin(), mr.id.end()).c_str());
        item->SetAttribute("path", std::string(mr.path.begin(), mr.path.end()).c_str());
        mediaElem->InsertEndChild(item);
    }
    root->InsertEndChild(mediaElem);
    // Timeline
    XMLElement* tlElem = doc.NewElement("Timeline");
    for (const auto& c : m_project.timeline) {
        XMLElement* clip = doc.NewElement("Clip");
        clip->SetAttribute("mediaId", std::string(c.mediaId.begin(), c.mediaId.end()).c_str());
        clip->SetAttribute("start", c.start);
        clip->SetAttribute("duration", c.duration);
        clip->SetAttribute("track", c.track);
        tlElem->InsertEndChild(clip);
    }
    root->InsertEndChild(tlElem);
    doc.InsertEndChild(root);
    XMLError err = doc.SaveFile(std::string(filePath.begin(), filePath.end()).c_str());
    return err == XML_SUCCESS;
}
