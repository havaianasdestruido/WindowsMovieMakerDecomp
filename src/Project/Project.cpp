#include "Project.h"
#include <fstream>
#include <sstream>
#include <regex>

namespace {

// UTF-16 LE with BOM, escaped attribute values
std::wstring EscapeXml(const std::wstring& in) {
    std::wstring out;
    out.reserve(in.size());
    for (wchar_t ch : in) {
        switch (ch) {
        case L'&': out += L"&amp;"; break;
        case L'<': out += L"&lt;"; break;
        case L'>': out += L"&gt;"; break;
        case L'"': out += L"&quot;"; break;
        case L'\'': out += L"&apos;"; break;
        default: out += ch; break;
        }
    }
    return out;
}

std::wstring UnescapeXml(const std::wstring& in) {
    std::wstring out;
    out.reserve(in.size());
    for (size_t i = 0; i < in.size(); ++i) {
        if (in[i] == L'&') {
            size_t semi = in.find(L';', i);
            if (semi != std::wstring::npos) {
                std::wstring ent = in.substr(i + 1, semi - i - 1);
                if (ent == L"amp") out += L'&';
                else if (ent == L"lt") out += L'<';
                else if (ent == L"gt") out += L'>';
                else if (ent == L"quot") out += L'"';
                else if (ent == L"apos") out += L'\'';
                else out += in.substr(i, semi - i + 1);
                i = semi;
                continue;
            }
        }
        out += in[i];
    }
    return out;
}

std::wstring Attr(const std::wstring& line, const wchar_t* name) {
    std::wstring pat = std::wstring(name) + L"=\"([^\"]*)\"";
    std::wregex re(pat);
    std::wsmatch m;
    if (std::regex_search(line, m, re))
        return UnescapeXml(m[1].str());
    return std::wstring();
}

} // namespace

bool ProjectManager::CreateNew(const std::wstring& title) {
    m_project.title = title;
    m_project.media.clear();
    m_project.timeline.clear();
    return true;
}

bool ProjectManager::Load(const std::wstring& filePath) {
    std::wifstream file(filePath, std::ios::binary);
    if (!file)
        return false;

    std::wstringstream ss;
    ss << file.rdbuf();
    std::wstring xml = ss.str();

    // Skip UTF-8 / UTF-16 BOM if present
    if (!xml.empty() && xml[0] == 0xFEFF)
        xml.erase(0, 1);
    else if (xml.size() >= 2 && static_cast<unsigned char>(xml[0]) == 0xEF)
        xml.erase(0, 3);

    std::wstring attr;
    std::wstring mediaId, mediaPath, clipMediaId;
    double start = 0.0, duration = 0.0;
    int track = 0;

    std::wistringstream lines(xml);
    std::wstring line;
    while (std::getline(lines, line)) {
        std::wregex projectTag(L"<Project[^>]*>");
        std::wsmatch pm;
        if (std::regex_search(line, pm, projectTag)) {
            attr = Attr(pm[0].str(), L"title");
            if (!attr.empty())
                m_project.title = attr;
            m_project.media.clear();
            m_project.timeline.clear();
            continue;
        }

        std::wregex itemTag(L"<Item[^>]*>");
        std::wsmatch im;
        if (std::regex_search(line, im, itemTag)) {
            mediaId = Attr(im[0].str(), L"id");
            mediaPath = Attr(im[0].str(), L"path");
            MediaRef mr;
            mr.id = mediaId;
            mr.path = mediaPath;
            m_project.media.push_back(mr);
            continue;
        }

        std::wregex clipTag(L"<Clip[^>]*>");
        std::wsmatch cm;
        if (std::regex_search(line, cm, clipTag)) {
            clipMediaId = Attr(cm[0].str(), L"mediaId");
            std::wstring s = Attr(cm[0].str(), L"start");
            std::wstring d = Attr(cm[0].str(), L"duration");
            std::wstring t = Attr(cm[0].str(), L"track");
            start = s.empty() ? 0.0 : wcstod(s.c_str(), nullptr);
            duration = d.empty() ? 0.0 : wcstod(d.c_str(), nullptr);
            track = t.empty() ? 0 : _wtoi(t.c_str());
            Clip c{};
            c.mediaId = clipMediaId;
            c.start = start;
            c.duration = duration;
            c.track = track;
            m_project.timeline.push_back(c);
            continue;
        }
    }

    return true;
}

bool ProjectManager::Save(const std::wstring& filePath) const {
    std::wofstream file(filePath, std::ios::binary);
    if (!file)
        return false;

    file << L"\xFEFF"; // UTF-16 LE BOM
    file << L"<?xml version=\"1.0\" encoding=\"utf-16\"?>\r\n";
    file << L"<Project title=\"" << EscapeXml(m_project.title) << L"\">\r\n";
    file << L"  <Media>\r\n";
    for (const auto& mr : m_project.media) {
        file << L"    <Item id=\"" << EscapeXml(mr.id)
             << L"\" path=\"" << EscapeXml(mr.path) << L"\"/>\r\n";
    }
    file << L"  </Media>\r\n";
    file << L"  <Timeline>\r\n";
    for (const auto& c : m_project.timeline) {
        file << L"    <Clip mediaId=\"" << EscapeXml(c.mediaId)
             << L"\" start=\"" << c.start
             << L"\" duration=\"" << c.duration
             << L"\" track=\"" << c.track << L"\"/>\r\n";
    }
    file << L"  </Timeline>\r\n";
    file << L"</Project>\r\n";

    file.flush();
    return file.good();
}
