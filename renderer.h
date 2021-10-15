#pragma once

#include <glad/glad.h>
#include <vector>
#include "shaders.h"

class Renderer {

  public:
    Renderer(uint8_t *screen_ptr);
    ~Renderer();

    void Render(Shader *myShader);
    void Init();
    void UpdateTexture();

  private:
    uint8_t *screen;
    uint8_t screenData[64 * 32];

    GLuint texture, VBO, VAO, EBO;

};
