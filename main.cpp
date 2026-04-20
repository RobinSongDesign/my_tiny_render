#include "tgaimage.h"
#include "model.h"

const TGAColor white = {{255, 255, 255, 255}, 4};
const TGAColor red = {{0, 0, 255, 255}, 4};
const TGAColor blue = {{255, 255, 0, 255}, 4};
const TGAColor green = {{0, 255, 0}, 4};

const int height = 400;
const int width = 400;

void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color)
{
    bool steep = false;
    if (std::abs(x0 - x1) < std::abs(y0 - y1))
    {                      // 如果高度大于宽度
        std::swap(x0, y0); // 坐标转置
        std::swap(x1, y1);
        steep = true;
    }

    if (x0 > x1)
    {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }

    float k = (float)(y1 - y0) / (float)(x1 - x0);
    for (int x = x0; x <= x1; x++)
    {
        int y = y0 + k * (x - x0);

        if (steep)
        {
            image.set(y, x, color);
        }
        else
        {
            image.set(x, y, color);
        }
    }
}

vec3 barycentric(vec2 A, vec2 B, vec2 C, vec2 P)
{
    // u * AB_x + v * AC_x + PA_x = 0
    // u * AB_y + v * AC_y + PA_y = 0

    vec3 x_coor{(double)B.x - A.x, (double)C.x - A.x, (double)A.x - P.x};
    vec3 y_coor = {(double)B.y - A.y, (double)C.y - A.y, (double)A.y - P.y};

    vec3 w = cross(x_coor, y_coor);
    if (std::abs(w.z) < 1)
        return vec3{-1, 1, 1};

    float u = w.x / w.z;
    float v = w.y / w.z;
    return vec3{1.f - (u + v), u, v};
}
void triangle(vec2 *pts, TGAImage &image, TGAColor color)
{
    vec2 bboxmin = {pts[0].x, pts[0].y};
    vec2 bboxmax = {pts[0].x, pts[0].y};
    vec2 clamp = {(double)image.width() - 1, (double)image.height() - 1};

    // find bbox
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            bboxmin[j] = std::max(0.0, std::min(bboxmin[j], pts[i][j]));
            bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], pts[i][j]));
        }
    }

    vec2 P;
    for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++)
    {
        for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++)
        {
            vec3 ba_screen = barycentric(pts[0], pts[1], pts[2], P);
            if (ba_screen.x < 0 || ba_screen.y < 0 || ba_screen.z < 0)
                continue;

            image.set(P.x, P.y, color);
        }
    }
}

void triangle(vec2 *pts, TGAImage &image, TGAColor *colors)
{
    vec2 bboxmin = {pts[0].x, pts[0].y};
    vec2 bboxmax = {pts[0].x, pts[0].y};
    vec2 clamp = {(double)image.width() - 1, (double)image.height() - 1};

    // find bbox
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            bboxmin[j] = std::max(0.0, std::min(bboxmin[j], pts[i][j]));
            bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], pts[i][j]));
        }
    }

    vec2 P;
    for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++)
    {
        for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++)
        {
            vec3 ba_screen = barycentric(pts[0], pts[1], pts[2], P);
            if (ba_screen.x < 0 || ba_screen.y < 0 || ba_screen.z < 0)
                continue;

            unsigned char r = ba_screen.x * colors[0].bgra[0] + ba_screen.y * colors[1].bgra[0] + ba_screen.z * colors[2].bgra[0];
            unsigned char g = ba_screen.x * colors[0].bgra[1] + ba_screen.y * colors[1].bgra[1] + ba_screen.z * colors[2].bgra[1];
            unsigned char b = ba_screen.x * colors[0].bgra[2] + ba_screen.y * colors[1].bgra[2] + ba_screen.z * colors[2].bgra[2];

            image.set(P.x, P.y, TGAColor{r, g, b, 255});
        }
    }
}

void rendermodel(vec3 lightdir, TGAImage &image, Model &model)
{
    for (int i = 0; i < model.nfaces(); i++)
    {
        vec3 worldcoor[3];
        vec2 screencoor[3];

        for (int j = 0; j < 3; j++)
        {
            vec4 vertex = model.vert(i, j);
            worldcoor[j] = vertex.xyz();
            screencoor[j] = vec2{(vertex.x + 1.) / 2 * image.width(), (vertex.y + 1.) / 2 * image.height()};
        }

        //normal
        vec3 normal = cross((worldcoor[2] - worldcoor[0]), (worldcoor[1] - worldcoor[0]));
        normal = normalized(normal);

        //lighting intensity
        double intensity = normal * lightdir; 

        if (intensity > 0) {
        // 根据强度着色 (Flat Shading)
        triangle(screencoor, image, TGAColor{
            (unsigned char)(intensity * 255), 
            (unsigned char)(intensity * 255), 
            (unsigned char)(intensity * 255), 
            255
        });
    }
    }
}

int main()
{
    TGAImage image(width, height, TGAImage::RGB);

    Model model ("./obj/african_head/african_head.obj");

    rendermodel(vec3{0,0,-1}, image, model);

    // image.flip_vertically();

    image.write_tga_file("./result/rendermodel0.tga");

    return 0;
}