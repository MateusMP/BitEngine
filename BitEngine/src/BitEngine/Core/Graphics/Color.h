#pragma once

namespace BitEngine {


template<unsigned R, unsigned G, unsigned B, unsigned A>
struct ColorRGBA_
{
    static_assert(R != G && G != B && B != A, "Invalid color index");

public:
    ColorRGBA_()
        : ColorRGB_(0, 0, 0, 0)
    {}

    ColorRGBA_(float r, float g, float b, float a) {
        this->r(r);
        this->g(g);
        this->b(b);
        this->a(a);
    }
    enum Channel { RED = R, GREEN = G, BLUE = B, ALPHA = A, CHANNELS = 4 };
    float& operator [] (Channel x) {
        return colors[x];
    }
    inline float r() const { return colors[R]; }
    inline float g() const { return colors[G]; }
    inline float b() const { return colors[B]; }
    inline float a() const { return colors[A]; }
    inline float r(float x) { return colors[R] = x; }
    inline float g(float x) { return colors[G] = x; }
    inline float b(float x) { return colors[B] = x; }
    inline float a(float x) { return colors[A] = x; }

    float colors[4];
};

template<unsigned R, unsigned G, unsigned B>
struct ColorRGB_
{
    static_assert(R != G && G != B, "Invalid color index");

public:
    ColorRGB_()
        : ColorRGB_(0, 0, 0)
    {}
    
    ColorRGB_(float r, float g, float b) {
        this->r(r);
        this->g(g);
        this->b(b);
    }
    enum Channel { RED = R, GREEN = G, BLUE = B, CHANNELS = 3 };
    float& operator [] (Channel x) {
        return colors[x];
    }
    inline float r() const { return colors[R]; }
    inline float g() const { return colors[G]; }
    inline float b() const { return colors[B]; }
    inline float r(float x) { return colors[R] = x; }
    inline float g(float x) { return colors[G] = x; }
    inline float b(float x) { return colors[B] = x; }

    float colors[3];
};
typedef ColorRGBA_<0, 1, 2, 3> ColorRGBA;
typedef ColorRGB_<0, 1, 2> ColorRGB;

}