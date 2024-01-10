#include <iostream>
#include <string>
#include <fstream>
#include <Windows.h>

std::string rgb_entrada;

unsigned short ancho_in = 4;
unsigned short alto_in = 4;
unsigned short ancho_fin = 1920;
unsigned short alto_fin = 1080;

std::string rgb_salida((ancho_fin * 3) * alto_fin, '\0');

unsigned char enc_bmp[54] = { 0x42, 0x4D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x36, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

float def_lims(float val_var, float val_max)
{
    if (val_var < 0)
    {
        val_var = 0;
    }
    else if (val_var > val_max)
    {
        val_var = val_max;
    }

    return val_var;
};

float bicub_hermite(float f_p0, float f_p1, float f_p2, float f_p3, float xy_frac)
{
    float coef_a = - (0.5f * f_p0) + (1.5f * f_p1) - (1.5f * f_p2) + (0.5f * f_p3);
    float coef_b = (f_p0) - (2.5f * f_p1) + (2.0f * f_p2) - (0.5f * f_p3);
    float coef_c = - (0.5f * f_p0) + (0.5f * f_p2);
    float coef_d = f_p1;

    return (coef_a * (xy_frac * xy_frac * xy_frac)) + (coef_b * (xy_frac * xy_frac)) + (coef_c * (xy_frac)) + (coef_d);
};

unsigned char *pixel_lims(unsigned short x_ent2, unsigned short y_ent2)
{
    x_ent2 = def_lims(x_ent2, ancho_in - 1);
    y_ent2 = def_lims(y_ent2, alto_in - 1);

    return reinterpret_cast<unsigned char *>(&rgb_entrada[(y_ent2 * (ancho_in * 3)) + (x_ent2 * 3)]);
};

std::string pixel_interp(float &cord_u2, float &cord_v2)
{
    float cord_x = (cord_u2 * ancho_in);
    unsigned short x_ent = cord_x;
    float x_frac = cord_x - short(cord_x);

    float cord_y = (cord_v2 * alto_in);
    unsigned short y_ent = cord_y;
    float y_frac = cord_y - short(cord_y);

    unsigned char *p00 = pixel_lims(x_ent - 1, y_ent - 1);
    unsigned char *p10 = pixel_lims(x_ent + 0, y_ent - 1);
    unsigned char *p20 = pixel_lims(x_ent + 1, y_ent - 1);
    unsigned char *p30 = pixel_lims(x_ent + 2, y_ent - 1);

    unsigned char *p01 = pixel_lims(x_ent - 1, y_ent + 0);
    unsigned char *p11 = pixel_lims(x_ent + 0, y_ent + 0);
    unsigned char *p21 = pixel_lims(x_ent + 1, y_ent + 0);
    unsigned char *p31 = pixel_lims(x_ent + 2, y_ent + 0);

    unsigned char *p02 = pixel_lims(x_ent - 1, y_ent + 1);
    unsigned char *p12 = pixel_lims(x_ent + 0, y_ent + 1);
    unsigned char *p22 = pixel_lims(x_ent + 1, y_ent + 1);
    unsigned char *p32 = pixel_lims(x_ent + 2, y_ent + 1);

    unsigned char *p03 = pixel_lims(x_ent - 1, y_ent + 2);
    unsigned char *p13 = pixel_lims(x_ent + 0, y_ent + 2);
    unsigned char *p23 = pixel_lims(x_ent + 1, y_ent + 2);
    unsigned char *p33 = pixel_lims(x_ent + 2, y_ent + 2);

    std::string interp_2d_pixel(3, '\0');
    for (unsigned int i = 0; i < 3; ++i)
    {
        float fila_0 = bicub_hermite(p00[i], p10[i], p20[i], p30[i], x_frac);
        float fila_1 = bicub_hermite(p01[i], p11[i], p21[i], p31[i], x_frac);
        float fila_2 = bicub_hermite(p02[i], p12[i], p22[i], p32[i], x_frac);
        float fila_3 = bicub_hermite(p03[i], p13[i], p23[i], p33[i], x_frac);

        float pixel_val = bicub_hermite(fila_0, fila_1, fila_2, fila_3, y_frac);

        pixel_val = def_lims(pixel_val, 255.0f);

        interp_2d_pixel[i] = static_cast<unsigned char>(pixel_val);
    }

    return interp_2d_pixel;
};

void redim_img()
{
    char *it_rgb_salida_y = &rgb_salida[0];

    for (unsigned short filas_y = 0; filas_y < alto_fin; ++filas_y)
    {
        char *it_rgb_salida_x = it_rgb_salida_y;
        float cord_v = float(filas_y) / float(alto_fin - 1);

        for (unsigned short columnas_x = 0; columnas_x < ancho_fin; ++columnas_x)
        {
            float cord_u = float(columnas_x) / float(ancho_fin - 1);
            std::string pixel_rgb_salida(3, '\0');

            pixel_rgb_salida = pixel_interp(cord_u, cord_v);

            it_rgb_salida_x[0] = pixel_rgb_salida[0];
            it_rgb_salida_x[1] = pixel_rgb_salida[1];
            it_rgb_salida_x[2] = pixel_rgb_salida[2];
            it_rgb_salida_x += 3;
        }

        it_rgb_salida_y += ancho_fin * 3;
    }
};

int main()
{
    wchar_t w_usr[256];
    DWORD tam_w_usr = 256;

    GetUserName(w_usr, &tam_w_usr);
    std::string n_usr(w_usr, w_usr + tam_w_usr);
    n_usr.pop_back();
    
    enc_bmp[18] = static_cast<unsigned char>(ancho_fin - (unsigned char(ancho_fin / 256) * 256));
    enc_bmp[19] = static_cast<unsigned char>(ancho_fin / 256);
    enc_bmp[22] = static_cast<unsigned char>(alto_fin - (unsigned char(alto_fin / 256) * 256));
    enc_bmp[23] = static_cast<unsigned char>(alto_fin / 256);

    rgb_entrada.push_back(0x00);
    rgb_entrada.push_back(0x00);
    rgb_entrada.push_back(0x00);
    rgb_entrada.push_back(0xA0);
    rgb_entrada.push_back(0xA0);
    rgb_entrada.push_back(0xA0);
    rgb_entrada.push_back(0x50);
    rgb_entrada.push_back(0x50);
    rgb_entrada.push_back(0x50);
    rgb_entrada.push_back(0xF0);
    rgb_entrada.push_back(0xF0);
    rgb_entrada.push_back(0xF0);
    rgb_entrada.push_back(0x00);
    rgb_entrada.push_back(0x00);
    rgb_entrada.push_back(0x00);
    rgb_entrada.push_back(0x50);
    rgb_entrada.push_back(0x50);
    rgb_entrada.push_back(0x50);
    rgb_entrada.push_back(0xF0);
    rgb_entrada.push_back(0xF0);
    rgb_entrada.push_back(0xF0);
    rgb_entrada.push_back(0x00);
    rgb_entrada.push_back(0x00);
    rgb_entrada.push_back(0x00);
    rgb_entrada.push_back(0xA0);
    rgb_entrada.push_back(0xA0);
    rgb_entrada.push_back(0xA0);
    rgb_entrada.push_back(0xA0);
    rgb_entrada.push_back(0xA0);
    rgb_entrada.push_back(0xA0);
    rgb_entrada.push_back(0xF0);
    rgb_entrada.push_back(0xF0);
    rgb_entrada.push_back(0xF0);
    rgb_entrada.push_back(0x50);
    rgb_entrada.push_back(0x50);
    rgb_entrada.push_back(0x50);
    rgb_entrada.push_back(0x00);
    rgb_entrada.push_back(0x00);
    rgb_entrada.push_back(0x00);
    rgb_entrada.push_back(0xA0);
    rgb_entrada.push_back(0xA0);
    rgb_entrada.push_back(0xA0);
    rgb_entrada.push_back(0x50);
    rgb_entrada.push_back(0x50);
    rgb_entrada.push_back(0x50);
    rgb_entrada.push_back(0xF0);
    rgb_entrada.push_back(0xF0);
    rgb_entrada.push_back(0xF0);

    std::string bmp_salida;

    for (unsigned char ch_ch : enc_bmp)
    {
        bmp_salida.push_back(ch_ch);
    }

    redim_img();

    for (unsigned char ch_ch : rgb_salida)
    {
        bmp_salida.push_back(ch_ch);
    }

    std::ofstream bmp_arch("C:/Users/" + n_usr + "/Desktop/bmp_example.bmp", std::ios::binary);
    bmp_arch.write(bmp_salida.data(), bmp_salida.size());
    bmp_arch.close();
    std::cout << "Finished";

    return 0;
}
