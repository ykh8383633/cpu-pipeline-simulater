#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>

/**************************************/

struct Control
{
	unsigned char RegDst = 0;
	unsigned char Jump = 0;
	unsigned char Branch = 0;
	unsigned char MemRead = 0;
	unsigned char MemtoReg = 0;
	unsigned char ALUOp = 0;
	unsigned char MemWrite = 0;
	unsigned char ALUSrc = 0;
	unsigned char RegWrite = 0;
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
void Initial_control();

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

	if (!(fp = fopen(".//input_file//3.txt", "r")))
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
	unsigned int mem_value = 0;
	struct Control control_reg[4];
	unsigned int if_id[7];
	unsigned int id_ex[6], ex_mem[6], mem_wb[3];
	///////////////////////////////////////////
	while (pc < 64)
	{
		// pc +4
		pc_add_4 = Add(pc, 4);

		// instruction fetch
		inst = Inst_Fetch(pc);
		printf("Instruction = %08x \n", inst);

		////////// mem/wb ?? ????////////////
		mem_wb[0] = mem_value; // ???? ???????? ??
		mem_wb[1] = ex_mem[1]; // ALU???? ???? // mem/wb ?? ????
		mem_wb[2] = ex_mem[3]; // ?????????? ????????

		////////// ex/mem ?? ????////////////
		ex_mem[0] = (alu.ALU_result == 0);  // zero
		ex_mem[1] = alu.ALU_result; // ALU???? ????
		ex_mem[2] = id_ex[1]; // ?????????? ?????? ?? 
		ex_mem[3] = Mux(control_reg[1].RegDst, id_ex[3], id_ex[4]); // ?????????? ???? ???? ???? ?? ????
		ex_mem[4] = Add(id_ex[5], Shift_Left_2(id_ex[2])); // beq?? ???? ???? ?????? ????
		ex_mem[5] = id_ex[5]; // ?? ?????? pc + 4
		ex_mem[6] = id_ex[6]; // jump?? ???? ???? ???? ????

		////////// id_ex ?? ????////////////
		id_ex[0] = reg_read.Read_data_1;  // ???????? ?????? ?? ????
		id_ex[1] = reg_read.Read_data_2; // ???????? ?????? ?? ????
		id_ex[2] = Sign_Extend(if_id[4]); // inst_15_0 ?? sign_extend?? ?? ????
		id_ex[3] = if_id[2]; // inst_20_16 ?? ????
		id_ex[4] = if_id[3]; // inst_15_11 ?? ????
		id_ex[5] = if_id[6];  // ?? ?????? pc + 4
		id_ex[6] = if_id[5];  // jump?? ???? ???? ???? ????

		////////// ifi/id ?? ????////////////
		if_id[0] = inst_31_26;//
		if_id[1] = inst_25_21;//
		if_id[2] = inst_20_16;// // instruction decode?? ???? ???????? if/id?? ????
		if_id[3] = inst_15_11;//
		if_id[4] = inst_15_0; //
		if_id[5] - inst_25_0; //
		if_id[6] = pc_add_4;     // ?? ?????? pc + 4

		// instruction decode
		inst_31_26 = inst >> 26;
		inst_25_21 = (inst & 0x03e00000) >> 21;
		inst_20_16 = (inst & 0x001f0000) >> 16;
		inst_15_11 = (inst & 0x0000f800) >> 11;
		inst_15_0 = inst & 0x0000ffff;
		inst_25_0 = inst & 0x03ffffff;


		// register read


		Register_Read(if_id[1], if_id[2]);

		// create control signal

		Control_Signal(if_id[0]);

		////////////////////////////////
		/*
		control_reg[3] = mem/wb?? control??;
		control_reg[2] = ex/mem?? control??;
		control_reg[1] = id/ex?? control??;
		*/
		///////////////////////////////
		control_reg[3] = control_reg[2];
		control_reg[2] = control_reg[1];
		control_reg[1] = control_reg[0];
		control_reg[0] = control;

		// create ALU control signal


		unsigned char ALU_control_signal = ALU_Control_Signal(control_reg[1].ALUOp);
		unsigned int ALU_input = Mux(control_reg[1].ALUSrc, id_ex[1], id_ex[2]);

		// ALU

		ALU_func(ALU_control_signal, id_ex[0], ALU_input);



		// memory access


		mem_value = Memory_Access(control_reg[2].MemWrite, control_reg[2].MemRead, ex_mem[1], ex_mem[2]);


		// register write
		Register_Write(control_reg[3].RegWrite, mem_wb[2], Mux(control_reg[3].MemtoReg, mem_wb[1], mem_wb[0]));


		total_cycle++;

		// result
		/********************************/
		printf("PC : %d \n", pc);
		printf("Total cycle : %d \n", total_cycle);

		if (total_cycle <= 1) {
			pc = pc + 4;
		}
		else {
			pc = Mux((control_reg[2].Branch && ex_mem[0]), pc_add_4, ex_mem[4]);
			pc = Mux(control_reg[2].Jump, pc, (Shift_Left_2(ex_mem[6]) | ((ex_mem[5] >> 28) << 28)));
		}

		printf("=== ID/EX === \n");
		printf("WB - RegWrite: %d, MemtoReg: %d\n", control_reg[0].RegWrite, control_reg[0].MemtoReg);
		printf("M  - Branch  : %d, MemRead : %d, MemWrite: %d \n", control_reg[0].Branch, control_reg[0].MemRead, control_reg[0].MemWrite);
		printf("EX - RegDst  : %d, ALUOp   : %d, ALUSrc  : %d \n", control_reg[0].RegDst, control_reg[0].ALUOp, control_reg[0].ALUSrc);
		printf("=== EX/MEM === \n");
		printf("WB - RegWrite: %d, MemtoReg: %d\n", control_reg[1].RegWrite, control_reg[1].MemtoReg);
		printf("M  - Branch  : %d, MemRead : %d, MemWrite: %d \n", control_reg[1].Branch, control_reg[1].MemRead, control_reg[1].MemWrite);
		printf("=== MEM/WB === \n");
		printf("WB - RegWrite: %d, MemtoReg: %d \n", control_reg[2].RegWrite, control_reg[2].MemtoReg);

		print_reg_mem();
		/********************************/

		system("pause");
	}

	printf("\n ***** Processor END !!! ***** \n");



	return 0;
}

unsigned int Inst_Fetch(unsigned int read_addr)
{
	return mem[read_addr];
}

void Register_Read(unsigned int read_reg_1, unsigned int read_reg_2)
{
	reg_read.Read_data_1 = reg[read_reg_1]; reg_read.Read_data_2 = reg[read_reg_2];
}


void Control_Signal(unsigned int opcode)
{   //op ?????? ???? control ?? ????
	unsigned int r_format = (opcode == 0), lw = (opcode == 35), sw = (opcode == 43), beq = (opcode == 4);
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
	unsigned char ALUop1 = signal << 7;
	ALUop1 = ALUop1 >> 7;
	unsigned char ALUop2 = signal << 6;
	ALUop2 = ALUop2 >> 7;
	if (ALUop1 == 0 && ALUop2 == 0) {  // lw?? ????
		return 2;
	}
	if (ALUop1 == 1) { // beq?? ????
		return 6;
	}
	if (ALUop2 == 1) { // R-type?? ????
		return 2;
	}
	return 0;
}

void ALU_func(unsigned char ALU_control, unsigned int a, unsigned int b) //////
{
	if (ALU_control == 2) {
		alu.ALU_result = a + b;  // lw ???? r_type?? ???? ??????
	}
	if (ALU_control == 6) {  // beq?? ???? ????
		alu.ALU_result = a - b;
	}
}

unsigned int Memory_Access(unsigned char MemWrite, unsigned char MemRead, unsigned int addr, unsigned int write_data) //////////
{
	if (MemRead == 1) {  // lw?? ????
		return mem[addr];
	}
	if (MemWrite == 1) {  // sw?? ????
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
	if (signal == 0) {   // ???????? 0???? 
		return a_0; // ?????? ???? ????
	}
	else { // 1????
		return b_1; // ?????? ???? ????
	}
}

unsigned int Add(unsigned int a, unsigned int b)
{
	return a + b;  // ?? ???? ???? ????
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

