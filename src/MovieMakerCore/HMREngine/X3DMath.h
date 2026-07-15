#pragma once
// X3DMath.h - Math types for HMREngine

#include <DirectXMath.h>
#include <cmath>
#include <algorithm>

namespace HMREngine
{
    using namespace DirectX;

    struct Vec2 : public XMFLOAT2
    {
        Vec2() : XMFLOAT2(0.0f, 0.0f) {}
        Vec2(float x, float y) : XMFLOAT2(x, y) {}
        Vec2(const XMFLOAT2& v) : XMFLOAT2(v) {}
        Vec2(const XMVECTOR& v) { XMStoreFloat2(this, v); }

        Vec2 operator+(const Vec2& o) const { return Vec2(x + o.x, y + o.y); }
        Vec2 operator-(const Vec2& o) const { return Vec2(x - o.x, y - o.y); }
        Vec2 operator*(float s) const { return Vec2(x * s, y * s); }
        Vec2 operator/(float s) const { float inv = 1.0f / s; return Vec2(x * inv, y * inv); }
        bool operator==(const Vec2& o) const { return x == o.x && y == o.y; }
        bool operator!=(const Vec2& o) const { return !(*this == o); }

        float Length() const { return sqrtf(x * x + y * y); }
        float LengthSq() const { return x * x + y * y; }
        Vec2 Normalized() const { float l = Length(); return l > 0 ? *this / l : Vec2(); }
        float Dot(const Vec2& o) const { return x * o.x + y * o.y; }
    };

    struct Vec3 : public XMFLOAT3
    {
        Vec3() : XMFLOAT3(0.0f, 0.0f, 0.0f) {}
        Vec3(float x, float y, float z) : XMFLOAT3(x, y, z) {}
        Vec3(const XMFLOAT3& v) : XMFLOAT3(v) {}
        Vec3(const XMVECTOR& v) { XMStoreFloat3(this, v); }

        Vec3 operator+(const Vec3& o) const { return Vec3(x + o.x, y + o.y, z + o.z); }
        Vec3 operator-(const Vec3& o) const { return Vec3(x - o.x, y - o.y, z - o.z); }
        Vec3 operator*(float s) const { return Vec3(x * s, y * s, z * s); }
        Vec3 operator/(float s) const { float inv = 1.0f / s; return Vec3(x * inv, y * inv, z * inv); }
        Vec3 operator-() const { return Vec3(-x, -y, -z); }
        bool operator==(const Vec3& o) const { return x == o.x && y == o.y && z == o.z; }
        bool operator!=(const Vec3& o) const { return !(*this == o); }

        float Length() const { return sqrtf(x * x + y * y + z * z); }
        float LengthSq() const { return x * x + y * y + z * z; }
        Vec3 Normalized() const { float l = Length(); return l > 0 ? *this / l : Vec3(); }
        float Dot(const Vec3& o) const { return x * o.x + y * o.y + z * o.z; }
        Vec3 Cross(const Vec3& o) const
        {
            return Vec3(y * o.z - z * o.y, z * o.x - x * o.z, x * o.y - y * o.x);
        }
    };

    struct Vec4 : public XMFLOAT4
    {
        Vec4() : XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f) {}
        Vec4(float x, float y, float z, float w) : XMFLOAT4(x, y, z, w) {}
        Vec4(const XMFLOAT4& v) : XMFLOAT4(v) {}
        Vec4(const XMVECTOR& v) { XMStoreFloat4(this, v); }
        Vec4(const Vec3& v, float w) : XMFLOAT4(v.x, v.y, v.z, w) {}

        Vec4 operator+(const Vec4& o) const { return Vec4(x + o.x, y + o.y, z + o.z, w + o.w); }
        Vec4 operator-(const Vec4& o) const { return Vec4(x - o.x, y - o.y, z - o.z, w - o.w); }
        Vec4 operator*(float s) const { return Vec4(x * s, y * s, z * s, w * s); }
        bool operator==(const Vec4& o) const { return x == o.x && y == o.y && z == o.z && w == o.w; }
        bool operator!=(const Vec4& o) const { return !(*this == o); }

        Vec3 ToVec3() const { return Vec3(x, y, z); }
    };

    struct Matrix4f;

    struct Rotation4f
    {
        float x, y, z, w;

        Rotation4f() : x(0.0f), y(0.0f), z(0.0f), w(1.0f) {}
        Rotation4f(float ix, float iy, float iz, float iw) : x(ix), y(iy), z(iz), w(iw) {}
        Rotation4f(const XMFLOAT4& q) : x(q.x), y(q.y), z(q.z), w(q.w) {}

        static Rotation4f FromAxisAngle(const Vec3& axis, float radians)
        {
            XMVECTOR v = XMQuaternionRotationAxis(XMLoadFloat3(&axis), radians);
            Rotation4f r;
            XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&r), v);
            return r;
        }

        static Rotation4f FromEulerAngles(float pitch, float yaw, float roll)
        {
            XMVECTOR v = XMQuaternionRotationRollPitchYaw(pitch, yaw, roll);
            Rotation4f r;
            XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&r), v);
            return r;
        }

        static Rotation4f FromMatrix(const Matrix4f& m);

        Rotation4f operator*(const Rotation4f& o) const
        {
            XMVECTOR v1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(this));
            XMVECTOR v2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&o));
            XMVECTOR r = XMQuaternionMultiply(v1, v2);
            Rotation4f result;
            XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), r);
            return result;
        }

        Vec3 Rotate(const Vec3& v) const
        {
            XMVECTOR q = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(this));
            XMVECTOR vec = XMLoadFloat3(&v);
            XMVECTOR rotated = XMVector3Rotate(vec, q);
            return Vec3(rotated);
        }

        Rotation4f Conjugate() const { return Rotation4f(-x, -y, -z, w); }
        Rotation4f Inverse() const { return Conjugate(); }

        float Length() const { return sqrtf(x * x + y * y + z * z + w * w); }
        Rotation4f Normalized() const
        {
            float l = Length();
            if (l > 0) { float inv = 1.0f / l; return Rotation4f(x * inv, y * inv, z * inv, w * inv); }
            return Rotation4f();
        }
    };

    struct Matrix3f
    {
        float m[3][3];

        Matrix3f() { memset(m, 0, sizeof(m)); m[0][0] = m[1][1] = m[2][2] = 1.0f; }

        static Matrix3f Identity() { return Matrix3f(); }

        static Matrix3f FromRotation(const Rotation4f& q)
        {
            Matrix3f r;
            float xx = q.x * q.x, yy = q.y * q.y, zz = q.z * q.z;
            float xy = q.x * q.y, xz = q.x * q.z, yz = q.y * q.z;
            float wx = q.w * q.x, wy = q.w * q.y, wz = q.w * q.z;
            r.m[0][0] = 1 - 2 * (yy + zz); r.m[0][1] = 2 * (xy - wz);     r.m[0][2] = 2 * (xz + wy);
            r.m[1][0] = 2 * (xy + wz);     r.m[1][1] = 1 - 2 * (xx + zz); r.m[1][2] = 2 * (yz - wx);
            r.m[2][0] = 2 * (xz - wy);     r.m[2][1] = 2 * (yz + wx);     r.m[2][2] = 1 - 2 * (xx + yy);
            return r;
        }

        Vec3 operator*(const Vec3& v) const
        {
            return Vec3(
                m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z,
                m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z,
                m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z
            );
        }
    };

    struct Matrix4f : public XMFLOAT4X4
    {
        Matrix4f() { XMStoreFloat4x4(this, XMMatrixIdentity()); }

        static Matrix4f Identity() { return Matrix4f(); }

        static Matrix4f FromTranslation(const Vec3& t)
        {
            Matrix4f r;
            XMStoreFloat4x4(&r, XMMatrixTranslation(t.x, t.y, t.z));
            return r;
        }

        static Matrix4f FromScaling(const Vec3& s)
        {
            Matrix4f r;
            XMStoreFloat4x4(&r, XMMatrixScaling(s.x, s.y, s.z));
            return r;
        }

        static Matrix4f FromRotation(const Rotation4f& q)
        {
            Matrix4f r;
            XMStoreFloat4x4(&r, XMMatrixRotationQuaternion(XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&q))));
            return r;
        }

        static Matrix4f FromTranslationRotationScale(const Vec3& t, const Rotation4f& q, const Vec3& s)
        {
            Matrix4f r;
            XMStoreFloat4x4(&r,
                XMMatrixScaling(s.x, s.y, s.z) *
                XMMatrixRotationQuaternion(XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&q))) *
                XMMatrixTranslation(t.x, t.y, t.z)
            );
            return r;
        }

        static Matrix4f LookAtLH(const Vec3& eye, const Vec3& target, const Vec3& up)
        {
            Matrix4f r;
            XMStoreFloat4x4(&r, XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up)));
            return r;
        }

        static Matrix4f PerspectiveFovLH(float fovY, float aspect, float zn, float zf)
        {
            Matrix4f r;
            XMStoreFloat4x4(&r, XMMatrixPerspectiveFovLH(fovY, aspect, zn, zf));
            return r;
        }

        static Matrix4f OrthoLH(float w, float h, float zn, float zf)
        {
            Matrix4f r;
            XMStoreFloat4x4(&r, XMMatrixOrthographicLH(w, h, zn, zf));
            return r;
        }

        Matrix4f operator*(const Matrix4f& o) const
        {
            Matrix4f r;
            XMMATRIX m1 = XMLoadFloat4x4(this);
            XMMATRIX m2 = XMLoadFloat4x4(&o);
            XMStoreFloat4x4(&r, XMMatrixMultiply(m1, m2));
            return r;
        }

        Vec3 TransformPoint(const Vec3& p) const
        {
            XMMATRIX m = XMLoadFloat4x4(this);
            XMVECTOR v = XMLoadFloat3(&p);
            XMVECTOR result = XMVector3TransformCoord(v, m);
            return Vec3(result);
        }

        Vec3 TransformDirection(const Vec3& d) const
        {
            XMMATRIX m = XMLoadFloat4x4(this);
            XMVECTOR v = XMLoadFloat3(&d);
            XMVECTOR result = XMVector3TransformNormal(v, m);
            return Vec3(result);
        }

        Matrix4f Inverted() const
        {
            Matrix4f r;
            XMMATRIX m = XMLoadFloat4x4(this);
            XMVECTOR det;
            XMStoreFloat4x4(&r, XMMatrixInverse(&det, m));
            return r;
        }

        Matrix4f Transposed() const
        {
            Matrix4f r;
            XMMATRIX m = XMLoadFloat4x4(this);
            XMStoreFloat4x4(&r, XMMatrixTranspose(m));
            return r;
        }

        const float* Data() const { return &m[0][0]; }
    };

    inline Rotation4f Rotation4f::FromMatrix(const Matrix4f& mat)
    {
        XMMATRIX m = XMLoadFloat4x4(&mat);
        XMVECTOR q = XMQuaternionRotationMatrix(m);
        Rotation4f r;
        XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&r), q);
        return r;
    }

    struct Rgb
    {
        float r, g, b;

        Rgb() : r(0.0f), g(0.0f), b(0.0f) {}
        Rgb(float rr, float gg, float bb) : r(rr), g(gg), b(bb) {}

        Rgb operator+(const Rgb& o) const { return Rgb(r + o.r, g + o.g, b + o.b); }
        Rgb operator-(const Rgb& o) const { return Rgb(r - o.r, g - o.g, b - o.b); }
        Rgb operator*(float s) const { return Rgb(r * s, g * s, b * s); }
        Rgb operator*(const Rgb& o) const { return Rgb(r * o.r, g * o.g, b * o.b); }

        Rgb Lerp(const Rgb& o, float t) const
        {
            return Rgb(r + (o.r - r) * t, g + (o.g - g) * t, b + (o.b - b) * t);
        }

        bool operator==(const Rgb& o) const { return r == o.r && g == o.g && b == o.b; }

        Vec3 ToVec3() const { return Vec3(r, g, b); }
    };

    struct Rgba : public XMFLOAT4
    {
        Rgba() : XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) {}
        Rgba(float r, float g, float b, float a) : XMFLOAT4(r, g, b, a) {}
        Rgba(const Rgb& rgb, float alpha) : XMFLOAT4(rgb.r, rgb.g, rgb.b, alpha) {}

        Rgba operator+(const Rgba& o) const { return Rgba(x + o.x, y + o.y, z + o.z, w + o.w); }
        Rgba operator*(float s) const { return Rgba(x * s, y * s, z * s, w * s); }

        Rgba Lerp(const Rgba& o, float t) const
        {
            return Rgba(x + (o.x - x) * t, y + (o.y - y) * t, z + (o.z - z) * t, w + (o.w - w) * t);
        }

        bool operator==(const Rgba& o) const { return x == o.x && y == o.y && z == o.z && w == o.w; }

        Rgb ToRgb() const { return Rgb(x, y, z); }

        static Rgba FromColorREF(COLORREF c, float alpha = 1.0f)
        {
            return Rgba(
                (float)(GetRValue(c)) / 255.0f,
                (float)(GetGValue(c)) / 255.0f,
                (float)(GetBValue(c)) / 255.0f,
                alpha
            );
        }
    };

    struct Frustum
    {
        Vec4 planes[6]; // left, right, bottom, top, near, far

        void ExtractFromVP(const Matrix4f& viewProj)
        {
            // Left
            planes[0] = Vec4(
                viewProj._14 + viewProj._11, viewProj._24 + viewProj._21,
                viewProj._34 + viewProj._31, viewProj._44 + viewProj._41);
            // Right
            planes[1] = Vec4(
                viewProj._14 - viewProj._11, viewProj._24 - viewProj._21,
                viewProj._34 - viewProj._31, viewProj._44 - viewProj._41);
            // Bottom
            planes[2] = Vec4(
                viewProj._14 + viewProj._12, viewProj._24 + viewProj._22,
                viewProj._34 + viewProj._32, viewProj._44 + viewProj._42);
            // Top
            planes[3] = Vec4(
                viewProj._14 - viewProj._12, viewProj._24 - viewProj._22,
                viewProj._34 - viewProj._32, viewProj._44 - viewProj._42);
            // Near
            planes[4] = Vec4(
                viewProj._13, viewProj._23, viewProj._33, viewProj._43);
            // Far
            planes[5] = Vec4(
                viewProj._14 - viewProj._13, viewProj._24 - viewProj._23,
                viewProj._34 - viewProj._33, viewProj._44 - viewProj._43);

            for (int i = 0; i < 6; i++)
            {
                float len = sqrtf(planes[i].x * planes[i].x + planes[i].y * planes[i].y + planes[i].z * planes[i].z);
                if (len > 0) { planes[i] = planes[i] * (1.0f / len); }
            }
        }

        bool IsBoxInside(const Vec3& center, const Vec3& extents) const
        {
            for (int i = 0; i < 6; i++)
            {
                float dist = planes[i].x * center.x + planes[i].y * center.y + planes[i].z * center.z + planes[i].w;
                float radius = fabsf(planes[i].x) * extents.x + fabsf(planes[i].y) * extents.y + fabsf(planes[i].z) * extents.z;
                if (dist + radius < 0) return false;
            }
            return true;
        }
    };

    inline float Lerp(float a, float b, float t) { return a + (b - a) * t; }
    inline float Clamp(float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }
    inline float Saturate(float v) { return Clamp(v, 0.0f, 1.0f); }
    inline float SmoothStep(float edge0, float edge1, float x)
    {
        float t = Saturate((x - edge0) / (edge1 - edge0));
        return t * t * (3.0f - 2.0f * t);
    }
    inline float DegreesToRadians(float d) { return d * 3.14159265358979323846f / 180.0f; }
    inline float RadiansToDegrees(float r) { return r * 180.0f / 3.14159265358979323846f; }

} // namespace HMREngine
