#include <stdio.h>
#include "shell.h"
#include <string.h>

char instruction[400]; //存储输入的指令
int curr_pos = 0; //当前指令的位置
char conversion[100]; //存储转换后的指令


void uintToStr(uint32_t u){
    //作用：将内存中的一个无符号整数转换为其二进制字符串表示。
    //首先将整数转换为十六进制字符串，然后将每个十六进制字符转换为其对应的4位二进制字符串
    char hexStr[50];
    char temp[50];
    char* hexToBin[16] = {
        "0000", "0001", "0010", "0011",
        "0100", "0101", "0110", "0111",
        "1000", "1001", "1010", "1011",
        "1100", "1101", "1110", "1111"
    };
    sprintf(hexStr, "%x", mem_read_32(NEXT_STATE.PC));
    int size = strlen(hexStr);
    // if (strlen(hexStr) != 8) {
    //     fprintf(stderr, "Invalid hex string length. Expected 8, got %zu\n", strlen(hexStr));
    // }
    /*
    犯了一个灰常愚蠢的错误，居然把hexStr和temp搞混了，debug了好久，生气气！！！
    前面都木有问题，就是syscall其实本来只有1位，如果不补全会有溢出的错误！！！生气！
    */
    //将十六进制字符串左填充为8个字符
    for(int i=0;i<8-size;i++){
        temp[i] = '0';
    }

    for(int i=8-size;i<8;i++){
        temp[i] = hexStr[i-8+size];
    }
    //temp是处理后的十六进制字符串
    //将一个十六进制字符转换为其对应的4位二进制字符串
    for(int i = 0; i < 8; i++) 
    {
        int hexDigit;
        if(temp[i] >= '0' && temp[i] <= '9') {
            hexDigit = temp[i] - '0';
        } 
        else if(temp[i] >= 'a' && temp[i] <= 'f') {
            hexDigit = temp[i] - 'a' + 10;
        }
        else {
            fprintf(stderr, "Invalid hex character: %c\n", hexStr[i]);
        }
        strncpy(&conversion[i * 4], hexToBin[hexDigit],4);
    }
}

int signExtend(int imm){
    // 如果imm的最高位（第16位，0-indexed）是1，那么进行符号扩展，否则直接返回
    return (imm & 0x8000) ? (imm | 0xFFFF0000) : imm;
}

int convertBin(int length){
    //将一个二进制字符串转换为其整数表示
    int result = 0;
    for(int i = 0; i < length; i++) {
        if(instruction[curr_pos] == '1') //如果当前位置为1
            result += 1 << (length - i - 1); //左移,相当于乘以2
        curr_pos++;
    }
    return result;
}

int convertChar(char* str, int length,int start){
    //将一个二进制字符串转换为其整数表示
    int result = 0;
    for(int i = start; str[i]!='\0'; i++) 
    {
        if(str[i] == '1') //如果当前位置为1
            result += 1 << (length - i - 1); //左移,相当于乘以2
    }
    return result;
}

int ComplementToBinary(int length){
    int result = 0;
    if(instruction[curr_pos] == '1'){
        result = convertBin(length) | 0xffff0000;
        return result;
    }
    result += convertBin(length);
    return result;
}

int ComplementToBinaryChar(char* str,int length){
    int result = 0;
    if(str[0] == '1'){
        result = convertChar(str,length - 1,0) | 0xffff0000;
        return result;
    }
    result += convertChar(str,length - 1,0);
    return result; 
}



void process_instruction()
{
    /* execute one instruction here. You should use CURRENT_STATE and modify
     * values in NEXT_STATE. You can call mem_read_32() and mem_write_32() to
     * access memory. */
    curr_pos = 0; //当前指令的位置
    uintToStr(mem_read_32(CURRENT_STATE.PC)); //将当前指令转换为其二进制字符串表示
    strncpy(instruction, conversion, 33);//将转换后的指令存入instruction中
    printf("%s\n", instruction);
    if(convertBin(32) == 12){
        if(NEXT_STATE.REGS[2] == 10){
            RUN_BIT = 0;
            return;
        }
    }

    curr_pos = 0; //当前指令的位置
    int InstructionType = convertBin(6); //根据指令前六位指令类型
    if(InstructionType == 0)
    {
        //R型指令
        int rs = convertBin(5); //rs
        int rt = convertBin(5); //rt
        int rd = convertBin(5); //rd
        int shamt = convertBin(5); //shamt
        int funct = convertBin(6); //funct
        switch (funct)
        {
        case 32:
            //add
            printf("instruction ADD $%d $%d $%d executed\n", rd, rs, rt);
            NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] + CURRENT_STATE.REGS[rt];
            break;
        case 33:
            //addu
            printf("instruction ADDU $%d $%d $%d executed\n", rd, rs, rt);
            NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] + CURRENT_STATE.REGS[rt];
            break;
        case 34:
            //sub
            printf("instruction SUB $%d $%d $%d executed\n", rd, rs, rt);
            NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] - CURRENT_STATE.REGS[rt];
            break;
        case 35:
            //subu
            printf("instruction SUBU $%d $%d $%d executed\n", rd, rs, rt);
            NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] - CURRENT_STATE.REGS[rt];
            break;
        case 36:
            //and
            printf("instruction AND $%d $%d $%d executed\n", rd, rs, rt);
            NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] & CURRENT_STATE.REGS[rt];
            break;
        case 37:
            //or
            printf("instruction OR $%d $%d $%d executed\n", rd, rs, rt);
            NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] | CURRENT_STATE.REGS[rt];
            break;
        case 38:
            //xor
            printf("instruction XOR $%d $%d $%d executed\n", rd, rs, rt);
            NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] ^ CURRENT_STATE.REGS[rt];
            break;
        case 39:
            //nor
            printf("instruction NOR $%d $%d $%d executed\n", rd, rs, rt);
            NEXT_STATE.REGS[rd] = ~(CURRENT_STATE.REGS[rs] | CURRENT_STATE.REGS[rt]);
            break;
        case 42:
            //slt   
            printf("instruction SLT $%d $%d $%d executed\n", rd, rs, rt);
            if ((int32_t)CURRENT_STATE.REGS[rs] < (int32_t)CURRENT_STATE.REGS[rt]) {
                NEXT_STATE.REGS[rd] = 1;
            } 
            else {
                NEXT_STATE.REGS[rd] = 0;
            }
            break;
        case 43:
            //sltu
            printf("instruction SLTU $%d $%d $%d executed\n", rd, rs, rt);
            if ((uint32_t)CURRENT_STATE.REGS[rs] < (uint32_t)CURRENT_STATE.REGS[rt]) {
                NEXT_STATE.REGS[rd] = 1;
            } 
            else {
                NEXT_STATE.REGS[rd] = 0;
            }
        case 0:
            //sll
            printf("instruction SLL $%d $%d %d executed\n", rd, rt, shamt);
            NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rt] << shamt;
            break;
        case 2:
            //srl
            printf("instruction SRL $%d $%d %d executed\n", rd, rt, shamt);
            NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rt] >> shamt;
            break;
        case 3:
            // sra
            printf("instruction SRA $%d $%d %d executed\n", rd, rt, shamt);
            uint32_t result = CURRENT_STATE.REGS[rt] >> shamt;
            // 检查原寄存器中数据最高位是否为1
            if (CURRENT_STATE.REGS[rt] & 0x80000000) {
                // 构建一个掩码，将result左边的shamt个位置1，而后面的位保持不变
                uint32_t mask = (1U << (32 - shamt)) - 1;
                result = result | (~mask);
            }
            NEXT_STATE.REGS[rd] = result;
            break;
        case 4:
            //sllv
            printf("instruction SLLV $%d $%d $%d executed\n", rd, rt, rs);
            NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rt] << CURRENT_STATE.REGS[rs];
            break;
        case 6:
            //srlv
            printf("instruction SRLV $%d $%d $%d executed\n", rd, rt, rs);
            NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rt] >> CURRENT_STATE.REGS[rs];
            break;
        case 7:
            //srav
            printf("instruction SRAV $%d $%d $%d executed\n", rd, rt, rs);
            uint32_t result1 = CURRENT_STATE.REGS[rt] >> CURRENT_STATE.REGS[rs];
            // 检查原寄存器中数据最高位是否为1
            if (CURRENT_STATE.REGS[rt] & 0x80000000) {
                // 构建一个掩码，将result左边的shamt个位置1，而后面的位保持不变
                uint32_t mask = (1U << (32 - CURRENT_STATE.REGS[rs])) - 1;
                result1 = result1 | (~mask);
            }
            NEXT_STATE.REGS[rd] = result1;
            break;
        case 8:
            //jr
            printf("instruction JR $%d executed\n", rs);
            NEXT_STATE.PC = CURRENT_STATE.REGS[rs];
            break;
        case 9:
            //jalr
            printf("instruction JALR $%d $%d executed\n", rd, rs);
            NEXT_STATE.REGS[rd] = CURRENT_STATE.PC;
            NEXT_STATE.PC = CURRENT_STATE.REGS[rs] - 4;
            break;
        case 16:
            //mfhi
            printf("instruction MFHI $%d executed\n", rd);
            NEXT_STATE.REGS[rd] = CURRENT_STATE.HI;
            break;
        case 17:
            //mthi
            printf("instruction MTHI $%d executed\n", rs);
            NEXT_STATE.HI = CURRENT_STATE.REGS[rs];
            break;
        case 18:
            //mflo
            printf("instruction MFLO $%d executed\n", rd);
            NEXT_STATE.REGS[rd] = CURRENT_STATE.LO;
            break;
        case 19:
            //mtlo
            printf("instruction MTLO $%d executed\n", rs);
            NEXT_STATE.LO = CURRENT_STATE.REGS[rs];
            break;
        case 24:
            //mult
            printf("instruction MULT $%d $%d executed\n", rs, rt);
            int64_t result2 = (int64_t)CURRENT_STATE.REGS[rs] * (int64_t)CURRENT_STATE.REGS[rt];
            CURRENT_STATE.HI = result2 >> 32;
            CURRENT_STATE.LO = result2 & 0xffffffff;
            printf("HI: 0x%08X\n", CURRENT_STATE.HI);
            printf("LO: 0x%08X\n", CURRENT_STATE.LO);
            break;
        case 25:
            //multu
            printf("instruction MULTU $%d $%d executed\n", rs, rt);
            uint64_t result3 = (uint64_t)CURRENT_STATE.REGS[rs] * (uint64_t)CURRENT_STATE.REGS[rt];
            CURRENT_STATE.HI = result3 >> 32;
            CURRENT_STATE.LO = result3 & 0xffffffff;
            printf("HI: 0x%08X\n", CURRENT_STATE.HI);
            printf("LO: 0x%08X\n", CURRENT_STATE.LO);
            break;
        case 26:
            //div
            printf("instruction DIV $%d $%d executed\n", rs, rt);
            if(CURRENT_STATE.REGS[rt] == 0){
                printf("divided by zero!\n");
                break;
            }
            CURRENT_STATE.LO = CURRENT_STATE.REGS[rs] / CURRENT_STATE.REGS[rt];
            CURRENT_STATE.HI = CURRENT_STATE.REGS[rs] % CURRENT_STATE.REGS[rt];
            printf("HI: 0x%08X\n", CURRENT_STATE.HI);
            printf("LO: 0x%08X\n", CURRENT_STATE.LO);
            break;
        case 27:
            //divu
            printf("instruction DIVU $%d $%d executed\n", rs, rt);
            if(CURRENT_STATE.REGS[rt] == 0){
                printf("divided by zero!\n");
                break;
            }
            CURRENT_STATE.LO = CURRENT_STATE.REGS[rs] / CURRENT_STATE.REGS[rt];
            CURRENT_STATE.HI = CURRENT_STATE.REGS[rs] % CURRENT_STATE.REGS[rt];
            printf("HI: 0x%08X\n", CURRENT_STATE.HI);
            printf("LO: 0x%08X\n", CURRENT_STATE.LO);
            break;
        
        default:
            break;
        }
    }
    else if(InstructionType == 2 || InstructionType == 3)
    {
        //J型指令
        int address = convertBin(26) * 4; //地址，由于MIPS是基于字的，地址为字对齐，因此*4
        switch (InstructionType)
        {
        case 2:
            //j
            printf("instruction J %d executed\n", address);
            NEXT_STATE.PC =(uint32_t) address - 4;
            break;
        
        case 3:
            //jal
            printf("instruction JAL %d executed\n", address);
            NEXT_STATE.REGS[31] = CURRENT_STATE.PC;
            NEXT_STATE.PC = address - 4;
            break;
        
        default:
            break;
        }
    }
    else
    {
        //I型指令
        int rs = convertBin(5); //rs
        int rt = convertBin(5); //rt
        int immediate; //立即数
        //注意在mips指令中I型指令的立即数是补码形式，因此应当再求一次补码～
        char subbuff[50];
        switch (InstructionType)
        {
        case 1:
            //bltz
            printf("%d",NEXT_STATE.REGS[rs]);
            if(rt == 0){
                //bltz
                immediate = ComplementToBinary(16);
                if(NEXT_STATE.REGS[rs] & 0x80000000){
                    NEXT_STATE.PC += immediate * 4 ;
                }
                printf("instruction BLTZ $%d %d executed\n", rs, immediate);
            }
            else if(rt == 1){
                //bgez
                immediate = ComplementToBinary(16);
                if(!(NEXT_STATE.REGS[rs] & 0x80000000) ){
                    NEXT_STATE.PC += immediate * 4;
                }
                printf("instruction BGEZ $%d %d executed\n", rs, immediate);
            }
            else if(rt == 16){
                //bltzal
                immediate = ComplementToBinary(16);
                printf("%s %u","rs val:",NEXT_STATE.REGS[rs]);
                if(NEXT_STATE.REGS[rs] & 0x80000000){
                    
                    NEXT_STATE.REGS[31] = CURRENT_STATE.PC;
                    NEXT_STATE.PC += immediate<<2 ;
                }
                printf("instruction BLTZAL $%d %d executed\n", rs, immediate);
            }
            else if(rt == 17){
                //bgezal
                immediate = ComplementToBinary(16);
                printf("%s %u","rs val:",NEXT_STATE.REGS[rs]);
                if( !(NEXT_STATE.REGS[rs] & 0x80000000) ){
                    NEXT_STATE.REGS[31] = CURRENT_STATE.PC;
                    NEXT_STATE.PC += immediate<<2 ;
                }
                printf("instruction BGEZAL $%d %d executed\n", rs, immediate);
            }
            else{
                printf("error!\n");
            }
            break;
        case 4:
            //beq
            immediate = ComplementToBinary(16);
            if(NEXT_STATE.REGS[rs] == CURRENT_STATE.REGS[rt]){
                NEXT_STATE.PC += immediate * 4;
            }
            printf("instruction BEQ $%d $%d %d executed\n", rs, rt, immediate);
            break;
        case 5:
            //bne
            immediate = ComplementToBinary(16);
            if(NEXT_STATE.REGS[rs] != CURRENT_STATE.REGS[rt]){
                NEXT_STATE.PC += immediate * 4 ;
            }
            printf("instruction BNE $%d $%d %d executed\n", rs, rt, immediate);
            break;
        case 6:
            //blez
            immediate = ComplementToBinary(16);
            if(NEXT_STATE.REGS[rs] <= 0){
                NEXT_STATE.PC += immediate * 4 ;
            }
            printf("instruction BLEZ $%d %d executed\n", rs, immediate);
            break;
        case 7:
            //bgtz
            immediate = ComplementToBinary(16);
            if(NEXT_STATE.REGS[rs] > 0){
                NEXT_STATE.PC += immediate * 4;
            }
            printf("instruction BGTZ $%d %d executed\n", rs, immediate);
            break;
        case 8:
            //addi
            immediate = ComplementToBinary(16);
            NEXT_STATE.REGS[rt] = CURRENT_STATE.REGS[rs] + immediate;
            printf("instruction ADDI $%d $%d %d executed\n", rt, rs, immediate);
            break;
        case 9:
            //addiu
            immediate = ComplementToBinary(16);
            NEXT_STATE.REGS[rt] = CURRENT_STATE.REGS[rs] + immediate;
            printf("instruction ADDIU $%d $%d %d executed\n", rt, rs, immediate);
            break;
        case 10:
            //slti
            immediate = ComplementToBinary(16);
            NEXT_STATE.REGS[rt] = CURRENT_STATE.REGS[rs] < immediate? 1 : 0;
            printf("instruction SLTI $%d $%d %d executed\n", rs, rt, immediate);//这里的rs和rt是反的,为什么呢？
            break;
        case 11:
            //sltiu
            immediate = convertBin(16);
            NEXT_STATE.REGS[rt] = CURRENT_STATE.REGS[rs] < immediate ? 1 : 0;
            printf("instruction SLTIU $%d $%d %d executed\n", rs, rt, immediate);
        case 12:
            //andi
            immediate = convertBin(16);
            NEXT_STATE.REGS[rt] = CURRENT_STATE.REGS[rs] & immediate;
            printf("instruction ANDI $%d $%d %d executed\n", rt, rs, immediate);
            break;
        case 13:
            //ori
            immediate = convertBin(16);
            NEXT_STATE.REGS[rt] = CURRENT_STATE.REGS[rs] | immediate;
            printf("instruction ORI $%d $%d %d executed\n", rt, rs, immediate);
            break;
        case 14:
            //xori
            immediate = convertBin(16);
            NEXT_STATE.REGS[rt] = CURRENT_STATE.REGS[rs] ^ immediate;
            printf("instruction XORI $%d $%d %d executed\n", rt, rs, immediate);
            break;
        case 15:
            //lui
            immediate = ComplementToBinary(16);
            NEXT_STATE.REGS[rt] = immediate << 16;
            printf("instruction LUI $%d %d executed\n", rt, immediate);
            break;
        case 32:
            //lb
            immediate = ComplementToBinary(16);
            uintToStr(mem_read_32(CURRENT_STATE.REGS[rs] + ComplementToBinary(16)));
            strncpy(subbuff, conversion, 8); 
            NEXT_STATE.REGS[rt] = ComplementToBinaryChar(subbuff,8); //将转换后的指令存入instruction中
            printf("instruction LB $%d (%d) $%d executed\n", rt, immediate,rs); 
            break;
        case 33:
            //lh
            immediate = ComplementToBinary(16);
            uintToStr(mem_read_32(CURRENT_STATE.REGS[rs] + immediate));
            strncpy(subbuff, conversion, 16);
            NEXT_STATE.REGS[rt] = ComplementToBinaryChar(subbuff,16); //将转换后的指令存入instruction中
            printf("instruction LH $%d (%d) $%d executed\n", rt, immediate,rs); 
            break;
        case 35:
            //lw
            immediate = ComplementToBinary(16);
            NEXT_STATE.REGS[rt] = mem_read_32(CURRENT_STATE.REGS[rs] + immediate);
            printf("instruction LW $%d (%d) $%d executed\n", rt, immediate,rs);
            break;
        case 36:
            //lbu
            immediate = ComplementToBinary(16);
            uintToStr(mem_read_32(CURRENT_STATE.REGS[rs] + immediate));
            strncpy(subbuff, conversion, 8);
            NEXT_STATE.REGS[rt] = ComplementToBinaryChar(subbuff,8); //将转换后的指令存入instruction中
            printf("instruction LBU $%d (%d) $%d executed\n", rt, immediate,rs); 
            break;
        case 37:
            //lhu
            immediate = ComplementToBinary(16);
            uintToStr(mem_read_32(CURRENT_STATE.REGS[rs] + immediate));
            strncpy(subbuff, conversion, 16);
            NEXT_STATE.REGS[rt] = ComplementToBinaryChar(subbuff,16); //将转换后的指令存入instruction中
            printf("instruction LHU $%d (%d) $%d executed\n", rt, immediate,rs);
            break;
        case 40:
            //sb
            immediate = ComplementToBinary(16);
            uintToStr(NEXT_STATE.REGS[rt]);
            strncpy(subbuff,conversion,8);
            mem_write_32(ComplementToBinary(16)+CURRENT_STATE.REGS[rs],ComplementToBinaryChar(subbuff,8));
            printf("instruction SB $%d (%d) $%d executed\n", rt, immediate,rs);
            break;
        case 41:
            //sh
            immediate = ComplementToBinary(16);
            uintToStr(NEXT_STATE.REGS[rt]);
            strncpy(subbuff, conversion, 16);
            mem_write_32(CURRENT_STATE.REGS[rs] + immediate,ComplementToBinaryChar(subbuff,16));
            printf("instruction SH $%d (%d) $%d executed\n", rt, immediate,rs);
            break;
        case 43:
            //sw
            immediate = ComplementToBinary(16);
            mem_write_32(CURRENT_STATE.REGS[rs] + immediate,NEXT_STATE.REGS[rt]);
            printf("instruction SW $%d (%d) $%d executed\n", rt, immediate,rs);
            break;
        }
    }
    NEXT_STATE.PC += 4;
    printf("\n");
}
