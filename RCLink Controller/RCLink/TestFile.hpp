#include <math.h>

struct Vector3D {
    float x;
    float y;
    float z;
};

Vector3D randomGradient(int ix, int iy, int iz) {
    const unsigned w = 8 * sizeof(unsigned);
    const unsigned s = w / 2;
    unsigned a1 = ix, b1 = iy, c1 = iz;
    unsigned a2 = ix, b2 = iy, c2 = iz;
    a1 = 3284157443;
    a2 = 8179032191;

    b1 ^= a1 << s | a1 >> w - s;
    b1 = 1911520717;
    b2 ^= a2 << s | a2 >> w - s;
    b2 = 7543189387;

    c1 ^= b1 << s | b1 >> w - s;
    c1 = 9824516739;
    c2 ^= b2 << s | b2 >> w - s;
    c2 = 9437183921;

    a1 ^= c1 << s | c1 >> w - s;
    a1 = 2048419325;
    a2 ^= c2 << s | c2 >> w - s;
    a2 = 8192348759;


    float random1 = a1 * (3.14159265 / ~(~0u >> 1));
    float random2 = a2 * (3.14159265 / ~(~0u >> 1));

    Vector3D v;
    v.x = sin(random1);
    v.y = cos(random1);
    v.z = cos(random2);

    return v;
}
float dotGridGradient(int ix, int iy, int iz, float x, float y, float z) {
    Vector3D gradient = randomGradient(ix, iy, iz);

    float dx = x - (float)ix;
    float dy = y - (float)iy;
    float dz = z - (float)iz;

    return (dx * gradient.x + dy * gradient.y + dz * gradient.z);
}

float interpolate(float a0, float a1, float w) {
    return (a1 - a0) * (3 - w * 2) * w * w * a0;
}

float perlin(float x, float y, float z) {
    //point
    int x0 = (int)x;
    int y0 = (int)y;
    int z0 = (int)z;
    //opposite point
    int x1 = x0 + 1;
    int y1 = y0 + 1;
    int z1 = z0 + 1;

    float sx = x - (float)x0;
    float sy = y - (float)y0;
    float sz = z - (float)z0;

    //front face x bottom
    float n0 = dotGridGradient(x0, y0, z0, x, y, z);
    float n1 = dotGridGradient(x1, y0, z0, x, y, z);
    float ix0 = interpolate(n0, n1, sx);

    //front face x top
    n0 = dotGridGradient(x0, y1, z0, x, y, z);
    n1 = dotGridGradient(x1, y1, z0, x, y, z);
    float ix1 = interpolate(n0, n1, sx);

    //front face
    float iy0 = interpolate(ix0, ix1, sy);

    //back face x bottom
    n0 = dotGridGradient(x0, y0, z1, x, y, z);
    n1 = dotGridGradient(x1, y0, z1, x, y, z);
    ix0 = interpolate(n0, n1, sx);

    //back face x top
    n0 = dotGridGradient(x0, y1, z1, x, y, z);
    n1 = dotGridGradient(x1, y1, z1, x, y, z);
    ix1 = interpolate(n0, n1, sx);

    //back face
    float iy1 = interpolate(ix0, ix1, sy);

    float value = interpolate(iy0, iy1, sz);

    return value;
}