#include "pch.h"

// X3DReader.cpp - HMREngine::X3DReader implementation

#include "X3DReader.h"
#include "Scene.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

namespace HMREngine
{
    X3DReader::X3DReader()
    {
    }

    X3DReader::~X3DReader()
    {
    }

    std::vector<char> X3DReader::ReadFile(const std::string& path)
    {
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file.is_open()) return {};

        size_t size = static_cast<size_t>(file.tellg());
        std::vector<char> buffer(size);
        file.seekg(0);
        file.read(buffer.data(), size);
        return buffer;
    }

    std::string X3DReader::ExtractBasePath(const std::string& filePath)
    {
        size_t pos = filePath.find_last_of("/\\");
        return pos != std::string::npos ? filePath.substr(0, pos + 1) : "";
    }

    HRESULT X3DReader::LoadFromFile(const std::string& filePath, Scene* scene)
    {
        auto data = ReadFile(filePath);
        if (data.empty()) return E_FAIL;

        return LoadFromMemory(data.data(), data.size(), scene);
    }

    HRESULT X3DReader::LoadFromMemory(const char* data, size_t length, Scene* scene)
    {
        X3DParseContext ctx;
        ctx.scene = scene;
        ctx.basePath = "";

        return ParseX3D(data, length, ctx);
    }

    HRESULT X3DReader::ParseX3D(const char* data, size_t length, X3DParseContext& ctx)
    {
        std::string content(data, length);

        // Detect format
        if (content.find("<X3D") != std::string::npos || content.find("<Scene") != std::string::npos)
        {
            return ParseXML(data, length, ctx);
        }
        else if (content.find("#X3D") != std::string::npos || content.find("PROFILE") != std::string::npos)
        {
            return ParseX3DV1(data, length, ctx);
        }
        else if (content[0] == '{')
        {
            return ParseJSON(data, length, ctx);
        }

        return ParseX3DV1(data, length, ctx);
    }

    HRESULT X3DReader::ParseX3DV1(const char* data, size_t length, X3DParseContext& ctx)
    {
        std::string content(data, length);
        std::istringstream stream(content);
        std::string line;
        int lineNum = 0;

        while (std::getline(stream, line))
        {
            lineNum++;
            line = Trim(line);
            if (line.empty() || line[0] == '#') continue;

            if (!ParseNodeLine(line, ctx))
            {
                if (!ParseFieldLine(line, ctx.GetCurrentNode()))
                {
                    // Try as a node start
                    auto spacePos = line.find(' ');
                    std::string token = spacePos != std::string::npos ? line.substr(0, spacePos) : line;
                    token = ToLower(token);

                    if (token == "def" || token == "PROTO")
                    {
                        // Handle DEF and PROTO
                    }
                }
            }
        }

        return S_OK;
    }

    HRESULT X3DReader::ParseX3DV2(const char* data, size_t length, X3DParseContext& ctx)
    {
        return ParseX3DV1(data, length, ctx);
    }

    HRESULT X3DReader::ParseXML(const char* data, size_t length, X3DParseContext& ctx)
    {
        std::string content(data, length);

        // Create root layer
        CComObject<LayerNode>* rootLayer = nullptr;
        CComObject<LayerNode>::CreateInstance(&rootLayer);
        rootLayer->AddRef();
        ctx.PushNode(rootLayer);

        // Parse XML elements (simplified - in production would use MSXML)
        size_t pos = 0;
        while (pos < content.size())
        {
            size_t tagStart = content.find('<', pos);
            if (tagStart == std::string::npos) break;

            size_t tagEnd = content.find('>', tagStart);
            if (tagEnd == std::string::npos) break;

            std::string tag = content.substr(tagStart + 1, tagEnd - tagStart - 1);
            pos = tagEnd + 1;

            if (tag[0] == '/') continue; // closing tag

            // Extract tag name
            std::string tagName;
            auto spacePos = tag.find(' ');
            if (spacePos != std::string::npos)
                tagName = tag.substr(0, spacePos);
            else
                tagName = tag;

            if (tagName == "Scene" || tagName == "Group" || tagName == "Layer")
            {
                X3DChildNode* node = CreateNode(tagName);
                if (node)
                {
                    X3DChildNode* parent = ctx.GetCurrentNode();
                    if (parent) parent->AddChild(node);
                    ctx.PushNode(node);
                }
            }
        }

        // Set scene root
        if (ctx.scene && rootLayer)
        {
            ctx.scene->SetRootLayerSet(static_cast<LayerSetNode*>(rootLayer));
        }

        rootLayer->Release();
        return S_OK;
    }

    HRESULT X3DReader::ParseJSON(const char* data, size_t length, X3DParseContext& ctx)
    {
        std::string content(data, length);
        size_t pos = 0;

        auto skipWhitespace = [&]() {
            while (pos < content.size() && (content[pos] == ' ' || content[pos] == '\t' ||
                content[pos] == '\r' || content[pos] == '\n')) pos++;
        };

        auto readToken = [&]() -> std::string {
            skipWhitespace();
            if (pos >= content.size()) return "";
            if (content[pos] == '"') {
                pos++;
                size_t start = pos;
                while (pos < content.size() && content[pos] != '"') pos++;
                return content.substr(start, pos++ - start);
            }
            size_t start = pos;
            while (pos < content.size() && content[pos] != ',' && content[pos] != '}' &&
                   content[pos] != ']' && content[pos] != ' ' && content[pos] != '\n') pos++;
            return content.substr(start, pos - start);
        };

        auto expect = [&](char c) {
            skipWhitespace();
            if (pos < content.size() && content[pos] == c) { pos++; return true; }
            return false;
        };

        std::function<void(int)> parseObject;
        parseObject = [&](int depth) {
            if (depth >= static_cast<int>(X3DParseContext::kMaxNestingDepth))
                return;
            skipWhitespace();
            if (!expect('{')) return;
            while (pos < content.size()) {
                skipWhitespace();
                if (content[pos] == '}') { pos++; return; }
                std::string key = readToken();
                if (key.empty()) return;
                if (!expect(':')) return;
                skipWhitespace();
                if (content[pos] == '{') {
                    X3DChildNode* child = CreateNode(key);
                    if (child) {
                        X3DChildNode* parent = ctx.GetCurrentNode();
                        if (parent) parent->AddChild(child);
                        ctx.PushNode(child);
                        parseObject(depth + 1);
                        ctx.PopNode();
                    } else {
                        parseObject(depth + 1);
                    }
                } else if (content[pos] == '[') {
                    pos++;
                    while (pos < content.size() && content[pos] != ']') pos++;
                    if (pos < content.size()) pos++;
                } else {
                    std::string val = readToken();
                    if (ctx.GetCurrentNode()) {
                        X3DFieldNode* field = CreateField(key);
                        if (field) {
                            switch (field->GetFieldType()) {
                            case FieldType::SFFloat:
                            case FieldType::SFDouble: { float fv; if (ParseFloat(val, fv)) static_cast<SFFloat*>(field)->m_value = fv; break; }
                            case FieldType::SFInt32: { int iv; if (ParseInt(val, iv)) static_cast<SFInt32*>(field)->m_value = iv; break; }
                            case FieldType::SFBool: { bool bv; if (ParseBool(val, bv)) static_cast<SFBool*>(field)->m_value = bv; break; }
                            case FieldType::SFVec3f: { Vec3 vv; if (ParseVec3f(val, vv)) static_cast<SFVec3f*>(field)->m_value = vv; break; }
                            case FieldType::SFRotation: { Rotation4f rv; if (ParseRotation(val, rv)) static_cast<SFRotation*>(field)->m_value = rv; break; }
                            case FieldType::SFColor: { Rgb cv; if (ParseColor(val, cv)) static_cast<SFColor*>(field)->m_value = cv; break; }
                            case FieldType::SFString: static_cast<SFString*>(field)->m_value = val; break;
                            default: break;
                            }
                            ctx.GetCurrentNode()->SetField(key, field);
                            field->Release();
                        }
                    }
                }
                skipWhitespace();
                if (content[pos] == ',') pos++;
            }
        };

        parseObject(0);
        return S_OK;
    }

    bool X3DReader::ParseNodeLine(const std::string& line, X3DParseContext& ctx)
    {
        auto tokens = Split(line, ' ');
        if (tokens.empty()) return false;

        std::string nodeName = ToLower(tokens[0]);

        if (nodeName == "shape" || nodeName == "transform" || nodeName == "group" ||
            nodeName == "switch" || nodeName == "layer" || nodeName == "layerSet" ||
            nodeName == "viewport" || nodeName == "imagetexture" || nodeName == "material" ||
            nodeName == "appearance" || nodeName == "indexedfaceset" || nodeName == "coordinate" ||
            nodeName == "normal" || nodeName == "color" || nodeName == "texturecoordinate" ||
            nodeName == "timesensor" || nodeName == "positioninterpolator" ||
            nodeName == "orientationinterpolator" || nodeName == "scalarinterpolator" ||
            nodeName == "text" || nodeName == "fontstyle" || nodeName == "grid")
        {
            X3DChildNode* node = CreateNode(tokens[0]);
            if (!node) return false;

            // Check for DEF
            if (tokens.size() > 2 && ToLower(tokens[1]) == "def" && ctx.GetCurrentNode())
            {
                ctx.scene->RegisterNode(tokens[2], node);
            }

            X3DChildNode* parent = ctx.GetCurrentNode();
            if (parent) parent->AddChild(node);
            ctx.PushNode(node);

            return true;
        }
        else if (nodeName == "}")
        {
            ctx.PopNode();
            return true;
        }

        return false;
    }

    bool X3DReader::ParseFieldLine(const std::string& line, X3DChildNode* node)
    {
        if (!node) return false;

        auto eqPos = line.find('=');
        if (eqPos == std::string::npos) return false;

        std::string fieldName = Trim(line.substr(0, eqPos));
        std::string valueStr = Trim(line.substr(eqPos + 1));

        X3DFieldNode* field = CreateField(fieldName);
        if (!field) return false;

        // Parse value based on field type
        switch (field->GetFieldType())
        {
        case FieldType::SFBool:
        {
            bool val;
            if (ParseBool(valueStr, val)) static_cast<SFBool*>(field)->m_value = val;
            break;
        }
        case FieldType::SFFloat:
        {
            float val;
            if (ParseFloat(valueStr, val)) static_cast<SFFloat*>(field)->m_value = val;
            break;
        }
        case FieldType::SFInt32:
        {
            int val;
            if (ParseInt(valueStr, val)) static_cast<SFInt32*>(field)->m_value = val;
            break;
        }
        case FieldType::SFVec3f:
        {
            Vec3 val;
            if (ParseVec3f(valueStr, val)) static_cast<SFVec3f*>(field)->m_value = val;
            break;
        }
        case FieldType::SFRotation:
        {
            Rotation4f val;
            if (ParseRotation(valueStr, val)) static_cast<SFRotation*>(field)->m_value = val;
            break;
        }
        case FieldType::SFColor:
        {
            Rgb val;
            if (ParseColor(valueStr, val)) static_cast<SFColor*>(field)->m_value = val;
            break;
        }
        case FieldType::SFString:
        {
            static_cast<SFString*>(field)->m_value = valueStr;
            break;
        }
        default:
            break;
        }

        node->SetField(fieldName, field);
        field->Release();
        return true;
    }

    X3DChildNode* X3DReader::CreateNode(const std::string& typeName)
    {
        std::string lower = ToLower(typeName);

        if (lower == "group") { CComObject<GroupNode>* n; CComObject<GroupNode>::CreateInstance(&n); n->AddRef(); return n; }
        if (lower == "transform") { CComObject<TransformNode>* n; CComObject<TransformNode>::CreateInstance(&n); n->AddRef(); return n; }
        if (lower == "switch") { CComObject<SwitchNode>* n; CComObject<SwitchNode>::CreateInstance(&n); n->AddRef(); return n; }
        if (lower == "shape") { CComObject<ShapeNode>* n; CComObject<ShapeNode>::CreateInstance(&n); n->AddRef(); return n; }
        if (lower == "appearance") { CComObject<AppearanceNode>* n; CComObject<AppearanceNode>::CreateInstance(&n); n->AddRef(); return n; }
        if (lower == "material") { CComObject<MaterialNode>* n; CComObject<MaterialNode>::CreateInstance(&n); n->AddRef(); return n; }
        if (lower == "imagetexture") { CComObject<ImageTextureNode>* n; CComObject<ImageTextureNode>::CreateInstance(&n); n->AddRef(); return n; }
        if (lower == "movietexture") { CComObject<MovieTextureNode>* n; CComObject<MovieTextureNode>::CreateInstance(&n); n->AddRef(); return n; }
        if (lower == "motiontexture") { CComObject<MotionTextureNode>* n; CComObject<MotionTextureNode>::CreateInstance(&n); n->AddRef(); return n; }
        if (lower == "texturetransform") { CComObject<TextureTransformNode>* n; CComObject<TextureTransformNode>::CreateInstance(&n); n->AddRef(); return n; }
        if (lower == "textureproperties") { CComObject<TexturePropertiesNode>* n; CComObject<TexturePropertiesNode>::CreateInstance(&n); n->AddRef(); return n; }
        if (lower == "lineproperties") { CComObject<LinePropertiesNode>* n; CComObject<LinePropertiesNode>::CreateInstance(&n); n->AddRef(); return n; }
        if (lower == "fillproperties") { CComObject<FillPropertiesNode>* n; CComObject<FillPropertiesNode>::CreateInstance(&n); n->AddRef(); return n; }
        if (lower == "indexedfaceset") { CComObject<IndexedFaceSetNode>* n; CComObject<IndexedFaceSetNode>::CreateInstance(&n); n->AddRef(); return n; }
        if (lower == "coordinate") { CComObject<CoordinateNode>* n; CComObject<CoordinateNode>::CreateInstance(&n); n->AddRef(); return n; }
        if (lower == "normal") { CComObject<NormalNode>* n; CComObject<NormalNode>::CreateInstance(&n); n->AddRef(); return n; }
        if (lower == "color") { CComObject<ColorNode>* n; CComObject<ColorNode>::CreateInstance(&n); n->AddRef(); return n; }
        if (lower == "colorrgba") { CComObject<ColorRGBANode>* n; CComObject<ColorRGBANode>::CreateInstance(&n); n->AddRef(); return n; }
        if (lower == "texturecoordinate") { CComObject<TextureCoordinateNode>* n; CComObject<TextureCoordinateNode>::CreateInstance(&n); n->AddRef(); return n; }
        if (lower == "timesensor") { CComObject<TimeSensorNode>* n; CComObject<TimeSensorNode>::CreateInstance(&n); n->AddRef(); return n; }
        if (lower == "timetrigger") { CComObject<TimeTriggerNode>* n; CComObject<TimeTriggerNode>::CreateInstance(&n); n->AddRef(); return n; }
        if (lower == "positioninterpolator") { CComObject<PositionInterpolatorNode>* n; CComObject<PositionInterpolatorNode>::CreateInstance(&n); n->AddRef(); return n; }
        if (lower == "orientationinterpolator") { CComObject<OrientationInterpolatorNode>* n; CComObject<OrientationInterpolatorNode>::CreateInstance(&n); n->AddRef(); return n; }
        if (lower == "scalarinterpolator") { CComObject<ScalarInterpolatorNode>* n; CComObject<ScalarInterpolatorNode>::CreateInstance(&n); n->AddRef(); return n; }
        if (lower == "colorinterpolator") { CComObject<ColorInterpolatorNode>* n; CComObject<ColorInterpolatorNode>::CreateInstance(&n); n->AddRef(); return n; }
        if (lower == "coordinateinterpolator") { CComObject<CoordinateInterpolatorNode>* n; CComObject<CoordinateInterpolatorNode>::CreateInstance(&n); n->AddRef(); return n; }
        if (lower == "integersequencer") { CComObject<IntegerSequencerNode>* n; CComObject<IntegerSequencerNode>::CreateInstance(&n); n->AddRef(); return n; }
        if (lower == "booleansequencer") { CComObject<BooleanSequencerNode>* n; CComObject<BooleanSequencerNode>::CreateInstance(&n); n->AddRef(); return n; }
        if (lower == "text") { CComObject<TextNode>* n; CComObject<TextNode>::CreateInstance(&n); n->AddRef(); return n; }
        if (lower == "fontstyle") { CComObject<FontStyleNode>* n; CComObject<FontStyleNode>::CreateInstance(&n); n->AddRef(); return n; }
        if (lower == "grid") { CComObject<GridNode>* n; CComObject<GridNode>::CreateInstance(&n); n->AddRef(); return n; }
        if (lower == "layer") { CComObject<LayerNode>* n; CComObject<LayerNode>::CreateInstance(&n); n->AddRef(); return n; }
        if (lower == "layerset") { CComObject<LayerSetNode>* n; CComObject<LayerSetNode>::CreateInstance(&n); n->AddRef(); return n; }
        if (lower == "viewport") { CComObject<ViewportNode>* n; CComObject<ViewportNode>::CreateInstance(&n); n->AddRef(); return n; }
        if (lower == "metadatastring") { CComObject<MetadataStringNode>* n; CComObject<MetadataStringNode>::CreateInstance(&n); n->AddRef(); return n; }
        if (lower == "metadatadouble") { CComObject<MetadataDoubleNode>* n; CComObject<MetadataDoubleNode>::CreateInstance(&n); n->AddRef(); return n; }
        if (lower == "metadatafloat") { CComObject<MetadataFloatNode>* n; CComObject<MetadataFloatNode>::CreateInstance(&n); n->AddRef(); return n; }
        if (lower == "metadatainteger") { CComObject<MetadataIntegerNode>* n; CComObject<MetadataIntegerNode>::CreateInstance(&n); n->AddRef(); return n; }
        if (lower == "metadatabool") { CComObject<MetadataBoolNode>* n; CComObject<MetadataBoolNode>::CreateInstance(&n); n->AddRef(); return n; }
        if (lower == "metadataset") { CComObject<MetadataSetNode>* n; CComObject<MetadataSetNode>::CreateInstance(&n); n->AddRef(); return n; }
        if (lower == "lineproperties") { CComObject<LinePropertiesNode>* n; CComObject<LinePropertiesNode>::CreateInstance(&n); n->AddRef(); return n; }
        if (lower == "fillproperties") { CComObject<FillPropertiesNode>* n; CComObject<FillPropertiesNode>::CreateInstance(&n); n->AddRef(); return n; }

        return nullptr;
    }

    X3DFieldNode* X3DReader::CreateField(const std::string& typeName)
    {
        std::string lower = ToLower(typeName);
        CComObject<X3DFieldNode>* field = nullptr;

        if (lower == "diffusecolor" || lower == "specularcolor" || lower == "emissivecolor" || lower == "ambientcolor")
        {
            CComObject<SFColor>* f; CComObject<SFColor>::CreateInstance(&f); f->AddRef();
            return f;
        }
        if (lower == "transparency" || lower == "shininess")
        {
            CComObject<SFFloat>* f; CComObject<SFFloat>::CreateInstance(&f); f->AddRef();
            return f;
        }
        if (lower == "url" || lower == "string")
        {
            CComObject<SFString>* f; CComObject<SFString>::CreateInstance(&f); f->AddRef();
            return f;
        }
        if (lower == "translation" || lower == "center" || lower == "scale")
        {
            CComObject<SFVec3f>* f; CComObject<SFVec3f>::CreateInstance(&f); f->AddRef();
            return f;
        }
        if (lower == "rotation")
        {
            CComObject<SFRotation>* f; CComObject<SFRotation>::CreateInstance(&f); f->AddRef();
            return f;
        }
        if (lower == "enabled")
        {
            CComObject<SFBool>* f; CComObject<SFBool>::CreateInstance(&f); f->AddRef();
            return f;
        }
        if (lower == "coordindex" || lower == "normalindex" || lower == "texcoordindex")
        {
            CComObject<MFInt32>* f; CComObject<MFInt32>::CreateInstance(&f); f->AddRef();
            return f;
        }
        if (lower == "point" || lower == "vector")
        {
            CComObject<MFVec3f>* f; CComObject<MFVec3f>::CreateInstance(&f); f->AddRef();
            return f;
        }
        if (lower == "color")
        {
            CComObject<MFColor>* f; CComObject<MFColor>::CreateInstance(&f); f->AddRef();
            return f;
        }
        if (lower == "key")
        {
            CComObject<MFFloat>* f; CComObject<MFFloat>::CreateInstance(&f); f->AddRef();
            return f;
        }
        if (lower == "keyvalue")
        {
            CComObject<MFVec3f>* f; CComObject<MFVec3f>::CreateInstance(&f); f->AddRef();
            return f;
        }

        return nullptr;
    }

    // Value parsers
    bool X3DReader::ParseBool(const std::string& s, bool& out)
    {
        std::string l = ToLower(Trim(s));
        if (l == "true" || l == "1" || l == "yes") { out = true; return true; }
        if (l == "false" || l == "0" || l == "no") { out = false; return true; }
        return false;
    }

    bool X3DReader::ParseInt(const std::string& s, int& out)
    {
        try { out = std::stoi(Trim(s)); return true; }
        catch (...) { return false; }
    }

    bool X3DReader::ParseFloat(const std::string& s, float& out)
    {
        try { out = std::stof(Trim(s)); return true; }
        catch (...) { return false; }
    }

    bool X3DReader::ParseDouble(const std::string& s, double& out)
    {
        try { out = std::stod(Trim(s)); return true; }
        catch (...) { return false; }
    }

    bool X3DReader::ParseVec2f(const std::string& s, Vec2& out)
    {
        auto tokens = Split(Trim(s), ' ');
        if (tokens.size() >= 2)
        {
            float x, y;
            if (ParseFloat(tokens[0], x) && ParseFloat(tokens[1], y))
            {
                out = Vec2(x, y);
                return true;
            }
        }
        return false;
    }

    bool X3DReader::ParseVec3f(const std::string& s, Vec3& out)
    {
        auto tokens = Split(Trim(s), ' ');
        if (tokens.size() >= 3)
        {
            float x, y, z;
            if (ParseFloat(tokens[0], x) && ParseFloat(tokens[1], y) && ParseFloat(tokens[2], z))
            {
                out = Vec3(x, y, z);
                return true;
            }
        }
        return false;
    }

    bool X3DReader::ParseRotation(const std::string& s, Rotation4f& out)
    {
        auto tokens = Split(Trim(s), ' ');
        if (tokens.size() >= 4)
        {
            float x, y, z, w;
            if (ParseFloat(tokens[0], x) && ParseFloat(tokens[1], y) &&
                ParseFloat(tokens[2], z) && ParseFloat(tokens[3], w))
            {
                out = Rotation4f(x, y, z, w);
                return true;
            }
        }
        return false;
    }

    bool X3DReader::ParseColor(const std::string& s, Rgb& out)
    {
        auto tokens = Split(Trim(s), ' ');
        if (tokens.size() >= 3)
        {
            float r, g, b;
            if (ParseFloat(tokens[0], r) && ParseFloat(tokens[1], g) && ParseFloat(tokens[2], b))
            {
                out = Rgb(r, g, b);
                return true;
            }
        }
        return false;
    }

    bool X3DReader::ParseColorRGBA(const std::string& s, Rgba& out)
    {
        auto tokens = Split(Trim(s), ' ');
        if (tokens.size() >= 4)
        {
            float r, g, b, a;
            if (ParseFloat(tokens[0], r) && ParseFloat(tokens[1], g) &&
                ParseFloat(tokens[2], b) && ParseFloat(tokens[3], a))
            {
                out = Rgba(r, g, b, a);
                return true;
            }
        }
        return false;
    }

    bool X3DReader::ParseFloatArray(const std::string& s, std::vector<float>& out)
    {
        auto tokens = Split(Trim(s), ' ');
        for (auto& t : tokens)
        {
            float v;
            if (ParseFloat(t, v)) out.push_back(v);
        }
        return !out.empty();
    }

    bool X3DReader::ParseIntArray(const std::string& s, std::vector<int>& out)
    {
        auto tokens = Split(Trim(s), ' ');
        for (auto& t : tokens)
        {
            int v;
            if (ParseInt(t, v)) out.push_back(v);
        }
        return !out.empty();
    }

    bool X3DReader::ParseVec3fArray(const std::string& s, std::vector<Vec3>& out)
    {
        auto tokens = Split(Trim(s), ' ');
        for (size_t i = 0; i + 2 < tokens.size(); i += 3)
        {
            float x, y, z;
            if (ParseFloat(tokens[i], x) && ParseFloat(tokens[i + 1], y) && ParseFloat(tokens[i + 2], z))
            {
                out.push_back(Vec3(x, y, z));
            }
        }
        return !out.empty();
    }

    HRESULT X3DReader::SaveToFile(const std::string& filePath, Scene* scene)
    {
        if (!scene) return E_POINTER;

        std::ofstream file(filePath);
        if (!file.is_open()) return E_FAIL;

        file << "#X3D V3.0 utf8\n";
        file << "PROFILE Immersive\n";
        file << "BEGIN Scene\n";

        // Serialize scene graph
        auto* rootLayerSet = scene->GetRootLayerSet();
        if (rootLayerSet)
        {
            for (size_t i = 0; i < rootLayerSet->GetNumChildren(); i++)
            {
                auto* child = rootLayerSet->GetChild(i);
                if (child)
                {
                    file << child->GetNodeTypeName() << " {\n";
                    file << "}\n";
                }
            }
        }

        file << "END\n";
        return S_OK;
    }

    // String utilities
    std::string X3DReader::Trim(const std::string& s)
    {
        size_t start = s.find_first_not_of(" \t\r\n");
        size_t end = s.find_last_not_of(" \t\r\n");
        return start != std::string::npos ? s.substr(start, end - start + 1) : "";
    }

    std::vector<std::string> X3DReader::Split(const std::string& s, char delim)
    {
        std::vector<std::string> tokens;
        std::istringstream iss(s);
        std::string token;
        while (std::getline(iss, token, delim))
        {
            auto trimmed = Trim(token);
            if (!trimmed.empty()) tokens.push_back(trimmed);
        }
        return tokens;
    }

    std::string X3DReader::ToLower(const std::string& s)
    {
        std::string result = s;
        std::transform(result.begin(), result.end(), result.begin(), ::tolower);
        return result;
    }

    bool X3DReader::StartsWith(const std::string& s, const std::string& prefix)
    {
        return s.size() >= prefix.size() && s.compare(0, prefix.size(), prefix) == 0;
    }

    bool X3DReader::EndsWith(const std::string& s, const std::string& suffix)
    {
        return s.size() >= suffix.size() && s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0;
    }

    HRESULT X3DReader::ParseXMLElement(void* xmlNode, X3DParseContext& ctx)
    {
        // xmlNode is actually IXmlReader* cast to void*
        if (!xmlNode || !ctx.scene) return E_POINTER;

        IXmlReader* reader = static_cast<IXmlReader*>(xmlNode);
        XmlNodeType nodeType;
        const WCHAR* localName = nullptr;

        while (reader->Read(&nodeType) == S_OK)
        {
            if (nodeType == XmlNodeType_Element)
            {
                reader->GetLocalName(&localName, nullptr);
                if (!localName) continue;

                int charCount = 0;
                reader->GetLocalNameLength(&localName, &charCount);
                // Convert wide to narrow
                std::string tagName(charCount, '\0');
                for (int i = 0; i < charCount; i++)
                    tagName[i] = static_cast<char>(localName[i]);

                std::string lower = ToLower(tagName);
                X3DChildNode* node = CreateNode(tagName);
                if (node)
                {
                    X3DChildNode* parent = ctx.GetCurrentNode();
                    if (parent) parent->AddChild(node);
                    ctx.PushNode(node);

                    // Read attributes
                    HRESULT hr = reader->MoveToFirstAttribute();
                    while (SUCCEEDED(hr))
                    {
                        const WCHAR* attrName = nullptr;
                        const WCHAR* attrValue = nullptr;
                        reader->GetLocalName(&attrName, nullptr);
                        reader->GetValue(&attrValue, nullptr);

                        if (attrName && attrValue)
                        {
                            std::string aName, aVal;
                            int aLen = 0;
                            reader->GetLocalNameLength(&attrName, &aLen);
                            aName.resize(aLen);
                            for (int i = 0; i < aLen; i++) aName[i] = static_cast<char>(attrName[i]);

                            int vLen = 0;
                            reader->GetValueLength(&attrValue, &vLen);
                            aVal.resize(vLen);
                            for (int i = 0; i < vLen; i++) aVal[i] = static_cast<char>(attrValue[i]);

                            X3DFieldNode* field = CreateField(aName);
                            if (field)
                            {
                                switch (field->GetFieldType())
                                {
                                case FieldType::SFFloat: { float fv; if (ParseFloat(aVal, fv)) static_cast<SFFloat*>(field)->m_value = fv; break; }
                                case FieldType::SFInt32: { int iv; if (ParseInt(aVal, iv)) static_cast<SFInt32*>(field)->m_value = iv; break; }
                                case FieldType::SFBool: { bool bv; if (ParseBool(aVal, bv)) static_cast<SFBool*>(field)->m_value = bv; break; }
                                case FieldType::SFVec3f: { Vec3 vv; if (ParseVec3f(aVal, vv)) static_cast<SFVec3f*>(field)->m_value = vv; break; }
                                case FieldType::SFRotation: { Rotation4f rv; if (ParseRotation(aVal, rv)) static_cast<SFRotation*>(field)->m_value = rv; break; }
                                case FieldType::SFColor: { Rgb cv; if (ParseColor(aVal, cv)) static_cast<SFColor*>(field)->m_value = cv; break; }
                                case FieldType::SFString: static_cast<SFString*>(field)->m_value = aVal; break;
                                default: break;
                                }
                                node->SetField(aName, field);
                                field->Release();
                            }
                        }
                        hr = reader->MoveToNextAttribute();
                    }
                    reader->MoveToElement();
                }
            }
            else if (nodeType == XmlNodeTypeEndElement)
            {
                ctx.PopNode();
            }
        }

        return S_OK;
    }

    X3DChildNode* X3DReader::CreateNodeFromXML(void* xmlElement)
    {
        if (!xmlElement) return nullptr;

        IXmlReader* reader = static_cast<IXmlReader*>(xmlElement);
        const WCHAR* localName = nullptr;
        int charCount = 0;
        reader->GetLocalName(&localName, &charCount);
        if (!localName || charCount == 0) return nullptr;

        std::string tagName(charCount, '\0');
        for (int i = 0; i < charCount; i++)
            tagName[i] = static_cast<char>(localName[i]);

        return CreateNode(tagName);
    }

} // namespace HMREngine
