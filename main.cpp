//standard headers
#include <cstdio>
#include <stdint.h>
#include <thread>
//opengl headers
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "shaders.h"
//my headers
#include "chip8.cpp"
#include "renderer.h"

#define TICKS 700

void process_input(GLFWwindow *window, Chip8 *chip8);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void argc_check(int8_t argc, int8_t num, std::string arg_name);

int PIXEL_SIZE = 16;
int SCR_WIDTH = 64 * PIXEL_SIZE;
int SCR_HEIGHT = 32 * PIXEL_SIZE;

bool *key_pressed;
int *last_key_pressed;


int main(int argc, char* argv[]){
  
  Chip8 chip8;
  key_pressed = chip8.key_pressed;
  last_key_pressed = &chip8.last_key_pressed;

  if(argc < 2) {
    throw std::invalid_argument("Usage: chip8 <ROM file> (optional args)");
  }

  //TODO implement arguments (ticks, memory etc.)
  for(int i = 1; i < argc; ++i) {
    
    std::string arg = argv[i];
    
    if((arg == "-h") || (arg == "--help")) {
      //show_usage(argv[0]);
      return 0;
    } 
    else if ((arg == "-t") || (arg == "--ticks")) {
       argc_check(argc, i, "ticks");
       //ticks = argv[i + 1];
       //i++;
    }
    else {
    
    }
  }

  if(!chip8.LoadRom(argv[1])){ 
    puts("Invalid ROM");
    return -1;
  }

  Renderer renderer(chip8.screen);

  //https://www.glfw.org/docs/3.3/intro_guide.html
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "CHIP8", NULL, NULL);


  if(!window){
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(window);
  // we don't want v-sync since we're waiting in our loop
  glfwSwapInterval(0);

  glfwSetKeyCallback(window, key_callback);

  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetErrorCallback([](int, const char* description) { printf("Error: %s \n", description);});

  if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
    std::cout << "Failed to initialize GLAD" << std::endl;
    return -1;
  }

  Shader myShader("vshader.vs", "fshader.fs");
  
  renderer.Init();
  
  myShader.use();
  myShader.setInt("myTex", 0);
  
  uint8_t interval = 1000 / TICKS;

  while(!glfwWindowShouldClose(window)){
    
    auto start = std::chrono::steady_clock::now();

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    chip8.MainLoop();
     
    if(chip8.redraw_screen){
      renderer.Render(&myShader);

      glfwSwapBuffers(window);
      chip8.redraw_screen = 0;
     }
    
    glfwPollEvents();

    auto end = std::chrono::steady_clock::now();
    auto delta_time = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();
    //printf("delta_time: %d\n", delta_time);
    
    if(delta_time < interval){
      std::this_thread::sleep_for(std::chrono::milliseconds(interval - delta_time));
    }
  }

  glfwTerminate();
  return 0;
}


int keyboard[16] = {
  GLFW_KEY_X,
  GLFW_KEY_1, GLFW_KEY_2, GLFW_KEY_3, 
  GLFW_KEY_Q, GLFW_KEY_W, GLFW_KEY_E,
  GLFW_KEY_A, GLFW_KEY_S, GLFW_KEY_D,

  GLFW_KEY_Z, GLFW_KEY_C, GLFW_KEY_4,
  GLFW_KEY_R, GLFW_KEY_F, GLFW_KEY_V
};

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods){

 if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);

  *last_key_pressed = -1;
  
  for(int i = 0; i < 16; i++){
    
    if(key == keyboard[i] && action == GLFW_PRESS) {
      key_pressed[i] = true;
      *last_key_pressed = i;
    }
    else if(key == keyboard[i] && action == GLFW_RELEASE) {
      key_pressed[i] = false;
    }
  }
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height){
  glViewport(0, 0, width, height);
}


void argc_check(int8_t argc, int8_t num, std::string arg_name){
 
  if(argc <= num + 1){
    std::cerr << arg_name << " option requires an argument\n";
  }
}
