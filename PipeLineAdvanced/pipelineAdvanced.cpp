#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>

/**************************************/

struct Control
{
	unsigned char RegDst;
	unsigned char Jump;
	unsigned char Branch;
	unsigned char MemRead;
	unsigned char MemtoReg;
	unsigned char ALUOp;
	unsigned char MemWrite;
	unsigned char ALUSrc;
	unsigned char RegWrite;
};

struct Reg_Read
{
	unsigned int Read_data_1;
	unsigned int Read_data_2;
};

struct ALU
{
	unsigned char zero; // 1: enable, 0: disable
	unsigned int ALU_result;
};

struct Control control;
struct Reg_Read reg_read;
struct ALU alu;
unsigned int mem[64] = { 0 };
unsigned int reg[32] = { 0 };

/**************************************/

unsigned int Inst_Fetch(unsigned int read_addr);
void Register_Read(unsigned int read_reg_1, unsigned int read_reg_2);
void Control_Signal(unsigned int opcode);
unsigned char ALU_Control_Signal(unsigned char signal);
void ALU_func(unsigned char ALU_control, unsigned int a, unsigned int b);
unsigned int Memory_Access(unsigned char MemWrite, unsigned char MemRead, unsigned int addr, unsigned int write_data);
void Register_Write(unsigned char RegWrite, unsigned int Write_reg, unsigned int Write_data);
unsigned int Sign_Extend(unsigned int inst_16);
unsigned int Shift_Left_2(unsigned int inst);
unsigned int Add(unsigned int a, unsigned int b);
unsigned int Mux(char signal, unsigned int a_0, unsigned int b_1);
void print_reg_mem(void);

/**************************************/

int main(void)
{
	unsigned int pc = 0;
	FILE *fp;
	unsigned int inst = 0;
	unsigned int inst_31_26 = 0;
	unsigned int inst_25_21 = 0;
	unsigned int inst_20_16 = 0;
	unsigned int inst_15_11 = 0;
	unsigned int inst_15_0 = 0;
	unsigned int inst_ext_32 = 0;
	unsigned int inst_ext_shift = 0;
	unsigned int pc_add_4 = 0;
	unsigned int pc_add_inst = 0;
	unsigned int mux_result = 0;
	unsigned char ALU_control = 0;
	unsigned int inst_25_0 = 0;
	unsigned int jump_addr = 0;
	unsigned int mem_result = 0;
	int total_cycle = 0;

	// register initialization
	/**************************************/
	reg[8] = 41621;
	reg[9] = 41621;
	//reg[11] = 3633;
	reg[16] = 40;
	/**************************************/

	// memory initialization
	/**************************************/
	mem[40] = 3578;

	if (!(fp = fopen(".//input_file//5.txt", "r")))
	{
		printf("error: file open fail !!\n");
		exit(1);
	}

	while (feof(fp) == false)
	{
		fscanf(fp, "%x", &inst);
		mem[pc] = inst;
		pc = pc + 4;
	}
	/**************************************/

	// control initialization
	/**************************************/
	control.RegDst = 0;
	control.Jump = 0;
	control.Branch = 0;
	control.MemRead = 0;
	control.ALUOp = 0;
	control.MemWrite = 0;
	control.ALUSrc = 0;
	control.RegWrite = 0;
	/**************************************/

	print_reg_mem();

	printf("\n ***** Processor START !!! ***** \n");

	pc = 0;

	unsigned int write_reg;
	while (pc < 64)
	{
		// pc +4
		pc_add_4 = Add(pc, 4);  

		// instruction fetch
		inst = Inst_Fetch(pc);
		printf("Instruction = %08x \n", inst);


		// instruction decode
		inst_31_26 = inst >> 26;
		inst_25_21 = (inst & 0x03e00000) >> 21;
		inst_20_16 = (inst & 0x001f0000) >> 16;
		inst_15_11 = (inst & 0x0000f800) >> 11;
		inst_15_0 = inst & 0x0000ffff;
		inst_25_0 = inst & 0x03ffffff;

		//printf("%x, %x, %x, %x, %x, %x", inst_31_26, inst_25_21, inst_20_16, inst_15_11, inst_15_0, inst_25_0);


		


		// register read
		
		Register_Read(inst_25_21, inst_20_16);

		// create control signal
		
		Control_Signal(inst_31_26);  // op 코드를 입력해 컨트롤 값들을 설정

		// create ALU control signal
		
		unsigned char ALU_control_signal = ALU_Control_Signal(control.ALUOp);
		unsigned int ALU_input = Mux(control.ALUSrc, reg_read.Read_data_2, Sign_Extend(inst_15_0));
		
		// ALU
		
		ALU_func(ALU_control_signal, reg_read.Read_data_1, ALU_input);
		
		// memory access

		unsigned int mem_value = Memory_Access(control.MemWrite, control.MemRead, alu.ALU_result, reg_read.Read_data_2);

		// register write

		Register_Write(control.RegWrite, Mux(control.RegDst, inst_20_16, inst_15_11), Mux(control.MemtoReg, alu.ALU_result, mem_value));

		// next pc
		if (alu.ALU_result == 0) {
			alu.zero = 1;
		}
		else {
			alu.zero = 0;
		}
		pc = Mux((control.Branch && alu.zero), pc_add_4, Add(pc_add_4, Shift_Left_2(ALU_input))); // alu값이 0이고 branch이면 현제값에 주소를 더해 계산한 주소로, 아니라면  pc+4로 pc 설정
		pc = Mux(control.Jump, pc, (Shift_Left_2(inst_25_0) | ((pc_add_4 >> 28) << 28)));  // jump인 경우 해당 주소로 pc를 설정 


		total_cycle++;

		// result
		/********************************/
		printf("PC : %d \n", pc);
		printf("Total cycle : %d \n", total_cycle);
		print_reg_mem();
		/********************************/

		system("pause");
	}

	printf("\n ***** Processor END !!! ***** \n");



	return 0;
}

unsigned int Inst_Fetch(unsigned int read_addr)   ///////
{
	return mem[read_addr];
}

void Register_Read(unsigned int read_reg_1, unsigned int read_reg_2) //////
{
	reg_read.Read_data_1 = reg[read_reg_1]; reg_read.Read_data_2 = reg[read_reg_2];
}

/*
void Control_Signal(unsigned int opcode) /////
{
	if (opcode == 0) {  // R-type
		control.ALUOp = 2;    //
		control.RegDst = 1;   //
		control.RegWrite = 1;  //
	}
	else if (opcode == 35) { // lw
		control.ALUSrc = 1;  //
		control.MemtoReg = 1;
		control.RegWrite = 1;  //
		control.MemRead = 1;  //
	}
	else if (opcode == 43) { // sw
		control.ALUSrc = 1;
		control.MemWrite = 1;
	}
	else if (opcode == 4) { // beq
		control.Branch = 1;
		control.ALUOp = 1;  // 
	}
	else if (opcode == 2) { // jump
		control.Jump = 1;
	}
}*/


void Control_Signal(unsigned int opcode)
{
	unsigned int r_format = (opcode == 0), lw = (opcode == 35), sw = (opcode == 43), beq = (opcode == 4);  //op 코드에 맞게 control 값 설정
	control.Jump = (opcode == 2);
	control.RegDst = r_format;
	control.ALUSrc = lw || sw;
	control.MemtoReg = lw;
	control.RegWrite = r_format || lw;
	control.MemRead = lw;
	control.MemWrite = sw;
	control.Branch = beq;
	control.ALUOp = (r_format * 2) + beq;
}

unsigned char ALU_Control_Signal(unsigned char signal) //////
{
	unsigned char ALUop1 = signal << 7;   // ALUop 1의 자리수 추출
	ALUop1 = ALUop1 >> 7;
	unsigned char ALUop2 = signal << 6;  // ALUop 2의 자리수 추출
	ALUop2 = ALUop2 >> 7;
	if (ALUop1 == 0 && ALUop2 == 0) {  // lw일 경우
		return 2;
	}
	if (ALUop1 == 1) { // beq인 경우
		return 6;
	}
	if (ALUop2 == 1) { // R-type인 경우
		return 2;
	}
	return 0;
}

void ALU_func(unsigned char ALU_control, unsigned int a, unsigned int b) //////
{
	if (ALU_control == 2) {
		alu.ALU_result = a + b;  // lw 혹은 r_type인 경우 더하기
	}
	if (ALU_control == 6) {  // beq인 경우 빼기
		alu.ALU_result = a - b;
	}
}

unsigned int Memory_Access(unsigned char MemWrite, unsigned char MemRead, unsigned int addr, unsigned int write_data) //////////
{
	if (MemRead == 1) {   // lw이면 메모리 값을 읽어 반환
		return mem[addr];   
	}
	if (MemWrite == 1) { // sw이면 메모리에 값을 저장
		mem[addr] = write_data;
	}
	return 0;
}

void Register_Write(unsigned char RegWrite, unsigned int Write_reg, unsigned int Write_data)
{
	if (RegWrite == 1) { 
		reg[Write_reg] = Write_data;
	}
}

unsigned int Sign_Extend(unsigned int inst_16)
{
	unsigned int inst_32 = 0;
	if ((inst_16 & 0x00008000)) // minus
	{
		inst_32 = inst_16 | 0xffff0000;
	}
	else // plus
	{
		inst_32 = inst_16;
	}

	return inst_32;
}

unsigned int Shift_Left_2(unsigned int inst)
{
	return inst << 2;
}

unsigned int Mux(char signal, unsigned int a_0, unsigned int b_1)
{
	if (signal == 0) {   // 시그널이 0이면 
		return a_0; // 첫번째 값을 반환
	}
	else { // 1이면
		return b_1; // 두번째 값을 반환
	}
}

unsigned int Add(unsigned int a, unsigned int b)
{
	return a + b;  // 두 값을 더해 반환
}



void print_reg_mem(void)
{
	int reg_index = 0;
	int mem_index = 0;

	printf("\n===================== REGISTER =====================\n");

	for (reg_index = 0; reg_index < 8; reg_index++)
	{
		printf("reg[%02d] = %08d        reg[%02d] = %08d        reg[%02d] = %08d        reg[%02d] = %08d \n",
			reg_index, reg[reg_index], reg_index + 8, reg[reg_index + 8], reg_index + 16, reg[reg_index + 16], reg_index + 24, reg[reg_index + 24]);
	}

	printf("===================== REGISTER =====================\n");

	printf("\n===================== MEMORY =====================\n");

	for (mem_index = 0; mem_index < 32; mem_index = mem_index + 4)
	{
		printf("mem[%02d] = %012d        mem[%02d] = %012d \n",
			mem_index, mem[mem_index], mem_index + 32, mem[mem_index + 32]);
	}
	printf("===================== MEMORY =====================\n");
}

