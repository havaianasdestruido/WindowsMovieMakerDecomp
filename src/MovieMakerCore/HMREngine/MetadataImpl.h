#pragma once
// MetadataImpl.h - MetadataT, MetadataSet, MetadataString/Double/Float/Integer/Bool

#include "HMREngine.h"
#include "X3DNodes.h"
#include <string>
#include <vector>

namespace HMREngine
{
    class MetadataStringImpl : public ImplObject<MetadataStringImpl>
    {
    public:
        HRESULT Initialize(MetadataStringNode* node);
        void Shutdown();

        const std::string& GetName() const { return m_name; }
        const std::string& GetReference() const { return m_reference; }
        const std::string& GetValue() const { return m_value; }
        const MFString& GetValues() const { return m_values; }

    private:
        std::string m_name;
        std::string m_reference;
        std::string m_value;
        MFString m_values;
    };

    class MetadataDoubleImpl : public ImplObject<MetadataDoubleImpl>
    {
    public:
        HRESULT Initialize(MetadataDoubleNode* node);
        void Shutdown();

        const std::string& GetName() const { return m_name; }
        double GetValue() const { return m_value; }
        const MFDouble& GetValues() const { return m_values; }

    private:
        std::string m_name;
        std::string m_reference;
        double m_value = 0.0;
        MFDouble m_values;
    };

    class MetadataFloatImpl : public ImplObject<MetadataFloatImpl>
    {
    public:
        HRESULT Initialize(MetadataFloatNode* node);
        void Shutdown();

        const std::string& GetName() const { return m_name; }
        float GetValue() const { return m_value; }
        const MFFloat& GetValues() const { return m_values; }

    private:
        std::string m_name;
        std::string m_reference;
        float m_value = 0.0f;
        MFFloat m_values;
    };

    class MetadataIntegerImpl : public ImplObject<MetadataIntegerImpl>
    {
    public:
        HRESULT Initialize(MetadataIntegerNode* node);
        void Shutdown();

        const std::string& GetName() const { return m_name; }
        int GetValue() const { return m_value; }
        const MFInt32& GetValues() const { return m_values; }

    private:
        std::string m_name;
        std::string m_reference;
        int m_value = 0;
        MFInt32 m_values;
    };

    class MetadataBoolImpl : public ImplObject<MetadataBoolImpl>
    {
    public:
        HRESULT Initialize(MetadataBoolNode* node);
        void Shutdown();

        const std::string& GetName() const { return m_name; }
        bool GetValue() const { return m_value; }
        const MFBool& GetValues() const { return m_values; }

    private:
        std::string m_name;
        std::string m_reference;
        bool m_value = false;
        MFBool m_values;
    };

    class MetadataSetImpl : public ImplObject<MetadataSetImpl>
    {
    public:
        MetadataSetImpl();
        virtual ~MetadataSetImpl();

        HRESULT Initialize(MetadataSetNode* node);
        void Shutdown();

        const std::string& GetName() const { return m_name; }
        size_t GetNumMetadata() const { return m_metadata.size(); }
        X3DChildNode* GetMetadata(size_t index) const;

    private:
        std::string m_name;
        std::string m_reference;
        std::vector<SAIPtr<X3DChildNode>> m_metadata;
    };

} // namespace HMREngine
