#include <math.h>
#include "cube.h"
#include "item.h"
#include "matrix.h"
#include "util.h"

void make_cube_faces(
    float *data, float ao[6][4], float light[6][4],
    int left, int right, int top, int bottom, int front, int back,
    int wleft, int wright, int wtop, int wbottom, int wfront, int wback,
    float n)
{
    static const float positions[6][4][3] = {
        {{-1, -1, -1}, {-1, -1, +1}, {-1, +1, -1}, {-1, +1, +1}},
        {{+1, -1, -1}, {+1, -1, +1}, {+1, +1, -1}, {+1, +1, +1}},
        {{-1, +1, -1}, {-1, +1, +1}, {+1, +1, -1}, {+1, +1, +1}},
        {{-1, -1, -1}, {-1, -1, +1}, {+1, -1, -1}, {+1, -1, +1}},
        {{-1, -1, -1}, {-1, +1, -1}, {+1, -1, -1}, {+1, +1, -1}},
        {{-1, -1, +1}, {-1, +1, +1}, {+1, -1, +1}, {+1, +1, +1}}
    };
    static const float normals[6][3] = {
        {-1, 0, 0},
        {+1, 0, 0},
        {0, +1, 0},
        {0, -1, 0},
        {0, 0, -1},
        {0, 0, +1}
    };
    static const float uvs[6][4][2] = {
        {{0, 0}, {1, 0}, {0, 1}, {1, 1}},
        {{1, 0}, {0, 0}, {1, 1}, {0, 1}},
        {{0, 1}, {0, 0}, {1, 1}, {1, 0}},
        {{0, 0}, {0, 1}, {1, 0}, {1, 1}},
        {{0, 0}, {0, 1}, {1, 0}, {1, 1}},
        {{1, 0}, {1, 1}, {0, 0}, {0, 1}}
    };
    static const float indices[6][6] = {
        {0, 3, 2, 0, 1, 3},
        {0, 3, 1, 0, 2, 3},
        {0, 3, 2, 0, 1, 3},
        {0, 3, 1, 0, 2, 3},
        {0, 3, 2, 0, 1, 3},
        {0, 3, 1, 0, 2, 3}
    };
    static const float flipped[6][6] = {
        {0, 1, 2, 1, 3, 2},
        {0, 2, 1, 2, 3, 1},
        {0, 1, 2, 1, 3, 2},
        {0, 2, 1, 2, 3, 1},
        {0, 1, 2, 1, 3, 2},
        {0, 2, 1, 2, 3, 1}
    };
    float *d = data;
    float s = 0.0625;
    float a = 0 + 1 / 2048.0;
    float b = s - 1 / 2048.0;
    int faces[6] = {left, right, top, bottom, front, back};
    int tiles[6] = {wleft, wright, wtop, wbottom, wfront, wback};
    for (int i = 0; i < 6; i++) {
        if (faces[i] == 0) {
            continue;
        }
        float du = (tiles[i] % 16) * s;
        float dv = (tiles[i] / 16) * s;
        int flip = ao[i][0] + ao[i][3] > ao[i][1] + ao[i][2];
        for (int v = 0; v < 6; v++) {
            int j = flip ? flipped[i][v] : indices[i][v];
            *(d++) = n * positions[i][j][0];
            *(d++) = n * positions[i][j][1];
            *(d++) = n * positions[i][j][2];
            *(d++) = normals[i][0];
            *(d++) = normals[i][1];
            *(d++) = normals[i][2];
            *(d++) = du + (uvs[i][j][0] ? b : a);
            *(d++) = dv + (uvs[i][j][1] ? b : a);
            *(d++) = ao[i][j];
            *(d++) = light[i][j];
        }
    }
}

void make_rotated_cube(float *data, float ao[6][4], float light[6][4],
                       int left, int right, int top, int bottom, int front, int back,
                       float x, float y, float z, float n, float rx, float ry, float rz,
                       int w, const int blocks[256][6]) {
    int wleft = blocks[w][0];
    int wright = blocks[w][1];
    int wtop = blocks[w][2];
    int wbottom = blocks[w][3];
    int wfront = blocks[w][4];
    int wback = blocks[w][5];
    make_cube_faces(
        data, ao, light,
        left, right, top, bottom, front, back,
        wleft, wright, wtop, wbottom, wfront, wback,
        n);
    float ma[16];
    float mb[16];
    mat_identity(ma);
    /* Create rotation transformation */
    mat_rotate(mb, 0, 1, 0, ry);
    mat_multiply(ma, mb, ma);
    mat_rotate(mb, 1, 0, 0, rx);
    mat_multiply(ma, mb, ma);
    mat_rotate(mb, 0, 0, 1, rz);
    mat_multiply(ma, mb, ma);
    /* Apply to normals */
    mat_apply(data, ma, (left + right + top + bottom + front + back)*6, 3, 10);
    /* Create translation transformation */
    mat_translate(mb, x, y, z);
    /* Merge with rotation transformation */
    mat_multiply(ma, mb, ma);
    /* Apply to vertices */
    mat_apply(data, ma, (left + right + top + bottom + front + back)*6, 0, 10);
}

void make_cube(
    float *data, float ao[6][4], float light[6][4],
    int left, int right, int top, int bottom, int front, int back,
    float x, float y, float z, float n, int w, const int blocks[256][6])
{
    int wleft = blocks[w][0];
    int wright = blocks[w][1];
    int wtop = blocks[w][2];
    int wbottom = blocks[w][3];
    int wfront = blocks[w][4];
    int wback = blocks[w][5];
    make_cube_faces(
        data, ao, light,
        left, right, top, bottom, front, back,
        wleft, wright, wtop, wbottom, wfront, wback,
        n);
    float ma[16];
    float mb[16];
    mat_identity(ma);
    mat_translate(mb, x, y, z);
    mat_multiply(ma, mb, ma);
    mat_apply(data, ma, (left + right + top + bottom + front + back)*6, 0, 10);
}

void make_plant(
    float *data, float ao, float light,
    float px, float py, float pz, float n, int w, float rotation, const int blocks[256][6])
{
    static const float positions[4][4][3] = {
        {{ 0, -1, -1}, { 0, -1, +1}, { 0, +1, -1}, { 0, +1, +1}},
        {{ 0, -1, -1}, { 0, -1, +1}, { 0, +1, -1}, { 0, +1, +1}},
        {{-1, -1,  0}, {-1, +1,  0}, {+1, -1,  0}, {+1, +1,  0}},
        {{-1, -1,  0}, {-1, +1,  0}, {+1, -1,  0}, {+1, +1,  0}}
    };
    static const float normals[4][3] = {
        {-1, 0, 0},
        {+1, 0, 0},
        {0, 0, -1},
        {0, 0, +1}
    };
    static const float uvs[4][4][2] = {
        {{0, 0}, {1, 0}, {0, 1}, {1, 1}},
        {{1, 0}, {0, 0}, {1, 1}, {0, 1}},
        {{0, 0}, {0, 1}, {1, 0}, {1, 1}},
        {{1, 0}, {1, 1}, {0, 0}, {0, 1}}
    };
    static const float indices[4][6] = {
        {0, 3, 2, 0, 1, 3},
        {0, 3, 1, 0, 2, 3},
        {0, 3, 2, 0, 1, 3},
        {0, 3, 1, 0, 2, 3}
    };
    float *d = data;
    float s = 0.0625;
    float a = 0;
    float b = s;
    float du = (blocks[w][0] % 16) * s;
    float dv = (blocks[w][0] / 16) * s;
    for (int i = 0; i < 4; i++) {
        for (int v = 0; v < 6; v++) {
            int j = indices[i][v];
            *(d++) = n * positions[i][j][0];
            *(d++) = n * positions[i][j][1];
            *(d++) = n * positions[i][j][2];
            *(d++) = normals[i][0];
            *(d++) = normals[i][1];
            *(d++) = normals[i][2];
            *(d++) = du + (uvs[i][j][0] ? b : a);
            *(d++) = dv + (uvs[i][j][1] ? b : a);
            *(d++) = ao;
            *(d++) = light;
        }
    }
    float ma[16];
    float mb[16];
    mat_identity(ma);
    mat_rotate(mb, 0, 1, 0, RADIANS(rotation));
    mat_multiply(ma, mb, ma);
    mat_apply(data, ma, 24, 3, 10);
    mat_translate(mb, px, py, pz);
    mat_multiply(ma, mb, ma);
    mat_apply(data, ma, 24, 0, 10);
}


int _make_sphere(
    float *data, float r, int detail,
    float *a, float *b, float *c,
    float *ta, float *tb, float *tc)
{
    if (detail == 0) {
        float *d = data;
        *(d++) = a[0] * r; *(d++) = a[1] * r; *(d++) = a[2] * r;
        *(d++) = a[0]; *(d++) = a[1]; *(d++) = a[2];
        *(d++) = ta[0]; *(d++) = ta[1];
        *(d++) = b[0] * r; *(d++) = b[1] * r; *(d++) = b[2] * r;
        *(d++) = b[0]; *(d++) = b[1]; *(d++) = b[2];
        *(d++) = tb[0]; *(d++) = tb[1];
        *(d++) = c[0] * r; *(d++) = c[1] * r; *(d++) = c[2] * r;
        *(d++) = c[0]; *(d++) = c[1]; *(d++) = c[2];
        *(d++) = tc[0]; *(d++) = tc[1];
        return 1;
    }
    else {
        float ab[3], ac[3], bc[3];
        for (int i = 0; i < 3; i++) {
            ab[i] = (a[i] + b[i]) / 2;
            ac[i] = (a[i] + c[i]) / 2;
            bc[i] = (b[i] + c[i]) / 2;
        }
        normalize(ab + 0, ab + 1, ab + 2);
        normalize(ac + 0, ac + 1, ac + 2);
        normalize(bc + 0, bc + 1, bc + 2);
        float tab[2], tac[2], tbc[2];
        tab[0] = 0; tab[1] = 1 - acosf(ab[1]) / PI;
        tac[0] = 0; tac[1] = 1 - acosf(ac[1]) / PI;
        tbc[0] = 0; tbc[1] = 1 - acosf(bc[1]) / PI;
        int total = 0;
        int n;
        n = _make_sphere(data, r, detail - 1, a, ab, ac, ta, tab, tac);
        total += n; data += n * 24;
        n = _make_sphere(data, r, detail - 1, b, bc, ab, tb, tbc, tab);
        total += n; data += n * 24;
        n = _make_sphere(data, r, detail - 1, c, ac, bc, tc, tac, tbc);
        total += n; data += n * 24;
        n = _make_sphere(data, r, detail - 1, ab, bc, ac, tab, tbc, tac);
        total += n; data += n * 24;
        return total;
    }
}

void make_sphere(float *data, float r, int detail) {
    // detail, triangles, floats
    // 0, 8, 192
    // 1, 32, 768
    // 2, 128, 3072
    // 3, 512, 12288
    // 4, 2048, 49152
    // 5, 8192, 196608
    // 6, 32768, 786432
    // 7, 131072, 3145728
    static int indices[8][3] = {
        {4, 3, 0}, {1, 4, 0},
        {3, 4, 5}, {4, 1, 5},
        {0, 3, 2}, {0, 2, 1},
        {5, 2, 3}, {5, 1, 2}
    };
    static float positions[6][3] = {
        { 0, 0,-1}, { 1, 0, 0},
        { 0,-1, 0}, {-1, 0, 0},
        { 0, 1, 0}, { 0, 0, 1}
    };
    static float uvs[6][3] = {
        {0, 0.5}, {0, 0.5},
        {0, 0}, {0, 0.5},
        {0, 1}, {0, 0.5}
    };
    int total = 0;
    for (int i = 0; i < 8; i++) {
        int n = _make_sphere(
            data, r, detail,
            positions[indices[i][0]],
            positions[indices[i][1]],
            positions[indices[i][2]],
            uvs[indices[i][0]],
            uvs[indices[i][1]],
            uvs[indices[i][2]]);
        total += n; data += n * 24;
    }
}

void make_character(
    float *data,
    float x, float y, float n, float m, char c, float z)
{
    float *d = data;
    float s = 0.0625;
    float a = s;
    float b = s * 2;
    int w = c - 32;
    float du = (w % 16) * a;
    float dv = 1 - (w / 16) * b - b;
    *(d++) = x - n; *(d++) = y - m; *(d++) = z;
    *(d++) = 0.0; *(d++) = 1.0; *(d++) = 0.0;
    *(d++) = du + 0; *(d++) = dv;
    *(d++) = 0; *(d++) = 0;

    *(d++) = x + n; *(d++) = y - m; *(d++) = z;
    *(d++) = 0.0; *(d++) = 1.0; *(d++) = 0.0;
    *(d++) = du + a; *(d++) = dv;
    *(d++) = 0; *(d++) = 0;

    *(d++) = x + n; *(d++) = y + m; *(d++) = z;
    *(d++) = 0.0; *(d++) = 1.0; *(d++) = 0.0;
    *(d++) = du + a; *(d++) = dv + b;
    *(d++) = 0; *(d++) = 0;

    *(d++) = x - n; *(d++) = y - m; *(d++) = z;
    *(d++) = 0.0; *(d++) = 1.0; *(d++) = 0.0;
    *(d++) = du + 0; *(d++) = dv;
    *(d++) = 0; *(d++) = 0;

    *(d++) = x + n; *(d++) = y + m; *(d++) = z;
    *(d++) = 0.0; *(d++) = 1.0; *(d++) = 0.0;
    *(d++) = du + a; *(d++) = dv + b;
    *(d++) = 0; *(d++) = 0;

    *(d++) = x - n; *(d++) = y + m; *(d++) = z;
    *(d++) = 0.0; *(d++) = 1.0; *(d++) = 0.0;
    *(d++) = du + 0; *(d++) = dv + b;
    *(d++) = 0; *(d++) = 0;
}
