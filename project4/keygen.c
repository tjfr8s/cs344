#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "keygen.h"

int main(int argc, char** argv) {
    int keyLength;
    if (argc > 1) {
        keyLength = string_to_int(argv[1]);
        srand(time(0));
        create_key(keyLength);
    }
    return 0;
}

int string_to_int(char* digitString) {
    int stringLength = strlen(digitString);
    int i;
    int result = 0;
    for (i = 0; i < stringLength; i++) {
        result *= 10;
        result += digitString[i] - 48;
    }

    return result;
}

char generate_random_key_char() {
    char charOptions[27] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K',
        'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y',
        'Z', ' '};
    int randomCharIndex = rand() % 27;
    return charOptions[randomCharIndex];
}

void create_key(int keyLength) {
    char key[keyLength + 1];

    int i;
    for (i = 0; i < keyLength; i++) {
        key[i] = generate_random_key_char();
    }
    key[keyLength] = '\n';
    printf(key);
}
