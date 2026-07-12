#pragma once
// X3DReader.h - HMREngine::X3DReader (CComObject)

#include "HMREngine.h"
#include "X3DNodes.h"
#include <atlbase.h>
#include <atlcom.h>
#include <string>
#include <vector>

namespace HMREngine
{
    class Scene;

    struct X3DParseContext
    {
        Scene* scene = nullptr;
        std::vector<X3DChildNode*> nodeStack;
        std::string currentProto;
        std::string basePath;

        X3DChildNode* GetCurrentNode() const
        {
            return nodeStack.empty() ? nullptr : nodeStack.back();
        }

        void PushNode(X3DChildNode* node)
        {
            nodeStack.push_back(node);
        }

        void PopNode()
        {
            if (!nodeStack.empty()) nodeStack.pop_back();
        }
    };

    class ATL_NO_VTABLE X3DReader :
        public CComObjectRootEx<CComSingleThreadModel>,
        public CComCoClass<X3DReader>
    {
    public:
        X3DReader();
        virtual ~X3DReader();

        DECLARE_REGISTRY_RESOURCEID(IDR_X3DREADER)
        DECLARE_NOT_AGGREGATABLE(X3DReader)

        BEGIN_COM_MAP(X3DReader)
            COM_INTERFACE_ENTRY(IUnknown)
        END_COM_MAP()

        // File I/O
        HRESULT LoadFromFile(const std::string& filePath, Scene* scene);
        HRESULT LoadFromMemory(const char* data, size_t length, Scene* scene);
        HRESULT SaveToFile(const std::string& filePath, Scene* scene);

        // Parsing
        HRESULT ParseX3D(const char* data, size_t length, X3DParseContext& ctx);
        HRESULT ParseX3DV1(const char* data, size_t length, X3DParseContext& ctx);
        HRESULT ParseX3DV2(const char* data, size_t length, X3DParseContext& ctx);
        HRESULT ParseXML(const char* data, size_t length, X3DParseContext& ctx);
        HRESULT ParseJSON(const char* data, size_t length, X3DParseContext& ctx);

        // Node creation
        X3DChildNode* CreateNode(const std::string& typeName);
        X3DFieldNode* CreateField(const std::string& typeName);

        // Error info
        int GetLastLine() const { return m_lastLine; }
        std::string GetLastErrorMessage() const { return m_lastError; }

    private:
        int m_lastLine = 0;
        std::string m_lastError;

        // File reading
        std::vector<char> ReadFile(const std::string& path);
        std::string ExtractBasePath(const std::string& filePath);

        // Line-based parser helpers
        bool ParseFieldLine(const std::string& line, X3DChildNode* node);
        bool ParseNodeLine(const std::string& line, X3DParseContext& ctx);

        // XML helpers
        HRESULT ParseXMLElement(void* xmlNode, X3DParseContext& ctx);
        X3DChildNode* CreateNodeFromXML(void* xmlElement);

        // Field value parsing
        bool ParseBool(const std::string& s, bool& out);
        bool ParseInt(const std::string& s, int& out);
        bool ParseFloat(const std::string& s, float& out);
        bool ParseDouble(const std::string& s, double& out);
        bool ParseVec2f(const std::string& s, Vec2& out);
        bool ParseVec3f(const std::string& s, Vec3& out);
        bool ParseRotation(const std::string& s, Rotation4f& out);
        bool ParseColor(const std::string& s, Rgb& out);
        bool ParseColorRGBA(const std::string& s, Rgba& out);
        bool ParseFloatArray(const std::string& s, std::vector<float>& out);
        bool ParseIntArray(const std::string& s, std::vector<int>& out);
        bool ParseVec3fArray(const std::string& s, std::vector<Vec3>& out);

        // String utilities
        std::string Trim(const std::string& s);
        std::vector<std::string> Split(const std::string& s, char delim);
        std::string ToLower(const std::string& s);
        bool StartsWith(const std::string& s, const std::string& prefix);
        bool EndsWith(const std::string& s, const std::string& suffix);

        void SetError(const std::string& msg, int line = 0)
        {
            m_lastError = msg;
            m_lastLine = line;
        }
    };

    OBJECT_ENTRY_AUTO(__uuidof(X3DReader), X3DReader)

} // namespace HMREngine
