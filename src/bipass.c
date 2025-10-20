#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <curl/curl.h>
#include <openssl/sha.h>

#define WORDLIST_URL "https://raw.githubusercontent.com/bitcoin/bips/master/bip-0039/english.txt"
#define MAX_WORDS 2048
#define MAX_WORD_LEN 16

typedef struct {
    char *data;
    size_t size;
} memory_t;

static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    memory_t *mem = (memory_t *)userp;

    char *ptr = realloc(mem->data, mem->size + realsize + 1);
    if (!ptr) {
        fprintf(stderr, "Out of memory\n");
        return 0;
    }

    mem->data = ptr;
    memcpy(&(mem->data[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->data[mem->size] = 0;

    return realsize;
}

char **fetch_wordlist(void) {
    CURL *curl;
    CURLcode res;
    memory_t chunk = {0};

    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Failed to initialize curl\n");
        return NULL;
    }

    curl_easy_setopt(curl, CURLOPT_URL, WORDLIST_URL);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "bipass/1.0");
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        fprintf(stderr, "Failed to fetch wordlist: %s\n", curl_easy_strerror(res));
        free(chunk.data);
        return NULL;
    }

    char **wordlist = malloc(MAX_WORDS * sizeof(char *));
    if (!wordlist) {
        free(chunk.data);
        return NULL;
    }

    int word_count = 0;
    char *line = strtok(chunk.data, "\n");
    while (line && word_count < MAX_WORDS) {
        wordlist[word_count] = strdup(line);
        word_count++;
        line = strtok(NULL, "\n");
    }

    free(chunk.data);

    if (word_count != MAX_WORDS) {
        fprintf(stderr, "Warning: Expected %d words, got %d\n", MAX_WORDS, word_count);
    }

    return wordlist;
}

void free_wordlist(char **wordlist) {
    if (!wordlist) return;
    for (int i = 0; i < MAX_WORDS; i++) {
        free(wordlist[i]);
    }
    free(wordlist);
}

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

void generate_mnemonic(char **wordlist, int word_count, char *output, size_t output_len) {
    int entropy_bits = (word_count * 11) - (word_count / 3);
    int entropy_bytes = entropy_bits / 8;
    int checksum_bits = word_count / 3;

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

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <word_count>\n", argv[0]);
        fprintf(stderr, "Valid word counts: 12, 15, 18, 21, 24\n");
        return 1;
    }

    int word_count = atoi(argv[1]);
    if (word_count != 12 && word_count != 15 && word_count != 18 &&
        word_count != 21 && word_count != 24) {
        fprintf(stderr, "Invalid word count. Must be 12, 15, 18, 21, or 24\n");
        return 1;
    }

    curl_global_init(CURL_GLOBAL_DEFAULT);

    char **wordlist = fetch_wordlist();
    if (!wordlist) {
        curl_global_cleanup();
        return 1;
    }

    char mnemonic[512] = {0};
    generate_mnemonic(wordlist, word_count, mnemonic, sizeof(mnemonic));

    printf("%s\n", mnemonic);

    explicit_bzero(mnemonic, sizeof(mnemonic));
    free_wordlist(wordlist);
    curl_global_cleanup();

    return 0;
}
