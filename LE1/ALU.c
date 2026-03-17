/* Authors: Ellery Von E. Salas
            Nathaniel Padas
            Christian Andrie Asne
            Gabriel G. Potazo
   Date: March 19, 2026
*/

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

uint16_t ACC;
bool zero_flag = 0;
bool overflow_flag = 0;
bool sign_flag = 0;
bool carry_flag = 0;
unsigned char op1;
unsigned char op2;
unsigned char cont_sig;

unsigned char twosComp(unsigned char data){

    return ~data + 1;
}

//void printBin(int data, unsigned char data_width){
//    return 0;
//}



int ALU(unsigned char operand1, unsigned char operand2, unsigned char control_signals){
    uint16_t result = 0;

    switch(control_signals){
        case 1: //addition
            result = operand1 + operand2;
            ACC = result;
            return result & 0xFF;
            break;
        case 2: //subtraction
            result = operand1 + twosComp(operand2);
            ACC = result;
            return result & 0xFF;
            break;
        case 3: //multiplication booth's algorithm
            uint8_t multiplier = operand1;
            uint8_t multiplicand = operand2;
            uint8_t count = 8;

            while (count--){
                if (multiplier & 1){
                    result += multiplicand;
                }
                multiplicand <<= 1;
                multiplier >>= 1;
            }
            ACC = result;
            return (int)result;
            break;
        case 4: //AND
            result = operand1 & operand2;
            return result;
            break;
        case 5: //OR
            result = operand1 | operand2;
            return result;
            break;
        case 6: //NOT
            result = ~operand1;
            operand2 = 0;
            return result;
            break;
        case 7: //XOR
            result = operand1 ^ operand2;
            return result;
            break;
        case 8: //Shift Right (logical)
            return operand1 >> operand2;
            break;
        case 9: //Shift Left (logical)
            result = operand1 << operand2;
            return result;
            break;
        default:
            return 0;
    }
}

unsigned char setFlags(unsigned int ACC){
    if(ACC == 0){
        zero_flag = 1;
    }
    if((ACC & 0xFF)== 00010000){
        carry_flag = 1;
        overflow_flag = 1;
    }
    //sign flag
}
int main(){
    op1 = 6;
    op2 = 0;
    cont_sig = 3;
    printf("ALU System \n\n");
    printf("Operand 1 = %d\n",op1);
    printf("Operand 2 = %d\n",op2);
    printf("Control Signal = %d\n\n",cont_sig);

    unsigned char result = ALU(op1,op2,cont_sig);
    setFlags(ACC);

    printf("Result = %u\n\n",result);
    printf("Zero Flag = %u\n",zero_flag);
    printf("Carry Flag = %u\n",carry_flag);
    printf("Overflow Flag = %u\n",overflow_flag);
    printf("Sign Flag = %u\n\n\n",sign_flag);

return 0;
}
