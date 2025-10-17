#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/bio.h>
#include <openssl/err.h>

const int KEY_LENGTH = 2048;

typedef struct KeyPair {
    char *private_key;
    char *public_key;
} KeyPair;

char* convertToASCII(const char* input) {
    static char asciiString[9];

    // Format the ASCII values into the output string
    sprintf(asciiString, "%d%d%d%d", input[0], input[1], input[2], input[3]);
    asciiString[8] = '\0'; // Null-terminate string
    
    return asciiString;
}


char* generateRandomString() {
    srand(time(0)); // Seed with time
    static char randomString[5]; // Allocate space for 4 characters + null terminator
    for (int i = 0; i < 4; i++) {
        randomString[i] = '0' + (rand() % 10); // Generate a random digit (0-9)
    }
    randomString[4] = '\0'; // Null-terminate the string
    return randomString;
}

struct KeyPair generate_rsa_keypair() {
    RSA *rsa = NULL;
    BIGNUM *bne = NULL;
    BIO *bio_private = NULL, *bio_public = NULL;
    char *private_key_string = NULL;
    char *public_key_string = NULL;
    int ret;

    // Initialize the big number
    bne = BN_new();
    ret = BN_set_word(bne, RSA_F4);
    if (ret != 1) {
        printf("Error initializing big number\n");
        goto cleanup;
    }

    // Generate RSA key pair
    rsa = RSA_new();
    ret = RSA_generate_key_ex(rsa, KEY_LENGTH, bne, NULL);
    if (ret != 1) {
        printf("Error generating RSA key pair\n");
        goto cleanup;
    }

    // Create BIO objects for the keys
    bio_private = BIO_new(BIO_s_mem());
    bio_public = BIO_new(BIO_s_mem());
    if (!bio_private || !bio_public) {
        printf("Error creating BIO objects\n");
        goto cleanup;
    }

    // Write the keys to the BIO objects
    if (!PEM_write_bio_RSAPrivateKey(bio_private, rsa, NULL, NULL, 0, NULL, NULL)) {
        printf("Error writing private key\n");
        goto cleanup;
    }
    if (!PEM_write_bio_RSAPublicKey(bio_public, rsa)) {
        printf("Error writing public key\n");
        goto cleanup;
    }

    // Allocate the key strings in memory
    size_t private_len = BIO_pending(bio_private);
    size_t public_len = BIO_pending(bio_public);
    // + 1 for null terminating string
    private_key_string = (char *)malloc(private_len + 1); 
    public_key_string = (char *)malloc(public_len + 1);

    if (!private_key_string || !public_key_string) {
        printf("Error allocating memory for key strings\n");
        if (private_key_string) free(private_key_string);
        if (public_key_string) free(public_key_string);
    }

    BIO_read(bio_private, private_key_string, private_len);
    BIO_read(bio_public, public_key_string, public_len);

     // Null-terminate the strings
    private_key_string[private_len] = '\0';
    public_key_string[public_len] = '\0';

    cleanup: 
        // Cleanup resources
        if (bne) BN_free(bne);
        if (rsa) RSA_free(rsa);
        if (bio_private) BIO_free_all(bio_private);
        if (bio_public) BIO_free_all(bio_public);

    struct KeyPair rsa_pair;
    rsa_pair.private_key = private_key_string;
    rsa_pair.public_key = public_key_string;
    return rsa_pair;
}

int main(int argc, char *argv[]) {
    // Check the number of arguments
    if (argc != 2) {
        fprintf(stderr, "Usage: init <filename>\n");
        return 62;
    }

    // Get the base path and construct filenames
    char *base_path = argv[1];
    char bank_file[1024], atm_file[1024];

    snprintf(bank_file, sizeof(bank_file), "%s.bank", base_path);
    snprintf(atm_file, sizeof(atm_file), "%s.atm", base_path);

    // Check if either file already exists
    if (access(bank_file, F_OK) == 0 || access(atm_file, F_OK) == 0) {
        fprintf(stderr, "Error: one of the files already exists\n");
        return 63;
    }

    // Create the .bank file
    FILE *bank_fp = fopen(bank_file, "w");
    if (!bank_fp) {
        fprintf(stderr, "Error creating initialization files\n");
        return 64;
    }

    // Create the .atm file
    FILE *atm_fp = fopen(atm_file, "w");
    if (!atm_fp) {
        fprintf(stderr, "Error creating initialization files\n");
        return 64;
    }

    // Generate a random 4 digit keyword string (8 digits in ASCII) used to check if a pin for an atm card is correct
    char *ascii_keyword = convertToASCII(generateRandomString());

    // Write keyword to bank and atm files
    fprintf(bank_fp, "%s\n", ascii_keyword);
    fprintf(atm_fp, "%s\n", ascii_keyword);

    // Generate two RSA key pairs and exchange public keys
    // to allow atm and bank to talk to each other securely
    struct KeyPair bank_keypair = generate_rsa_keypair();
    fprintf(bank_fp, "%s\n", bank_keypair.private_key);
    fprintf(atm_fp, "%s\n", bank_keypair.public_key);

    struct KeyPair atm_keypair = generate_rsa_keypair();
    fprintf(atm_fp, "%s\n", atm_keypair.private_key);
    fprintf(bank_fp, "%s\n", atm_keypair.public_key);

    // Close .bank and .atm file
    fclose(bank_fp);
    fclose(atm_fp);

    // Free keypair from memory
    free(bank_keypair.private_key);
    free(bank_keypair.public_key);
    free(atm_keypair.private_key);
    free(atm_keypair.public_key);

    // Success message
    printf("Successfully initialized bank state\n");
    return 0;
}
