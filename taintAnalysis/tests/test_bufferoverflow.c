#include <stdio.h>
#include <string.h>

int main() {
    char user_input[100]; // Taint Source:  User input is untrusted
    char buffer[10];
    printf("Enter a string: ");
    fgets(user_input, sizeof(user_input), stdin); // Reads up to 99 characters plus null terminator
    // Remove the trailing newline character, if present
    user_input[strcspn(user_input, "\n")] = 0;
    // Passing potentially tainted user_input directly to the risky function strcpy
    // Taint Sink: strcpy()
    strcpy(buffer,user_input);
    printf("Copied data: %s\n", buffer);
    return 0;
}
