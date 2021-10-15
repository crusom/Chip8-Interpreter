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

class Chip8 {
  public:
    Chip8();
    ~Chip8(){};
    void MainLoop();
    bool LoadRom(const char * filename);
    void Reset();
    void InitOpcodeTable();
    
    uint8_t memory[4096];
    // registers
    uint8_t V[16];
    // store memory address
    uint16_t I;
    // program counter
    uint16_t pc; 

    uint8_t screen[64 * 32];
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
    
    uint16_t opcode;
    uint8_t delay_timer;
    uint8_t sound_timer;
    //std::chrono::time_point<std::chrono::_V2::steady_clock, std::chrono::duration<long int, std::ratio<1, 1000000000>>> time_delay_timer;
    //std::chrono::time_point<std::chrono::_V2::steady_clock, std::chrono::duration<long int, std::ratio<1, 1000000000>>> time_sound_timer;

    // functions
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
    //time_delay_timer = std::chrono::steady_clock::now();
    //time_sound_timer = std::chrono::steady_clock::now();
    redraw_screen = false;
    last_key_pressed = -1;

    int i = 0;
    
    memset(screen, 0, 2048 * sizeof(screen[0]));
    for(i = 0; i < 16; i++){
      V[i] = 0; 
      key_pressed[i] = false;
    }

    memset(memory, 0, 4096 * sizeof(memory[0]));

    for(i = 0; i < 80; i++) memory[i] = fontset[i];
    
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
      { 0x00E0, 0xFFFF, &Chip8::Opcode00E0 },
      { 0x00EE, 0xFFFF, &Chip8::Opcode00EE },
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
    };
  }
  // "its usually not implemented this days"
  // however maybe make this optional?
  void Chip8::Opcode0NNN(Args args) {}

  void Chip8::Opcode00E0(Args args) {

    memset(screen, 0, 2048 * sizeof(screen[0]));
    redraw_screen = true;
  }

  // return from the subroutine;  
  void Chip8::Opcode00EE(Args args) {
  
    if(call_stack.empty()) return;
    
    pc = call_stack.top();
    call_stack.pop();
 }

  void Chip8::Opcode1NNN(Args args) {
    
    pc = args.NNN;
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

  void Chip8::Opcode8XY6(Args args) {
    // TODO optional or configurable
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
    V[args.X] = V[args.Y];
    
    uint8_t bit = V[args.X] & 128;
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
    // ambiguous instruction
    //pc = args.NNN + V[args.X];
  
    pc = (args.NNN + V[0]) & 0xfff;
  }
  
  // random
  void Chip8::OpcodeCXNN(Args args) {

    V[args.X] = rand() & args.NN;
  }
  

  void Chip8::OpcodeDXYN(Args args) {   

    uint8_t x = V[args.X] % 64;
    uint8_t y = V[args.Y] % 32;
    uint8_t n = args.N;
    bool flipped = false;

    
    for(int j = 0;  j < n; j++, y++) {
 
      // we need to wrap y around every time in order to
      // do not exceed the height
      // otherwise it may be glichy ;o
      y %= 32;

      uint8_t pixel = memory[I + j];
   
      for(int k = 0; k < 8; k++) {
        // read from right to left
        // otherwise screen is mirrored
        uint8_t bit = ((pixel << k) & 128);
        if(bit && screen[x + k + y * 64]) {
          flipped |= true;
        }

        //  Sprites are XORed onto the existing screen
        //  If this causes any pixels to be erased, VF is set to 1, otherwise it is set to 0.
        //  http://devernay.free.fr/hacks/chip8/C8TECH10.HTM#8xy3
        
        // wrap around cause when pixel goes through x = 64 then y needs to stay the same, not
        // be increased
        screen[(x + k) % 64 + y * 64] ^= bit;
      }

    }
    
    // if any bit is flipped
    V[0xF] = flipped;

    redraw_screen = true;
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
/*
    auto current_time = std::chrono::steady_clock::now();
    auto delta_time = (time_delay_timer - current_time) / std::chrono::milliseconds(1);
    time_delay_timer = current_time;

    uint8_t fixed_delta_time = uint8_t(delta_time * 0.06);

    delay_timer = std::max(0, (int)delay_timer - (int)fixed_delta_time);
    delay_timer = std::max(0, (int)delay_timer - (int)fixed_delta_time);

    printf("fixed_delta_time: %d\n", fixed_delta_time);   
  */ 
    V[args.X] = delay_timer;
    
  }
  
  void Chip8::OpcodeFX15(Args args) {

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
  // TODO optional COSMAC VIP behaviour?  
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

    for(const auto& entry : opcode_table) {

      if((opcode & entry.mask) == entry.opcode) {
        //printf("opcode: %x\n", entry.opcode);
        auto handler = entry.handler;
        // calling opcode through function pointer
        (this->*handler)(args);
        break;
      }
    } 
    if(delay_timer > 0){
      //printf("delaytimer: %d\n", delay_timer);
      --sound_timer;
      --delay_timer;
    }
}
