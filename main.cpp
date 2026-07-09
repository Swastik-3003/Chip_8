#include<iostream>
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

const unsigned int VIDEO_WIDTH=32;
const unsigned int VIDEO_HEIGHT=64;
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
		
		std::default_random_engine randGen;
		std::uniform_int_distribution<uint8_t> randByte;

		Chip8(): 
		randGen(std::chrono::system_clock::now().time_since_epoch().count()){
			pc=START_ADDRESS;

			for(unsigned int i=0; i<FONTSET_SIZE; i++){
				memory[FONTSET_START_ADDRESS+i]=fontset[i];
			}
			randByte = std::uniform_int_distribution<uint8_t>(0, 255U);
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
			index=address;
		}
		void OP_Bnnn(){// JP V0,addr jump to addr+v[0]
			uint16_t address= opcode & 0x0FFFu;
			pc=register[0]+address;
		}
		void OP_Cxkk(){//RND Vx, byte
			uint8_t Vx = (opcode & 0x0F00u)>>8u;
			uint8_t byte = (opcode && 0x00FFu);
			register[Vx] = (randByte(randGen) & byte);
		}
		void OP_Dxyn(){// draw at vx,vy from I
			uint8_t Vx = (opcode & 0x0F00u)>>8u;
			uint8_t Vy = (opcode & 0x00F0u)>>4u;
			uint8_t height = opcode & 0x000Fu;

			uint8_t x = register[Vx]%VIDEO_WIDTH;
			uint8_t y = register[Vy]%VIDEO_HEIGHT;
			register[0xF] = 0;
			for(unsigned int row = 0; row<height; row++){
				uint8_t spriteByte = memory[index+row];
				for(unsigned int col = 0; col<8; col++){
					uint8_t spritePixel = spriteByte & (0x80u >> col);
					uint32_t* screenPixel = &video[(y+row)*VIDEO_WIDTH + (x+col)];

					if(spritePixel){
						if(*screenPixel == 0xFFFFFFFF){
							register[0xF]=1;
						}
						*screenPixel ^= 0xFFFFFFFF;
					}
				}
			}
		}
		void OP_Ex9E(){// SKP Vx skip if key with val at vx is press
			uint8_t Vx = (opcode & 0x0F00u)>>8u;
			uint8_t key = register[Vx];
			if(keypad[key]){
				pc+=2;
			}
		}
		void OP_ExA1(){//SKNP Vx not skip if key with val at vx is press
			uint8_t Vx = (opcode & 0x0F00u)>>8u;
			uint8_t key = register[Vx];
			if(!keypad[key]){
				pc+=2;
			}
		}
		void OP_Fx07(){//LD Vx, DT set delay timer
			uint8_t Vx = (opcode & 0x0F00u)>>8u;
			register[Vx] = delayTimer;
		}
		void OP_Fx0A(){//LD Vx,K set Vx=key pressed and wait for it
			uint8_t Vx = (opcode & 0x0F00u)>>8u;
			if(keypad[0]){
				register[Vx]=0;
			}
			else if (keypad[1]){
				registers[Vx] = 1;
			}
			else if (keypad[2]){
				registers[Vx] = 2;
			}
			else if (keypad[3]){
				registers[Vx] = 3;
			}
			else if (keypad[4]){
				registers[Vx] = 4;
			}
			else if (keypad[5]){
				registers[Vx] = 5;
			}
			else if (keypad[6]){
				registers[Vx] = 6;
			}
			else if (keypad[7]){
				registers[Vx] = 7;
			}
			else if (keypad[8]){
				registers[Vx] = 8;
			}
			else if (keypad[9]){
				registers[Vx] = 9;
			}
			else if (keypad[10]){
				registers[Vx] = 10;
			}
			else if (keypad[11]){
				registers[Vx] = 11;
			}
			else if (keypad[12]){
				registers[Vx] = 12;
			}
			else if (keypad[13]){
				registers[Vx] = 13;
			}
			else if (keypad[14]){
				registers[Vx] = 14;
			}
			else if (keypad[15]){
				registers[Vx] = 15;
			}
			else{
				pc -= 2;
			}
		}
		void OP_Fx15(){//LD DT, Vx
			uint8_t Vx = (opcode & 0x0F00u)>>8u;
			delayTimer = register[Vx];
		}
		void OP_Fx18(){//LD ST, Vx
			uint8_t Vx= (opcode & 0x0F00u)>>8u;
			soundTimer = register[Vx];
		}
		void OP_Fx1E(){//ADD I,Vx
			uint8_t Vx = (opcode & 0x0F00u)>>8u;
			index += register[Vx];
		}
		void OP_Fx29(){//LD F, Vx set I = location of sprite for digit in vx
			uint8_t Vx = (opcode & 0x0F00u)>>8u;
			uint8_t digit = register[Vx];
			index = FONTSET_START_ADDRESS + (5*digit);
		}
		void OP_Fx33(){//LD B, Vx BCD represenation of Vx stored in I,I+1,I+2
			uint8_t Vx = (opcode & 0x0F00u)>>8u;
			uint8_t number = register[Vx];
			memory[index+2] = number%10;
			number/=10;
			memory[index+1] = number%10;
			number/=10;
			number[index] = number;
		}
		void OP_Fx55(){//LD [I],Vx stores data at register 0 thru Vx at I's
			uint8_t Vx = (opcode & 0x0F00u)>>8u;
			for(int i=0; i<=Vx; i++){
				memory[index+i] = register[i];
			}
		}
		void OP_Fx65(){//LD Vx,[I] stores data at register 0 to Vx at I's
			uint8_t Vx = (opcode & 0x0F00u)>>8u;
			for(int i=0; i<=Vx; i++){
				register[i] = memory[index+i];
			}
		}
		
};

int main(){

}