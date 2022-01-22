#include <cstdio>
#include <stdint.h>
#include <cstring>
#include <memory>
#include <cstdlib>
#include <vector>
#include <stack>
#include <time.h>
#include <filesystem>
#include <chrono>

#include <algorithm>

class Chip8 {
  public:
    Chip8();
    ~Chip8(){};
    void MainLoop();
    bool LoadRom(const char * filename);
    void Reset();
    void InitOpcodeTable();
    void DebugRender();
    //void SChipExtend();
    
    uint8_t memory[4096];
    // registers
    uint8_t V[16];
    // store memory address
    uint16_t I;
    // program counter
    uint16_t pc; 

    int X, Y;

    std::vector<uint8_t> screen;
    uint8_t screen_width, screen_height;

    bool key_pressed[16];
    int last_key_pressed;
    bool redraw_screen;

    typedef union {
      // bits are read from the end
      // N = last 4 bits
      // we use Big-endian, so N is the lowest 4 bits of the instruction
      // [first_semibit, X, Y, N] 16bits
      struct {
        uint16_t N : 4;
        uint16_t Y : 4;
        uint16_t X : 4;
        uint16_t HB : 4; //Highest Bit
      };

      uint16_t NNN : 12;
      uint16_t NN : 8;  
      
      uint16_t value;
    } Args;

    std::string curr_opcode = "";
    bool disas = false;
    bool is_extended = false;
    
    struct Quirks {
      bool jump = false;
      bool shift = false;
      bool clip_sprite = false;
    } Quirks;

    struct Quirks quirks;
    bool hires = false;
    std::vector<int> breakpoints;

  private:
 

    struct OpcodeTableEntry {
      uint16_t opcode;
      uint16_t mask;
      /*
       A handle can be anything from an integer index to a pointer to a resource in kernel space. 
       The idea is that they provide an abstraction of a resource, so you don't need to know much about the resource itself to use it.
       
       https://stackoverflow.com/questions/1303123/what-is-a-handle-in-c
        
       in this case we're calling associated opcode function
      */
      void (Chip8::*handler)(Args);
      
    };

    std::vector<OpcodeTableEntry> opcode_table;
    std::stack<uint16_t> call_stack;
    // schip
    uint8_t rpl_flags[8];

    uint16_t opcode;
    uint8_t delay_timer;
    uint8_t sound_timer;
    //std::chrono::steady_clock time_delay_timer;
    std::chrono::time_point<std::chrono::_V2::steady_clock, std::chrono::duration<long int, std::ratio<1, 1000000000>>> time_delay_timer;
    //std::chrono::time_point<std::chrono::_V2::steady_clock, std::chrono::duration<long int, std::ratio<1, 1000000000>>> time_sound_timer;

    void Opcode0NNN(Args args);
    void Opcode00E0(Args args);
    void Opcode00EE(Args args);
    void Opcode1NNN(Args args);
    void Opcode2NNN(Args args);
    void Opcode3XNN(Args args);
    void Opcode4XNN(Args args);
    void Opcode5XY0(Args args);
    void Opcode6XNN(Args args);
    void Opcode7XNN(Args args);
    void Opcode8XY0(Args args);
    void Opcode8XY1(Args args);
    void Opcode8XY2(Args args);
    void Opcode8XY3(Args args);
    void Opcode8XY4(Args args);
    void Opcode8XY5(Args args);
    void Opcode8XY6(Args args);
    void Opcode8XY7(Args args);
    void Opcode8XYE(Args args);
    void Opcode9XY0(Args args);
    void OpcodeANNN(Args args);
    void OpcodeBNNN(Args args);
    void OpcodeCXNN(Args args);
    void OpcodeDXYN(Args args);
    void OpcodeEX9E(Args args);
    void OpcodeEXA1(Args args);
    void OpcodeFX07(Args args);
    void OpcodeFX0A(Args args);
    void OpcodeFX15(Args args);
    void OpcodeFX18(Args args);
    void OpcodeFX1E(Args args);
    void OpcodeFX29(Args args);
    void OpcodeFX33(Args args);
    void OpcodeFX55(Args args);
    void OpcodeFX65(Args args);
    //SChip
    void Opcode00CN(Args args); 
    void Opcode00FB(Args args);
    void Opcode00FC(Args args);
    void Opcode00FD(Args args);
    void Opcode00FE(Args args);
    void Opcode00FF(Args args);
    void OpcodeFX30(Args args);
    void OpcodeFX75(Args args);
    void OpcodeFX85(Args args);

    void Disassembly(Args args, uint16_t opcode);
};



  // every char is 5 bytes long
  uint8_t fontset[80] ={ 
      0xF0, 0x90, 0x90, 0x90, 0xF0, //0
      0x20, 0x60, 0x20, 0x20, 0x70, //1
      0xF0, 0x10, 0xF0, 0x80, 0xF0, //2
      0xF0, 0x10, 0xF0, 0x10, 0xF0, //3
      0x90, 0x90, 0xF0, 0x10, 0x10, //4
      0xF0, 0x80, 0xF0, 0x10, 0xF0, //5
      0xF0, 0x80, 0xF0, 0x90, 0xF0, //6
      0xF0, 0x10, 0x20, 0x40, 0x40, //7
      0xF0, 0x90, 0xF0, 0x90, 0xF0, //8
      0xF0, 0x90, 0xF0, 0x10, 0xF0, //9
      0xF0, 0x90, 0xF0, 0x90, 0x90, //A
      0xE0, 0x90, 0xE0, 0x90, 0xE0, //B
      0xF0, 0x80, 0x80, 0x80, 0xF0, //C
      0xE0, 0x90, 0x90, 0x90, 0xE0, //D
      0xF0, 0x80, 0xF0, 0x80, 0xF0, //E
      0xF0, 0x80, 0xF0, 0x80, 0x80  //F
  };

  
  uint8_t fontset_extended[100] ={ 
      //0xF0, 0xF0, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0xF0, 0xF0, //0
      //0x20, 0x20, 0x60, 0x60, 0x20, 0x20, 0x20, 0x20, 0x70, 0x70, //1
      //0xF0, 0xF0, 0x10, 0x10, 0xF0, 0xF0, 0x80, 0x80, 0xF0, 0xF0, //2
      //0xF0, 0xF0, 0x10, 0x10, 0xF0, 0xF0, 0x10, 0x10, 0xF0, 0xF0, //3
      //0x90, 0x90, 0x90, 0x90, 0xF0, 0xF0, 0x10, 0x10, 0x10, 0x10, //4
      //0xF0, 0xF0, 0x80, 0x80, 0xF0, 0xF0, 0x10, 0x10, 0xF0, 0xF0, //5
      //0xF0, 0xF0, 0x80, 0x80, 0xF0, 0xF0, 0x90, 0x90, 0xF0, 0xF0, //6
      //0xF0, 0xF0, 0x10, 0x10, 0x20, 0x20, 0x40, 0x40, 0x40, 0x40, //7
      //0xF0, 0xF0, 0x90, 0x90, 0xF0, 0xF0, 0x90, 0x90, 0xF0, 0xF0, //8
      //0xF0, 0xF0, 0x90, 0x90, 0xF0, 0xF0, 0x10, 0x10, 0xF0, 0xF0, //9
      // Schip
      0x3C, 0x7E, 0xE7, 0xC3, 0xC3, 0xC3, 0xC3, 0xE7, 0x7E, 0x3C,
      0x18, 0x38, 0x58, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3C,
      0x3E, 0x7F, 0xC3, 0x06, 0x0C, 0x18, 0x30, 0x60, 0xFF, 0xFF,
      0x3C, 0x7E, 0xC3, 0x03, 0x0E, 0x0E, 0x03, 0xC3, 0x7E, 0x3C,
      0x06, 0x0E, 0x1E, 0x36, 0x66, 0xC6, 0xFF, 0xFF, 0x06, 0x06,
      0xFF, 0xFF, 0xC0, 0xC0, 0xFC, 0xFE, 0x03, 0xC3, 0x7E, 0x3C,
      0x3E, 0x7C, 0xE0, 0xC0, 0xFC, 0xFE, 0xC3, 0xC3, 0x7E, 0x3C,
      0xFF, 0xFF, 0x03, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x60, 0x60,
      0x3C, 0x7E, 0xC3, 0xC3, 0x7E, 0x7E, 0xC3, 0xC3, 0x7E, 0x3C,
      0x3C, 0x7E, 0xC3, 0xC3, 0x7F, 0x3F, 0x03, 0x03, 0x3E, 0x7C,
  };



  void Chip8::DebugRender(){
    for (int y = 0; y < screen_height - 1; y++)
    {
        for (int x = 0; x < screen_width - 1; x++)
        {
            if (screen.at(x + y * screen_width) == 0)
                printf(" ");
            else
                printf("O");
        }
        printf("\n");
    }
    printf("\n");

  }


  Chip8::Chip8(){
    InitOpcodeTable();
    Reset();
  }

  void Chip8::Reset(){

    // most programs written for the original system begin at memory location 512 (0x200)
    pc = 0x200;
    I = 0;
    opcode = 0;
    delay_timer = 0;
    sound_timer = 0;
    time_delay_timer = std::chrono::steady_clock::now();
    //time_sound_timer = std::chrono::steady_clock::now();
    redraw_screen = false;
    last_key_pressed = -1;
   
    screen.resize(2048);
    screen_width = 64;
    screen_height = 32;

    int i = 0;

    for(i = 0; i < 16; i++){
      V[i] = 0; 
      key_pressed[i] = false;
    }

    memset(memory, 0, 4096 * sizeof(memory[0]));

    for(i = 0; i < 80; i++) memory[i] = fontset[i];
    for(i = 0; i < 100; i++) memory[i + 80] = fontset_extended[i];

    srand(time(NULL));
  }

  void Chip8::InitOpcodeTable() {

    /* 
    "However, what about the case where you have 26 non-contiguous indices, that vary in value from 0 to 1000? 
    A jump table would have 974 null entries or 1948 “wasted” bytes on the average microcontroller.
    By coding this as a jump table, you ensure that the compiler does what you want."
    that's why jump table is faster and cleaner in our case

    https://rmbconsulting.us/Publications/jump-Tables.pdf


    we put 'F' in mask when we have constants because we don't want to include variables in our opcode
    32:47min https://www.youtube.com/watch?v=locDS3uHv_E
    */

    opcode_table = std::vector<OpcodeTableEntry> {
      // opcode|mask|function pointer
      // chip8
      { 0x00E0, 0xFFFF, &Chip8::Opcode00E0 },
      { 0x00EE, 0xFFFF, &Chip8::Opcode00EE },
      
      // Schip extension
      { 0x00C0, 0xFFF0, &Chip8::Opcode00CN },
      { 0x00FB, 0xFFFF, &Chip8::Opcode00FB },
      { 0x00FC, 0xFFFF, &Chip8::Opcode00FC },
      { 0x00FD, 0xFFFF, &Chip8::Opcode00FD },
      { 0x00FE, 0xFFFF, &Chip8::Opcode00FE },
      { 0x00FF, 0xFFFF, &Chip8::Opcode00FF }, 
      // chip8 continuation
      { 0x0000, 0xF000, &Chip8::Opcode0NNN },
      { 0x1000, 0xF000, &Chip8::Opcode1NNN },
      { 0x2000, 0xF000, &Chip8::Opcode2NNN },
      { 0x3000, 0xF000, &Chip8::Opcode3XNN },
      { 0x4000, 0xF000, &Chip8::Opcode4XNN },
      { 0x5000, 0xF00F, &Chip8::Opcode5XY0 },
      { 0x6000, 0xF000, &Chip8::Opcode6XNN },
      { 0x7000, 0xF000, &Chip8::Opcode7XNN },
      { 0x8000, 0xF00F, &Chip8::Opcode8XY0 },
      { 0x8001, 0xF00F, &Chip8::Opcode8XY1 },
      { 0x8002, 0xF00F, &Chip8::Opcode8XY2 },
      { 0x8003, 0xF00F, &Chip8::Opcode8XY3 },
      { 0x8004, 0xF00F, &Chip8::Opcode8XY4 },
      { 0x8005, 0xF00F, &Chip8::Opcode8XY5 },
      { 0x8006, 0xF00F, &Chip8::Opcode8XY6 },
      { 0x8007, 0xF00F, &Chip8::Opcode8XY7 },
      { 0x800E, 0xF00F, &Chip8::Opcode8XYE },
      { 0x9000, 0xF00F, &Chip8::Opcode9XY0 },
      { 0xA000, 0xF000, &Chip8::OpcodeANNN },
      { 0xB000, 0xF000, &Chip8::OpcodeBNNN },
      { 0xC000, 0xF000, &Chip8::OpcodeCXNN },
      { 0xD000, 0xF000, &Chip8::OpcodeDXYN },
      { 0xE09E, 0xF0FF, &Chip8::OpcodeEX9E },
      { 0xE0A1, 0xF0FF, &Chip8::OpcodeEXA1 },
      { 0xF007, 0xF0FF, &Chip8::OpcodeFX07 },
      { 0xF00A, 0xF0FF, &Chip8::OpcodeFX0A },
      { 0xF015, 0xF0FF, &Chip8::OpcodeFX15 },
      { 0xF018, 0xF0FF, &Chip8::OpcodeFX18 },
      { 0xF01E, 0xF0FF, &Chip8::OpcodeFX1E },
      { 0xF029, 0xF0FF, &Chip8::OpcodeFX29 },
      { 0xF033, 0xF0FF, &Chip8::OpcodeFX33 },
      { 0xF055, 0xF0FF, &Chip8::OpcodeFX55 },
      { 0xF065, 0xF0FF, &Chip8::OpcodeFX65 },
      // schip continuation
      { 0xF030, 0xF0FF, &Chip8::OpcodeFX30 },
      { 0xF075, 0xF0FF, &Chip8::OpcodeFX75 },
      { 0xF085, 0xF0FF, &Chip8::OpcodeFX85 },
    };
  }
  // "its usually not implemented this days"
  // however maybe make this optional?
  void Chip8::Opcode0NNN(Args args) {}

  void Chip8::Opcode00E0(Args args) {
    
    std::fill(screen.begin(), screen.end(), 0);
    redraw_screen = true;
  }

  // return from the subroutine;  
  void Chip8::Opcode00EE(Args args) {
  
    if(call_stack.empty()) return;
    
    pc = call_stack.top();
    call_stack.pop();
 }

  void Chip8::Opcode1NNN(Args args) {
    
    if(quirks.jump) {
      pc = args.NNN + V[args.X];
    } 
    else {
      pc = args.NNN;
    }
  }
 
  //call the subroutine
  //Instruction should first should push the current PC to the stack, so the subroutine can return later
  void Chip8::Opcode2NNN(Args args) {

    call_stack.push(pc);
    pc = args.NNN;
  }
  
  void Chip8::Opcode3XNN(Args args) {

    if(V[args.X] == args.NN) pc += 2;
  }

  
  void Chip8::Opcode4XNN(Args args) {

    if(V[args.X] != args.NN) pc += 2;
  }
  
  void Chip8::Opcode5XY0(Args args) {

    if(V[args.X] == V[args.Y]) pc += 2;
  }

  void Chip8::Opcode6XNN(Args args) {
 
    V[args.X] = args.NN;
  }

  void Chip8::Opcode7XNN(Args args) {
    
    V[args.X] += args.NN;
  }

  void Chip8::Opcode8XY0(Args args) {

    V[args.X] = V[args.Y];
  }

  void Chip8::Opcode8XY1(Args args) {

    V[args.X] |= V[args.Y];
  }

  void Chip8::Opcode8XY2(Args args) {
    
    V[args.X] &= V[args.Y];
  }

  void Chip8::Opcode8XY3(Args args) {

    V[args.X] ^= V[args.Y];
  }

  void Chip8::Opcode8XY4(Args args) {

    uint16_t temp = (uint16_t)V[args.X] + (uint16_t)V[args.Y];
    V[args.X] = (uint8_t)temp;
    V[0xf] = (temp >= 0x100);
  }

  void Chip8::Opcode8XY5(Args args) {
   
    int16_t temp = (int16_t)V[args.X] - (int16_t)V[args.Y];
    V[args.X] = (uint8_t)temp;
    // if Y is smaller, everything is ok and we dont carry the flag
    // if Y is bigger, then we have underflow and we carry the flag
    V[0xf] = (temp >= 0);
  }

  //https://github.com/Chromatophore/HP48-Superchip/blob/master/investigations/quirk_shift.md
  void Chip8::Opcode8XY6(Args args) {
    // TODO optional or configurable
    if(!quirks.shift)
      V[args.X] = V[args.Y]; 
    
    uint8_t bit = (V[args.X] & 1);
    V[args.X] = (V[args.X] >> 1);
    V[0xf] = bit;
  }

  void Chip8::Opcode8XY7(Args args) {
    // if X is smaller, everything is ok and we dont carry the flag
    // if X is bigger, then we have underflow and we carry the flag
    int16_t temp = (int16_t)V[args.Y] - (int16_t)V[args.X];
    V[args.X] = (uint8_t)temp;
    V[0xf] = (temp >= 0);
  }

  void Chip8::Opcode8XYE(Args args) {
    // TODO optional or configurable
    if(!quirks.shift)
      V[args.X] = V[args.Y];
    
    uint8_t bit = (V[args.X] & 128) >> 7;
    V[args.X] = V[args.X] << 1;
    V[0xf] = bit;
  }

  void Chip8::Opcode9XY0(Args args) {

    if(V[args.X] != V[args.Y]) pc += 2;
  }

  void Chip8::OpcodeANNN(Args args) {

    I = args.NNN;
  }

  void Chip8::OpcodeBNNN(Args args) {
    // TODO may be wrong implemented (?)
    //pc = args.NNN + V[args.X];
  
    pc = (args.NNN + V[0]) & 0xfff;
  }
  
  // random
  void Chip8::OpcodeCXNN(Args args) {

    V[args.X] = rand() & args.NN;
  }
  

  void Chip8::OpcodeDXYN(Args args) {   
    
    // x,y,n are the same for extended and non-extended mode
    uint8_t n = args.N;
    uint8_t x, y;
    //x = V[args.X];
    //y = V[args.Y];
    x = V[args.X] % screen_width;
    y = V[args.Y] % screen_height;
    bool flipped = false;

    if(hires && n == 0){

      for(int i = 0; i < 32; i+=2, y++){ 
        //https://github.com/Chromatophore/HP48-Superchip/blob/master/investigations/quirk_collide.md
        //if(y > screen_height){ 
        //  V[0xF] = y - screen_height;
        //  break;
       // }
        for(int j = 0; j < 2; j++){

          uint8_t pixel = memory[I + i + j];
            
          for(int k = 0; k < 8; k++) {
            uint8_t bit = ((pixel << k) & 128);
            if(x % screen_width + k + (j * 8) >= screen_width || ((y - (i<<1)) % screen_height) + (i<<1) >= screen_height) continue; 

            if(bit && screen[(j * 8 + x + k) + y * screen_width]) {
              flipped |= true;
            }
              
              screen[(x + k + j * 8) % screen_width + y * screen_width] ^= bit;
            }

        
        } 
      }
        V[0xF] = flipped;
        redraw_screen = true;
    }
    

    else {
        
      // wrap x and y in default chip8
      for(int j = 0;  j < n; j++, y++) {

        // we need to wrap y around every time in order to
        // do not exceed the height
        // otherwise it may be glichy ;o
        y %= screen_height;

        uint8_t pixel = memory[I + j];
        for(int k = 0; k < 8; k++) {
          // bits are stored in Big-endian
          // so read from left to right
          uint8_t bit = ((pixel << k) & 128);
          
          if(bit && screen[x + k + y * screen_width]) {
            flipped |= true;
          }

          //  Sprites are XORed onto the existing screen
          //  If this causes any pixels to be erased, VF is set to 1, otherwise it is set to 0.
          //  http://devernay.free.fr/hacks/chip8/C8TECH10.HTM#8xy3
          
          // wrap around cause when pixel goes through x = 64 then y needs to stay the same, not
          // be increased
          screen[(x + k) % screen_width + y * screen_width] ^= bit;
        }

      }
      
      // if any bit is flipped
      V[0xF] = flipped;

      redraw_screen = true;
    }
  }

  void Chip8::OpcodeEX9E(Args args) {
    int idx = V[args.X];
    bool pressed = false;
    if(idx < 0x10) {
      pressed = key_pressed[idx];
    }

    if(pressed) {    
      pc += 2;
    }
  }
  
  void Chip8::OpcodeEXA1(Args args) {
    int idx = V[args.X];
    bool pressed = false;
    if(idx < 0x10) {
      pressed = key_pressed[idx];
    }

    if(!pressed){  
      pc += 2;
    }
  }
  
  void Chip8::OpcodeFX07(Args args) {

    if(delay_timer > 0){

      auto current_time = std::chrono::steady_clock::now();
      auto delta_time = std::chrono::duration_cast<std::chrono::nanoseconds>(current_time-time_delay_timer).count();
      
      //printf("delta_time: %d\n", delta_time);
      // 1 / 0.00000006 == 16666666 :)
      if(delta_time > 16666666){
        
        time_delay_timer = current_time;
        
       // printf("delay timer1: %d\n", delay_timer);
       // printf("delay_timer: %d\n", (int)delay_timer);
       // printf("subst: %d\n", (int)(delta_time * 0.00000006));
        delay_timer = std::max(0, (int)delay_timer - (int)(delta_time * 0.00000006));

      // printf("delay_timer after sub: %d\n\n", delay_timer);    
      }
    }
    
    V[args.X] = delay_timer;
  }
  
  void Chip8::OpcodeFX15(Args args) {
  
    time_delay_timer = std::chrono::steady_clock::now();
    delay_timer = V[args.X];
  }

  void Chip8::OpcodeFX18(Args args) {

    sound_timer = V[args.X];
  }

  void Chip8::OpcodeFX1E(Args args) {
    if(I + V[args.X] > 4095) V[0xf] = 1;
    I += V[args.X];
  }

  void Chip8::OpcodeFX0A(Args args) {
    // wait until we any key is pressed
    // just repeat this opcode over and over
    if(last_key_pressed == -1) pc -= 2;
    else V[args.X] = last_key_pressed;
  }
  
  void Chip8::OpcodeFX29(Args args) {
    // chars are 5 bytes long and they lie in the
    // beginning of the memory
    I = V[args.X] * 5;
  }

  void Chip8::OpcodeFX33(Args args) {

    memory[I] = V[args.X] / 100;
    memory[I + 1] = (V[args.X] / 10) % 10;
    memory[I + 2] = V[args.X] % 10;
  }

  //"However, modern interpreters (starting with CHIP48 and SUPER-CHIP in the early 90s) used a temporary variable
  //for indexing, so when the instruction was finished, it would still hold the same value as it did before.
  //https://tobiasvl.github.io/blog/write-a-chip-8-emulator/#fx55-and-fx65-store-and-load-memory
  // TODO optional COSMAC VIP behaviour quirk? 
  void Chip8::OpcodeFX55(Args args) {
    for(int i = 0; i <= args.X; i++){
      memory[I + i] = V[i];
    }
  }

  void Chip8::OpcodeFX65(Args args) {
    for(int i = 0; i <= args.X; i++){
      V[i] = memory[I + i];
    }
  }



  //SChip opcodes 

  void Chip8::Opcode00CN(Args args){

    unsigned int lines = args.N * screen_width;
    std::fill(screen.end() - lines, screen.end(), 0);
    std::rotate(screen.rbegin(), screen.rbegin() + lines, screen.rend());
  }
  
  
  //Scroll display 4 pixels right
  void Chip8::Opcode00FB(Args args){
    
    for(int y = 0; y < screen_height; y++){
      
      uint16_t xline = y * screen_width;
      // delete 4 bytes from the left  
      for(int x = 0; x < 4; x++){
        screen.at(xline + x) = 0;
      }
      // go through line and move screen to erased left
      for(int x = 0; x < screen_width - 4; x++){
        screen.at(xline + x) = screen.at(xline + x + 4); 
      } 
      // and erase the last 4 bytes
      for(int x = screen_width - 4; x < screen_width; x++){
        screen.at(xline + x) = 0;
      }

      //next line :)
    }
 
    redraw_screen = true;
  }
    
  //Scroll display 4 pixels left
  void Chip8::Opcode00FC(Args args){

    for(int y = 0; y < screen_height; y++){
      
      uint16_t xline = ((y + 1) * screen_width) - 1;
      // delete 4 bytes from the right  
      for(int x = 0; x < 4; x++){
        screen.at(xline - x) = 0;
      }
      // go through line and move screen to erased right
      for(int x = 0; x < screen_width - 4; x++){
        screen.at(xline - x) = screen.at(xline - x - 4); 
      } 
      // and erase the first 4 bytes
      for(int x = screen_width - 4; x < screen_width; x++){
        screen.at(xline - x) = 0;
      }

      //next line :)
    }

    redraw_screen = true;
  
  }
  
  //Exit CHIP interpreter
  void Chip8::Opcode00FD(Args args){
    //TODO flag that informes to close?
  }
  
  //Disable extended screen mode
  void Chip8::Opcode00FE(Args args){
    //TODO test
    screen.resize(64 * 32);
    std::fill(screen.begin(), screen.end(), 0);
    screen_width = 64;
    screen_height = 32;
    hires = false;
  }
  
  //Enable extended screen mode for full-screen graphics
  void Chip8::Opcode00FF(Args args){
    //TODO test
    screen.resize(128 * 64);
    std::fill(screen.begin(), screen.end(), 0);
    screen_width = 128;
    screen_height = 64;
    hires = true;    
  }
  
  // 10 bytes font
  void Chip8::OpcodeFX30(Args args){
    // 80 is offset, because its filled with non-extended fontset
    I = 80 + V[args.X] * 10;
  }
  
  void Chip8::OpcodeFX75(Args args){
    for(int i = 0; i <= args.X; i++)
      rpl_flags[i] = V[i];
  }
  
  void Chip8::OpcodeFX85(Args args){
    for(int i = 0; i <= args.X; i++)
      V[i] = rpl_flags[i];
  }

  void Chip8::Disassembly(Args args, uint16_t opcode){
    
    printf("%04x ", opcode);

    // instruction names taken from http://devernay.free.fr/hacks/chip8/C8TECH10.HTM#3.1
    switch(opcode){


      case 0x00E0: printf("CLS"); break; 
      case 0x00EE: printf("RET"); break;
      case 0x0000: printf("\x1B[91mMachine code not implemented\033[0m");break;
      case 0x1000: printf("JP %03x", args.NNN); break;
      case 0x2000: printf("CALL #%03x", args.NNN); break; 
      case 0x3000: printf("SE Vx, #%02x", args.NN); break;
      case 0x4000: printf("SNE Vx, #%02x", args.NN); break; 
      case 0x5000: printf("SE Vx, Vy"); break;
      case 0x6000: printf("LD Vx, #%02x", args.NN); break;
      case 0x7000: printf("ADD Vx, #%02x", args.NN); break;
      case 0x8000: printf("LD Vx, Vy"); break;
      case 0x8001: printf("OR Vx, Vy"); break;
      case 0x8002: printf("AND Vx, Vy"); break;
      case 0x8003: printf("XOR Vx, Vy"); break;
      // -S suffix means carry byte is set
      case 0x8004: printf("ADDS Vx, Vy"); break;
      case 0x8005: printf("SUBS Vx, Vy"); break;
      // "If the least-significant bit of Vx is 1, then VF is set to 1, otherwise 0. Then Vx is divided by 2."
      case 0x8006: printf("SHR Vx, Vy"); break;
      case 0x8007: printf("SUBNS Vx, Vy"); break;
      case 0x800E: printf("SHL Vx, Vy"); break;
      case 0x9000: printf("SNE Vx, Vy"); break;
      case 0xA000: printf("LD I, #%03x", args.NNN); break;
      case 0xB000: printf("JP %02x, %03x", V[0], args.NNN); break;
      case 0xC000: printf("RND Vx, %02x", args.NN); break;
      case 0xD000: printf("DRW x:%02x, y:%02x, n:%01x", V[args.X], V[args.Y], args.N); break;
      case 0xE09E: printf("SKP %02x (pressed)", V[args.X]); break;
      case 0xE0A1: printf("SKNP %02x (Not pressed)", V[args.X]); break;
      case 0xF007: printf("LD Vx, %02x (DelayTimer)", delay_timer); break;
      case 0xF00A: printf("LD Vx, %02x (KeyPressed)", last_key_pressed); break;
      case 0xF015: printf("LD DT, %02x", V[args.X]); break;
      case 0xF018: printf("LD ST, %02x", V[args.X]); break;
      case 0xF01E: printf("ADD I, %02x", V[args.X]); break;
      case 0xF029: printf("LD F, %02x (normal font)", V[args.X]); break;
      case 0xF033: printf("LD B, %02x", V[args.X]); break;
      case 0xF055: printf("LD [I], %02x", V[args.X]); break;
      case 0xF065: printf("LD Vx, [I]"); break;
    
      // s-chip
      case 0x00C0: printf("SCRL DWN #%02x", args.N); break;
      case 0x00FB: printf("SCRL RIGHT"); break;
      case 0x00FC: printf("SCRL LEFT"); break;
      case 0x00FD: printf("EXIT"); break;
      case 0x00FE: printf("EXTENDED MODE OFF"); break;
      case 0x00FF: printf("EXTENDED MODE ON"); break;
      case 0xF030: printf("LD F, %02x (extended font)", V[args.X]); break;
      case 0xF075: printf("ST FLAG %02x", V[args.X]); break;
      case 0xF085: printf("LD FLAG %02x", V[args.X]); break;

    }
    X = args.X, Y = args.Y;
    printf(" X: %01x, Y: %01x", args.X, args.Y);
    printf("\n");
  }
  
  


  //https://dev.krzaq.cc/post/you-dont-need-a-stateful-deleter-in-your-unique_ptr-usually/
  struct FileDeleter {
    void operator()(FILE* ptr) const {
      fclose(ptr);
    }
  };

  bool Chip8::LoadRom(const char * filename){
    
    std::unique_ptr<FILE, FileDeleter> f;
    // reset takes ownership of pointer
    f.reset(fopen(filename, "rb"));
    if (f == nullptr) {
      return false;
    }

    return fread(&memory[0] + 512, 1, 4096 - 512, f.get()) > 0;  
  }


/*
  void Chip8::SChipExtend(){
    std::vector<OpcodeTableEntry> schip_opcode_table = {
      // opcode|mask|function pointer
      { 0x00C0, 0xFFFF, &Chip8::Opcode00CN },
      { 0x00FB, 0xFFFF, &Chip8::Opcode00FB },
      { 0x00FC, 0xFFFF, &Chip8::Opcode00FC },
      { 0x00FD, 0xFFFF, &Chip8::Opcode00FD },
      { 0x00FE, 0xFFFF, &Chip8::Opcode00FE },
      { 0x00FF, 0xFFFF, &Chip8::Opcode00FF }, 
      { 0xF030, 0xF0FF, &Chip8::OpcodeFX30 },
      { 0xF075, 0xF0FF, &Chip8::OpcodeFX75 },
      { 0xF085, 0xF0FF, &Chip8::OpcodeFX85 },
    };

    opcode_table.insert(opcode_table.end(), schip_opcode_table.begin(), schip_opcode_table.end());

  }
*/

  void Chip8::MainLoop(){

    if (pc + 1 >= 4096) {
      printf("error: pc out of bound (%.4x)\n", pc);
      return;
    }


    // first byte, shift and add second byte
    opcode = memory[pc] << 8;
    opcode |= memory[pc + 1];
    /*
    You should then immediately increment the PC by 2, to be ready to fetch the next opcode. 
    Some people do this during the “execute” stage, since some instructions will increment it by 2 more to skip an instruction, 
    but in my opinion that’s very error-prone. Code duplication is a bad thing. 
    If you forget to increment it in one of the instructions, you’ll have problems. Do it here!
    https://tobiasvl.github.io/blog/write-a-chip-8-emulator/#fetch
    */
    pc += 2;
    Args args;
    args.value = opcode;
    
    if(breakpoints.size() != 0){
      for(auto bp : breakpoints) 
        if(bp == pc){      
          return;
      }
    }
    
    if(disas) printf("pc: %03x ", pc);

    std::stringstream ss;
    bool called = false; 
  
    for(const auto& entry : opcode_table) {
      
      if((opcode & entry.mask) == entry.opcode) {
        //std::cout <<std::hex <<entry.mask <<"\t";
        called = true;
        auto handler = entry.handler;
        // calling opcode through function pointer
        (this->*handler)(args);
        if(disas) Disassembly(args, entry.opcode);
        break;
      }
    }
    if(!called) std::cout << "\x1B[91munknown opcode: \033[0m" << std::hex << opcode << "\n";
}
