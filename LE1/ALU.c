/*
 * ALU.c
 * Authors: Ellery Von E. Salas, Nathaniel Padas, Christian Andrie Asne, Gabriel G. Potazo
 * Date: March 19, 2026
 *
 * 8-bit ALU simulator with arithmetic (add, sub, mul via Booth's algorithm),
 * logic (AND, OR, NOT, XOR, shift left/right), and flag handling (C, Z, OV, S).
 */

#include <stdio.h>
#include <stdint.h>

// Global flags (unsigned char as per specification)
unsigned char carry_flag = 0;
unsigned char zero_flag = 0;
unsigned char overflow_flag = 0;
unsigned char sign_flag = 0;

// 16-bit accumulator to hold intermediate results (for carry/overflow detection)
uint16_t ACC = 0;

// Function prototypes
unsigned char twosComp(unsigned char data);
void printBin(int data, unsigned char data_width);
void setFlags(uint16_t acc);

// ----------------------------------------------------------------------
// Compute two's complement of an 8-bit number
unsigned char twosComp(unsigned char data) {
    return ~data + 1;
}

// ----------------------------------------------------------------------
// Print an integer in binary with exactly 'data_width' digits (leading zeros)
void printBin(int data, unsigned char data_width) {
    for (int i = data_width - 1; i >= 0; i--) {
        printf("%d", (data >> i) & 1);
    }
}

// ----------------------------------------------------------------------
// Set zero and sign flags based on the lower 8 bits of ACC
void setFlags(uint16_t acc) {
    uint8_t res = acc & 0xFF;
    zero_flag = (res == 0) ? 1 : 0;
    sign_flag = (res & 0x80) ? 1 : 0;
}

// ----------------------------------------------------------------------
// ALU: performs operation specified by control_signals on operand1 and operand2
// Returns the 8‑bit result (as int) and sets global flags and ACC.
int ALU(unsigned char operand1, unsigned char operand2, unsigned char control_signals) {
    uint16_t result16;
    uint8_t result8;
    uint8_t temp;

    printf("\n========================================\n");
    switch (control_signals) {
        case 0x01:  // Addition
            printf("ADDITION:\n");
            printf("  operand1: "); printBin(operand1, 8); printf(" (%d)\n", (int8_t)operand1);
            printf("  operand2: "); printBin(operand2, 8); printf(" (%d)\n", (int8_t)operand2);

            result16 = operand1 + operand2;
            ACC = result16;
            result8 = result16 & 0xFF;

            // Set flags
            carry_flag = (result16 >> 8) & 1;
            zero_flag = (result8 == 0) ? 1 : 0;
            sign_flag = (result8 & 0x80) ? 1 : 0;
            // Overflow for signed addition
            if ( ((operand1 & 0x80) == (operand2 & 0x80)) &&
                 ((result8 & 0x80) != (operand1 & 0x80)) ) {
                overflow_flag = 1;
            } else {
                overflow_flag = 0;
            }

            printf("  Sum (16-bit): "); printBin(result16, 16); printf(" (carry=%d)\n", carry_flag);
            printf("  8-bit result: "); printBin(result8, 8); printf(" (%d)\n", (int8_t)result8);
            break;

        case 0x02:  // Subtraction (via two's complement)
            printf("SUBTRACTION:\n");
            printf("  operand1: "); printBin(operand1, 8); printf(" (%d)\n", (int8_t)operand1);
            printf("  operand2: "); printBin(operand2, 8); printf(" (%d)\n", (int8_t)operand2);

            temp = twosComp(operand2);
            printf("  Two's complement of operand2: "); printBin(temp, 8); printf(" (%d)\n", (int8_t)temp);

            result16 = operand1 + temp;
            ACC = result16;
            result8 = result16 & 0xFF;

            // Set flags
            carry_flag = (result16 >> 8) & 1;
            zero_flag = (result8 == 0) ? 1 : 0;
            sign_flag = (result8 & 0x80) ? 1 : 0;
            // Overflow for subtraction (same as addition of operand1 and -operand2)
            if ( ((operand1 & 0x80) == (temp & 0x80)) &&
                 ((result8 & 0x80) != (operand1 & 0x80)) ) {
                overflow_flag = 1;
            } else {
                overflow_flag = 0;
            }

            printf("  Difference (16-bit): "); printBin(result16, 16); printf(" (carry=%d)\n", carry_flag);
            printf("  8-bit result: "); printBin(result8, 8); printf(" (%d)\n", (int8_t)result8);
            break;

        case 0x03:  // Multiplication (Booth's algorithm)
            printf("MULTIPLICATION (Booth's algorithm):\n");
            int8_t M = (int8_t)operand1;   // multiplicand (signed)
            int8_t Q = (int8_t)operand2;   // multiplier (signed)
            int16_t A = 0;
            int8_t Q_1 = 0;
            int count = 8;

            printf("  Initial: A="); printBin((uint16_t)A, 16);
            printf(" Q="); printBin((uint8_t)Q, 8); printf(" Q-1=%d\n", Q_1);

            while (count--) {
                // Booth decision
                if ( ((Q & 1) == 0) && (Q_1 == 1) ) {
                    A = A + M;
                    printf("    Add M  -> A="); printBin((uint16_t)A, 16); printf("\n");
                } else if ( ((Q & 1) == 1) && (Q_1 == 0) ) {
                    A = A - M;
                    printf("    Sub M  -> A="); printBin((uint16_t)A, 16); printf("\n");
                }

                // Arithmetic right shift of (A, Q, Q_1)
                int8_t old_Q0 = Q & 1;
                // Save LSB of A before shifting
                uint16_t old_A = A;
                // Shift A right arithmetically (preserve sign)
                uint16_t new_A = (old_A >> 1) & 0x7FFF;  // clear sign bit initially
                if (old_A & 0x8000) new_A |= 0x8000;     // set sign bit if original was negative
                // Shift Q right, bring in old LSB of A as new MSB of Q
                uint8_t new_Q = ((uint8_t)Q >> 1) | ((old_A & 1) << 7);
                Q_1 = old_Q0;
                Q = (int8_t)new_Q;
                A = (int16_t)new_A;

                printf("    Shift  -> A="); printBin((uint16_t)A, 16);
                printf(" Q="); printBin((uint8_t)Q, 8); printf(" Q-1=%d\n", Q_1);
            }

            // Final product is {A, Q} as a 16-bit signed integer
            uint16_t product = ((uint16_t)A << 8) | (uint8_t)Q;
            ACC = product;
            result8 = product & 0xFF;

            // Set flags
            zero_flag = (result8 == 0) ? 1 : 0;
            sign_flag = (result8 & 0x80) ? 1 : 0;
            carry_flag = 0;  // multiplication does not affect carry
            // Overflow: check if 16‑bit product fits in signed 8‑bit
            int16_t prod_signed = (int16_t)product;
            int8_t low_signed = (int8_t)result8;
            overflow_flag = ((int16_t)low_signed != prod_signed) ? 1 : 0;

            printf("  Product (16-bit): "); printBin(product, 16); printf("\n");
            printf("  8-bit result: "); printBin(result8, 8); printf(" (%d)\n", (int8_t)result8);
            break;

        case 0x04:  // AND
            printf("AND:\n");
            result8 = operand1 & operand2;
            ACC = result8;
            setFlags(ACC);
            carry_flag = 0;
            overflow_flag = 0;
            printf("  operand1: "); printBin(operand1, 8); printf("\n");
            printf("  operand2: "); printBin(operand2, 8); printf("\n");
            printf("  result: "); printBin(result8, 8); printf("\n");
            break;

        case 0x05:  // OR
            printf("OR:\n");
            result8 = operand1 | operand2;
            ACC = result8;
            setFlags(ACC);
            carry_flag = 0;
            overflow_flag = 0;
            printf("  operand1: "); printBin(operand1, 8); printf("\n");
            printf("  operand2: "); printBin(operand2, 8); printf("\n");
            printf("  result: "); printBin(result8, 8); printf("\n");
            break;

        case 0x06:  // NOT (operand2 ignored)
            printf("NOT (operand2 ignored):\n");
            result8 = ~operand1;
            ACC = result8;
            setFlags(ACC);
            carry_flag = 0;
            overflow_flag = 0;
            printf("  operand1: "); printBin(operand1, 8); printf("\n");
            printf("  result: "); printBin(result8, 8); printf("\n");
            break;

        case 0x07:  // XOR
            printf("XOR:\n");
            result8 = operand1 ^ operand2;
            ACC = result8;
            setFlags(ACC);
            carry_flag = 0;
            overflow_flag = 0;
            printf("  operand1: "); printBin(operand1, 8); printf("\n");
            printf("  operand2: "); printBin(operand2, 8); printf("\n");
            printf("  result: "); printBin(result8, 8); printf("\n");
            break;

        case 0x08:  // Shift Right Logical
            printf("SHIFT RIGHT LOGICAL:\n");
            {
                unsigned char shift = operand2;
                printf("  operand1: "); printBin(operand1, 8); printf("\n");
                printf("  shift amount: %u\n", shift);
                if (shift == 0) {
                    result8 = operand1;
                    carry_flag = 0;
                } else if (shift <= 7) {
                    carry_flag = (operand1 >> (shift - 1)) & 1;
                    result8 = operand1 >> shift;
                } else if (shift == 8) {
                    carry_flag = (operand1 >> 7) & 1;
                    result8 = 0;
                } else {
                    carry_flag = 0;
                    result8 = 0;
                }
                ACC = result8;
                setFlags(ACC);
                overflow_flag = 0;
                printf("  result: "); printBin(result8, 8); printf(" (carry=%d)\n", carry_flag);
            }
            break;

        case 0x09:  // Shift Left Logical
            printf("SHIFT LEFT LOGICAL:\n");
            {
                unsigned char shift = operand2;
                printf("  operand1: "); printBin(operand1, 8); printf("\n");
                printf("  shift amount: %u\n", shift);
                if (shift == 0) {
                    result8 = operand1;
                    carry_flag = 0;
                } else if (shift <= 7) {
                    carry_flag = (operand1 >> (8 - shift)) & 1;
                    result8 = operand1 << shift;
                } else if (shift == 8) {
                    carry_flag = operand1 & 1;
                    result8 = 0;
                } else {
                    carry_flag = 0;
                    result8 = 0;
                }
                ACC = result8;
                setFlags(ACC);
                overflow_flag = 0;
                printf("  result: "); printBin(result8, 8); printf(" (carry=%d)\n", carry_flag);
            }
            break;

        default:
            printf("Unknown control signal: 0x%02X\n", control_signals);
            return 0;
    }

    // Print all flags
    printf("  Flags: Z=%d  S=%d  C=%d  OV=%d\n", zero_flag, sign_flag, carry_flag, overflow_flag);
    printf("========================================\n");

    return (int)(ACC & 0xFF);   // return 8‑bit result
}

// ----------------------------------------------------------------------
int main() {
    unsigned char op1, op2, ctrl;
    int choice;

    printf("8-bit ALU Simulator\n");
    printf("===================\n");

    // Fixed test cases from lab handout (Figure 2) – addition first now
    printf("\n--- Fixed test cases from handout ---\n");
    ALU(0x03, 0x05, 0x01);   // 3 + 5   (addition first)
    ALU(0x03, 0x05, 0x02);   // 3 - 5
    ALU(0x88, 0x85, 0x01);   // -120 + -123
    ALU(0xC0, 0x02, 0x03);   // -64 * 2

    // Additional test cases to cover all operations
    printf("\n--- Additional test cases for all operations ---\n");
    ALU(0x55, 0xAA, 0x04);   // AND
    ALU(0x55, 0xAA, 0x05);   // OR
    ALU(0x55, 0x00, 0x06);   // NOT
    ALU(0x55, 0xAA, 0x07);   // XOR
    ALU(0xF0, 0x02, 0x08);   // Shift right logical 0xF0 >> 2
    ALU(0x0F, 0x03, 0x09);   // Shift left logical 0x0F << 3

    // Interactive testing loop
    printf("\n--- Interactive mode ---\n");
    do {
        printf("\nEnter operand1 (0-255), operand2 (0-255), control signal (1-9): ");
        scanf("%hhu %hhu %hhu", &op1, &op2, &ctrl);
        if (ctrl >= 1 && ctrl <= 9) {
            ALU(op1, op2, ctrl);
        } else {
            printf("Invalid control signal. Use 1..9\n");
        }
        printf("Continue? (1=yes, 0=no): ");
        scanf("%d", &choice);
    } while (choice);

    return 0;
}
