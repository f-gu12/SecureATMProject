#include "bank.h"
#include "ports.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include "hash_table.h"
#include <limits.h>
#include "list.h"

HashTable* ht = NULL;
Bank* bank_create()
{
    Bank *bank = (Bank*) malloc(sizeof(Bank));
    if(bank == NULL)
    {
        perror("Could not allocate Bank");
        exit(1);
    }
    ht = hash_table_create(50);

    // Set up the network state
    bank->sockfd=socket(AF_INET,SOCK_DGRAM,0);

    bzero(&bank->rtr_addr,sizeof(bank->rtr_addr));
    bank->rtr_addr.sin_family = AF_INET;
    bank->rtr_addr.sin_addr.s_addr=inet_addr("127.0.0.1");
    bank->rtr_addr.sin_port=htons(ROUTER_PORT);

    bzero(&bank->bank_addr, sizeof(bank->bank_addr));
    bank->bank_addr.sin_family = AF_INET;
    bank->bank_addr.sin_addr.s_addr=inet_addr("127.0.0.1");
    bank->bank_addr.sin_port = htons(BANK_PORT);
    bind(bank->sockfd,(struct sockaddr *)&bank->bank_addr,sizeof(bank->bank_addr));

    // Set up the protocol state
    // TODO set up more, as needed


    return bank;
}

char* convertToASCII(const char* input) {
    static char asciiString[9];
    
    // Format the ASCII values into the output string
    sprintf(asciiString, "%d%d%d%d", input[0], input[1], input[2], input[3]);
    asciiString[8] = '\0';
    
    return asciiString;
}

int isNumericString(const char *str) {
    // Check for NULL pointer or empty string
    if (str == NULL || *str == '\0') {
        return 0; // Not numeric
    }

    // Iterate through each character in the string
    while (*str) {
        if (!isdigit((unsigned char)*str)) {
            return 0; // Found a non-digit character
        }
        str++; // Move to the next character
    }

    return 1; // All characters are digits
}

void bank_free(Bank *bank)
{
    if(bank != NULL)
    {
        close(bank->sockfd);
        free(bank);
    }
}

ssize_t bank_send(Bank *bank, char *data, size_t data_len)
{
    // Returns the number of bytes sent; negative on error
    return sendto(bank->sockfd, data, data_len, 0,
                  (struct sockaddr*) &bank->rtr_addr, sizeof(bank->rtr_addr));
}

ssize_t bank_recv(Bank *bank, char *data, size_t max_data_len)
{
    // Returns the number of bytes received; negative on error
    return recvfrom(bank->sockfd, data, max_data_len, 0, NULL, NULL);
}

void bank_process_local_command(Bank *bank, char *command, size_t len)
{
    //Create user variables
    char *line;
    char *arguments[4];
    int index = 0;

    char filename[1024];
    char userenter[1024];

    char *username;
    char *action;
    char *pin;
    char *amount;

    //used for deposit fixing 
    int number_input_amount;
    int number_current_amount;
    int new_amount;

    char *new_key[20]; //used to update new key value pair 

    print_hash_table(ht);
    size_t cmd_len = strlen(command);
    if (cmd_len > 0 && command[cmd_len - 1] == '\n') {
        command[cmd_len - 1] = '\0'; // Remove trailing newline
    }

    //reads a string with maximum 250 length
    // fgets(userenter, sizeof(line), command);
    //printf("The command: %s\n", command);
    // line = strtok(command, " \n");

    line = strtok(command, " ");

    if(strcmp(line, "create-user") == 0){
        username = strtok(NULL, " ");
        if (username == NULL) {
            printf("Usage:  create-user <user-name> <pin> <balance>\n");
            return 0;
        }
        sanitize_key(username);
        pin = strtok(NULL, " ");
        if (pin == NULL) {
            printf("Usage:  create-user <user-name> <pin> <balance>\n");
            return 0;
        }
        amount = strtok(NULL, " ");
        if (amount == NULL) {
            printf("Usage:  create-user <user-name> <pin> <balance>\n");
            return 0;
        }

        // printf("Username: %s\n", username);
        // printf("Pin: %s\n",  pin);
        // printf("Amount %s\n", amount);

        
        if(is_valid_input(username) != 1 || !isNumericString(pin) || strlen(pin) != 4 || !isNumericString(amount) || atoi(amount)< 0) {
            printf("Usage:  create-user <user-name> <pin> <balance>\n");
        } else {
            if(hash_table_find(ht, username) != NULL){ //does exist
                    printf("Error:  user %s already exists\n", username);
            } else {

                //hash_table_add(ht, username, amount);
                hash_table_add(ht, strdup(username), strdup(amount));


                //print_hash_table(ht);
                
                
                //Puts the card into the bin folder
                sprintf(filename, "%s.card", username);

                // Open the file for writing
                FILE *file = fopen(filename, "w");
                if (file == NULL) {
                    hash_table_del(ht, username);
                    printf("Error creating card file for user %s\n", username); // Print error if file creation fails
                    return 1;
                }
                //print_hash_table(ht);

                // Optional: Write some content to the file, UP TO DESIGN (EMAN AND KAYLEE)
                fprintf(file, "%s\n%s", username, pin);
                
                // Close the file
                fclose(file);
                printf("Created user %s\n", username);
                //print_hash_table(ht);
            }
        }

    
    } else if (strcmp(line, "deposit") == 0){
        username = strtok(NULL, " ");
        if (username == NULL) {
            printf("Usage:  deposit <user-name> <amt>\n");
            return 0;
        }
        sanitize_key(username);
        amount = strtok(NULL, " ");
        if (amount == NULL) {
            printf("Usage:  deposit <user-name> <amt>\n");
            return 0;
        }
            

        // printf("Username: %s\n", username);
        // printf("Amount %s\n", amount);
        // printf("Username from hash -> %s\n", hash_table_find(ht, username));

        // print_hash_table(ht);
        
        if(is_valid_input(username) != 1 || !isNumericString(amount) || atoi(amount) < 0){
            printf("Usage:  deposit <user-name> <amt>\n");
            
        } else {
            //int
            number_input_amount = atoi(amount);
            //int
            number_current_amount = atoi(hash_table_find(ht,username));
            //int

            new_amount = number_input_amount + number_current_amount;

            sprintf(new_key, "%d", new_amount);

            

            if(hash_table_find(ht,username) == NULL){ // if cannot find user
                printf("No such user\n");
            } else {
                long long check = strtoll(amount, NULL, 10);
                // printf("check: %llu\n", check);
                number_current_amount = atoi(hash_table_find(ht,username));
                if (check > INT_MAX || check + number_current_amount > INT_MAX) {
                    printf("Too rich for this program\n");
                } else {

                    new_amount = check + number_current_amount;

                    sprintf(new_key, "%d", new_amount);

                    // printf("New_key string: %s \n", new_key);

                    // printf("Int_max %d\n", INT_MAX);

                    // printf("Int_Max + number_curr: %d\n", INT_MAX + number_current_amount);

                    hash_table_del(ht, username); //delete key and then add new one 
                    hash_table_add(ht, strdup(username), strdup(new_key)); 
                    printf("$%s added to %s's account\n", amount, username);
                    // print_hash_table(ht);
                    // printf("new amount in hash table %s\n", hash_table_find(ht, username));
                }
                
            }
        }


    } else if (strcmp(line, "balance") == 0){
        username = strtok(NULL, " ");
        if (username == NULL) {
            printf("Usage:  balance <user-name>\n", username);
            return 0;
        }
        if(is_valid_input(username) != 1 ){
            printf("Usage:  balance %s\n", username);
        }
        if(is_valid_input(username) == 1){
            if(hash_table_find(ht,username) == NULL){ // if cannot find user
                printf("No such user\n");
            } else {
                printf("$%s\n", hash_table_find(ht, username));
            }
        }
    } else {
        printf("Invalid command\n");
    }

    return 1;
}


//check if its right length and upper case and lowere case lettere
int is_valid_input(const char *input) {
    // Check length constraint
    size_t length = strlen(input);
    if (length > 250) {
        return 0; // Invalid if input exceeds 250 characters
    }

    // Check if all characters are uppercase or lowercase letters
    for (size_t i = 0; i < length; i++) {
        if (!isalpha((unsigned char)input[i])) { // isalpha checks for a-z or A-Z
            return 0; // Invalid if any character is not a letter
        }
    }

    return 1; // Valid input
}

int check_overflow(int num1, int num2)
{
    // Checking if addition will cause overflow
    if (num1 > INT_MAX - num2)
        return -1;

    // No overflow occurred
    else
        return num1 + num2;
}

void bank_process_remote_command(Bank *bank, char *command, size_t len)
{
    // TODO: Implement the bank side of the ATM-bank protocol

	/*
	 * The following is a toy example that simply receives a
	 * string from the ATM, prepends "Bank got: " and echoes 
	 * it back to the ATM before printing it to stdout.
	 */

	/*
    char sendline[1000];
    command[len]=0;
    sprintf(sendline, "Bank got: %s", command);
    bank_send(bank, sendline, strlen(sendline));
    printf("Received the following:\n");
    fputs(command, stdout);
	*/

    // Ensure the incoming command is null-terminated
    command[len] = '\0';

    char *username;
    char *action;
    char *pin;
    char *amount;
    char *line;
    char *balance_in_bank;
        
    char filename[1024];

    char *new_key[20];

    int number_input_amount;
    int balance_int;
    int new_amount;

    printf("%s\n", command);

    line = strtok(command, " ");

    //ATM would not send me anything if it errors
    //It has to be correct 
    //I do not have to check if [a-zA-Z]+ will work or not cause ATM checks it

    // Prepare the response buffer
    char response[1024];

    if(strcmp(line, "begin-session") == 0){
        username = strtok(NULL, " ");
        //if user exists send the user back
        if(hash_table_find(ht, username) == NULL){
            snprintf(response, sizeof(response), "%s", "Invalid Command");
            bank_send(bank, response, strlen(response));
            printf("Received the following:\n");
            fputs(command, stdout);
        } else {
            snprintf(response, sizeof(response), "%s", command);
            bank_send(bank, response, strlen(response));
            printf("Received the following:\n");
            fputs(command, stdout);
        }
        

        //if not valid user send Invalid command 
        //PIN "pin username number"   
    //kaylle give me 
    //withdraw, user, balance
    } else if (strcmp(line, "withdraw") == 0){
        username = strtok(NULL, " ");
        amount = strtok(NULL, " ");

        balance_in_bank = hash_table_find(ht, username);
        balance_int = atoi(balance_in_bank);
        number_input_amount = atoi(amount);

        //not enough money in the funds
        if(number_input_amount > balance_int){
            snprintf(response, sizeof(response), "%s", "Invalid Command");
            bank_send(bank, response, strlen(response));
            printf("Received the following:\n");
            fputs(command, stdout);
        } else { 
            //update the value 
            new_amount = balance_int - number_input_amount;

            sprintf(new_key, "%d", new_amount);
            hash_table_del(ht, username);
            hash_table_add(ht, username, new_amount);
            
            snprintf(response, sizeof(response), "%s", command);
            bank_send(bank, response, strlen(response));
            printf("Received the following:\n");
            fputs(command, stdout);
        }
    //give balance, user
    //give back the balance username
    } else if (strcmp(line, "balance") == 0) {
        username = strtok(NULL, " ");
        snprintf(response, sizeof(response), "%s", hash_table_find(ht, username));
        bank_send(bank, response, strlen(response));
        printf("Received the following:\n");
        fputs(command, stdout);
    } 
}


void sanitize_key(char *key) {
    size_t len = strlen(key);
    while (len > 0 && (key[len - 1] == ' ' || key[len - 1] == '\n')) {
        key[--len] = '\0';
    }
}

void print_hash_table(HashTable *ht) {
    if (ht == NULL) {
        printf("Hash table is NULL\n");
        return;
    }

    printf("\nHash table contents:\n");
    printf("Number of bins: %u\n", ht->num_bins);
    printf("Size: %u\n", ht->size);

    for (uint32_t i = 0; i < ht->num_bins; i++) {
        List *bin = ht->bins[i];
        if (bin == NULL || bin->head == NULL) {
            continue; // Skip empty bins
        }

        printf("Bin %u:\n", i);
        ListElem *current = bin->head; // Adjust this to match your actual structure
        while (current != NULL) {
            char *key = (char *)current->key; // Ensure the key is correctly typed
            int *value = (int *)current->val; // Assuming values are integers
            printf("  Key: %s, Value: %d\n \n", key, *value);
            current = current->next; // Move to the next element in the list
        }
    }
}