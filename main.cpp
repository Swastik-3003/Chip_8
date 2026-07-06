#include<stdio.h>
#include<fstream>
#include<cstdint>

const unsigned int START_ADDRESS = 0x200;

const unsigned int FONTSET_SIZE = 80;
const unsigned int FONTSET_START_ADDRESS = 0x50;
uint8_t fontset[FONTSET_SIZE] ={
	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

class Chip8{
	public:
		uint8_t register[16]{};
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
			//rng to be added
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
			//OPCODE implementation
			void OP_00E0(){
				memset(video,0,sizeof(video));
			}
			void OP_00EE(){//RET
				--sp;
				pc=stack[sp];
			}
			void OP_1nnn(){//JMP
				uint16_t address=opcode & 0x0FFFu;
				pc=address;
			}
			void OP_2nnn(){//CALL
				uint16_t address=opcode & 0x0FFFu;
				stack[sp]=pc;
				++sp;
				pc=address;
			}
			void OP_3xkk(){//SE (skip inst if x=kk)
				uint8_t Vx=(opcode & 0x0F00u) >> 8u;
				uint8_t byte=opcode & 0x00FFu;
				if(register[Vx]==byte){
					pc+=2;
				}
			}
			void OP_4xkk(){//SNE (skip if x!=kk)
				uint8_t Vx=(opcode & 0x0F00u) >> 8u;
				uint8_t byte=opcode & 0x00FFu;
				if(register[Vx]!=byte){
					pc+=2;
				}
			}
			void OP_5xy0(){//SE (skip if vx=vy)
				uint8_t Vx=(opcode & 0x0F00u)>>8u;
				uint8_t Vy=(opcode & 0x00F0u)>>4u;
				if(register[Vx]==register[Vy]){
					pc+=2;
				}
			}
			void OP_6xkk(){//LD (set vx=kk)
				uint8_t Vx=(opcode & 0x0F00u)>>8u;
				uint8_t byte=(opcode & 0x00FFu);
				register[Vx]=byte;
			}
			void OP_7xkk(){//ADD (vx+=k)
				uint8_t Vx=(opcode & 0x0F00u)>>8u;
				uint8_t byte=(opcode & 0x00FFu);
				register[Vx]+=byte;
			}
			void OP_8xy0(){//LD (vx=vy)
				uint8_t Vx=(opcode & 0x0F00u)>>8u;
				uint8_t Vy=(opcode & 0x00F0u)>>4u;
				register[Vx]=register[Vy];
			}
			void OP_8xy1(){//OR vx=vx|vy
				uint8_t Vx=(opcode & 0x0F00u)>>8u;
				uint8_t Vy=(opcode & 0x00F0u)>>4u;
				register[Vx]|=register[Vy];
			}
			void OP_8xy2(){//AND vx=vx&vy
				uint8_t Vx=(opcode & 0x0F00u)>>8u;
				uint8_t Vy=(opcode & 0x00F0u)>>4u;
				register[Vx]&=register[Vy];
			}
			void OP_8xy3(){//XOR vx=vx^vy
				uint8_t Vx=(opcode & 0x0F00u)>>8u;
				uint8_t Vy=(opcode & 0x00F0u)>>4u;
				register[Vx]|^=register[Vy];
			}
			void OP_8xy4(){//ADD vx=vx+vy
				uint8_t Vx=(opcode & 0x0F00u)>>8u;
				uint8_t Vy=(opcode & 0x00F0u)>>4u;
				uint8_t sum=register[Vx]+register[Vy];
				if(sum>255U){
					register[0xF]=1;
				}
				else{
					register[0xF]=0;
				}
				register[Vx]=sum & 0xFFu;
			}
			void OP_8xy5(){//SUB vx=vx-vy
				uint8_t Vx=(opcode & 0x0F00u)>>8u;
				uint8_t Vy=(opcode & 0x00F0u)>>4u;
				
				if(register[Vx] > register[Vy]){
					register[0xF]=1;
				}
				else{
					register[0xF]=0;
				}
				register[Vx]-=register[Vy];
			}
			void OP_8xy6(){//SHR Vx, store(LSB in VF)
				uint8_t Vx=(opcode & 0x0F00u)>>8u;
				register[0xF]=(register[Vx] & 0x1u);
				register[Vx]>>=1;
			}
			void OP_8xy7(){//SUBN Vx=Vy-Vx
				uint8_t Vx=(opcode & 0x0F00u)>>8u;
				uint8_t Vy=(opcode & 0x00F0u)>>4u;
				
				if(register[Vx] > register[Vy]){
					register[0xF]=1;
				}
				else{
					register[0xF]=0;
				}
				register[Vx]=register[Vy]-register[Vx];
			}
			void OP_8xyE(){//SHL shift left & store MSB at VF
				uint8_t Vx=(opcode & 0x0F00u)>>8u;
				register[0xF]=(register[Vx] & 0x80u)>>7u;
				register[Vx]<<=1;
			}
			void OP_9xy0(){//SNE skip if Vx!=Vy
				uint8_t Vx=(opcode & 0x0F00u)>>8u;
				uint8_t Vy=(opcode & 0x00F0u)>>4u;
				if(register[Vx]!=register[Vy]){
					pc+=2;
				}
			}
			void OP_Annn(){// LD I,addr setI=addr
				uint16_t address= opcode & 0x0FFFu;
				
			}
		}
};

int main(){

}