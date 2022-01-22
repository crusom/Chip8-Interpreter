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

void process_input(GLFWwindow *window, Chip8 *chip8);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

const int PIXEL_SIZE = 16;
const int SCR_WIDTH = 64 * PIXEL_SIZE;
const int SCR_HEIGHT = 32 * PIXEL_SIZE;

bool hires = false;

bool *key_pressed;
int *last_key_pressed;

struct Settings { 
  unsigned int ticks_in_sec = 60 * 7;
  bool redraw_every_opcode = false;
  bool debugging_mode = false;
  bool logs = false;
};

struct Settings settings;

int main(int argc, char* argv[]){
  
  Chip8 chip8;
  key_pressed = chip8.key_pressed;
  last_key_pressed = &chip8.last_key_pressed;

  if(argc < 2) {
    throw std::invalid_argument("Usage: chip8 (optional args) <ROM file>");
  } 

  std::vector<std::string> args(argv, argv+argc);

  //TODO implement arguments (ticks, memory etc.)

  // An separate check for help and file dissassembly
  if((args[1] == "-h") || (args[1] == "--help")){
      puts("Usage: ./chip8 [options...] <file>\n\
              -t,  --ticks <num>             Ticks per second, must be 1-10000\n\
              -e,  --extend <version>        Extend chip8. Available: schip\n\
              -d,  --disassembly             Print executed instructions to stderr\n\
              -df, --disassembly-file <file> Dissasembly file and print\n\
              -r,  --refresh                 Set glfwSwapInterval(0) (Increases CPU usage)\n\
              -h,  --help                    Print this\n\
         ");
    return 0;
  }
    
  //TODO
  if((args[1] == "-df") || (args[1] == "--disassembly-file")) {}

  for(unsigned int i = 1; i < args.size() - 1; i++) {
    
    std::string arg = args[i];
    std::cout << "arg:" << arg << "\n";

    if ((arg == "-t") || (arg == "--ticks")) {
      std::string ticks = args.at(i + 1);
      bool is_num = true;
      
      for(unsigned int i = 0; i < ticks.length(); i++) {
        if(isspace(ticks[i])) continue;
        else if(ticks[i] < '0' || ticks[i] > '9') is_num = false;
      }

      if(is_num) {
        int num_ticks = (uint32_t)stol(ticks);
        
        if(num_ticks <= 0 || num_ticks >= 10000) {
          //std::cerr << "Invalid ticks number, must be between 1 and 10000: " << ticks << '\n';
          throw std::invalid_argument("Invalid ticks number, must be between 1 and 10000");
        } 
        
        settings.ticks_in_sec = (uint32_t)stol(args.at(i + 1));
      }
      else {
          //std::cerr << "invalid ticks number format, must be between 1 and 10000: " << ticks << '\n';  
          throw std::invalid_argument("Invalid ticks number format, must be between 1 and 10000");
      }
      i++;
    }

    //TODO

    else if((arg == "-b") || (arg == "--breakpoint")) {
      std::string bp = args.at(i + 1); 
      unsigned int bp_num = (unsigned int)stol(bp);

      std::stringstream ss;
      ss << std::hex << bp;
      ss >> bp_num;
      chip8.breakpoints.push_back(bp_num);
      i++;
    }
    
    // TODO explicit extension
    else if((arg == "-e") || (arg == "--extend")) {}
    // TODO
    else if((arg == "-q") || (arg == "--quirks")) {}
   


    //TODO logging to a file
    else if((arg == "-l") || (arg == "--log")) settings.logs = true;

    else if((arg == "-d") || (arg == "--disassembly")) {
      chip8.disas = true;
    }

    else if((arg == "-r") || (arg == "--refresh")) {
       settings.redraw_every_opcode = true;
    }

    else {
        std::cerr << "Invalid argument: " << arg << '\n';
        return -1;
    }

  }

  if(!chip8.LoadRom(argv[argc-1])){ 
    throw std::invalid_argument("Invalid ROM");
    return -1;
  }
  
  Renderer renderer(&chip8.screen);
  //https://www.glfw.org/docs/3.3/intro_guide.html
  glfwInit();

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "CHIP8", NULL, NULL);

  if(!window){
    std::cerr << "Failed to create GLFW window" << '\n';
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(window);

  glfwSetKeyCallback(window, key_callback);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetErrorCallback([](int, const char* description) { printf("Error: %s \n", description);});

  if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
    std::cerr << "Failed to initialize GLAD" << '\n';
    return -1;
  }


  Shader my_shader("vshader.vs", "fshader.fs");
  Shader text_shader("vtextshader.vs", "ftextshader.fs");
  
  renderer.Init();
  
  if(renderer.FontInit(text_shader, SCR_WIDTH, SCR_HEIGHT) != 0){
    std::cerr << "Failed to initialize font" << '\n';
    return -1;
  }
   
  if(settings.redraw_every_opcode){
    glfwSwapInterval(0);
  }
  else {
    glfwSwapInterval(1); // v-sync for stable fps by default
  }


  uint32_t interval = 1000000000 / settings.ticks_in_sec;   
  float scale = (float)PIXEL_SIZE / (float)renderer.font_size;
  
  //chip8.hires = true;
  bool extended_mode = 0;
  while(!glfwWindowShouldClose(window)){
          
    auto start = std::chrono::steady_clock::now();

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    chip8.MainLoop();
 
    if(chip8.redraw_screen) {
      if(extended_mode != chip8.hires){
        extended_mode = chip8.hires;
        renderer.ExtendedModeChange(extended_mode);
      }
      renderer.Render(my_shader, true); 
      
      if(settings.debugging_mode){
        renderer.Render(my_shader, false);

        std::string pc = "PC 0x";
        std::string i = "I 0x";

        std::stringstream sstream_pc, sstream_i;
        sstream_pc << std::hex << chip8.pc;
        sstream_i << std::hex << chip8.I;
        
        pc += sstream_pc.str();
        i += sstream_i.str();

        renderer.RenderText(text_shader, pc, 0.0f, 1 + SCR_HEIGHT - PIXEL_SIZE * (1), scale, glm::vec3(1.0, 0.0f, 0.0f));
        renderer.RenderText(text_shader, i, 0.0f, 1 + SCR_HEIGHT - PIXEL_SIZE * (2), scale, glm::vec3(1.0, 0.0f, 0.0f));
        

        for(int i = 0; i < 16; i++){
          std::string v =  "V" + std::to_string(i) + " 0x";
          std::stringstream v_hex;
          v_hex << std::hex << (int) chip8.V[i];
          v += v_hex.str();

          renderer.RenderText(text_shader, v, 0.0f, 1 + SCR_HEIGHT - PIXEL_SIZE * (i + 3), scale, glm::vec3(1.0, 0.0f, 0.0f));
        }

      }
      
      chip8.redraw_screen = 0;
      glfwSwapBuffers(window);
    }

    
    glfwPollEvents();

    auto end = std::chrono::steady_clock::now();
    auto delta_time = std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count();
      //printf("delta_time: %ld\n", delta_time);
      
    if(delta_time < interval){
      std::this_thread::sleep_for(std::chrono::nanoseconds(interval - delta_time));
    }
  }

  glfwDestroyWindow(window);
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

  if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
    glfwSetWindowShouldClose(window, true);
    return;
  }

  *last_key_pressed = -1;
  
  if(glfwGetKey(window, GLFW_KEY_F1) == GLFW_PRESS)
    settings.debugging_mode = !settings.debugging_mode;

  
  int k_inx = -1;
  switch(key){
    case GLFW_KEY_X:
      k_inx = 0;
      break;
    case GLFW_KEY_1:
      k_inx = 1;
      break;
    case GLFW_KEY_2:
      k_inx = 2;
      break;
    case GLFW_KEY_3:
      k_inx = 3;
      break;
    
    case GLFW_KEY_Q:
      k_inx = 4;
      break;
    case GLFW_KEY_W:
      k_inx = 5;
      break;
    case GLFW_KEY_E:
      k_inx = 6;
      break;

    case GLFW_KEY_A:
      k_inx = 7;
      break;
    case GLFW_KEY_S:
      k_inx = 8;
      break;
    case GLFW_KEY_D:
      k_inx = 9;
      break;

    case GLFW_KEY_Z:
      k_inx = 10;
      break;
    case GLFW_KEY_C:
      k_inx = 11;
      break;
    case GLFW_KEY_4:
      k_inx = 12;
      break;
    
    case GLFW_KEY_R:
      k_inx = 13;
      break;
    case GLFW_KEY_F:
      k_inx = 14;
      break;
    case GLFW_KEY_V:
      k_inx = 15;
      break;
  }

  if(k_inx == -1) return;

  if(action == GLFW_RELEASE) {
    key_pressed[k_inx] = false;
  }
  
  else {
    key_pressed[k_inx] = true;
    *last_key_pressed = k_inx;
  }

}


void framebuffer_size_callback(GLFWwindow *window, int width, int height){
  glViewport(0, 0, width, height);
}
