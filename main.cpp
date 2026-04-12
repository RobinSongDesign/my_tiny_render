#include "tgaimage.h"
#include "model.h"

const TGAColor white = {{255, 255, 255, 255}, 4};
const TGAColor red   = {{0, 0, 255, 255}, 4};
const TGAColor blue = {{255, 255, 0, 255}, 4};

const int height = 800;
const int width = 800;

void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) {
    bool steep = false;
    if (std::abs(x0 - x1) < std::abs(y0 - y1)) { // 如果高度大于宽度
        std::swap(x0, y0); // 坐标转置
        std::swap(x1, y1);
        steep = true;
    }
    
    if (x0 > x1) {
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
        else{
            image.set(x, y, color);
        }
        
    }
    
}

int main(){
    
    
    Model *model = new Model("obj/african_head/african_head.obj");

    for(int i = 0; i < model->nfaces(); i++)
    {
        std::vector<int> face = model -> face(i);
        for(int j = 0; j < 3; j++){
            Vec3f v0 = model->point(face[j]);
            Vec3f v1 = model->point(face[j+1]%3); //the next vertex

            int x0 = (v0.x + 1.) * width/2.;
            int x1 = (v1.x + 1.) * width/2.;
            int y0 = (v0.y + 1.) * height/2.;
            int y1 = (v1.y + 1.) * height/2.;
        }

        line(x0, y0, x1, y1, image, white);
    }
    
    
    TGAImage image(800, 800, TGAImage::RGB);

    line(20, 13, 40, 80, image, blue); 
    //line(13, 20, 80, 40, image, white); 

    image.flip_vertically();
    image.write_tga_file("output.tga");

    return 0;
}