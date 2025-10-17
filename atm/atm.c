#include "atm.h"
#include "ports.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
// TCP protocol stuff
// #include <arpa/inet.h>
// #include <strings.h>
// #include <unistd.h>

static bool active = false;
static char* user = NULL;

ATM* atm_create()
{
    ATM *atm = (ATM*) malloc(sizeof(ATM));
    if(atm == NULL)
    {
        perror("Could not allocate ATM");
        exit(1);
    }

    // Set up the network state
    atm->sockfd=socket(AF_INET,SOCK_DGRAM,0);

    bzero(&atm->rtr_addr,sizeof(atm->rtr_addr));
    atm->rtr_addr.sin_family = AF_INET;
    atm->rtr_addr.sin_addr.s_addr=inet_addr("127.0.0.1");
    atm->rtr_addr.sin_port=htons(ROUTER_PORT);

    bzero(&atm->atm_addr, sizeof(atm->atm_addr));
    atm->atm_addr.sin_family = AF_INET;
    atm->atm_addr.sin_addr.s_addr=inet_addr("127.0.0.1");
    atm->atm_addr.sin_port = htons(ATM_PORT);
    bind(atm->sockfd,(struct sockaddr *)&atm->atm_addr,sizeof(atm->atm_addr));

    // Set up the protocol state
    // TODO set up more, as needed
    // how to keep track of begin session 
    // TCP - retransmission of lost packets- dont take our fucking stuff
        //treats application data as a byte stream
        //how to establish:
        // client and server set initial sequence numbers
        // SYN message specifies sequence number in one direction
        // ACK message signals acceptance of this number
        //both are header options, combined in a single packet
        // seen in wireshark?
        // BANK IS SERVER AND ATM IS CLIENT
        // 1. create TCP socket
        // 2. connect newly created client socket to server
        // cons: not authenticated
        // ummm attacker can kill connection by sending rst messages
        // need to make TCP be able to recover dropped data and stuff
    // UMMMMMMMMMMM LOL WHATEVER IG
    // int sockfd, connfd;
    // struct sockaddr_in servaddr, cli;
    // // WHAT ARE WE TALKINGGGG ABOUTTTTTT
    // sockfd = socket(AF_INET, SOCK_STREAM, 0);
    // if (sockfd == -1) {
    //     printf("socket creation failed...\n");
    //     exit(0);
    // }
    // else
    //     printf("Socket successfully created..\n");
    // // WHO THE FUCK ARE YOU
    // bzero(&servaddr, sizeof(servaddr));

    // // assign IP, PORT
    // servaddr.sin_family = AF_INET;
    // servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    // servaddr.sin_port = htons(PORT);

    // // connect the client socket to server socket
    // if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr))
    //     != 0) {
    //     printf("connection with the server failed...\n");
    //     exit(0);
    // }
    // else
    //     printf("connected to the server..\n");

    // // function for chat
    // func(sockfd);

    // // close the socket
    // close(sockfd);


    return atm;
}

void atm_free(ATM *atm)
{
    if(atm != NULL)
    {
        close(atm->sockfd);
        free(atm);
    }
}

ssize_t atm_send(ATM *atm, char *data, size_t data_len)
{
    // Returns the number of bytes sent; negative on error
    return sendto(atm->sockfd, data, data_len, 0,
                  (struct sockaddr*) &atm->rtr_addr, sizeof(atm->rtr_addr));
}

ssize_t atm_recv(ATM *atm, char *data, size_t max_data_len)
{
    // Returns the number of bytes received; negative on error
    return recvfrom(atm->sockfd, data, max_data_len, 0, NULL, NULL);
}

static void active_session(ATM *atm, char *name)
{
    active = true;
    char user_input[1000];
    // name[strcspn(name, "\n")] = 0;
        // shouldnt need this bc its fixed in command part
    // WHYYYY WAS THIS CONST BEFORE I REMBED IT BUT NOW IM SCAREDDDD
    // IS THIS A VUNLNERABILTY
    char prompt[259] = "ATM (";
    // 1. if name is more than 250 chars, itll still work, just put the first 250 chars
    // 2. if prompt size is less than strncat 3rd param, it still works? thats bad right
    // 3. how to make prompt size less than 250... is that important? for memory?
    strncat(prompt, name, 250);
    strncat(prompt, "): ", 4);
    // HOW DO I KNOW THE NULL TERMINATOR IS ADDED WAHHHHHH

    printf("%s", prompt);
    fflush(stdout);

    while (active == true && fgets(user_input,1000,stdin) != NULL)
    {
        atm_process_command(atm, user_input);
        if (active) {
            printf("%s", prompt);
            fflush(stdout);
        }
    }
}

// check if its right length and upper case and lower case letter
bool is_valid_input(const char *input) {
    // Check length constraint
    size_t length = strlen(input);
    if (length > 250) {
        return 0; // Invalid if input exceeds 250 characters
    }

    // Check if all characters are uppercase or lowercase letters
    for (size_t i = 0; i < length; i++) {
        if (!isalpha((unsigned char)input[i])) { // isalpha checks for a-z or A-Z
            return false; // Invalid if any character is not a letter
        }
    }

    return true; // Valid input
}

bool isNumericString(const char *str) {
    // Check for NULL pointer or empty string
    if (str == NULL || *str == '\0') {
        return 0; // Not numeric
    }

    // Iterate through each character in the string
    while (*str) {
        if (!isdigit((unsigned char)*str)) {
            return false; // Found a non-digit character
        }
        str++; // Move to the next character
    }

    return true; // All characters are digits
}

char* convertToASCII(const char* input) {
    static char asciiString[9];
    
    // Format the ASCII values into the output string
    sprintf(asciiString, "%d%d%d%d", input[0], input[1], input[2], input[3]);
    asciiString[8] = '\0';
    
    return asciiString;
}


bool validate_user_exists(ATM *atm, char *username)
{
    char recvline[10000];
    int n;

    char check_username_comand[259];
    sprintf(check_username_comand, "begin-session %s", username);

    // Here is where we should encrypt message using bank public key

    atm_send(atm, check_username_comand, strlen(check_username_comand));
    n = 0;
    while (n == 0) {
        n = atm_recv(atm,recvline,10000);
    }
    recvline[n]=0;

    // Here is where we should decrypt using atm private key

    if (strcmp(recvline, "Invalid Command") == 0) {
        return false;
    }
    return true;
}

void atm_process_command(ATM *atm, char *command)
{
    // TODO: Implement the ATM's side of the ATM-bank protocol

	/*
	 * The following is a toy example that simply sends the
	 * user's command to the bank, receives a message from the
	 * bank, and then prints it to stdout.
	 */

	/*
    char recvline[10000];
    int n;

    atm_send(atm, command, strlen(command));
    n = atm_recv(atm,recvline,10000);
    recvline[n]=0;
    fputs(recvline,stdout);
	*/

    command[strcspn(command, "\n")] = '\0';
    // and if someone enters "\nfuck-you?"
    // does that even matter... itll be a wrong command
    char* by_word = strtok(command, " ");
    // by_word[strcspn(by_word, "\n")] = '\0';
    // shouldnt need this anymore bc this was for commands where they were the only arg
    

    if (by_word != NULL) {
        // is strcmp secure??????????
        // something is wrong with strnlen
        // how the fuck do i check if by_wordis less than command
        // also if by-word is a pointer to a null terminate shtring
        if (strncmp(by_word, "begin-session", strlen(by_word)) == 0) {
            // how to implement time outs/limited attampts?
            char* username = strtok(NULL, " ");
            char* user_card_file[255]; // add the 250 max with .card extension
            sprintf(user_card_file, "%s.card", username);

            if (active) {
                printf("A user is already logged in\n");
            } else if (username == NULL || strtok(NULL, " ") != NULL || !is_valid_input(username)) {
                // if username is not a valid input or not provided
                printf("Usage: begin-session <user-name>\n");
            } else if (!validate_user_exists(atm, username)) {
                // User does not exist
                printf("No such user\n");
            } else {
                // try to open card
                // todo: use pin xor card value, if equal to keyword in atm file, pin is correct
                char pin_ans[1000]; 
                static const char pin_prompt[] = "PIN? ";
                printf("%s", pin_prompt);
                fflush(stdout);

                if (fgets(pin_ans, 1000, stdin) != NULL) {
                    // check pin here
                    pin_ans[strcspn(pin_ans, "\n")] = '\0';
                    FILE *fp;
                    char line[256];

                    fp = fopen(user_card_file, "r");
                    if (fp == NULL) {
                        printf("No such user\n");
                    } else {
                        // Read and discard the first line
                        if (fgets(line, sizeof(line), fp) == NULL) {
                            printf("%s\n", line);
                            printf("first line error Not authorized\n");
                        } else {
                            // Read the second line
                            if (fgets(line, sizeof(line), fp) == NULL) {
                                printf("%s\n", line);
                                printf("second line error Not authorized\n");
                            } else {
                                // check pin
                                if (strcmp(pin_ans, line) != 0) {
                                    printf("cmp: %d\n", strcmp(pin_ans, line));
                                    printf("%s\n", line);
                                    printf("did not check properly Not authorized\n");
                                } else {
                                    user = username;
                                    active_session(&atm, username);
                                }
                            }
                        }
                    }
                } else {
                    printf("Not authorized\n");
                }
                 
            }

        } else if (strncmp(by_word, "withdraw", strlen(by_word)) == 0) {
            char* amt = strtok(NULL, " ");
            // still need to validate data
            if (!active) {
                printf("No user logged in\n");
            } else if (amt == NULL || strtok(NULL, " ") != NULL || !(isNumericString(amt)) || atoi(amt) < 0) {
                printf("Usage: withdraw <amt>\n");
            } else {
                printf("user: %s\n", user);
                char recvline[10000];
                int n;

                char withdraw[259];
                sprintf(withdraw, "withdraw %s %s", user, amt);

                // Here is where we should encrypt message using bank public key

                atm_send(atm, withdraw, strlen(withdraw));
                n = 0;
                while (n == 0) {
                    n = atm_recv(atm,recvline,10000);
                }
                recvline[n]=0;

                // Here is where we should decrypt using atm private key

                if (strcmp(recvline, "Invalid Command") == 0) {
                    printf("Insufficient funds");
                } else {
                    printf("$%s dispensed", amt);
                }
            }
        
        } else if (strncmp(by_word, "balance", strlen(by_word)) == 0) {
            char* end = strtok(NULL, " ");
            if (!active) {
                printf("No user logged in\n");
            } else if (end == NULL) {
                printf("user: %s\n", user);
                char recvline[10000];
                int n;

                char bal[259];
                sprintf(bal, "balance %s", user);

                // Here is where we should encrypt message using bank public key

                atm_send(atm, bal, strlen(bal));
                n = 0;
                while (n == 0) {
                    n = atm_recv(atm,recvline,10000);
                }
                recvline[n]=0;

                // Here is where we should decrypt using atm private key
                printf("$%s", recvline);
            } else {
                printf("Usage: balance\n");
            }

        // THIS SHOULD WORK??????
        } else if (strncmp(by_word, "end-session", strlen(by_word)) == 0) {
            char* end = strtok(NULL, " ");
            if (!active) {
                printf("No user logged in\n");
            } else if (end == NULL) {
                active = false;
                printf("User logged out\n");
            } else {
                // are we supposed to do something if additional arguments given for this command?
                // invalid or just terminate?
                // how to terminate session?
                printf("Invalid command\n");
            }
        } else {
            printf("Invalid command\n");
        }
    } else {
        printf("Invalid command\n");
    }

    // // Buffer for receiving responses from the bank
    // char recvline[10000];
    // int n;

    // // Ensure the command is null-terminated
    // command[strcspn(command, "\n")] = '\0';

    // // Send the command to the bank
    // if (atm_send(atm, command, strlen(command)) < 0) {
    //     perror("Error sending command to the bank");
    //     return;
    // }

    // // Receive the response from the bank
    // n = atm_recv(atm, recvline, sizeof(recvline) - 1);
    // if (n < 0) {
    //     perror("Error receiving response from the bank");
    //     return;
    // }

    // // Null-terminate and print the response
    // recvline[n] = '\0';
    // printf("Bank response: %s\n", recvline);

}
