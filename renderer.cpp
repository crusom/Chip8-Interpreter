#include "renderer.h"
#include <cstring>


Renderer::~Renderer(){

  glDeleteVertexArrays(1, &VAO);
  glDeleteVertexArrays(1, &VAO_text);
  glDeleteBuffers(1, &VBO);
  glDeleteBuffers(1, &VBO_text);
  glDeleteBuffers(1, &EBO);
  //delete[] screenData;
}

Renderer::Renderer(std::vector<uint8_t> *screen_ptr): screen(screen_ptr){};


void Renderer::Init(){
  //For 2D image textures, 0,0 in texture coordinates corresponds to the bottom left corner of the image
  //pos x, y and z coordinate, tex x, y
  float vertices[] = {
    //"OpenGL texture are loaded left to right, bottom to top.
    //Many image loaders however will store the image in memory left to right, top to bottom."

    -1.0f, -1.0f, 0.0f, 0.0f, 1.0f,  // Bottom left
    -1.0f,  1.0f, 0.0f, 0.0f, 0.0f,  // Top left
     1.0f,  1.0f, 0.0f, 1.0f, 0.0f,  // Top right
     1.0f, -1.0f, 0.0f, 1.0f, 1.0f   // Bottom right
     };

  unsigned int indices[] = {
   0, 1, 3,  // First triangle
   1, 2, 3   // Second triangle
  };
  
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);

  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
 
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);


  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(0));
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);


  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(VAO);

  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
 
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  screenSize = screen->size();
  screenData.resize(screenSize);
  //screenData = new uint8_t[screenSize];
  //memset(screenData, 0, screenSize * sizeof(screenData[0]));

  //printf("screenSize: %d", screenSize);
  
  if(screenSize == 2048){
   screenWidth = 64;
   screenHeight = 32;
  } else {
    screenWidth = 128;
    screenHeight = 64;
 }

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, screenWidth, screenHeight, 0, GL_RED, GL_UNSIGNED_BYTE, &screenData[0]);

  const GLint swizzleMask[] = {GL_RED, GL_RED, GL_RED, GL_ONE};
  glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);

}

void Renderer::ExtendedModeChange(int mode){
  glBindTexture(GL_TEXTURE_2D, texture);
  // change to normal mode
  if(mode == 0){
    screenData.resize(64 * 32);
    screenSize = 64 * 32;
    screenWidth = 64;
    screenHeight = 32;
  }
  else {
    screenData.resize(128 * 64);
    screenSize = 128 * 64;
    screenWidth = 128;
    screenHeight = 64;
  }
  // override TexImage  
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, screenWidth, screenHeight, 0, GL_RED, GL_UNSIGNED_BYTE, &screenData[0]);
  const GLint swizzleMask[] = {GL_RED, GL_RED, GL_RED, GL_ONE};
  glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
}

void Renderer::UpdateTexture() { 
  
  for(unsigned int i = 0; i < screenSize; i++){
    if(screen->at(i) == 0)
      screenData.at(i) = 0;
    else
      screenData.at(i) = 255;
 }
 
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, screenWidth, screenHeight, GL_RED, GL_UNSIGNED_BYTE, &screenData[0]);
  const GLint swizzleMask[] = {GL_RED, GL_RED, GL_RED, GL_ONE};
  glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);

}



void Renderer::Render(Shader &shader, bool update_texture){
  
  glBindTexture(GL_TEXTURE_2D, texture);
  glBindVertexArray(VAO);
 
  shader.use();
  shader.setInt("myTex", 0);
  if(update_texture) UpdateTexture();
  
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
  
  glBindVertexArray(0);
  glBindTexture(GL_TEXTURE_2D, 0);

}


//https://learnopengl.com/In-Practice/Text-Rendering
int Renderer::FontInit(Shader &text_shader, int fb_width, int fb_height) {
  
  projection = glm::ortho(0.0f, static_cast<float>(fb_width), 0.0f, static_cast<float>(fb_height));
  text_shader.use();
  glUniformMatrix4fv(glGetUniformLocation(text_shader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));


  FT_Library ft;
  // All functions return a value different than 0 whenever an error occurred
  if (FT_Init_FreeType(&ft))
  {
      std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
      return -1;
  }

  // find path to font
  std::string font_name = std::filesystem::path("fonts/arial.ttf");
  if (font_name.empty())
  {
      std::cout << "ERROR::FREETYPE: Failed to load font_name" << std::endl;
      return -1;
  }

  // load font as face
  FT_Face face;
  if (FT_New_Face(ft, font_name.c_str(), 0, &face)) {
      std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
      return -1;
  }
  else {
      // set size to load glyphs as
      // higher size == higher quality so its better to just scale down 
      
      font_size = 32;
      FT_Set_Pixel_Sizes(face, 0, font_size);

      // disable byte-alignment restriction
      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
     
      // load only chars that we'll use to save memory
      std::string chars_to_init = "ABCDEFabcdefxPCIV0123456789 ";

      for (unsigned char c = 0; c < chars_to_init.length(); c++)
      {
          // Load character glyph 
          if (FT_Load_Char(face, chars_to_init[c], FT_LOAD_RENDER))
          {
              std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
              continue;
          }
          // generate texture
          unsigned int glyph_texture;
          glGenTextures(1, &glyph_texture);
          glBindTexture(GL_TEXTURE_2D, glyph_texture);
          glTexImage2D(
              GL_TEXTURE_2D,
              0,
              GL_RED,
              face->glyph->bitmap.width,
              face->glyph->bitmap.rows,
              0,
              GL_RED,
              GL_UNSIGNED_BYTE,
              face->glyph->bitmap.buffer
          );
          // set texture options
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
          // now store character for later use
          // TODO maybe make one character per pixel
          Character character = {
              glyph_texture,
              glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
              glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
              static_cast<unsigned int>(face->glyph->advance.x)
          };
          Characters.insert(std::pair<char, Character>(chars_to_init[c], character));
      }
      glBindTexture(GL_TEXTURE_2D, 0);
  }
  // destroy FreeType once we're finished
  FT_Done_Face(face);
  FT_Done_FreeType(ft);
  
  // configure VAO/VBO for texture quads
  glGenVertexArrays(1, &VAO_text);
  glGenBuffers(1, &VBO_text);
  
  glBindVertexArray(VAO_text);
  glBindBuffer(GL_ARRAY_BUFFER, VBO_text);
  
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
  glEnableVertexAttribArray(0);
  
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  return 0;
}


void Renderer::RenderText(Shader &text_shader, std::string text, float x, float y, float scale, glm::vec3 color){
  
  glEnable(GL_CULL_FACE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  
  text_shader.use();
  glUniform3f(glGetUniformLocation(text_shader.ID, "textColor"), color.x, color.y, color.z);
  
  glActiveTexture(GL_TEXTURE0);
  glBindVertexArray(VAO_text);

  std::string::const_iterator c;
  for(c = text.begin(); c != text.end(); c++){
    
    Character ch = Characters[*c];

    float xpos = x + ch.Bearing.x * scale;
    float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

    float w = ch.Size.x * scale;
    float h = ch.Size.y * scale;
  
    float font_vertices[6][4] = {
      { xpos,     ypos + h,   0.0f, 0.0f },            
      { xpos,     ypos,       0.0f, 1.0f },
      { xpos + w, ypos,       1.0f, 1.0f },

      { xpos,     ypos + h,   0.0f, 0.0f },
      { xpos + w, ypos,       1.0f, 1.0f },
      { xpos + w, ypos + h,   1.0f, 0.0f }           
    };
  
    glBindTexture(GL_TEXTURE_2D, ch.TextureID);

    glBindBuffer(GL_ARRAY_BUFFER, VBO_text);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(font_vertices), font_vertices);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glDrawArrays(GL_TRIANGLES, 0, 6);
    x += (ch.Advance >> 6) * scale;
  }
  glBindVertexArray(0);
  glBindTexture(GL_TEXTURE_2D, 0);

  glDisable(GL_CULL_FACE);
  glDisable(GL_BLEND);
}
