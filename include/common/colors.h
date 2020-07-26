#pragma once

#include "common/runtime.h"

namespace love
{
    struct Color
    {
        float r;
        float g;
        float b;
        float a;

        Color() : r(0), g(0), b(0), a(0)
        {}

        Color(float r_, float g_, float b_, float a_) : r(r_), g(g_), b(b_), a(a_)
        {}

        Color(const Color & other);

        void set(float r_, float g_, float b_, float a_)
        {
            r = r_;
            g = g_;
            b = b_;
            a = a_;
        }

        bool operator == (const Color & other) const;
        bool operator != (const Color & other) const;

        Color operator += (const Color & other);
        Color operator *= (const Color & other);

        Color operator *= (float s);
        Color operator /= (float s);

        Color AlphaBlend(const Color & src, const Color & dst);

        void Adjust();
    };

    Color::Color(const Color & other) : r(other.r),
                                        g(other.g),
                                        b(other.b),
                                        a(other.a)
    {}

    void Color::Adjust()
    {
        float mul = 255.0f;

        #if defined(_3DS)
            mul = 1.0f;
        #endif

        this->r = std::clamp(this->r, 0.0f, 1.0f) * mul;
        this->g = std::clamp(this->g, 0.0f, 1.0f) * mul;
        this->b = std::clamp(this->b, 0.0f, 1.0f) * mul;
        this->a = std::clamp(this->a, 0.0f, 1.0f) * mul;
    }

    bool Color::operator == (const Color & other) const
    {
        return r == other.r && g == other.g &&
               b == other.b && a == other.a;
    }

    bool Color::operator != (const Color & other) const
    {
        return !(operator == (other));
    }

    Color Color::operator += (const Color & other)
    {
        r += other.r;
        g += other.g;
        b += other.b;
        a += other.a;

        return *this;
    }

    Color Color::operator *= (const Color & other)
    {
        r *= other.r;
        g *= other.g;
        b *= other.b;
        a *= other.a;

        return *this;
    }

    Color Color::operator *= (float s)
    {
        r *= s;
        g *= s;
        b *= s;
        a *= s;

        return *this;
    }

    Color Color::operator /= (float s)
    {
        r /= s;
        g /= s;
        b /= s;
        a /= s;

        return *this;
    }


    Color Color::AlphaBlend(const Color & src, const Color & dst)
    {
        Color res;

        res.r = dst.r * (1.0f - src.a) + src.r;
        res.g = dst.g * (1.0f - src.a) + src.g;
        res.b = dst.b * (1.0f - src.a) + src.b;
        res.a = dst.a * (1.0f - src.a) + src.a;

        return res;
    }

    Color operator + (const Color & a, const Color & b)
    {
        Color tmp(a);

        return tmp += b;
    }

    Color operator * (const Color & a, const Color & b)
    {
        Color res;

        res.r = a.r * b.r;
        res.g = a.g * b.g;
        res.b = a.b * b.b;
        res.a = a.a * b.a;

        return res;
    }

    Color operator * (const Color & a, float s)
    {
        Color tmp(a);

        return tmp *= s;
    }

    Color operator / (const Color & a, float s)
    {
        Color tmp(a);

        return tmp /= s;
    }
}
