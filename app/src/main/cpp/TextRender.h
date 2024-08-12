#pragma once
#define STB_TRUETYPE_IMPLEMENTATION
#include <stb/stb_truetype.h>

// Font atlas struct
typedef struct
{
    GLuint texture;
    stbtt_bakedchar cdata[96]; // ASCII 32..126 is 95 glyphs
    int w, h;
} FontAtlas;

FontAtlas  font;
GLuint     TextVAO, TextVBO;
GLint      ortho_projection_loc;
GLint      textUniformLocation;
GLint      text_shader_id;
Shader     *text_shader;
glm::mat4  ortho_projection;


#include "stdlib.h"
#include "stdio.h"

void init_text_render_data(float width, float height)
{
    text_shader = new Shader("shaders/text.vs", "shaders/text.fs");
    text_shader_id = text_shader->program_;
    text_shader->use();

    glGenVertexArrays(1, &TextVAO);
    glGenBuffers(1, &TextVBO);
    glBindVertexArray(TextVAO);
    glBindBuffer(GL_ARRAY_BUFFER, TextVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    ortho_projection = glm::ortho(0.0f, width, height, 0.0f, -1.0f, 1.0f);
    ortho_projection_loc = glGetUniformLocation(text_shader_id, "projection");
    textUniformLocation = glGetUniformLocation(text_shader_id, "text");
    
    aout << "Text Shader program created with ID: " << text_shader_id << std::endl;
}


void load_font(std::string filename, float font_height)
{
    aout << "Loading font: " << filename << std::endl;
    unsigned char* ttf_buffer =(unsigned char *) malloc(1 << 20);
    unsigned char* temp_bitmap =(unsigned char *) malloc(512 * 512);

    fread(ttf_buffer, 1, 1 << 20, fopen(filename.c_str(), "rb"));
    font.w = 512;
    font.h = 512;
    stbtt_BakeFontBitmap(ttf_buffer, 0, font_height, temp_bitmap, font.w, font.h, 32, 96, font.cdata);

    glGenTextures(1, &font.texture);
    glBindTexture(GL_TEXTURE_2D, font.texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, font.w, font.h, 0, GL_RED, GL_UNSIGNED_BYTE, temp_bitmap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    aout << "Font texture created with ID: " << font.texture << std::endl;

    free(ttf_buffer);
    free(temp_bitmap);    
}


void render_text(std::string textStr, float x, float y, float scale, float r, float g, float b)
{
    glUseProgram(text_shader_id);
	glUniformMatrix4fv(ortho_projection_loc, 1, GL_FALSE, &ortho_projection[0][0]);
    glUniform3f(glGetUniformLocation(text_shader_id, "textColor"), r, g, b);
    glUniform1i(textUniformLocation, 0);

    glBindVertexArray(TextVAO);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindTexture(GL_TEXTURE_2D, font.texture);

    GLuint indices[] = {0, 1, 2, 0, 2, 3};

    const char *text = textStr.c_str();

    while (*text) {
        if (*text >= 32 && *text < 128)
        {
            stbtt_aligned_quad q;
            stbtt_GetBakedQuad(font.cdata, font.w, font.h, *text - 32, &x, &y, &q, 1);

            float vertices[4][4] = {
                { q.x0 * scale, q.y0 * scale, q.s0, q.t0 },
                { q.x0 * scale, q.y1 * scale, q.s0, q.t1 },
                { q.x1 * scale, q.y1 * scale, q.s1, q.t1 },
                { q.x1 * scale, q.y0 * scale, q.s1, q.t0 }
            };

            glBindBuffer(GL_ARRAY_BUFFER, TextVBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, indices);

            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
        ++text;
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}