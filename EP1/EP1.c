#include "driverEP1.h"
#include <stdio.h>

#define GET_BITS(value, start, count) (((value) >> (start)) & ((1 << (count)) - 1))

void operacaoArit(unsigned short int ri, unsigned short int* a, unsigned short int* b, unsigned short int* c,  unsigned short int* d,  unsigned short int* r,  unsigned short int* psw) {
    unsigned short int operacao = GET_BITS(ri, 9, 3);
    unsigned short int res = GET_BITS(ri, 6, 3);
    unsigned short int Op1 = GET_BITS(ri, 3, 3);
    unsigned short int Op2 = GET_BITS(ri, 0, 3);

    unsigned short int valorOp1;
    unsigned short int valorOp2;

    switch (Op1) {
        case 0: valorOp1 = *a; break;
        case 1: valorOp1 = *b; break;
        case 2: valorOp1 = *c; break;
        case 3: valorOp1 = *d; break;
        case 6: valorOp1 = *r; break;
        case 7: valorOp1 = *psw; break;
        default: break;
    }

    if (Op2 >= 4) {
        switch(Op2) {
            case 4: valorOp2 = *a; break;
            case 5: valorOp2 = *b; break;
            case 6: valorOp2 = *c; break;
            case 7: valorOp2 = *d; break;
        }
    } else valorOp2 = 0;

    *psw &= ~((1 << 15) | (1 << 14) | (1 << 13) | (1 << 12) | (1 << 11));

    unsigned short int resultado = 0;
    int provisorioRes = 0;
    switch (operacao) {
        case 0: resultado = 0x0000; break;
        case 1: resultado = 0xFFFF; break;
        case 2: resultado = ~valorOp1; break;
        case 3: resultado = valorOp1 & valorOp2; break;
        case 4: resultado = valorOp1 | valorOp2; break;
        case 5: resultado = valorOp1 ^ valorOp2; break;
        case 6:
            if (valorOp1 + valorOp2 >= 65536) {
                provisorioRes = valorOp1 + valorOp2 - 65536;
                resultado = (unsigned short int) provisorioRes;
                *psw |= (1<<15);
            } else resultado = valorOp1 + valorOp2;
            break;
        case 7:
            if (valorOp1 - valorOp2 < 0) {
                provisorioRes = valorOp1 - valorOp2 + 65536;
                resultado = (unsigned short int) provisorioRes;
                *psw |= (1<<14);
            } else resultado = valorOp1 - valorOp2;
            break;
    }

    int testa = valorOp1 - valorOp2;
    if (testa < 0) {
        *psw |= (1 << 13);
    } else if (testa > 0) {
        *psw |= (1 << 11);
    } else {
        *psw |= (1 << 12);
    }

    unsigned short int* regResultado = NULL;
    switch(res) {
        case 0: regResultado = a; break;
        case 1: regResultado = b; break;
        case 2: regResultado = c; break;
        case 3: regResultado = d; break;
        case 6: regResultado = r; break;
        default: break;
    }

    if (regResultado != NULL) {
        *regResultado = resultado;
    }

}


int processa (short int* M, int memSize) {
    unsigned short int ri, pc, a, b, c, d, r;
    a = 0; b = 0; c = 0; d = 0; r = 0;

    unsigned short int psw = 0x0088;
    pc = 0;

    do {
        ri = M[pc];

        unsigned short int opcode = (ri & 0xF000) >> 12;
        unsigned short int endereco = ri & 0x0FFF;
        unsigned short int temp;

        switch(opcode) {
            case 0:
            break;
           
            case 1:
            a = (unsigned short int) M[endereco];
            break;

            case 2:
            M[endereco] = (short int)((unsigned short int)a);
            break;

            case 3:
            r = pc+1;
            pc = endereco-1;
            break;

            case 4:
            if (a != 0) {
                r = pc+1;
                pc = endereco-1;
            } break;

            case 5:
            temp = r;
            r = pc+1;
            pc = temp-1;
            break;

            case 6:
            operacaoArit(ri, &a, &b, &c, &d, &r, &psw);
            break;
        }
    pc++;
    if (pc >= memSize) pc = 0;
    } while ((ri&0xF000)!=0xF000);

    return 0;
}
