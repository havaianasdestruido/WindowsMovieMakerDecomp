#pragma once
#include <string>
#include <vector>

struct MediaRef { std::wstring id; std::wstring path; };
struct Clip { std::wstring mediaId; double start; double duration; int track; };
struct Project {
    std::wstring title;
    std::vector<MediaRef> media;
    std::vector<Clip> timeline;
};

class ProjectManager {
public:
    bool CreateNew(const std::wstring& title);
    bool Load(const std::wstring& filePath);
    bool Save(const std::wstring& filePath) const;
    const Project& GetProject() const { return m_project; }
private:
    Project m_project;
};
