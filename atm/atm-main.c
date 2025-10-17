/* 
 * The main program for the ATM.
 *
 * You are free to change this as necessary.
 */

#include "atm.h"
#include <stdio.h>
#include <stdlib.h>

static const char prompt[] = "ATM: ";

int main(int argc, char**argv)
{

    if (argc != 2) {
        // what to do?
        perror("dont do that");
        return 64;
    }
    FILE *param;
    // this is okay because it will only ever be init file
    // do we need to read to file? if file doesnt exist it will make a new one
    // shouldnt need to worry about that bc it will only be init file
    // but do we need to read or write or smth else to file?
    param = fopen(argv[1], "r");
    if (param == NULL) {
        perror("Error opening ATM initialization file");
        return 64;
    }
    // is fopen secure? when to close file?

    // verify user here? since we need to update prompt var
    // end student changes

    char user_input[1000];

    ATM *atm = atm_create();

    printf("%s", prompt);
    fflush(stdout);

    while (fgets(user_input,10000,stdin) != NULL)
    {
        atm_process_command(atm, user_input);
        printf("%s", prompt);
        fflush(stdout);
    }
	return EXIT_SUCCESS;
}
