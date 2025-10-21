#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <openssl/sha.h>

#include "wordlist.h"

#define MAX_WORD_LEN 16

int generate_entropy(unsigned char *entropy, size_t len) {
    FILE *f = fopen("/dev/urandom", "rb");
    if (!f) {
        perror("Failed to open /dev/urandom");
        return -1;
    }

    size_t bytes_read = fread(entropy, 1, len, f);
    fclose(f);

    if (bytes_read != len) {
        fprintf(stderr, "Failed to read enough entropy\n");
        return -1;
    }

    return 0;
}

void generate_mnemonic(const char **wordlist, int word_count, char *output, size_t output_len) {
    int entropy_bits = (word_count * 11) - (word_count / 3);
    int entropy_bytes = entropy_bits / 8;

    unsigned char entropy[32] = {0};
    unsigned char hash[SHA256_DIGEST_LENGTH];

    if (generate_entropy(entropy, entropy_bytes) != 0) {
        fprintf(stderr, "Failed to generate entropy\n");
        return;
    }

    SHA256(entropy, entropy_bytes, hash);

    unsigned char combined[33];
    memcpy(combined, entropy, entropy_bytes);
    combined[entropy_bytes] = hash[0];

    output[0] = '\0';

    for (int i = 0; i < word_count; i++) {
        int bit_offset = i * 11;
        int byte_offset = bit_offset / 8;
        int bit_in_byte = bit_offset % 8;

        unsigned int index = 0;
        if (bit_in_byte <= 5) {
            index = ((combined[byte_offset] << 8) | combined[byte_offset + 1]) >> (5 - bit_in_byte);
        } else {
            index = ((combined[byte_offset] << 16) | (combined[byte_offset + 1] << 8) | combined[byte_offset + 2]) >> (13 - bit_in_byte);
        }
        index &= 0x7FF;

        if (i > 0) {
            strncat(output, " ", output_len - strlen(output) - 1);
        }
        strncat(output, wordlist[index], output_len - strlen(output) - 1);
    }
}

void generate_password(const char **wordlist, int word_count, char *output, size_t output_len) {
    // Generate password mode: random words without BIP39 checksum
    // Uses 2 bytes of entropy per word to cover full 2048 wordlist (11 bits needed)
    int entropy_bytes = word_count * 2;
    unsigned char entropy[200] = {0};  // Max 100 words * 2 bytes

    if (generate_entropy(entropy, entropy_bytes) != 0) {
        fprintf(stderr, "Failed to generate entropy\n");
        return;
    }

    output[0] = '\0';

    for (int i = 0; i < word_count; i++) {
        // Use 2 bytes (16 bits) of entropy and mask to 11 bits for wordlist index
        unsigned int index = ((entropy[i * 2] << 8) | entropy[i * 2 + 1]) & 0x7FF;

        if (i > 0) {
            strncat(output, " ", output_len - strlen(output) - 1);
        }
        strncat(output, wordlist[index], output_len - strlen(output) - 1);
    }

    explicit_bzero(entropy, sizeof(entropy));
}

void print_usage(const char *program_name) {
    printf("Usage:\n");
    printf("  BIP39 Mode (for cryptocurrency wallets):\n");
    printf("    %s <word_count>\n", program_name);
    printf("    Valid word counts: 12, 15, 18, 21, 24\n");
    printf("    Generates BIP39-compliant mnemonic with checksum validation\n\n");

    printf("  Password Mode (for secure passphrases):\n");
    printf("    %s --password <word_count>\n", program_name);
    printf("    Valid word counts: 1-100\n");
    printf("    Generates random words without BIP39 checksum (not for crypto wallets)\n\n");

    printf("  Validation:\n");
    printf("    %s --validate \"<mnemonic>\"\n", program_name);
    printf("    Validates a BIP39 mnemonic phrase\n\n");

    printf("  Help:\n");
    printf("    %s --help\n\n", program_name);

    printf("Examples:\n");
    printf("  %s 12                    # Generate 12-word BIP39 mnemonic\n", program_name);
    printf("  %s --password 7          # Generate 7-word secure password\n", program_name);
    printf("  %s --validate \"word1 word2...\" # Validate mnemonic\n", program_name);
}

int find_word_index(const char *word) {
    for (int i = 0; i < WORD_COUNT; i++) {
        if (strcasecmp(word, wordlist[i]) == 0) {
            return i;
        }
    }
    return -1;
}

int validate_mnemonic(const char *mnemonic_phrase, char *error_reason, size_t reason_len) {
    char *mnemonic_copy = strdup(mnemonic_phrase);
    if (!mnemonic_copy) {
        snprintf(error_reason, reason_len, "Memory allocation failed");
        return 0;
    }

    char *words[24];
    int word_count = 0;
    char *token = strtok(mnemonic_copy, " ");
    while (token != NULL && word_count < 24) {
        words[word_count++] = token;
        token = strtok(NULL, " ");
    }

    if (word_count != 12 && word_count != 15 && word_count != 18 &&
        word_count != 21 && word_count != 24) {
        snprintf(error_reason, reason_len, "Invalid number of words (%d)", word_count);
        free(mnemonic_copy);
        return 0;
    }

    int indices[24];
    for (int i = 0; i < word_count; i++) {
        indices[i] = find_word_index(words[i]);
        if (indices[i] == -1) {
            snprintf(error_reason, reason_len, "Invalid word '%s'", words[i]);
            free(mnemonic_copy);
            return 0;
        }
    }

    free(mnemonic_copy);

    int total_bits = word_count * 11;
    int checksum_bits = word_count / 3;
    int entropy_bits = total_bits - checksum_bits;
    int entropy_bytes = entropy_bits / 8;

    unsigned char combined_data[33] = {0};
    int bit_offset = 0;

    for (int i = 0; i < word_count; i++) {
        unsigned int index = indices[i];
        for (int j = 10; j >= 0; j--) {
            if ((index >> j) & 1) {
                combined_data[bit_offset / 8] |= (1 << (7 - (bit_offset % 8)));
            }
            bit_offset++;
        }
    }

    unsigned char entropy[32] = {0};
    memcpy(entropy, combined_data, entropy_bytes);
    if (entropy_bits % 8 != 0) {
        entropy[entropy_bytes - 1] &= (0xFF << (8 - (entropy_bits % 8)));
    }

    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(entropy, entropy_bytes, hash);

    unsigned char calculated_checksum = hash[0] >> (8 - checksum_bits);

    unsigned char provided_checksum = 0;
    int current_bit = entropy_bits;
    for (int i = 0; i < checksum_bits; i++) {
        int byte_idx = current_bit / 8;
        int bit_idx_in_byte = current_bit % 8;
        if ((combined_data[byte_idx] >> (7 - bit_idx_in_byte)) & 1) {
            provided_checksum |= (1 << (checksum_bits - 1 - i));
        }
        current_bit++;
    }

    explicit_bzero(combined_data, sizeof(combined_data));
    explicit_bzero(entropy, sizeof(entropy));

    if (provided_checksum != calculated_checksum) {
        snprintf(error_reason, reason_len, "Checksum mismatch");
        return 0;
    }

    return 1;
}

int main(int argc, char *argv[]) {
    // Handle --help flag
    if (argc == 2 && strcmp(argv[1], "--help") == 0) {
        print_usage(argv[0]);
        return 0;
    }

    // Handle --validate flag
    if (argc == 3 && strcmp(argv[1], "--validate") == 0) {
        char reason[128] = {0};
        if (validate_mnemonic(argv[2], reason, sizeof(reason))) {
            printf("Valid BIP39 mnemonic\n");
            return 0;
        } else {
            fprintf(stderr, "Invalid BIP39 mnemonic: %s\n", reason);
            return 1;
        }
    }

    // Handle --password flag
    if (argc == 3 && strcmp(argv[1], "--password") == 0) {
        int word_count = atoi(argv[2]);
        if (word_count < 1 || word_count > 100) {
            fprintf(stderr, "Invalid word count for password mode. Must be 1-100\n");
            return 1;
        }

        char password[2048] = {0};
        generate_password(wordlist, word_count, password, sizeof(password));

        printf("%s\n", password);

        explicit_bzero(password, sizeof(password));
        return 0;
    }

    // Handle BIP39 mode (default)
    if (argc != 2) {
        print_usage(argv[0]);
        return 1;
    }

    int word_count = atoi(argv[1]);
    if (word_count != 12 && word_count != 15 && word_count != 18 &&
        word_count != 21 && word_count != 24) {
        fprintf(stderr, "Invalid word count for BIP39 mode. Must be 12, 15, 18, 21, or 24\n");
        fprintf(stderr, "For other word counts, use: %s --password <count>\n", argv[0]);
        return 1;
    }

    char mnemonic[512] = {0};
    generate_mnemonic(wordlist, word_count, mnemonic, sizeof(mnemonic));

    printf("%s\n", mnemonic);

    explicit_bzero(mnemonic, sizeof(mnemonic));

    return 0;
}
