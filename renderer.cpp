#include "renderer.h"
#include <cstring>
Renderer::~Renderer(){

  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
  glDeleteBuffers(1, &EBO);

}

Renderer::Renderer(uint8_t *screen_ptr): screen(screen_ptr){};


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

 
  memset(screenData, 0, 2048 * sizeof(screenData[0]));
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 64, 32, 0, GL_RED, GL_UNSIGNED_BYTE, &screenData);

  const GLint swizzleMask[] = {GL_RED, GL_RED, GL_RED, GL_ONE};
  glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);

}



void Renderer::UpdateTexture() { 
  
  for(int i = 0; i < 2048; i++){
    if(screen[i] == 0)
      screenData[i] = 0;
    else
      screenData[i] = 255;
 }
 
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 64, 32, GL_RED, GL_UNSIGNED_BYTE, &screenData);
  const GLint swizzleMask[] = {GL_RED, GL_RED, GL_RED, GL_ONE};
  glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
}



void Renderer::Render(Shader *myShader){
 
  UpdateTexture();

  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
}
