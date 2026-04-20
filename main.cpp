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
void triangle(vec3 *pts, TGAImage &image, TGAColor color, std::vector<double> &zbuffer)
{
    int minX = std::max(0, (int)std::floor(std::min(pts[0].x, std::min(pts[1].x, pts[2].x))));
    int maxX = std::min(image.width() - 1, (int)std::ceil(std::max(pts[0].x, std::max(pts[1].x, pts[2].x))));
    int minY = std::max(0, (int)std::floor(std::min(pts[0].y, std::min(pts[1].y, pts[2].y))));
    int maxY = std::min(image.height() - 1, (int)std::ceil(std::max(pts[0].y, std::max(pts[1].y, pts[2].y))));

    vec2 P;
    for (int x = minX; x <= maxX; x++) {
        for (int y = minY; y <= maxY; y++) {
            
            vec3 ba_screen = barycentric(pts[0].xy(), pts[1].xy(), pts[2].xy(), vec2{(double)x, (double)y});
            
            if (ba_screen.x < 0 || ba_screen.y < 0 || ba_screen.z < 0) continue;

            double frag_z = 0;
            for (int i = 0; i < 3; i++) {
                frag_z += pts[i].z * ba_screen[i];
            }

            int idx = x + y * width;
            if (zbuffer[idx] < frag_z) {
                zbuffer[idx] = frag_z;
                image.set(x, y, color);
            }
        }
    }
}

void gradienttriangle(vec3 *pts, TGAImage &image, TGAColor *colors)
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
            vec3 ba_screen = barycentric(pts[0].xy(), pts[1].xy(), pts[2].xy(), P);
            if (ba_screen.x < 0 || ba_screen.y < 0 || ba_screen.z < 0)
                continue;

            unsigned char r = ba_screen.x * colors[0].bgra[0] + ba_screen.y * colors[1].bgra[0] + ba_screen.z * colors[2].bgra[0];
            unsigned char g = ba_screen.x * colors[0].bgra[1] + ba_screen.y * colors[1].bgra[1] + ba_screen.z * colors[2].bgra[1];
            unsigned char b = ba_screen.x * colors[0].bgra[2] + ba_screen.y * colors[1].bgra[2] + ba_screen.z * colors[2].bgra[2];

            image.set(P.x, P.y, TGAColor{r, g, b, 255});
        }
    }
}

void rendermodel(vec3 lightdir, TGAImage &image, Model &model, std::vector<double> &zbuffer)
{
    for (int i = 0; i < model.nfaces(); i++)
    {
        vec3 worldcoor[3];
        vec3 screencoor[3];

        for (int j = 0; j < 3; j++)
        {
            vec4 vertex = model.vert(i, j);
            worldcoor[j] = vertex.xyz();
            screencoor[j] = vec3{
                (vertex.x + 1.) * image.width()  / 2., 
                (vertex.y + 1.) * image.height() / 2., 
                (vertex.z + 1.) * 255. / 2.
            };
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
        }, zbuffer);
    }
    }
}

int main()
{
    TGAImage image(width, height, TGAImage::RGB);

    Model model ("./obj/african_head/african_head.obj");

    std::vector<double> zbuffer(width * height, -std::numeric_limits<double>::max());

    rendermodel(vec3{0,0,-1}, image, model, zbuffer);

    // image.flip_vertically();

    TGAImage ZbufferImg(width, height, TGAImage::RGB);

    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            double z_val = zbuffer[i + j * width]; // 先拿 double 值

            // 1. 必须在转 unsigned char 之前做判断！
            if (z_val < -1000) {
                ZbufferImg.set(i, j, TGAColor{0, 0, 0, 255});
                continue;
            }
            
            unsigned char z = (unsigned char)std::min(255.0, std::max(0.0, z_val));
            ZbufferImg.set(i, j, TGAColor{z, z, z, 255});
        }
        
    }

    image.write_tga_file("./result/rendermodel1.tga");
    ZbufferImg.write_tga_file("./result/zbuffer.tga");

    return 0;
}