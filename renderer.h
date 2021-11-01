#pragma once

#include <glad/glad.h>
#include <vector>
#include "shaders.h"

class Renderer {

  public:
    Renderer(std::vector<uint8_t> *screen_ptr);
    ~Renderer();

    void Render(Shader *myShader);
    void Init();
    void UpdateTexture();

  private:
    std::vector<uint8_t> *screen;
    uint8_t *screenData, screenWidth, screenHeight;
    uint16_t screenSize;
    GLuint texture, VBO, VAO, EBO;

};
