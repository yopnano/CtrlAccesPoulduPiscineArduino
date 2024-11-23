#include "Utils.h"

const char *Bool_To_Char(const bool value) { return value ? "1" : "0"; }

void Bool_To_Char(const bool value, char *buffer)
{
    buffer[0] = value ? '1' : '0';
    buffer[1] = '\0'; // Terminer avec un caractère null
}