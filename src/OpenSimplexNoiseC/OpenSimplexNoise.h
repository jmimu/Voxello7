/**
  Open Simple Noise for C++

  Port to C++ from https://gist.github.com/KdotJPG/b1270127455a94ac5d19
  by Rickard Lundberg, 2019.

  C port by jmimu, 2022
*/
#pragma once
#include <stdint.h>

struct Noise
{
    short m_perm[256];
    short m_permGradIndex3d[256];
    char m_gradients2d[16];
    char m_gradients3d[72];
    char m_gradients4d[256];
};

struct Noise* createNoise();
struct Noise* createNoiseSeed(int64_t seed);
double eval2d(const struct Noise* noise, const double x, const double y);
double eval3d(const struct Noise* noise, double x, double y, double z);
double eval4d(const struct Noise* noise, double x, double y, double z, double w);
double extrapolate2d(const struct Noise* noise, int xsb, int ysb, double dx, double dy);
double extrapolate3d(const struct Noise* noise, int xsb, int ysb, int zsb, double dx, double dy, double dz);
double extrapolate4d(const struct Noise* noise, int xsb, int ysb, int zsb, int wsb, double dx, double dy, double dz, double dw);
