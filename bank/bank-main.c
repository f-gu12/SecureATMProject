/* 
 * The main program for the Bank.
 *
 * You are free to change this as necessary.
 */

#include <string.h>
#include <sys/select.h>
#include <stdio.h>
#include <stdlib.h>
#include "bank.h"
#include "ports.h"
#include "hash_table.h"


static const char prompt[] = "BANK: ";

//EDITSS

void append_to_string(char dest, const charto_append);

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
        perror("Error opening bank initialization file");
        return 64;
    }
    // is fopen secure? when to close file?
    // end student changes

   int n;
   char sendline[1000];
   char recvline[1000];

   Bank *bank = bank_create();

   printf("%s", prompt);
   fflush(stdout);

   while(1)
   {
       fd_set fds;
       FD_ZERO(&fds);
       FD_SET(0, &fds);
       FD_SET(bank->sockfd, &fds);
       select(bank->sockfd+1, &fds, NULL, NULL, NULL);

       if(FD_ISSET(0, &fds))
       {
           fgets(sendline, 10000,stdin);
           bank_process_local_command(bank, sendline, strlen(sendline));
           printf("%s", prompt);
           fflush(stdout);
       }
       else if(FD_ISSET(bank->sockfd, &fds))
       {
           n = bank_recv(bank, recvline, 10000);
           bank_process_remote_command(bank, recvline, n);
       }
   }

   return EXIT_SUCCESS;
}
