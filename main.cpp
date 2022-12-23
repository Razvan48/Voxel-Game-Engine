#include <iostream>
#include <glew.h>
#include <glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include <stdio.h>
#include <stdlib.h>

#include "Noise/FastNoise/FastNoise.cpp"

using namespace std;

float f_pi = atan(1) * 4.0f;

const char* c_sursa_shader_vertex_tinta =
"#version 330 core \n"
"layout (location = 0) in vec2 coordonate_varf; \n"
"void main() \n"
"{ \n"
"gl_Position = vec4(coordonate_varf, 0.0f, 1.0f); \n"
"} \n";

const char* c_sursa_shader_fragment_tinta =
"version 330 core \n"
"out vec4 coordonate_culoare; \n"
"void main() \n"
"{ \n"
"coordonate_culoare = vec4(1.0f, 1.0f, 1.0f, 1.0f); \n"
"} \n";

const char* c_sursa_shader_vertex_sector_lume_joc =
"#version 330 core \n"
"layout (location = 0) in vec3 coordonate_varf; \n"
"layout (location = 1) in vec3 coordonate_normala_varf; \n"
"layout (location = 2) in vec2 coordonate_textura_varf; \n"
"out vec3 coordonate_fata; \n"
"out vec3 coordonate_normala; \n"
"out vec2 coordonate_textura; \n"
"uniform mat4 matrice_vedere; \n"
"uniform mat4 matrice_proiectie; \n"
"void main() \n"
"{ \n"
"coordonate_fata = coordonate_varf; \n"
"coordonate_normala = coordonate_normala_varf; \n"
"coordonate_textura = vec2(coordonate_textura_varf.x, 1.0f - coordonate_textura_varf.y); \n"
"gl_Position = matrice_proiectie * matrice_vedere * vec4(coordonate_varf, 1.0f); \n"
"} \n";
const char* c_sursa_shader_fragment_sector_lume_joc =
"#version 330 core \n"
"in vec3 coordonate_fata; \n"
"in vec3 coordonate_normala; \n"
"in vec2 coordonate_textura; \n"
"out vec4 coordonate_culoare; \n"
"uniform vec3 coordonate_camera; \n"
"uniform sampler2D textura; \n"
"uniform vec3 directie_lumina_directionala; \n"

"void main() \n"
"{ \n"
"vec3 putere_ambienta = vec3(0.3f, 0.3f, 0.3f); \n"
"vec3 efect_luminos_ambienta = putere_ambienta * texture(textura, coordonate_textura).rgb; \n"

"vec3 putere_difuzie = vec3(0.5f, 0.5f, 0.5f); \n"
"vec3 normala_normalizata = normalize(coordonate_normala); \n"
"vec3 directie_normalizata_lumina = normalize(-directie_lumina_directionala); \n"
"float f_difuzie = max(dot(normala_normalizata, directie_normalizata_lumina), 0.0f); \n"
"vec3 efect_luminos_difuzie = putere_difuzie * f_difuzie * texture(textura, coordonate_textura).rgb; \n"

"vec3 suma_efecte_luminoase = efect_luminos_ambienta + efect_luminos_difuzie; \n"

"coordonate_culoare = vec4(suma_efecte_luminoase, 1.0f); \n"
"} \n";

int i_latime_fereastra_joc = 1024;
int i_inaltime_fereastra_joc = 768;

glm::vec3 coordonate_camera = glm::vec3(0.0f, 100.0f, 0.0f);
glm::vec3 directie_privire_camera = glm::vec3(1.0f, 0.0f, 0.0f);
glm::vec3 normala_camera = glm::vec3(0.0f, 1.0f, 0.0f);
float f_coordonata_unghiulara_x_jucator = 0.0f;
float f_coordonata_unghiulara_y_jucator = 0.0f;
float f_viteza_translatie_camera = 50.0f;
float f_coordonata_curenta_x_mouse = 0.0f;
float f_coordonata_curenta_y_mouse = 0.0f;
float f_viteza_rotatie_camera = 1.0f;
float f_senzitivitate_mouse = 5.0f;

float f_dimensiune_camp_vizual = 60.0f;
float f_distanta_minima_vedere = 0.1f;
float f_distanta_maxima_vedere = 1024.0f;

float f_modul_unghi_limita = 89.0f;

bool b_prima_data_intrare_mouse = 1;

float f_timp_cadru_anterior = 0.0f;
float f_durata_timp_cadru;

unsigned int shader_vertex_tinta;
unsigned int shader_fragment_tinta;
unsigned int program_shader_tinta;

float f_date_tinta[] =
{
    -0.25f,  0.0f,
     0.25f,  0.0f,

     0.0f,   0.25f,
     0.0f,  -0.25f,
};

/*
float f_date_tinta[] =
{
    -100.0f,  100.0f,
     100.0f,  100.0f,

     100.0f,   100.0f,
     100.0f,  -100.0f,
};
*/

unsigned int shader_vertex_sector_lume_joc;
unsigned int shader_fragment_sector_lume_joc;
unsigned int program_shader_sector_lume_joc;

int loc_matrice_vedere_sector_lume_joc;
int loc_matrice_proiectie_sector_lume_joc;
int loc_coordonate_camera_sector_lume_joc;
int loc_textura_sector_lume_joc;
int loc_directie_lumina_directionala_sector_lume_joc;

glm::mat4 matrice_proiectie_sector_lume_joc;
glm::mat4 matrice_vedere_sector_lume_joc;

glm::vec3 directie_lumina_directionala;

struct raza
{
    glm::vec3 viteza_raza;
    glm::vec3 coordonate_raza;

    int i_coordonata_x_sector_lume_joc;
    int i_coordonata_z_sector_lume_joc;

    int i_coordonata_locala_x_sector_lume_joc;
    int i_coordonata_locala_z_sector_lume_joc;

    int i_coordonata_x_in_sector;
    int i_coordonata_z_in_sector;

    float f_viteza_raza = 0.05f;
};

raza raza_editare_lume_joc;

float f_distanta_maxima_editare_lume_joc = 12.0f;

float f_timp_scurs_ultima_editare_lume_joc = -1.0f;
bool b_lume_joc_editata;

char c_cod_cub_selectat = 0;

void v_modificare_dimensiuni_fereastra_joc(GLFWwindow* fereastra_joc, int i_latime_fereastra_noua_joc, int i_inaltime_fereastra_noua_joc)
{
    glViewport(0, 0, i_latime_fereastra_noua_joc, i_inaltime_fereastra_noua_joc);
}

void v_date_intrare_mouse(GLFWwindow* fereastra, double d_coordonata_noua_x_mouse, double d_coordonata_noua_y_mouse)
{
    if (b_prima_data_intrare_mouse)
    {
        f_coordonata_curenta_x_mouse = (float)d_coordonata_noua_x_mouse;
        f_coordonata_curenta_y_mouse = (float)d_coordonata_noua_y_mouse;
        b_prima_data_intrare_mouse = 0;
    }

    float f_variatie_coordonata_x_mouse = (float)d_coordonata_noua_x_mouse - f_coordonata_curenta_x_mouse;
    float f_variatie_coordonata_y_mouse = (float)d_coordonata_noua_y_mouse - f_coordonata_curenta_y_mouse;

    f_coordonata_curenta_x_mouse = (float)d_coordonata_noua_x_mouse;
    f_coordonata_curenta_y_mouse = (float)d_coordonata_noua_y_mouse;

    f_coordonata_unghiulara_x_jucator += f_variatie_coordonata_x_mouse * f_senzitivitate_mouse * f_viteza_rotatie_camera * f_durata_timp_cadru;
    f_coordonata_unghiulara_y_jucator -= f_variatie_coordonata_y_mouse * f_senzitivitate_mouse * f_viteza_rotatie_camera * f_durata_timp_cadru;

    if (f_coordonata_unghiulara_y_jucator > f_modul_unghi_limita)
    {
        f_coordonata_unghiulara_y_jucator = f_modul_unghi_limita;
    }

    if (f_coordonata_unghiulara_y_jucator < -f_modul_unghi_limita)
    {
        f_coordonata_unghiulara_y_jucator = -f_modul_unghi_limita;
    }

    glm::vec3 directie_noua_privire_camera;

    directie_noua_privire_camera.x = cos(glm::radians(f_coordonata_unghiulara_x_jucator)) * cos(glm::radians(f_coordonata_unghiulara_y_jucator));
    directie_noua_privire_camera.y = sin(glm::radians(f_coordonata_unghiulara_y_jucator));
    directie_noua_privire_camera.z = sin(glm::radians(f_coordonata_unghiulara_x_jucator)) * cos(glm::radians(f_coordonata_unghiulara_y_jucator));

    directie_privire_camera = glm::normalize(directie_noua_privire_camera);
}

const int ci_latime_sector_lume_joc = 32;
const int ci_inaltime_sector_lume_joc = 256;
const int ci_lungime_sector_lume_joc = 32;

int i_numar_sectoare_latime_lume_joc = 501;
int i_numar_sectoare_lungime_lume_joc = 501;

struct sector_lume_joc
{
    int i_coordonata_x_sector_lume_joc = i_numar_sectoare_latime_lume_joc;
    int i_coordonata_z_sector_lume_joc = i_numar_sectoare_lungime_lume_joc;

    char c_cod_cub[ci_latime_sector_lume_joc][ci_inaltime_sector_lume_joc][ci_lungime_sector_lume_joc];

    int i_numar_varfuri_sector_lume_joc;
    int i_numar_fete_sector_lume_joc;
};

const int ci_numar_sectoare_vizibile_latime_lume_joc = 10;
const int ci_numar_sectoare_vizibile_lungime_lume_joc = 10;

int i_coordonata_x_sector_lume_joc_camera;
int i_coordonata_z_sector_lume_joc_camera;

int i_coordonata_veche_x_sector_lume_joc_camera;
int i_coordonata_veche_z_sector_lume_joc_camera;

int i_variatie_coordonata_x_sector_lume_joc_camera;
int i_variatie_coordonata_z_sector_lume_joc_camera;

int i_coordonata_locala_x_sector_lume_joc_camera = ci_numar_sectoare_vizibile_latime_lume_joc / 2;
int i_coordonata_locala_z_sector_lume_joc_camera = ci_numar_sectoare_vizibile_lungime_lume_joc / 2;

sector_lume_joc sector_lume_joc_vizibil[ci_numar_sectoare_vizibile_latime_lume_joc][ci_numar_sectoare_vizibile_lungime_lume_joc];
bool b_sector_ce_trebuie_generat[ci_numar_sectoare_vizibile_latime_lume_joc][ci_numar_sectoare_vizibile_lungime_lume_joc];

bool b_camera_translatata;

unsigned int vao_sector_lume_joc[ci_numar_sectoare_vizibile_latime_lume_joc][ci_numar_sectoare_vizibile_lungime_lume_joc];
unsigned int vbo_sector_lume_joc[ci_numar_sectoare_vizibile_latime_lume_joc][ci_numar_sectoare_vizibile_lungime_lume_joc];

unsigned int vao_tinta;
unsigned int vbo_tinta;

FastNoise generare_lume_joc;
int i_samanta_lume_joc;

int i_numar_texturi = 9; //Cate coloane de texturi avem in total in atlasul de texturi.

//i_tip_fata
/*
1 = OY pozitiv (fata de sus a cubului);
2 = OY negativ (fata de jos a cubului);
3 = OX pozitiv;
4 = OX negativ;
5 = OZ pozitiv;
6 = OZ negativ;
*/

//Atlasul de texturi:
/*
0 = textura eroare;
1 = textura iarba;
2 = textura pamant;
3 = textura piatra;
4 = textura nisip;
5 = textura trunchi de copac;
6 = textura frunze;
7 = textura apa;
8 = textura cactus;
*/

int i_inaltime_medie_sector_lume_joc = 20;
int i_amplitudine_sector_lume_joc = 17;

int i_inaltime_nivel_apa_lume_joc = 10;

int i_inaltime_minima_copac = 4;
int i_inaltime_minima_cactus = 2;

void v_adaugare_fata_cub(int i_textura_fata, int i_tip_fata, int i_coordonata_x_fata_cub, int i_coordonata_y_fata_cub, int i_coordonata_z_fata_cub, int i_coordonata_locala_x_sector_lume_joc, int i_coordonata_locala_z_sector_lume_joc)
{
    if (i_textura_fata != 0)
    {
        if (i_tip_fata == 1)
        {
            float f_date_fata[] =
            {
                //Aceasta este fata de sus a cubului (normala este inspre OY pozitiv).
             0.5f + i_coordonata_x_fata_cub,  0.5f + i_coordonata_y_fata_cub,  0.5f + i_coordonata_z_fata_cub,         0.0f,  1.0f,  0.0f,         i_textura_fata * (1.0f / i_numar_texturi) + 1.0f / i_numar_texturi,  0.833334f,
             0.5f + i_coordonata_x_fata_cub,  0.5f + i_coordonata_y_fata_cub, -0.5f + i_coordonata_z_fata_cub,         0.0f,  1.0f,  0.0f,         i_textura_fata * (1.0f / i_numar_texturi) + 1.0f / i_numar_texturi,  1.0f,
            -0.5f + i_coordonata_x_fata_cub,  0.5f + i_coordonata_y_fata_cub, -0.5f + i_coordonata_z_fata_cub,         0.0f,  1.0f,  0.0f,         i_textura_fata * (1.0f / i_numar_texturi) + 0.0f,                    1.0f,

            -0.5f + i_coordonata_x_fata_cub,  0.5f + i_coordonata_y_fata_cub, -0.5f + i_coordonata_z_fata_cub,         0.0f,  1.0f,  0.0f,         i_textura_fata * (1.0f / i_numar_texturi) + 0.0f,                    1.0f,
            -0.5f + i_coordonata_x_fata_cub,  0.5f + i_coordonata_y_fata_cub,  0.5f + i_coordonata_z_fata_cub,         0.0f,  1.0f,  0.0f,         i_textura_fata * (1.0f / i_numar_texturi) + 0.0f,                    0.833334f,
             0.5f + i_coordonata_x_fata_cub,  0.5f + i_coordonata_y_fata_cub,  0.5f + i_coordonata_z_fata_cub,         0.0f,  1.0f,  0.0f,         i_textura_fata * (1.0f / i_numar_texturi) + 1.0f / i_numar_texturi,  0.833334f,
            };
            glBufferSubData(GL_ARRAY_BUFFER, sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].i_numar_fete_sector_lume_joc * sizeof(f_date_fata), sizeof(f_date_fata), &f_date_fata);
        }
        else
        {
            if (i_tip_fata == 2)
            {
                float f_date_fata[] =
                {
                    //Aceasta este fata de jos a cubului (normala este inspre OY negativ).
                 0.5f + i_coordonata_x_fata_cub, -0.5f + i_coordonata_y_fata_cub, -0.5f + i_coordonata_z_fata_cub,         0.0f, -1.0f,  0.0f,         i_textura_fata * (1.0f / i_numar_texturi) + 1.0f / i_numar_texturi,  0.666667f,
                 0.5f + i_coordonata_x_fata_cub, -0.5f + i_coordonata_y_fata_cub,  0.5f + i_coordonata_z_fata_cub,         0.0f, -1.0f,  0.0f,         i_textura_fata * (1.0f / i_numar_texturi) + 1.0f / i_numar_texturi,  0.833334f,
                -0.5f + i_coordonata_x_fata_cub, -0.5f + i_coordonata_y_fata_cub,  0.5f + i_coordonata_z_fata_cub,         0.0f, -1.0f,  0.0f,         i_textura_fata * (1.0f / i_numar_texturi) + 0.0f,                    0.833334f,

                -0.5f + i_coordonata_x_fata_cub, -0.5f + i_coordonata_y_fata_cub,  0.5f + i_coordonata_z_fata_cub,         0.0f, -1.0f,  0.0f,         i_textura_fata * (1.0f / i_numar_texturi) + 0.0f,                    0.833334f,
                -0.5f + i_coordonata_x_fata_cub, -0.5f + i_coordonata_y_fata_cub, -0.5f + i_coordonata_z_fata_cub,         0.0f, -1.0f,  0.0f,         i_textura_fata * (1.0f / i_numar_texturi) + 0.0f,                    0.666667f,
                 0.5f + i_coordonata_x_fata_cub, -0.5f + i_coordonata_y_fata_cub, -0.5f + i_coordonata_z_fata_cub,         0.0f, -1.0f,  0.0f,         i_textura_fata * (1.0f / i_numar_texturi) + 1.0f / i_numar_texturi,  0.666667f,
                };
                glBufferSubData(GL_ARRAY_BUFFER, 192 * sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].i_numar_fete_sector_lume_joc, 192, &f_date_fata);
            }
            else
            {
                if (i_tip_fata == 3)
                {
                    float f_date_fata[] =
                    {
                        //Aceasta este fata laterala cu normala inspre OX pozitiv.
                     0.5f + i_coordonata_x_fata_cub, -0.5f + i_coordonata_y_fata_cub, -0.5f + i_coordonata_z_fata_cub,         1.0f,  0.0f,  0.0f,         i_textura_fata * (1.0f / i_numar_texturi) + 1.0f / i_numar_texturi,  0.5f,
                     0.5f + i_coordonata_x_fata_cub,  0.5f + i_coordonata_y_fata_cub, -0.5f + i_coordonata_z_fata_cub,         1.0f,  0.0f,  0.0f,         i_textura_fata * (1.0f / i_numar_texturi) + 1.0f / i_numar_texturi,  0.666667f,
                     0.5f + i_coordonata_x_fata_cub,  0.5f + i_coordonata_y_fata_cub,  0.5f + i_coordonata_z_fata_cub,         1.0f,  0.0f,  0.0f,         i_textura_fata * (1.0f / i_numar_texturi) + 0.0f,                    0.666667f,

                     0.5f + i_coordonata_x_fata_cub,  0.5f + i_coordonata_y_fata_cub,  0.5f + i_coordonata_z_fata_cub,         1.0f,  0.0f,  0.0f,         i_textura_fata * (1.0f / i_numar_texturi) + 0.0f,                    0.666667f,
                     0.5f + i_coordonata_x_fata_cub, -0.5f + i_coordonata_y_fata_cub,  0.5f + i_coordonata_z_fata_cub,         1.0f,  0.0f,  0.0f,         i_textura_fata * (1.0f / i_numar_texturi) + 0.0f,                    0.5f,
                     0.5f + i_coordonata_x_fata_cub, -0.5f + i_coordonata_y_fata_cub, -0.5f + i_coordonata_z_fata_cub,         1.0f,  0.0f,  0.0f,         i_textura_fata * (1.0f / i_numar_texturi) + 1.0f / i_numar_texturi,  0.5f,
                    };
                    glBufferSubData(GL_ARRAY_BUFFER, 192 * sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].i_numar_fete_sector_lume_joc, 192, &f_date_fata);
                }
                else
                {
                    if (i_tip_fata == 4)
                    {
                        float f_date_fata[] =
                        {
                            //Aceasta este fata laterala cu normala inspre OX negativ.
                        -0.5f + i_coordonata_x_fata_cub, -0.5f + i_coordonata_y_fata_cub,  0.5f + i_coordonata_z_fata_cub,        -1.0f,  0.0f,  0.0f,         i_textura_fata * (1.0f / i_numar_texturi) + 1.0f / i_numar_texturi,  0.333334f,
                        -0.5f + i_coordonata_x_fata_cub,  0.5f + i_coordonata_y_fata_cub,  0.5f + i_coordonata_z_fata_cub,        -1.0f,  0.0f,  0.0f,         i_textura_fata * (1.0f / i_numar_texturi) + 1.0f / i_numar_texturi,  0.5f,
                        -0.5f + i_coordonata_x_fata_cub,  0.5f + i_coordonata_y_fata_cub, -0.5f + i_coordonata_z_fata_cub,        -1.0f,  0.0f,  0.0f,         i_textura_fata * (1.0f / i_numar_texturi) + 0.0f,                    0.5f,

                        -0.5f + i_coordonata_x_fata_cub,  0.5f + i_coordonata_y_fata_cub, -0.5f + i_coordonata_z_fata_cub,        -1.0f,  0.0f,  0.0f,         i_textura_fata * (1.0f / i_numar_texturi) + 0.0f,                    0.5f,
                        -0.5f + i_coordonata_x_fata_cub, -0.5f + i_coordonata_y_fata_cub, -0.5f + i_coordonata_z_fata_cub,        -1.0f,  0.0f,  0.0f,         i_textura_fata * (1.0f / i_numar_texturi) + 0.0f,                    0.333334f,
                        -0.5f + i_coordonata_x_fata_cub, -0.5f + i_coordonata_y_fata_cub,  0.5f + i_coordonata_z_fata_cub,        -1.0f,  0.0f,  0.0f,         i_textura_fata * (1.0f / i_numar_texturi) + 1.0f / i_numar_texturi,  0.333334f,
                        };
                        glBufferSubData(GL_ARRAY_BUFFER, 192 * sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].i_numar_fete_sector_lume_joc, 192, &f_date_fata);
                    }
                    else
                    {
                        if (i_tip_fata == 5)
                        {
                            float f_date_fata[] =
                            {
                                //Aceasta este fata laterala cu normala inspre OZ pozitiv.
                             0.5f + i_coordonata_x_fata_cub, -0.5f + i_coordonata_y_fata_cub,  0.5f + i_coordonata_z_fata_cub,         0.0f,  0.0f,  1.0f,         i_textura_fata * (1.0f / i_numar_texturi) + 1.0f / i_numar_texturi,  0.166667f,
                             0.5f + i_coordonata_x_fata_cub,  0.5f + i_coordonata_y_fata_cub,  0.5f + i_coordonata_z_fata_cub,         0.0f,  0.0f,  1.0f,         i_textura_fata * (1.0f / i_numar_texturi) + 1.0f / i_numar_texturi,  0.333334f,
                            -0.5f + i_coordonata_x_fata_cub,  0.5f + i_coordonata_y_fata_cub,  0.5f + i_coordonata_z_fata_cub,         0.0f,  0.0f,  1.0f,         i_textura_fata * (1.0f / i_numar_texturi) + 0.0f,                    0.333334f,

                            -0.5f + i_coordonata_x_fata_cub,  0.5f + i_coordonata_y_fata_cub,  0.5f + i_coordonata_z_fata_cub,         0.0f,  0.0f,  1.0f,         i_textura_fata * (1.0f / i_numar_texturi) + 0.0f,                    0.333334f,
                            -0.5f + i_coordonata_x_fata_cub, -0.5f + i_coordonata_y_fata_cub,  0.5f + i_coordonata_z_fata_cub,         0.0f,  0.0f,  1.0f,         i_textura_fata * (1.0f / i_numar_texturi) + 0.0f,                    0.166667f,
                             0.5f + i_coordonata_x_fata_cub, -0.5f + i_coordonata_y_fata_cub,  0.5f + i_coordonata_z_fata_cub,         0.0f,  0.0f,  1.0f,         i_textura_fata * (1.0f / i_numar_texturi) + 1.0f / i_numar_texturi,  0.166667f,
                            };
                            glBufferSubData(GL_ARRAY_BUFFER, 192 * sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].i_numar_fete_sector_lume_joc, 192, &f_date_fata);
                        }
                        else
                        {
                            float f_date_fata[] =
                            {
                                //Aceasta este fata laterala cu normala inspre OZ negativ. 
                            -0.5f + i_coordonata_x_fata_cub, -0.5f + i_coordonata_y_fata_cub, -0.5f + i_coordonata_z_fata_cub,         0.0f,  0.0f, -1.0f,         i_textura_fata * (1.0f / i_numar_texturi) + 1.0f / i_numar_texturi,  0.0f,
                            -0.5f + i_coordonata_x_fata_cub,  0.5f + i_coordonata_y_fata_cub, -0.5f + i_coordonata_z_fata_cub,         0.0f,  0.0f, -1.0f,         i_textura_fata * (1.0f / i_numar_texturi) + 1.0f / i_numar_texturi,  0.166667f,
                             0.5f + i_coordonata_x_fata_cub,  0.5f + i_coordonata_y_fata_cub, -0.5f + i_coordonata_z_fata_cub,         0.0f,  0.0f, -1.0f,         i_textura_fata * (1.0f / i_numar_texturi) + 0.0f,                    0.166667f,

                             0.5f + i_coordonata_x_fata_cub,  0.5f + i_coordonata_y_fata_cub, -0.5f + i_coordonata_z_fata_cub,         0.0f,  0.0f, -1.0f,         i_textura_fata * (1.0f / i_numar_texturi) + 0.0f,                    0.166667f,
                             0.5f + i_coordonata_x_fata_cub, -0.5f + i_coordonata_y_fata_cub, -0.5f + i_coordonata_z_fata_cub,         0.0f,  0.0f, -1.0f,         i_textura_fata * (1.0f / i_numar_texturi) + 0.0f,                    0.0f,
                            -0.5f + i_coordonata_x_fata_cub, -0.5f + i_coordonata_y_fata_cub, -0.5f + i_coordonata_z_fata_cub,         0.0f,  0.0f, -1.0f,         i_textura_fata * (1.0f / i_numar_texturi) + 1.0f / i_numar_texturi,  0.0f,
                            };
                            glBufferSubData(GL_ARRAY_BUFFER, 192 * sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].i_numar_fete_sector_lume_joc, 192, &f_date_fata);
                        }
                    }
                }
            }
        }
    }

    sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].i_numar_fete_sector_lume_joc++;
    sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].i_numar_varfuri_sector_lume_joc += 6;
}

struct textura
{
    unsigned int ui_cod_textura;
    int i_latime, i_inaltime, i_nr_canale;
};

textura atlas_textura_cub;

void v_importare_textura(const char* adresa_textura, textura* textura)
{
    glGenTextures(1, &(textura->ui_cod_textura));
    glBindTexture(GL_TEXTURE_2D, textura->ui_cod_textura);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    unsigned char* date = stbi_load(adresa_textura, &(textura->i_latime), &(textura->i_inaltime), &(textura->i_nr_canale), 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textura->i_latime, textura->i_inaltime, 0, GL_RGBA, GL_UNSIGNED_BYTE, date);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(date);
}

void v_generare_desen_sector_lume_joc(int i_coordonata_locala_x_sector_lume_joc, int i_coordonata_locala_z_sector_lume_joc)
{
    int i_coordonata_x_in_sector;
    int i_coordonata_y_in_sector;
    int i_coordonata_z_in_sector;

    int i_cod_cub;

    int i_coordonata_x;
    int i_coordonata_z;

    glBindVertexArray(vao_sector_lume_joc[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc]);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_sector_lume_joc[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc]);

    sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].i_numar_varfuri_sector_lume_joc = 0;
    sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].i_numar_fete_sector_lume_joc = 0;

    for (i_coordonata_x_in_sector = 0; i_coordonata_x_in_sector < ci_latime_sector_lume_joc; i_coordonata_x_in_sector++)
    {
        i_coordonata_x = sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].i_coordonata_x_sector_lume_joc * ci_latime_sector_lume_joc + i_coordonata_x_in_sector;

        for (i_coordonata_z_in_sector = 0; i_coordonata_z_in_sector < ci_lungime_sector_lume_joc; i_coordonata_z_in_sector++)
        {
            i_coordonata_z = sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].i_coordonata_z_sector_lume_joc * ci_lungime_sector_lume_joc + i_coordonata_z_in_sector;

            for (i_coordonata_y_in_sector = 0; i_coordonata_y_in_sector < ci_inaltime_sector_lume_joc; i_coordonata_y_in_sector++)
            {
                if (sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector][i_coordonata_y_in_sector][i_coordonata_z_in_sector] != 0)
                {
                    i_cod_cub = sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector][i_coordonata_y_in_sector][i_coordonata_z_in_sector];

                    if (i_coordonata_y_in_sector == ci_inaltime_sector_lume_joc - 1)
                    {
                        v_adaugare_fata_cub(i_cod_cub, 1, i_coordonata_x, i_coordonata_y_in_sector, i_coordonata_z, i_coordonata_locala_x_sector_lume_joc, i_coordonata_locala_z_sector_lume_joc);
                    }
                    else
                    {
                        if (sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector][i_coordonata_y_in_sector + 1][i_coordonata_z_in_sector] == 0)//Va trebui aici sa verificam nu doar daca e aer(0), dar si apa, ca vom face apa practic transparenta.
                        {
                            v_adaugare_fata_cub(i_cod_cub, 1, i_coordonata_x, i_coordonata_y_in_sector, i_coordonata_z, i_coordonata_locala_x_sector_lume_joc, i_coordonata_locala_z_sector_lume_joc);
                        }
                    }
                    if (i_coordonata_y_in_sector == 0)
                    {
                        v_adaugare_fata_cub(i_cod_cub, 2, i_coordonata_x, i_coordonata_y_in_sector, i_coordonata_z, i_coordonata_locala_x_sector_lume_joc, i_coordonata_locala_z_sector_lume_joc);
                    }
                    else
                    {
                        if (sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector][i_coordonata_y_in_sector - 1][i_coordonata_z_in_sector] == 0)//Va trebui aici sa verificam nu doar daca e aer(0), dar si apa, ca vom face apa practic transparenta.
                        {
                            v_adaugare_fata_cub(i_cod_cub, 2, i_coordonata_x, i_coordonata_y_in_sector, i_coordonata_z, i_coordonata_locala_x_sector_lume_joc, i_coordonata_locala_z_sector_lume_joc);
                        }
                    }
                    if (i_coordonata_x_in_sector == ci_latime_sector_lume_joc - 1)
                    {
                        if (i_coordonata_locala_x_sector_lume_joc != ci_numar_sectoare_vizibile_latime_lume_joc - 1)
                        {
                            if (sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc + 1][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[0][i_coordonata_y_in_sector][i_coordonata_z_in_sector] == 0)//Va trebui aici sa verificam nu doar daca e aer(0), dar si apa, ca vom face apa practic transparenta.
                            {
                                v_adaugare_fata_cub(i_cod_cub, 3, i_coordonata_x, i_coordonata_y_in_sector, i_coordonata_z, i_coordonata_locala_x_sector_lume_joc, i_coordonata_locala_z_sector_lume_joc);
                            }
                        }
                        else
                        {
                            v_adaugare_fata_cub(i_cod_cub, 3, i_coordonata_x, i_coordonata_y_in_sector, i_coordonata_z, i_coordonata_locala_x_sector_lume_joc, i_coordonata_locala_z_sector_lume_joc);
                        }
                    }
                    else
                    {
                        if (sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector + 1][i_coordonata_y_in_sector][i_coordonata_z_in_sector] == 0)//Va trebui aici sa verificam nu doar daca e aer(0), dar si apa, ca vom face apa practic transparenta.
                        {
                            v_adaugare_fata_cub(i_cod_cub, 3, i_coordonata_x, i_coordonata_y_in_sector, i_coordonata_z, i_coordonata_locala_x_sector_lume_joc, i_coordonata_locala_z_sector_lume_joc);
                        }
                    }
                    if (i_coordonata_x_in_sector == 0)
                    {
                        if (i_coordonata_locala_x_sector_lume_joc != 0)
                        {
                            if (sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc - 1][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[ci_latime_sector_lume_joc - 1][i_coordonata_y_in_sector][i_coordonata_z_in_sector] == 0)//Va trebui aici sa verificam nu doar daca e aer(0), dar si apa, ca vom face apa practic transparenta.
                            {
                                v_adaugare_fata_cub(i_cod_cub, 4, i_coordonata_x, i_coordonata_y_in_sector, i_coordonata_z, i_coordonata_locala_x_sector_lume_joc, i_coordonata_locala_z_sector_lume_joc);
                            }
                        }
                        else
                        {
                            v_adaugare_fata_cub(i_cod_cub, 4, i_coordonata_x, i_coordonata_y_in_sector, i_coordonata_z, i_coordonata_locala_x_sector_lume_joc, i_coordonata_locala_z_sector_lume_joc);
                        }
                    }
                    else
                    {
                        if (sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector - 1][i_coordonata_y_in_sector][i_coordonata_z_in_sector] == 0)//Va trebui aici sa verificam nu doar daca e aer(0), dar si apa, ca vom face apa practic transparenta.
                        {
                            v_adaugare_fata_cub(i_cod_cub, 4, i_coordonata_x, i_coordonata_y_in_sector, i_coordonata_z, i_coordonata_locala_x_sector_lume_joc, i_coordonata_locala_z_sector_lume_joc);
                        }
                    }
                    if (i_coordonata_z_in_sector == ci_lungime_sector_lume_joc - 1)
                    {
                        if (i_coordonata_locala_z_sector_lume_joc != ci_numar_sectoare_vizibile_lungime_lume_joc - 1)
                        {
                            if (sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc + 1].c_cod_cub[i_coordonata_x_in_sector][i_coordonata_y_in_sector][0] == 0)//Va trebui aici sa verificam nu doar daca e aer(0), dar si apa, ca vom face apa practic transparenta.
                            {
                                v_adaugare_fata_cub(i_cod_cub, 5, i_coordonata_x, i_coordonata_y_in_sector, i_coordonata_z, i_coordonata_locala_x_sector_lume_joc, i_coordonata_locala_z_sector_lume_joc);
                            }
                        }
                        else
                        {
                            v_adaugare_fata_cub(i_cod_cub, 5, i_coordonata_x, i_coordonata_y_in_sector, i_coordonata_z, i_coordonata_locala_x_sector_lume_joc, i_coordonata_locala_z_sector_lume_joc);
                        }
                    }
                    else
                    {
                        if (sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector][i_coordonata_y_in_sector][i_coordonata_z_in_sector + 1] == 0)//Va trebui aici sa verificam nu doar daca e aer(0), dar si apa, ca vom face apa practic transparenta.
                        {
                            v_adaugare_fata_cub(i_cod_cub, 5, i_coordonata_x, i_coordonata_y_in_sector, i_coordonata_z, i_coordonata_locala_x_sector_lume_joc, i_coordonata_locala_z_sector_lume_joc);
                        }
                    }
                    if (i_coordonata_z_in_sector == 0)
                    {
                        if (i_coordonata_locala_z_sector_lume_joc != 0)
                        {
                            if (sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc - 1].c_cod_cub[i_coordonata_x_in_sector][i_coordonata_y_in_sector][ci_lungime_sector_lume_joc - 1] == 0)//Va trebui aici sa verificam nu doar daca e aer(0), dar si apa, ca vom face apa practic transparenta.
                            {
                                v_adaugare_fata_cub(i_cod_cub, 6, i_coordonata_x, i_coordonata_y_in_sector, i_coordonata_z, i_coordonata_locala_x_sector_lume_joc, i_coordonata_locala_z_sector_lume_joc);
                            }
                        }
                        else
                        {
                            v_adaugare_fata_cub(i_cod_cub, 6, i_coordonata_x, i_coordonata_y_in_sector, i_coordonata_z, i_coordonata_locala_x_sector_lume_joc, i_coordonata_locala_z_sector_lume_joc);
                        }
                    }
                    else
                    {
                        if (sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector][i_coordonata_y_in_sector][i_coordonata_z_in_sector - 1] == 0)//Va trebui aici sa verificam nu doar daca e aer(0), dar si apa, ca vom face apa practic transparenta.
                        {
                            v_adaugare_fata_cub(i_cod_cub, 6, i_coordonata_x, i_coordonata_y_in_sector, i_coordonata_z, i_coordonata_locala_x_sector_lume_joc, i_coordonata_locala_z_sector_lume_joc);
                        }
                    }
                }
            }
        }
    }
}

void v_date_intrare_tastatura(GLFWwindow* fereastra_joc)
{
    int i_coordonata_locala_x_sector_lume_joc;
    int i_coordonata_locala_z_sector_lume_joc;

    if (glfwGetKey(fereastra_joc, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(fereastra_joc, 1);
    }
    if (glfwGetKey(fereastra_joc, GLFW_KEY_W) == GLFW_PRESS)
    {
        coordonate_camera += directie_privire_camera * f_viteza_translatie_camera * f_durata_timp_cadru;
        b_camera_translatata = 1;
    }
    if (glfwGetKey(fereastra_joc, GLFW_KEY_S) == GLFW_PRESS)
    {
        coordonate_camera -= directie_privire_camera * f_viteza_translatie_camera * f_durata_timp_cadru;
        b_camera_translatata = 1;
    }
    if (glfwGetKey(fereastra_joc, GLFW_KEY_A) == GLFW_PRESS)
    {
        coordonate_camera -= glm::normalize(glm::cross(directie_privire_camera, normala_camera)) * f_viteza_translatie_camera * f_durata_timp_cadru;
        b_camera_translatata = 1;
    }
    if (glfwGetKey(fereastra_joc, GLFW_KEY_D) == GLFW_PRESS)
    {
        coordonate_camera += glm::normalize(glm::cross(directie_privire_camera, normala_camera)) * f_viteza_translatie_camera * f_durata_timp_cadru;
        b_camera_translatata = 1;
    }
    if (glfwGetKey(fereastra_joc, GLFW_KEY_Q) == GLFW_PRESS)
    {
        coordonate_camera.y += f_viteza_translatie_camera * f_durata_timp_cadru;
        b_camera_translatata = 1;
    }
    if (glfwGetKey(fereastra_joc, GLFW_KEY_E) == GLFW_PRESS)
    {
        coordonate_camera.y -= f_viteza_translatie_camera * f_durata_timp_cadru;
        b_camera_translatata = 1;
    }

    if (b_camera_translatata == 1)
    {
        if (coordonate_camera.x < 0.0f)
        {
            i_coordonata_veche_x_sector_lume_joc_camera = i_coordonata_x_sector_lume_joc_camera;
            i_coordonata_x_sector_lume_joc_camera = coordonate_camera.x / ci_latime_sector_lume_joc - 1;
        }
        else
        {
            i_coordonata_veche_x_sector_lume_joc_camera = i_coordonata_x_sector_lume_joc_camera;
            i_coordonata_x_sector_lume_joc_camera = coordonate_camera.x / ci_latime_sector_lume_joc;
        }
        if (coordonate_camera.z < 0.0f)
        {
            i_coordonata_veche_z_sector_lume_joc_camera = i_coordonata_z_sector_lume_joc_camera;
            i_coordonata_z_sector_lume_joc_camera = coordonate_camera.z / ci_lungime_sector_lume_joc - 1;
        }
        else
        {
            i_coordonata_veche_z_sector_lume_joc_camera = i_coordonata_z_sector_lume_joc_camera;
            i_coordonata_z_sector_lume_joc_camera = coordonate_camera.z / ci_lungime_sector_lume_joc;
        }

        i_variatie_coordonata_x_sector_lume_joc_camera = i_coordonata_x_sector_lume_joc_camera - i_coordonata_veche_x_sector_lume_joc_camera;
        i_variatie_coordonata_z_sector_lume_joc_camera = i_coordonata_z_sector_lume_joc_camera - i_coordonata_veche_z_sector_lume_joc_camera;

        if (i_variatie_coordonata_x_sector_lume_joc_camera > 0)
        {
            for (i_coordonata_locala_x_sector_lume_joc = i_variatie_coordonata_x_sector_lume_joc_camera; i_coordonata_locala_x_sector_lume_joc < ci_numar_sectoare_vizibile_latime_lume_joc; i_coordonata_locala_x_sector_lume_joc++)
            {
                for (i_coordonata_locala_z_sector_lume_joc = 0; i_coordonata_locala_z_sector_lume_joc < ci_numar_sectoare_vizibile_lungime_lume_joc; i_coordonata_locala_z_sector_lume_joc++)
                {
                    glBindBuffer(GL_COPY_READ_BUFFER, vbo_sector_lume_joc[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc]);
                    glBindBuffer(GL_COPY_WRITE_BUFFER, vbo_sector_lume_joc[i_coordonata_locala_x_sector_lume_joc - i_variatie_coordonata_x_sector_lume_joc_camera][i_coordonata_locala_z_sector_lume_joc]);
                    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, 192 * sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].i_numar_fete_sector_lume_joc);
                    sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc - i_variatie_coordonata_x_sector_lume_joc_camera][i_coordonata_locala_z_sector_lume_joc] = sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc];
                }
            }
        }
        else
        {
            if (i_variatie_coordonata_x_sector_lume_joc_camera < 0)
            {
                for (i_coordonata_locala_x_sector_lume_joc = ci_numar_sectoare_vizibile_latime_lume_joc - 1; i_coordonata_locala_x_sector_lume_joc >= -i_variatie_coordonata_x_sector_lume_joc_camera; i_coordonata_locala_x_sector_lume_joc--)
                {
                    for (i_coordonata_locala_z_sector_lume_joc = 0; i_coordonata_locala_z_sector_lume_joc < ci_numar_sectoare_vizibile_lungime_lume_joc; i_coordonata_locala_z_sector_lume_joc++)
                    {
                        glBindBuffer(GL_COPY_READ_BUFFER, vbo_sector_lume_joc[i_coordonata_locala_x_sector_lume_joc + i_variatie_coordonata_x_sector_lume_joc_camera][i_coordonata_locala_z_sector_lume_joc]);
                        glBindBuffer(GL_COPY_WRITE_BUFFER, vbo_sector_lume_joc[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc]);
                        glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, 192 * sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc + i_variatie_coordonata_x_sector_lume_joc_camera][i_coordonata_locala_z_sector_lume_joc].i_numar_fete_sector_lume_joc);
                        sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc] = sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc + i_variatie_coordonata_x_sector_lume_joc_camera][i_coordonata_locala_z_sector_lume_joc];
                    }
                }
            }
        }

        if (i_variatie_coordonata_z_sector_lume_joc_camera > 0)
        {
            for (i_coordonata_locala_z_sector_lume_joc = i_variatie_coordonata_z_sector_lume_joc_camera; i_coordonata_locala_z_sector_lume_joc < ci_numar_sectoare_vizibile_lungime_lume_joc; i_coordonata_locala_z_sector_lume_joc++)
            {
                for (i_coordonata_locala_x_sector_lume_joc = 0; i_coordonata_locala_x_sector_lume_joc < ci_numar_sectoare_vizibile_latime_lume_joc; i_coordonata_locala_x_sector_lume_joc++)
                {
                    glBindBuffer(GL_COPY_READ_BUFFER, vbo_sector_lume_joc[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc]);
                    glBindBuffer(GL_COPY_WRITE_BUFFER, vbo_sector_lume_joc[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc - i_variatie_coordonata_z_sector_lume_joc_camera]);
                    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, 192 * sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].i_numar_fete_sector_lume_joc);
                    sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc - i_variatie_coordonata_z_sector_lume_joc_camera] = sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc];
                }
            }
        }
        else
        {
            if (i_variatie_coordonata_z_sector_lume_joc_camera < 0)
            {
                for (i_coordonata_locala_z_sector_lume_joc = ci_numar_sectoare_vizibile_lungime_lume_joc - 1; i_coordonata_locala_z_sector_lume_joc >= -i_variatie_coordonata_z_sector_lume_joc_camera; i_coordonata_locala_z_sector_lume_joc--)
                {
                    for (i_coordonata_locala_x_sector_lume_joc = 0; i_coordonata_locala_x_sector_lume_joc < ci_numar_sectoare_vizibile_latime_lume_joc; i_coordonata_locala_x_sector_lume_joc++)
                    {
                        glBindBuffer(GL_COPY_READ_BUFFER, vbo_sector_lume_joc[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc + i_variatie_coordonata_z_sector_lume_joc_camera]);
                        glBindBuffer(GL_COPY_WRITE_BUFFER, vbo_sector_lume_joc[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc]);
                        glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, 192 * sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc + i_variatie_coordonata_z_sector_lume_joc_camera].i_numar_fete_sector_lume_joc);
                        sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc] = sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc + i_variatie_coordonata_z_sector_lume_joc_camera];
                    }
                }
            }
        }

        b_camera_translatata = 0;
    }

    if (glfwGetKey(fereastra_joc, GLFW_KEY_1) == GLFW_PRESS)
    {
        c_cod_cub_selectat = 1;
    }
    else
    {
        if (glfwGetKey(fereastra_joc, GLFW_KEY_2) == GLFW_PRESS)
        {
            c_cod_cub_selectat = 2;
        }
        else
        {
            if (glfwGetKey(fereastra_joc, GLFW_KEY_3) == GLFW_PRESS)
            {
                c_cod_cub_selectat = 3;
            }
            else
            {
                if (glfwGetKey(fereastra_joc, GLFW_KEY_4) == GLFW_PRESS)
                {
                    c_cod_cub_selectat = 4;
                }
                else
                {
                    if (glfwGetKey(fereastra_joc, GLFW_KEY_5) == GLFW_PRESS)
                    {
                        c_cod_cub_selectat = 5;
                    }
                    else
                    {
                        if (glfwGetKey(fereastra_joc, GLFW_KEY_6) == GLFW_PRESS)
                        {
                            c_cod_cub_selectat = 6;
                        }
                        else
                        {
                            if (glfwGetKey(fereastra_joc, GLFW_KEY_7) == GLFW_PRESS)
                            {
                                c_cod_cub_selectat = 7;
                            }
                            else
                            {
                                if (glfwGetKey(fereastra_joc, GLFW_KEY_8) == GLFW_PRESS)
                                {
                                    c_cod_cub_selectat = 8;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    if (glfwGetKey(fereastra_joc, GLFW_KEY_P) == GLFW_PRESS) //Momentan avem butonul de P pentru a plasa un bloc ales anterior.
    {
        if (!b_lume_joc_editata || (float)glfwGetTime() - f_timp_scurs_ultima_editare_lume_joc >= 0.5f)
        {

            raza_editare_lume_joc.viteza_raza = raza_editare_lume_joc.f_viteza_raza * directie_privire_camera;
            raza_editare_lume_joc.coordonate_raza = coordonate_camera - raza_editare_lume_joc.viteza_raza;

            do
            {
                raza_editare_lume_joc.coordonate_raza += raza_editare_lume_joc.viteza_raza;

                if (raza_editare_lume_joc.coordonate_raza.x < 0.0f)
                {
                    raza_editare_lume_joc.i_coordonata_x_sector_lume_joc = raza_editare_lume_joc.coordonate_raza.x / ci_latime_sector_lume_joc - 1;
                }
                else
                {
                    raza_editare_lume_joc.i_coordonata_x_sector_lume_joc = raza_editare_lume_joc.coordonate_raza.x / ci_latime_sector_lume_joc;
                }

                if (raza_editare_lume_joc.coordonate_raza.z < 0.0f)
                {
                    raza_editare_lume_joc.i_coordonata_z_sector_lume_joc = raza_editare_lume_joc.coordonate_raza.z / ci_lungime_sector_lume_joc - 1;
                }
                else
                {
                    raza_editare_lume_joc.i_coordonata_z_sector_lume_joc = raza_editare_lume_joc.coordonate_raza.z / ci_lungime_sector_lume_joc;
                }

                raza_editare_lume_joc.i_coordonata_x_in_sector = raza_editare_lume_joc.coordonate_raza.x - raza_editare_lume_joc.i_coordonata_x_sector_lume_joc * ci_latime_sector_lume_joc;
                raza_editare_lume_joc.i_coordonata_z_in_sector = raza_editare_lume_joc.coordonate_raza.z - raza_editare_lume_joc.i_coordonata_z_sector_lume_joc * ci_lungime_sector_lume_joc;

                raza_editare_lume_joc.i_coordonata_locala_x_sector_lume_joc = raza_editare_lume_joc.i_coordonata_x_sector_lume_joc + i_coordonata_locala_x_sector_lume_joc_camera - i_coordonata_x_sector_lume_joc_camera;
                raza_editare_lume_joc.i_coordonata_locala_z_sector_lume_joc = raza_editare_lume_joc.i_coordonata_z_sector_lume_joc + i_coordonata_locala_z_sector_lume_joc_camera - i_coordonata_z_sector_lume_joc_camera;

            } while ((((raza_editare_lume_joc.coordonate_raza.y >= (float)ci_inaltime_sector_lume_joc || raza_editare_lume_joc.coordonate_raza.y < 0.0f) || (raza_editare_lume_joc.i_coordonata_locala_x_sector_lume_joc >= ci_numar_sectoare_vizibile_latime_lume_joc || raza_editare_lume_joc.i_coordonata_locala_x_sector_lume_joc < 0) || (raza_editare_lume_joc.i_coordonata_locala_z_sector_lume_joc >= ci_numar_sectoare_vizibile_lungime_lume_joc || raza_editare_lume_joc.i_coordonata_locala_z_sector_lume_joc < 0)) || sector_lume_joc_vizibil[raza_editare_lume_joc.i_coordonata_locala_x_sector_lume_joc][raza_editare_lume_joc.i_coordonata_locala_z_sector_lume_joc].c_cod_cub[raza_editare_lume_joc.i_coordonata_x_in_sector][(int)raza_editare_lume_joc.coordonate_raza.y][raza_editare_lume_joc.i_coordonata_z_in_sector] == 0) && glm::length(raza_editare_lume_joc.coordonate_raza - coordonate_camera) <= f_distanta_maxima_editare_lume_joc);

            if (glm::length(raza_editare_lume_joc.coordonate_raza - coordonate_camera) <= f_distanta_maxima_editare_lume_joc)
            {

                raza_editare_lume_joc.coordonate_raza -= raza_editare_lume_joc.viteza_raza;

                if (raza_editare_lume_joc.coordonate_raza.x < 0.0f)
                {
                    raza_editare_lume_joc.i_coordonata_x_sector_lume_joc = raza_editare_lume_joc.coordonate_raza.x / ci_latime_sector_lume_joc - 1;
                }
                else
                {
                    raza_editare_lume_joc.i_coordonata_x_sector_lume_joc = raza_editare_lume_joc.coordonate_raza.x / ci_latime_sector_lume_joc;
                }

                if (raza_editare_lume_joc.coordonate_raza.z < 0.0f)
                {
                    raza_editare_lume_joc.i_coordonata_z_sector_lume_joc = raza_editare_lume_joc.coordonate_raza.z / ci_lungime_sector_lume_joc - 1;
                }
                else
                {
                    raza_editare_lume_joc.i_coordonata_z_sector_lume_joc = raza_editare_lume_joc.coordonate_raza.z / ci_lungime_sector_lume_joc;
                }

                raza_editare_lume_joc.i_coordonata_x_in_sector = raza_editare_lume_joc.coordonate_raza.x - raza_editare_lume_joc.i_coordonata_x_sector_lume_joc * ci_latime_sector_lume_joc;
                raza_editare_lume_joc.i_coordonata_z_in_sector = raza_editare_lume_joc.coordonate_raza.z - raza_editare_lume_joc.i_coordonata_z_sector_lume_joc * ci_lungime_sector_lume_joc;

                raza_editare_lume_joc.i_coordonata_locala_x_sector_lume_joc = raza_editare_lume_joc.i_coordonata_x_sector_lume_joc + i_coordonata_locala_x_sector_lume_joc_camera - i_coordonata_x_sector_lume_joc_camera;
                raza_editare_lume_joc.i_coordonata_locala_z_sector_lume_joc = raza_editare_lume_joc.i_coordonata_z_sector_lume_joc + i_coordonata_locala_z_sector_lume_joc_camera - i_coordonata_z_sector_lume_joc_camera;

                if ((raza_editare_lume_joc.coordonate_raza.y < ci_inaltime_sector_lume_joc && raza_editare_lume_joc.coordonate_raza.y >= 0) && (raza_editare_lume_joc.i_coordonata_locala_x_sector_lume_joc < ci_numar_sectoare_vizibile_latime_lume_joc && raza_editare_lume_joc.i_coordonata_locala_x_sector_lume_joc >= 0) && (raza_editare_lume_joc.i_coordonata_locala_z_sector_lume_joc < ci_numar_sectoare_vizibile_lungime_lume_joc && raza_editare_lume_joc.i_coordonata_locala_z_sector_lume_joc >= 0))
                {
                    sector_lume_joc_vizibil[raza_editare_lume_joc.i_coordonata_locala_x_sector_lume_joc][raza_editare_lume_joc.i_coordonata_locala_z_sector_lume_joc].c_cod_cub[raza_editare_lume_joc.i_coordonata_x_in_sector][(int)raza_editare_lume_joc.coordonate_raza.y][raza_editare_lume_joc.i_coordonata_z_in_sector] = c_cod_cub_selectat;
                    v_generare_desen_sector_lume_joc(raza_editare_lume_joc.i_coordonata_locala_x_sector_lume_joc, raza_editare_lume_joc.i_coordonata_locala_z_sector_lume_joc);

                    if (raza_editare_lume_joc.i_coordonata_x_in_sector == 0 && raza_editare_lume_joc.i_coordonata_locala_x_sector_lume_joc >= 1)
                    {
                        v_generare_desen_sector_lume_joc(raza_editare_lume_joc.i_coordonata_locala_x_sector_lume_joc - 1, raza_editare_lume_joc.i_coordonata_locala_z_sector_lume_joc);
                    }
                    if (raza_editare_lume_joc.i_coordonata_x_in_sector == ci_latime_sector_lume_joc - 1 && raza_editare_lume_joc.i_coordonata_locala_x_sector_lume_joc < ci_numar_sectoare_vizibile_latime_lume_joc - 1)
                    {
                        v_generare_desen_sector_lume_joc(raza_editare_lume_joc.i_coordonata_locala_x_sector_lume_joc + 1, raza_editare_lume_joc.i_coordonata_locala_z_sector_lume_joc);
                    }
                    if (raza_editare_lume_joc.i_coordonata_z_in_sector == 0 && raza_editare_lume_joc.i_coordonata_locala_z_sector_lume_joc >= 1)
                    {
                        v_generare_desen_sector_lume_joc(raza_editare_lume_joc.i_coordonata_locala_x_sector_lume_joc, raza_editare_lume_joc.i_coordonata_locala_z_sector_lume_joc - 1);
                    }
                    if (raza_editare_lume_joc.i_coordonata_x_in_sector == ci_lungime_sector_lume_joc - 1 && raza_editare_lume_joc.i_coordonata_locala_z_sector_lume_joc < ci_numar_sectoare_vizibile_lungime_lume_joc - 1)
                    {
                        v_generare_desen_sector_lume_joc(raza_editare_lume_joc.i_coordonata_locala_x_sector_lume_joc, raza_editare_lume_joc.i_coordonata_locala_z_sector_lume_joc + 1);
                    }

                    f_timp_scurs_ultima_editare_lume_joc = (float)glfwGetTime();
                    b_lume_joc_editata = 1;
                }

            }
        }
    }
    else
    {
        if (glfwGetKey(fereastra_joc, GLFW_KEY_X) == GLFW_PRESS) //Momentan avem butonul de X pentru spargerea blocurilor.
        {
            if (!b_lume_joc_editata || (float)glfwGetTime() - f_timp_scurs_ultima_editare_lume_joc >= 0.5f)
            {

                raza_editare_lume_joc.viteza_raza = raza_editare_lume_joc.f_viteza_raza * directie_privire_camera;
                raza_editare_lume_joc.coordonate_raza = coordonate_camera - raza_editare_lume_joc.viteza_raza;

                do
                {
                    raza_editare_lume_joc.coordonate_raza += raza_editare_lume_joc.viteza_raza;

                    if (raza_editare_lume_joc.coordonate_raza.x < 0.0f)
                    {
                        raza_editare_lume_joc.i_coordonata_x_sector_lume_joc = raza_editare_lume_joc.coordonate_raza.x / ci_latime_sector_lume_joc - 1;
                    }
                    else
                    {
                        raza_editare_lume_joc.i_coordonata_x_sector_lume_joc = raza_editare_lume_joc.coordonate_raza.x / ci_latime_sector_lume_joc;
                    }

                    if (raza_editare_lume_joc.coordonate_raza.z < 0.0f)
                    {
                        raza_editare_lume_joc.i_coordonata_z_sector_lume_joc = raza_editare_lume_joc.coordonate_raza.z / ci_lungime_sector_lume_joc - 1;
                    }
                    else
                    {
                        raza_editare_lume_joc.i_coordonata_z_sector_lume_joc = raza_editare_lume_joc.coordonate_raza.z / ci_lungime_sector_lume_joc;
                    }

                    raza_editare_lume_joc.i_coordonata_x_in_sector = raza_editare_lume_joc.coordonate_raza.x - raza_editare_lume_joc.i_coordonata_x_sector_lume_joc * ci_latime_sector_lume_joc;
                    raza_editare_lume_joc.i_coordonata_z_in_sector = raza_editare_lume_joc.coordonate_raza.z - raza_editare_lume_joc.i_coordonata_z_sector_lume_joc * ci_lungime_sector_lume_joc;

                    raza_editare_lume_joc.i_coordonata_locala_x_sector_lume_joc = raza_editare_lume_joc.i_coordonata_x_sector_lume_joc + i_coordonata_locala_x_sector_lume_joc_camera - i_coordonata_x_sector_lume_joc_camera;
                    raza_editare_lume_joc.i_coordonata_locala_z_sector_lume_joc = raza_editare_lume_joc.i_coordonata_z_sector_lume_joc + i_coordonata_locala_z_sector_lume_joc_camera - i_coordonata_z_sector_lume_joc_camera;

                } while ((((raza_editare_lume_joc.coordonate_raza.y >= (float)ci_inaltime_sector_lume_joc || raza_editare_lume_joc.coordonate_raza.y < 0.0f) || (raza_editare_lume_joc.i_coordonata_locala_x_sector_lume_joc >= ci_numar_sectoare_vizibile_latime_lume_joc || raza_editare_lume_joc.i_coordonata_locala_x_sector_lume_joc < 0) || (raza_editare_lume_joc.i_coordonata_locala_z_sector_lume_joc >= ci_numar_sectoare_vizibile_lungime_lume_joc || raza_editare_lume_joc.i_coordonata_locala_z_sector_lume_joc < 0)) || sector_lume_joc_vizibil[raza_editare_lume_joc.i_coordonata_locala_x_sector_lume_joc][raza_editare_lume_joc.i_coordonata_locala_z_sector_lume_joc].c_cod_cub[raza_editare_lume_joc.i_coordonata_x_in_sector][(int)raza_editare_lume_joc.coordonate_raza.y][raza_editare_lume_joc.i_coordonata_z_in_sector] == 0) && glm::length(raza_editare_lume_joc.coordonate_raza - coordonate_camera) <= f_distanta_maxima_editare_lume_joc);

                if (glm::length(raza_editare_lume_joc.coordonate_raza - coordonate_camera) <= f_distanta_maxima_editare_lume_joc)
                {
                    sector_lume_joc_vizibil[raza_editare_lume_joc.i_coordonata_locala_x_sector_lume_joc][raza_editare_lume_joc.i_coordonata_locala_z_sector_lume_joc].c_cod_cub[raza_editare_lume_joc.i_coordonata_x_in_sector][(int)raza_editare_lume_joc.coordonate_raza.y][raza_editare_lume_joc.i_coordonata_z_in_sector] = 0;
                    v_generare_desen_sector_lume_joc(raza_editare_lume_joc.i_coordonata_locala_x_sector_lume_joc, raza_editare_lume_joc.i_coordonata_locala_z_sector_lume_joc);

                    if (raza_editare_lume_joc.i_coordonata_x_in_sector == 0 && raza_editare_lume_joc.i_coordonata_locala_x_sector_lume_joc >= 1)
                    {
                        v_generare_desen_sector_lume_joc(raza_editare_lume_joc.i_coordonata_locala_x_sector_lume_joc - 1, raza_editare_lume_joc.i_coordonata_locala_z_sector_lume_joc);
                    }
                    if (raza_editare_lume_joc.i_coordonata_x_in_sector == ci_latime_sector_lume_joc - 1 && raza_editare_lume_joc.i_coordonata_locala_x_sector_lume_joc < ci_numar_sectoare_vizibile_latime_lume_joc - 1)
                    {
                        v_generare_desen_sector_lume_joc(raza_editare_lume_joc.i_coordonata_locala_x_sector_lume_joc + 1, raza_editare_lume_joc.i_coordonata_locala_z_sector_lume_joc);
                    }
                    if (raza_editare_lume_joc.i_coordonata_z_in_sector == 0 && raza_editare_lume_joc.i_coordonata_locala_z_sector_lume_joc >= 1)
                    {
                        v_generare_desen_sector_lume_joc(raza_editare_lume_joc.i_coordonata_locala_x_sector_lume_joc, raza_editare_lume_joc.i_coordonata_locala_z_sector_lume_joc - 1);
                    }
                    if (raza_editare_lume_joc.i_coordonata_x_in_sector == ci_lungime_sector_lume_joc - 1 && raza_editare_lume_joc.i_coordonata_locala_z_sector_lume_joc < ci_numar_sectoare_vizibile_lungime_lume_joc - 1)
                    {
                        v_generare_desen_sector_lume_joc(raza_editare_lume_joc.i_coordonata_locala_x_sector_lume_joc, raza_editare_lume_joc.i_coordonata_locala_z_sector_lume_joc + 1);
                    }

                    f_timp_scurs_ultima_editare_lume_joc = (float)glfwGetTime();
                    b_lume_joc_editata = 1;
                }
            }
        }
        else
        {
            b_lume_joc_editata = 0;
        }
    }
}

void v_generare_sector_lume_joc(int i_coordonata_locala_x_sector_lume_joc, int i_coordonata_locala_z_sector_lume_joc, int i_coordonata_x_sector_lume_joc, int i_coordonata_z_sector_lume_joc)
{
    sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].i_coordonata_x_sector_lume_joc = i_coordonata_x_sector_lume_joc;
    sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].i_coordonata_z_sector_lume_joc = i_coordonata_z_sector_lume_joc;

    int i_coordonata_x_in_sector;
    int i_coordonata_y_in_sector;
    int i_coordonata_z_in_sector;

    int i_inaltime_coloana_cuburi;

    int i_coordonata_x;
    int i_coordonata_z;

    for (i_coordonata_x_in_sector = 0, i_coordonata_x = sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].i_coordonata_x_sector_lume_joc * ci_latime_sector_lume_joc; i_coordonata_x_in_sector < ci_latime_sector_lume_joc; i_coordonata_x_in_sector++, i_coordonata_x++)
    {
        for (i_coordonata_z_in_sector = 0, i_coordonata_z = sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].i_coordonata_z_sector_lume_joc * ci_lungime_sector_lume_joc; i_coordonata_z_in_sector < ci_lungime_sector_lume_joc; i_coordonata_z_in_sector++, i_coordonata_z++)
        {
            i_inaltime_coloana_cuburi = i_inaltime_medie_sector_lume_joc + (int)(generare_lume_joc.GetNoise(i_coordonata_x, i_coordonata_z) * i_amplitudine_sector_lume_joc);

            for (i_coordonata_y_in_sector = ci_inaltime_sector_lume_joc - 1; i_coordonata_y_in_sector >= i_inaltime_coloana_cuburi; i_coordonata_y_in_sector--)
            {
                if (i_coordonata_y_in_sector >= i_inaltime_nivel_apa_lume_joc)
                {
                    sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector][i_coordonata_y_in_sector][i_coordonata_z_in_sector] = 0;
                }
                else
                {
                    sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector][i_coordonata_y_in_sector][i_coordonata_z_in_sector] = 7;
                }
            }

            for (i_coordonata_y_in_sector = i_inaltime_coloana_cuburi - 1; i_coordonata_y_in_sector >= 0; i_coordonata_y_in_sector--)
            {
                if (i_coordonata_y_in_sector < i_inaltime_nivel_apa_lume_joc + 3 && i_coordonata_y_in_sector == i_inaltime_coloana_cuburi - 1)
                {
                    sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector][i_coordonata_y_in_sector][i_coordonata_z_in_sector] = 4;
                }
                else
                {
                    if (i_coordonata_y_in_sector == i_inaltime_coloana_cuburi - 1)
                    {
                        sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector][i_coordonata_y_in_sector][i_coordonata_z_in_sector] = 1;
                    }
                    else
                    {
                        if (i_coordonata_y_in_sector >= i_inaltime_coloana_cuburi - 4)
                        {
                            sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector][i_coordonata_y_in_sector][i_coordonata_z_in_sector] = 2;
                        }
                        else
                        {
                            sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector][i_coordonata_y_in_sector][i_coordonata_z_in_sector] = 3;
                        }
                    }
                }
            }
        }

    }

    srand(i_coordonata_x_sector_lume_joc + i_coordonata_z_sector_lume_joc);

    int i_inaltime_copac;
    int i_inaltime_cactus;

    int i;

    for (i_coordonata_x_in_sector = 2, i_coordonata_x = sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].i_coordonata_x_sector_lume_joc * ci_latime_sector_lume_joc + 2; i_coordonata_x_in_sector < ci_latime_sector_lume_joc - 2; i_coordonata_x_in_sector++, i_coordonata_x++)
    {
        for (i_coordonata_z_in_sector = 2, i_coordonata_z = sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].i_coordonata_z_sector_lume_joc * ci_lungime_sector_lume_joc + 2; i_coordonata_z_in_sector < ci_lungime_sector_lume_joc - 2; i_coordonata_z_in_sector++, i_coordonata_z++)
        {
            i_inaltime_coloana_cuburi = i_inaltime_medie_sector_lume_joc + (int)(generare_lume_joc.GetNoise(i_coordonata_x, i_coordonata_z) * i_amplitudine_sector_lume_joc);

            if (i_inaltime_coloana_cuburi > i_inaltime_nivel_apa_lume_joc + 3)
            {
                if (rand() % 1000 % 989 == 0)
                {
                    i_inaltime_copac = i_inaltime_minima_copac + rand() % 4;

                    if (i_inaltime_coloana_cuburi + i_inaltime_copac + 1 <= ci_inaltime_sector_lume_joc)
                    {
                        for (i = 0; i < i_inaltime_copac; i++)
                        {
                            sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector][i_inaltime_coloana_cuburi + i][i_coordonata_z_in_sector] = 5;
                        }

                        ///
                        sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector][i_inaltime_coloana_cuburi + i_inaltime_copac][i_coordonata_z_in_sector] = 6;

                        if (sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector - 1][i_inaltime_coloana_cuburi + i_inaltime_copac][i_coordonata_z_in_sector] == 0)
                        {
                            sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector - 1][i_inaltime_coloana_cuburi + i_inaltime_copac][i_coordonata_z_in_sector] = 6;
                        }
                        if (sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector + 1][i_inaltime_coloana_cuburi + i_inaltime_copac][i_coordonata_z_in_sector] == 0)
                        {
                            sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector + 1][i_inaltime_coloana_cuburi + i_inaltime_copac][i_coordonata_z_in_sector] = 6;
                        }
                        if (sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector][i_inaltime_coloana_cuburi + i_inaltime_copac][i_coordonata_z_in_sector - 1] == 0)
                        {
                            sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector][i_inaltime_coloana_cuburi + i_inaltime_copac][i_coordonata_z_in_sector - 1] = 6;
                        }
                        if (sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector][i_inaltime_coloana_cuburi + i_inaltime_copac][i_coordonata_z_in_sector + 1] == 0)
                        {
                            sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector][i_inaltime_coloana_cuburi + i_inaltime_copac][i_coordonata_z_in_sector + 1] = 6;
                        }

                        ///
                        if (sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector - 1][i_inaltime_coloana_cuburi + i_inaltime_copac - 1][i_coordonata_z_in_sector] == 0)
                        {
                            sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector - 1][i_inaltime_coloana_cuburi + i_inaltime_copac - 1][i_coordonata_z_in_sector] = 6;
                        }
                        if (sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector + 1][i_inaltime_coloana_cuburi + i_inaltime_copac - 1][i_coordonata_z_in_sector] == 0)
                        {
                            sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector + 1][i_inaltime_coloana_cuburi + i_inaltime_copac - 1][i_coordonata_z_in_sector] = 6;
                        }
                        if (sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector][i_inaltime_coloana_cuburi + i_inaltime_copac - 1][i_coordonata_z_in_sector - 1] == 0)
                        {
                            sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector][i_inaltime_coloana_cuburi + i_inaltime_copac - 1][i_coordonata_z_in_sector - 1] = 6;
                        }
                        if (sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector][i_inaltime_coloana_cuburi + i_inaltime_copac - 1][i_coordonata_z_in_sector + 1] == 0)
                        {
                            sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector][i_inaltime_coloana_cuburi + i_inaltime_copac - 1][i_coordonata_z_in_sector + 1] = 6;
                        }
                        if (sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector - 1][i_inaltime_coloana_cuburi + i_inaltime_copac - 1][i_coordonata_z_in_sector - 1] == 0)
                        {
                            sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector - 1][i_inaltime_coloana_cuburi + i_inaltime_copac - 1][i_coordonata_z_in_sector - 1] = 6;
                        }
                        if (sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector + 1][i_inaltime_coloana_cuburi + i_inaltime_copac - 1][i_coordonata_z_in_sector + 1] == 0)
                        {
                            sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector + 1][i_inaltime_coloana_cuburi + i_inaltime_copac - 1][i_coordonata_z_in_sector + 1] = 6;
                        }
                        if (sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector + 1][i_inaltime_coloana_cuburi + i_inaltime_copac - 1][i_coordonata_z_in_sector - 1] == 0)
                        {
                            sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector + 1][i_inaltime_coloana_cuburi + i_inaltime_copac - 1][i_coordonata_z_in_sector - 1] = 6;
                        }
                        if (sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector - 1][i_inaltime_coloana_cuburi + i_inaltime_copac - 1][i_coordonata_z_in_sector + 1] == 0)
                        {
                            sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector - 1][i_inaltime_coloana_cuburi + i_inaltime_copac - 1][i_coordonata_z_in_sector + 1] = 6;
                        }

                        ///
                        if (sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector - 1][i_inaltime_coloana_cuburi + i_inaltime_copac - 2][i_coordonata_z_in_sector] == 0)
                        {
                            sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector - 1][i_inaltime_coloana_cuburi + i_inaltime_copac - 2][i_coordonata_z_in_sector] = 6;
                        }
                        if (sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector + 1][i_inaltime_coloana_cuburi + i_inaltime_copac - 2][i_coordonata_z_in_sector] == 0)
                        {
                            sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector + 1][i_inaltime_coloana_cuburi + i_inaltime_copac - 2][i_coordonata_z_in_sector] = 6;
                        }
                        if (sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector][i_inaltime_coloana_cuburi + i_inaltime_copac - 2][i_coordonata_z_in_sector - 1] == 0)
                        {
                            sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector][i_inaltime_coloana_cuburi + i_inaltime_copac - 2][i_coordonata_z_in_sector - 1] = 6;
                        }
                        if (sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector][i_inaltime_coloana_cuburi + i_inaltime_copac - 2][i_coordonata_z_in_sector + 1] == 0)
                        {
                            sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector][i_inaltime_coloana_cuburi + i_inaltime_copac - 2][i_coordonata_z_in_sector + 1] = 6;
                        }
                        if (sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector - 1][i_inaltime_coloana_cuburi + i_inaltime_copac - 2][i_coordonata_z_in_sector - 1] == 0)
                        {
                            sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector - 1][i_inaltime_coloana_cuburi + i_inaltime_copac - 2][i_coordonata_z_in_sector - 1] = 6;
                        }
                        if (sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector + 1][i_inaltime_coloana_cuburi + i_inaltime_copac - 2][i_coordonata_z_in_sector + 1] == 0)
                        {
                            sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector + 1][i_inaltime_coloana_cuburi + i_inaltime_copac - 2][i_coordonata_z_in_sector + 1] = 6;
                        }
                        if (sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector + 1][i_inaltime_coloana_cuburi + i_inaltime_copac - 2][i_coordonata_z_in_sector - 1] == 0)
                        {
                            sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector + 1][i_inaltime_coloana_cuburi + i_inaltime_copac - 2][i_coordonata_z_in_sector - 1] = 6;
                        }
                        if (sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector - 1][i_inaltime_coloana_cuburi + i_inaltime_copac - 2][i_coordonata_z_in_sector + 1] == 0)
                        {
                            sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector - 1][i_inaltime_coloana_cuburi + i_inaltime_copac - 2][i_coordonata_z_in_sector + 1] = 6;
                        }
                        //
                        if (sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector - 2][i_inaltime_coloana_cuburi + i_inaltime_copac - 2][i_coordonata_z_in_sector] == 0)
                        {
                            sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector - 2][i_inaltime_coloana_cuburi + i_inaltime_copac - 2][i_coordonata_z_in_sector] = 6;
                        }
                        if (sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector - 2][i_inaltime_coloana_cuburi + i_inaltime_copac - 2][i_coordonata_z_in_sector - 1] == 0)
                        {
                            sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector - 2][i_inaltime_coloana_cuburi + i_inaltime_copac - 2][i_coordonata_z_in_sector - 1] = 6;
                        }
                        if (sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector - 2][i_inaltime_coloana_cuburi + i_inaltime_copac - 2][i_coordonata_z_in_sector - 2] == 0)
                        {
                            sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector - 2][i_inaltime_coloana_cuburi + i_inaltime_copac - 2][i_coordonata_z_in_sector - 2] = 6;
                        }
                        if (sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector - 1][i_inaltime_coloana_cuburi + i_inaltime_copac - 2][i_coordonata_z_in_sector - 2] == 0)
                        {
                            sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector - 1][i_inaltime_coloana_cuburi + i_inaltime_copac - 2][i_coordonata_z_in_sector - 2] = 6;
                        }
                        if (sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector][i_inaltime_coloana_cuburi + i_inaltime_copac - 2][i_coordonata_z_in_sector - 2] == 0)
                        {
                            sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector][i_inaltime_coloana_cuburi + i_inaltime_copac - 2][i_coordonata_z_in_sector - 2] = 6;
                        }
                        if (sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector + 1][i_inaltime_coloana_cuburi + i_inaltime_copac - 2][i_coordonata_z_in_sector - 2] == 0)
                        {
                            sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector + 1][i_inaltime_coloana_cuburi + i_inaltime_copac - 2][i_coordonata_z_in_sector - 2] = 6;
                        }
                        if (sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector + 2][i_inaltime_coloana_cuburi + i_inaltime_copac - 2][i_coordonata_z_in_sector - 2] == 0)
                        {
                            sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector + 2][i_inaltime_coloana_cuburi + i_inaltime_copac - 2][i_coordonata_z_in_sector - 2] = 6;
                        }
                        if (sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector + 2][i_inaltime_coloana_cuburi + i_inaltime_copac - 2][i_coordonata_z_in_sector - 1] == 0)
                        {
                            sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector + 2][i_inaltime_coloana_cuburi + i_inaltime_copac - 2][i_coordonata_z_in_sector - 1] = 6;
                        }
                        if (sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector + 2][i_inaltime_coloana_cuburi + i_inaltime_copac - 2][i_coordonata_z_in_sector] == 0)
                        {
                            sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector + 2][i_inaltime_coloana_cuburi + i_inaltime_copac - 2][i_coordonata_z_in_sector] = 6;
                        }
                        if (sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector + 2][i_inaltime_coloana_cuburi + i_inaltime_copac - 2][i_coordonata_z_in_sector + 1] == 0)
                        {
                            sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector + 2][i_inaltime_coloana_cuburi + i_inaltime_copac - 2][i_coordonata_z_in_sector + 1] = 6;
                        }
                        if (sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector + 2][i_inaltime_coloana_cuburi + i_inaltime_copac - 2][i_coordonata_z_in_sector + 2] == 0)
                        {
                            sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector + 2][i_inaltime_coloana_cuburi + i_inaltime_copac - 2][i_coordonata_z_in_sector + 2] = 6;
                        }
                        if (sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector + 1][i_inaltime_coloana_cuburi + i_inaltime_copac - 2][i_coordonata_z_in_sector + 2] == 0)
                        {
                            sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector + 1][i_inaltime_coloana_cuburi + i_inaltime_copac - 2][i_coordonata_z_in_sector + 2] = 6;
                        }
                        if (sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector][i_inaltime_coloana_cuburi + i_inaltime_copac - 2][i_coordonata_z_in_sector + 2] == 0)
                        {
                            sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector][i_inaltime_coloana_cuburi + i_inaltime_copac - 2][i_coordonata_z_in_sector + 2] = 6;
                        }
                        if (sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector - 1][i_inaltime_coloana_cuburi + i_inaltime_copac - 2][i_coordonata_z_in_sector + 2] == 0)
                        {
                            sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector - 1][i_inaltime_coloana_cuburi + i_inaltime_copac - 2][i_coordonata_z_in_sector + 2] = 6;
                        }
                        if (sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector - 2][i_inaltime_coloana_cuburi + i_inaltime_copac - 2][i_coordonata_z_in_sector + 2] == 0)
                        {
                            sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector - 2][i_inaltime_coloana_cuburi + i_inaltime_copac - 2][i_coordonata_z_in_sector + 2] = 6;
                        }
                        if (sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector - 2][i_inaltime_coloana_cuburi + i_inaltime_copac - 2][i_coordonata_z_in_sector + 1] == 0)
                        {
                            sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector - 2][i_inaltime_coloana_cuburi + i_inaltime_copac - 2][i_coordonata_z_in_sector + 1] = 6;
                        }


                        ///
                        if (sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector - 1][i_inaltime_coloana_cuburi + i_inaltime_copac - 3][i_coordonata_z_in_sector] == 0)
                        {
                            sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector - 1][i_inaltime_coloana_cuburi + i_inaltime_copac - 3][i_coordonata_z_in_sector] = 6;
                        }
                        if (sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector + 1][i_inaltime_coloana_cuburi + i_inaltime_copac - 3][i_coordonata_z_in_sector] == 0)
                        {
                            sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector + 1][i_inaltime_coloana_cuburi + i_inaltime_copac - 3][i_coordonata_z_in_sector] = 6;
                        }
                        if (sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector][i_inaltime_coloana_cuburi + i_inaltime_copac - 3][i_coordonata_z_in_sector - 1] == 0)
                        {
                            sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector][i_inaltime_coloana_cuburi + i_inaltime_copac - 3][i_coordonata_z_in_sector - 1] = 6;
                        }
                        if (sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector][i_inaltime_coloana_cuburi + i_inaltime_copac - 3][i_coordonata_z_in_sector + 1] == 0)
                        {
                            sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector][i_inaltime_coloana_cuburi + i_inaltime_copac - 3][i_coordonata_z_in_sector + 1] = 6;
                        }
                        if (sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector - 1][i_inaltime_coloana_cuburi + i_inaltime_copac - 3][i_coordonata_z_in_sector - 1] == 0)
                        {
                            sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector - 1][i_inaltime_coloana_cuburi + i_inaltime_copac - 3][i_coordonata_z_in_sector - 1] = 6;
                        }
                        if (sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector + 1][i_inaltime_coloana_cuburi + i_inaltime_copac - 3][i_coordonata_z_in_sector + 1] == 0)
                        {
                            sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector + 1][i_inaltime_coloana_cuburi + i_inaltime_copac - 3][i_coordonata_z_in_sector + 1] = 6;
                        }
                        if (sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector + 1][i_inaltime_coloana_cuburi + i_inaltime_copac - 3][i_coordonata_z_in_sector - 1] == 0)
                        {
                            sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector + 1][i_inaltime_coloana_cuburi + i_inaltime_copac - 3][i_coordonata_z_in_sector - 1] = 6;
                        }
                        if (sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector - 1][i_inaltime_coloana_cuburi + i_inaltime_copac - 3][i_coordonata_z_in_sector + 1] == 0)
                        {
                            sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector - 1][i_inaltime_coloana_cuburi + i_inaltime_copac - 3][i_coordonata_z_in_sector + 1] = 6;
                        }
                        //
                        if (sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector - 2][i_inaltime_coloana_cuburi + i_inaltime_copac - 3][i_coordonata_z_in_sector] == 0)
                        {
                            sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector - 2][i_inaltime_coloana_cuburi + i_inaltime_copac - 3][i_coordonata_z_in_sector] = 6;
                        }
                        if (sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector - 2][i_inaltime_coloana_cuburi + i_inaltime_copac - 3][i_coordonata_z_in_sector - 1] == 0)
                        {
                            sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector - 2][i_inaltime_coloana_cuburi + i_inaltime_copac - 3][i_coordonata_z_in_sector - 1] = 6;
                        }
                        if (sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector - 2][i_inaltime_coloana_cuburi + i_inaltime_copac - 3][i_coordonata_z_in_sector - 2] == 0)
                        {
                            sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector - 2][i_inaltime_coloana_cuburi + i_inaltime_copac - 3][i_coordonata_z_in_sector - 2] = 6;
                        }
                        if (sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector - 1][i_inaltime_coloana_cuburi + i_inaltime_copac - 3][i_coordonata_z_in_sector - 2] == 0)
                        {
                            sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector - 1][i_inaltime_coloana_cuburi + i_inaltime_copac - 3][i_coordonata_z_in_sector - 2] = 6;
                        }
                        if (sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector][i_inaltime_coloana_cuburi + i_inaltime_copac - 3][i_coordonata_z_in_sector - 2] == 0)
                        {
                            sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector][i_inaltime_coloana_cuburi + i_inaltime_copac - 3][i_coordonata_z_in_sector - 2] = 6;
                        }
                        if (sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector + 1][i_inaltime_coloana_cuburi + i_inaltime_copac - 3][i_coordonata_z_in_sector - 2] == 0)
                        {
                            sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector + 1][i_inaltime_coloana_cuburi + i_inaltime_copac - 3][i_coordonata_z_in_sector - 2] = 6;
                        }
                        if (sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector + 2][i_inaltime_coloana_cuburi + i_inaltime_copac - 3][i_coordonata_z_in_sector - 2] == 0)
                        {
                            sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector + 2][i_inaltime_coloana_cuburi + i_inaltime_copac - 3][i_coordonata_z_in_sector - 2] = 6;
                        }
                        if (sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector + 2][i_inaltime_coloana_cuburi + i_inaltime_copac - 3][i_coordonata_z_in_sector - 1] == 0)
                        {
                            sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector + 2][i_inaltime_coloana_cuburi + i_inaltime_copac - 3][i_coordonata_z_in_sector - 1] = 6;
                        }
                        if (sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector + 2][i_inaltime_coloana_cuburi + i_inaltime_copac - 3][i_coordonata_z_in_sector] == 0)
                        {
                            sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector + 2][i_inaltime_coloana_cuburi + i_inaltime_copac - 3][i_coordonata_z_in_sector] = 6;
                        }
                        if (sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector + 2][i_inaltime_coloana_cuburi + i_inaltime_copac - 3][i_coordonata_z_in_sector + 1] == 0)
                        {
                            sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector + 2][i_inaltime_coloana_cuburi + i_inaltime_copac - 3][i_coordonata_z_in_sector + 1] = 6;
                        }
                        if (sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector + 2][i_inaltime_coloana_cuburi + i_inaltime_copac - 3][i_coordonata_z_in_sector + 2] == 0)
                        {
                            sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector + 2][i_inaltime_coloana_cuburi + i_inaltime_copac - 3][i_coordonata_z_in_sector + 2] = 6;
                        }
                        if (sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector + 1][i_inaltime_coloana_cuburi + i_inaltime_copac - 3][i_coordonata_z_in_sector + 2] == 0)
                        {
                            sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector + 1][i_inaltime_coloana_cuburi + i_inaltime_copac - 3][i_coordonata_z_in_sector + 2] = 6;
                        }
                        if (sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector][i_inaltime_coloana_cuburi + i_inaltime_copac - 3][i_coordonata_z_in_sector + 2] == 0)
                        {
                            sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector][i_inaltime_coloana_cuburi + i_inaltime_copac - 3][i_coordonata_z_in_sector + 2] = 6;
                        }
                        if (sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector - 1][i_inaltime_coloana_cuburi + i_inaltime_copac - 3][i_coordonata_z_in_sector + 2] == 0)
                        {
                            sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector - 1][i_inaltime_coloana_cuburi + i_inaltime_copac - 3][i_coordonata_z_in_sector + 2] = 6;
                        }
                        if (sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector - 2][i_inaltime_coloana_cuburi + i_inaltime_copac - 3][i_coordonata_z_in_sector + 2] == 0)
                        {
                            sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector - 2][i_inaltime_coloana_cuburi + i_inaltime_copac - 3][i_coordonata_z_in_sector + 2] = 6;
                        }
                        if (sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector - 2][i_inaltime_coloana_cuburi + i_inaltime_copac - 3][i_coordonata_z_in_sector + 1] == 0)
                        {
                            sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector - 2][i_inaltime_coloana_cuburi + i_inaltime_copac - 3][i_coordonata_z_in_sector + 1] = 6;
                        }
                    }
                }
            }
            else
            {
                if (i_inaltime_coloana_cuburi > i_inaltime_nivel_apa_lume_joc)
                {
                    if (rand() % 1000 % 989 == 0)
                    {
                        i_inaltime_cactus = i_inaltime_minima_cactus + rand() % 3;

                        if (i_inaltime_coloana_cuburi + i_inaltime_cactus <= ci_inaltime_sector_lume_joc)
                        {
                            for (i = 0; i < i_inaltime_cactus; i++)
                            {
                                sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].c_cod_cub[i_coordonata_x_in_sector][i_inaltime_coloana_cuburi + i][i_coordonata_z_in_sector] = 8;
                            }
                        }
                    }
                }
            }
        }
    }
}

int main()
{
    cout << "Introduceti un numar intreg ce va fi folosit pe post de seed." << '\n' << "Seed:" << '\n'; //Citim aici seed-ul lumii pe care o vom genera.
    cin >> i_samanta_lume_joc;

    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* fereastra_joc = glfwCreateWindow(i_latime_fereastra_joc, i_inaltime_fereastra_joc, "VoxelCraft", 0, 0);//glfwGetPrimaryMonitor()
    glfwMakeContextCurrent(fereastra_joc);
    glfwSetInputMode(fereastra_joc, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetFramebufferSizeCallback(fereastra_joc, v_modificare_dimensiuni_fereastra_joc);
    glfwSetCursorPosCallback(fereastra_joc, v_date_intrare_mouse);

    glewInit();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    shader_vertex_tinta = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(shader_vertex_tinta, 1, &c_sursa_shader_vertex_tinta, 0);
    glCompileShader(shader_vertex_tinta);

    shader_fragment_tinta = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(shader_fragment_tinta, 1, &c_sursa_shader_fragment_tinta, 0);
    glCompileShader(shader_fragment_tinta);

    program_shader_tinta = glCreateProgram();
    glAttachShader(program_shader_tinta, shader_vertex_tinta);
    glAttachShader(program_shader_tinta, shader_fragment_tinta);
    glLinkProgram(program_shader_tinta);
    glDeleteShader(shader_vertex_tinta);
    glDeleteShader(shader_fragment_tinta);
    //glUseProgram(program_shader_tinta); //Aici activam shader-ul.

    glGenVertexArrays(1, &vao_tinta);
    glGenBuffers(1, &vbo_tinta);
    glBindVertexArray(vao_tinta);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_tinta);
    glBufferData(GL_ARRAY_BUFFER, 32, f_date_tinta, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8, (void*)0);
    glEnableVertexAttribArray(0);

    ///

    shader_vertex_sector_lume_joc = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(shader_vertex_sector_lume_joc, 1, &c_sursa_shader_vertex_sector_lume_joc, 0);
    glCompileShader(shader_vertex_sector_lume_joc);

    shader_fragment_sector_lume_joc = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(shader_fragment_sector_lume_joc, 1, &c_sursa_shader_fragment_sector_lume_joc, 0);
    glCompileShader(shader_fragment_sector_lume_joc);

    program_shader_sector_lume_joc = glCreateProgram();
    glAttachShader(program_shader_sector_lume_joc, shader_vertex_sector_lume_joc);
    glAttachShader(program_shader_sector_lume_joc, shader_fragment_sector_lume_joc);
    glLinkProgram(program_shader_sector_lume_joc);
    glDeleteShader(shader_vertex_sector_lume_joc);
    glDeleteShader(shader_fragment_sector_lume_joc);
    glUseProgram(program_shader_sector_lume_joc); //Aici activam shader-ul.

    loc_matrice_vedere_sector_lume_joc = glGetUniformLocation(program_shader_sector_lume_joc, "matrice_vedere");
    loc_matrice_proiectie_sector_lume_joc = glGetUniformLocation(program_shader_sector_lume_joc, "matrice_proiectie");
    loc_coordonate_camera_sector_lume_joc = glGetUniformLocation(program_shader_sector_lume_joc, "coordonate_camera");
    loc_textura_sector_lume_joc = glGetUniformLocation(program_shader_sector_lume_joc, "textura");
    loc_directie_lumina_directionala_sector_lume_joc = glGetUniformLocation(program_shader_sector_lume_joc, "directie_lumina_directionala");

    matrice_proiectie_sector_lume_joc = glm::perspective(glm::radians(f_dimensiune_camp_vizual), (float)i_latime_fereastra_joc / (float)i_inaltime_fereastra_joc, f_distanta_minima_vedere, f_distanta_maxima_vedere);
    glUniformMatrix4fv(loc_matrice_proiectie_sector_lume_joc, 1, GL_FALSE, glm::value_ptr(matrice_proiectie_sector_lume_joc));

    v_importare_textura("resurse/texturi/atlas_textura_cub.png", &atlas_textura_cub); //Aici incarcam atlasul de texturi.

    glActiveTexture(GL_TEXTURE0); //Avem doar o textura momentan si putem de acum sa stabilim ca folosim GL_TEXTURE0, pentru ca nu avem mai mult de o textura in shader-ul de fragmentare.
    glUniform1i(loc_textura_sector_lume_joc, 0);
    glBindTexture(GL_TEXTURE_2D, atlas_textura_cub.ui_cod_textura);

    int i_coordonata_locala_x_sector_lume_joc;
    int i_coordonata_locala_z_sector_lume_joc;

    //Mai intai generam cate 2 buffer-e necesare pentru a desena un chunk/sector din lumea jocului.
    //18874368
    long long int lli_memorie_sector_lume_joc = 1887436;
    for (i_coordonata_locala_x_sector_lume_joc = 0; i_coordonata_locala_x_sector_lume_joc < ci_numar_sectoare_vizibile_latime_lume_joc; i_coordonata_locala_x_sector_lume_joc++)
    {
        for (i_coordonata_locala_z_sector_lume_joc = 0; i_coordonata_locala_z_sector_lume_joc < ci_numar_sectoare_vizibile_lungime_lume_joc; i_coordonata_locala_z_sector_lume_joc++)
        {
            glGenVertexArrays(1, &vao_sector_lume_joc[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc]);
            glGenBuffers(1, &vbo_sector_lume_joc[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc]);
            glBindVertexArray(vao_sector_lume_joc[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc]);
            glBindBuffer(GL_ARRAY_BUFFER, vbo_sector_lume_joc[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc]);
            glBufferData(GL_ARRAY_BUFFER, lli_memorie_sector_lume_joc, 0, GL_DYNAMIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 32, (void*)0);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 32, (void*)12);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 32, (void*)24);
            glEnableVertexAttribArray(2);
        }
    }

    int i_coordonata_x_sector_lume_joc;
    int i_coordonata_z_sector_lume_joc;

    generare_lume_joc.SetSeed(i_samanta_lume_joc); //Setam ce seed vom folosi pentru a genera lumea.
    generare_lume_joc.SetNoiseType(FastNoise::SimplexFractal);

    while (!glfwWindowShouldClose(fereastra_joc))
    {
        float f_timp_curent = (float)glfwGetTime();
        f_durata_timp_cadru = f_timp_curent - f_timp_cadru_anterior;
        f_timp_cadru_anterior = f_timp_curent;

        glClearColor(0.6f, 0.85098f, 0.917647f, 1.0f); //Este un albastru-deschis pentru cer. Putem ulterior sa inlocuim asta cu un texture cube.
        //glClearColor(0.3f, 0.4f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        matrice_vedere_sector_lume_joc = glm::lookAt(coordonate_camera, coordonate_camera + directie_privire_camera, normala_camera);
        glUniformMatrix4fv(loc_matrice_vedere_sector_lume_joc, 1, GL_FALSE, glm::value_ptr(matrice_vedere_sector_lume_joc));

        glUniform3fv(loc_coordonate_camera_sector_lume_joc, 1, glm::value_ptr(coordonate_camera));
        glUniform3fv(loc_directie_lumina_directionala_sector_lume_joc, 1, glm::value_ptr(directie_lumina_directionala));

        //directie_lumina_directionala.x = sin(glfwGetTime()) * 7.5f;
        //directie_lumina_directionala.y = cos(glfwGetTime()) * 7.5f;

        directie_lumina_directionala.x = 1.0f;
        directie_lumina_directionala.y = -1.0f;

        //glUseProgram(program_shader_sector_lume_joc);//Aici activam shader-ul. E bine de stiut.
        //glEnable(GL_DEPTH_TEST);
        //glEnable(GL_CULL_FACE);

        for (i_coordonata_locala_x_sector_lume_joc = 0; i_coordonata_locala_x_sector_lume_joc < ci_numar_sectoare_vizibile_latime_lume_joc; i_coordonata_locala_x_sector_lume_joc++)
        {
            for (i_coordonata_locala_z_sector_lume_joc = 0; i_coordonata_locala_z_sector_lume_joc < ci_numar_sectoare_vizibile_lungime_lume_joc; i_coordonata_locala_z_sector_lume_joc++)
            {
                b_sector_ce_trebuie_generat[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc] = 0;
            }
        }

        for (i_coordonata_locala_x_sector_lume_joc = 0; i_coordonata_locala_x_sector_lume_joc < ci_numar_sectoare_vizibile_latime_lume_joc; i_coordonata_locala_x_sector_lume_joc++)
        {
            i_coordonata_x_sector_lume_joc = i_coordonata_locala_x_sector_lume_joc - i_coordonata_locala_x_sector_lume_joc_camera + i_coordonata_x_sector_lume_joc_camera;
            for (i_coordonata_locala_z_sector_lume_joc = 0; i_coordonata_locala_z_sector_lume_joc < ci_numar_sectoare_vizibile_lungime_lume_joc; i_coordonata_locala_z_sector_lume_joc++)
            {
                i_coordonata_z_sector_lume_joc = i_coordonata_locala_z_sector_lume_joc - i_coordonata_locala_z_sector_lume_joc_camera + i_coordonata_z_sector_lume_joc_camera;

                if (sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].i_coordonata_x_sector_lume_joc != i_coordonata_x_sector_lume_joc || sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].i_coordonata_z_sector_lume_joc != i_coordonata_z_sector_lume_joc)
                {
                    b_sector_ce_trebuie_generat[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc] = 1;
                    v_generare_sector_lume_joc(i_coordonata_locala_x_sector_lume_joc, i_coordonata_locala_z_sector_lume_joc, i_coordonata_x_sector_lume_joc, i_coordonata_z_sector_lume_joc);
                }
            }
        }

        //Desenam chunk-urile.

        for (i_coordonata_locala_x_sector_lume_joc = 0; i_coordonata_locala_x_sector_lume_joc < ci_numar_sectoare_vizibile_latime_lume_joc; i_coordonata_locala_x_sector_lume_joc++)
        {
            for (i_coordonata_locala_z_sector_lume_joc = 0; i_coordonata_locala_z_sector_lume_joc < ci_numar_sectoare_vizibile_lungime_lume_joc; i_coordonata_locala_z_sector_lume_joc++)
            {
                if (b_sector_ce_trebuie_generat[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc] == 1)
                {
                    v_generare_desen_sector_lume_joc(i_coordonata_locala_x_sector_lume_joc, i_coordonata_locala_z_sector_lume_joc);
                }
                else
                {
                    glBindVertexArray(vao_sector_lume_joc[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc]);
                }
                glDrawArrays(GL_TRIANGLES, 0, sector_lume_joc_vizibil[i_coordonata_locala_x_sector_lume_joc][i_coordonata_locala_z_sector_lume_joc].i_numar_varfuri_sector_lume_joc);
            }
        }

        //Desenam crosshair-ul.
        //glUseProgram(program_shader_tinta);
        //glDisable(GL_DEPTH_TEST);
        //glDisable(GL_CULL_FACE);
        //glBindVertexArray(vao_tinta);
        //glDrawArrays(GL_LINES, 0, 4);

        v_date_intrare_tastatura(fereastra_joc);
        glfwSwapBuffers(fereastra_joc);
        glfwPollEvents();

        cout << "FPS :: " << 1.0f / f_durata_timp_cadru << '\n';
    }

    glfwDestroyWindow(fereastra_joc);
    glfwTerminate();

    return 0;
}
