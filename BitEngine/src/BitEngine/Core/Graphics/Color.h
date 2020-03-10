#pragma once

namespace BitEngine {



template<unsigned channels, unsigned R, unsigned G, unsigned B, unsigned A>
struct Color
{
    static_assert(R != G && G != B && B != A, "Invalid color index");

public:
    Color()
        : Color(0, 0, 0, 0)
    {}
    Color(float r, float g, float b, float a) {
        this->r(r);
        this->g(g);
        this->b(b);
        this->a(a);
    }
    enum Channel { RED = R, GREEN = G, BLUE = B, ALPHA = A, CHANNELS = channels };
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

private:
    float colors[channels];
};
typedef Color<4, 0, 1, 2, 3> ColorRGBA;
typedef Color<3, 0, 1, 2, 3> ColorRGB;

}