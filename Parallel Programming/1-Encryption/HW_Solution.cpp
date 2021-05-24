#include <cstring>
#include <iostream>
#include <algorithm>
#include "vv-aes.h"

/**
 * This function takes the characters stored in the 4x4 message array and substitutes each character for the
 * corresponding replacement as specified in the originalCharacter and substitutedCharacter array.
 * This corresponds to step 2.1 in the VV-AES explanation.
 */
void substitute_bytes() {
    // For each byte in the message
    for (int column = 0; column < MESSAGE_BLOCK_WIDTH; column++) {
        for (int row = 0; row < MESSAGE_BLOCK_WIDTH; row++) {
            // Replace the byte with the corresponding element in the resorted substituted character list
            message[row][column] = substitutedCharacter[message[row][column]];
        }
    }
}

/*
 * This function shifts each row by the number of places it is meant to be shifted according to the AES specification.
 * Row zero is shifted by zero places. Row one by one, etc.
 * This corresponds to step 2.2 in the VV-AES explanation.
 */
void shift_rows() {
    // Shift each row, where the row index corresponds to how many columns the data is shifted.
    for (int row = 0; row < MESSAGE_BLOCK_WIDTH; ++row) {
        std::rotate(message[row], message[row] + row, message[row] + MESSAGE_BLOCK_WIDTH); 
    }
}

/*
 * This function calculates x^n for polynomial evaluation.
 */
int power(int x, int n) {
    // Calculates x^n
    if (n == 0) {
        return 1;
    }
    return x * power(x, n - 1);
}




uint8_t precomputed_powers[UNIQUE_CHARACTERS][MESSAGE_BLOCK_WIDTH];

/*
 * This function computes powers of all required degrees for all characters
 */

void compute_powers() {
	for(int i = 0; i < UNIQUE_CHARACTERS; i++){
		for(int j = 0; j < MESSAGE_BLOCK_WIDTH; j++) {
			precomputed_powers[i][j] = power(i,j+1);
		}
	}
}

/*
 * This function evaluates four different polynomials, one for each row in the column.
 * Each polynomial evaluated is of the form
 * m'[row, column] = c[r][3] m[3][column]^4 + c[r][2] m[2][column]^3 + c[r][1] m[1][column]^2 + c[r][0]m[0][column]^1
 * where m' is the new message value, c[r] is an array of polynomial coefficients for the current result row (each
 * result row gets a different polynomial), and m is the current message value.
 * The function now uses the precomputed powers instead of calculating them every time.
 *
 */
void multiply_with_polynomial(int column) {
    for (int row = 0; row < MESSAGE_BLOCK_WIDTH; ++row) {
        int result = 0;
        for (int degree = 0; degree < MESSAGE_BLOCK_WIDTH; degree++) {
            result += polynomialCoefficients[row][degree] * precomputed_powers[message[degree][column]][degree];
        }
        message[row][column] = result;
    }
}

/*
 * For each column, mix the values by evaluating them as parameters of multiple polynomials.
 * This corresponds to step 2.3 in the VV-AES explanation.
 */
void mix_columns() {
    for (int column = 0; column < MESSAGE_BLOCK_WIDTH; ++column) {
        multiply_with_polynomial(column);
    }
}

/*
 * Add the current key to the message using the XOR operation.
 */
void add_key() {
    for (int column = 0; column < MESSAGE_BLOCK_WIDTH; column++) {
        for (int row = 0; row < MESSAGE_BLOCK_WIDTH; ++row) {
            // ^ == XOR
            message[row][column] = message[row][column] ^ key[row][column];
        }
    }
}

/*
 * This utility function sorts the given indices in ascending order,
 * while the data associated to the indices is sorted accordingly.
 * Any non-esoteric sorting algorithm would do here, because this function is not performance-critical.
 * 
 */
void sort(uint8_t *indeces, uint8_t *data){
	int min_position{};
	int min;
	for (int i = 0; i < UNIQUE_CHARACTERS; i++){
		min = UNIQUE_CHARACTERS + 1;
		for (int j = i; j < UNIQUE_CHARACTERS; j++){
			if (indeces[j] < min){
				min = indeces[j];
				min_position = j;
			}
		}
		if (i != min_position){		
			std::swap(indeces[i], indeces[min_position]);
			std::swap(data[i], data[min_position]);
		}
	}
}

/*
 * Your main encryption routine.
 */
int main() {

	// let substitutedCharacter be sorted for optimal performance when substituting bytes.
	sort(originalCharacter, substitutedCharacter);
	// precompute the powers required to mix columns.
	compute_powers();

    // Receive the problem from the system.
    readInput();

    // For extra security (and because Varys wasn't able to find enough test messages to keep you occupied) each message
    // is put through VV-AES lots of times. If we can't stop the adverse Maesters from decrypting our highly secure
    // encryption scheme, we can at least slow them down.
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        // For each message, we use a predetermined key (e.g. the password). In our case, its just pseudo random.
        set_next_key();

        // First, we add the key to the message once:
        add_key();

        // We do 9+1 rounds for 128 bit keys
        for (int round = 0; round < NUM_ROUNDS; round++) {
            //In each round, we use a different key derived from the original (refer to the key schedule).
            set_next_key();

            // These are the four steps described in the slides.
            substitute_bytes();
            shift_rows();
            mix_columns();
            add_key();
        }
        // Final round
        substitute_bytes();
        shift_rows();
        add_key();
    }

    // Submit our solution back to the system.
    writeOutput();
    return 0;
}
