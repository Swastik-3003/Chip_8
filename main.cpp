#include<stdio.h>
#include<fstream>
#include<cstdint>

class Chip8{
	public:
		uint8_t registers[16]{};
		uint8_t memory[4000]{};
		uint16_t index{};
		uint16_t pc{};
		uint16_t stack[16]{};
		uint8_t sp{};
		uint8_t delayTimer{};
		uint8_t soundTimer{};
		uint8_t keypad[16]{};
		uint32_t video[64*32];
		uint16_t opcode;

		Chip8(){
			pc=START_ADDRESS;
		}

		LoadROM(char const* filename){
			std::ifstream file(filename,std::ios::binary|std::ios::ate);
			
			if(file.is_open()){
				std::streampos size=file.tellg();
				char* buffer=new char[size];
				file.seekg(0,fstream::beg);
				file.read(buffer,size);
				file.close();

				for(long i=0; i<size; i++){
					memory[START_ADDRESS+i]=buffer[i];
				}

				delete buffer[];
			}
		}
};

int main(){

}