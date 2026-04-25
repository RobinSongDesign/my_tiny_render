#include "tgaimage.h"
#include "model.h"

const TGAColor white = {{255, 255, 255, 255}, 4};
const TGAColor red = {{0, 0, 255, 255}, 4};
const TGAColor blue = {{255, 255, 0, 255}, 4};
const TGAColor green = {{0, 255, 0}, 4};

const int height = 1024;
const int width = 1024;

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
void triangle(vec3 *pts, vec2 *uvs, TGAImage &image, TGAImage &diffuse_map, std::vector<double> &zbuffer, double *intensity)
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

            double u_p = ba_screen.x * uvs[0].x + ba_screen.y * uvs[1].x + ba_screen.z * uvs[2].x;
            double v_p = ba_screen.x * uvs[0].y + ba_screen.y * uvs[1].y + ba_screen.z * uvs[2].y;

            int tex_idx = u_p * diffuse_map.width();
            int tex_idy = v_p * diffuse_map.height();

            TGAColor color = diffuse_map.get(tex_idx, tex_idy);

            // double b_intensity = ba_screen.x * intensity[0] + ba_screen.y * intensity[1] + ba_screen.z * intensity[2];

            // for (int i = 0; i < 3; i++) {
            //     color.bgra[i] *= b_intensity; 
            // }

            int idx = x + y * width;
            if (zbuffer[idx] < frag_z) {
                zbuffer[idx] = frag_z;
                image.set(x, y, color);
            }
        }
    }
}

// void gradienttriangle(vec3 *pts, TGAImage &image, TGAColor *colors)
// {
//     vec2 bboxmin = {pts[0].x, pts[0].y};
//     vec2 bboxmax = {pts[0].x, pts[0].y};
//     vec2 clamp = {(double)image.width() - 1, (double)image.height() - 1}
//     // find bbox
//     for (int i = 0; i < 3; i++)
//     {
//         for (int j = 0; j < 2; j++)
//         {
//             bboxmin[j] = std::max(0.0, std::min(bboxmin[j], pts[i][j]));
//             bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], pts[i][j]));
//         }
//     }
//     vec2 P;
//     for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++)
//     {
//         for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++)
//         {
//             vec3 ba_screen = barycentric(pts[0].xy(), pts[1].xy(), pts[2].xy(), P);
//             if (ba_screen.x < 0 || ba_screen.y < 0 || ba_screen.z < 0)
//                 continue;
//             unsigned char r = ba_screen.x * colors[0].bgra[0] + ba_screen.y * colors[1].bgra[0] + ba_screen.z * colors[2].bgra[0];
//             unsigned char g = ba_screen.x * colors[0].bgra[1] + ba_screen.y * colors[1].bgra[1] + ba_screen.z * colors[2].bgra[1];
//             unsigned char b = ba_screen.x * colors[0].bgra[2] + ba_screen.y * colors[1].bgra[2] + ba_screen.z * colors[2].bgra[2];
//             image.set(P.x, P.y, TGAColor{r, g, b, 255});
//         }
//     }
// }


int main()
{
    //construct image
    TGAImage image(width, height, TGAImage::RGB);

    //load model
    Model model ("./obj/african_head/african_head.obj");

    //init zbuffer
    std::vector<double> zbuffer(width * height, -std::numeric_limits<double>::max());

    //init light direction
    vec3 lightdir{-1,0,-1};

    //load diffuse
    TGAImage diffuse_map;
    diffuse_map.read_tga_file("./obj/african_head/african_head_diffuse.tga");

    //render model
    for (int i = 0; i < model.nfaces(); i++)
    {
        vec3 worldcoor[3];
        vec3 screencoor[3];
        vec2 uvs[3];

        for (int j = 0; j < 3; j++)
        {
            vec4 vertex = model.vert(i, j);
            worldcoor[j] = vertex.xyz();
            screencoor[j] = vec3{
                (vertex.x + 1.) * image.width()  / 2., 
                (vertex.y + 1.) * image.height() / 2., 
                (vertex.z + 1.) * 255. / 2.
            };
            uvs[j] = model.uv(i,j);
        }

        //normal
        vec3 normal = cross((worldcoor[2] - worldcoor[0]), (worldcoor[1] - worldcoor[0]));
        normal = normalized(normal);

        //obj nrm intensity
        double intensity[3];
        for (int i = 0; i < 3; i++)
        {
            intensity[i] = model.normal(uvs[i]).xyz() * lightdir;
        }

        // Flat Shading
        triangle(screencoor, uvs, image, diffuse_map, zbuffer, intensity);
    }

    TGAImage ZbufferImg(width, height, TGAImage::RGB);

    image.write_tga_file("./result/rendermodel3.tga");
    ZbufferImg.write_tga_file("./result/zbuffer.tga");

    return 0;
}