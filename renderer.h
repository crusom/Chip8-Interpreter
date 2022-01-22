#pragma once

#include <glad/glad.h>
#include <vector>
#include <map>
#include <string>
#include <filesystem>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "shaders.h"
#include <ft2build.h>
#include FT_FREETYPE_H

class Renderer {

  public:
    Renderer(std::vector<uint8_t> *screen_ptr);
    ~Renderer();
    void Render(Shader &shader, bool update_texture);
    void RenderText(Shader &text_shader, std::string text, float x, float y, float scale, glm::vec3 color);
    void Init();
    int FontInit(Shader &text_shader, int fb_width, int fb_height);
    void UpdateTexture();
    void ExtendedModeChange(int mode);
    int font_size;

  private:
    std::vector<uint8_t> *screen;
    std::vector<uint8_t> screenData;
    //uint8_t *screenData;
    unsigned int screenSize, screenWidth, screenHeight;
    GLuint texture, VBO, VBO_text, VAO, VAO_text, EBO;
    glm::mat4 projection;

    struct Character {
      unsigned int TextureID;  // ID handle of the glyph texture
      glm::ivec2   Size;       // Size of glyph
      glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
      unsigned int Advance;    // Offset to advance to next glyph
    };

    std::map<char, Character> Characters;
};
