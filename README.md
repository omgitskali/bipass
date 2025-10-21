# bipass

A simple BIP39 seed phrase generator written in C that generates cryptographically secure mnemonic phrases.

## Features

- Generates BIP39-compliant mnemonic phrases
- Supports 12, 15, 18, 21, and 24-word phrases
- Uses cryptographically secure entropy from `/dev/urandom`
- Fetches official BIP39 wordlist from GitHub at runtime
- Minimal binary size (< 20KB)
- Memory-safe with proper cleanup of sensitive data

## Installation

### Prerequisites

- OpenSSL development headers (`libssl-dev` on Debian/Ubuntu)
- GCC or compatible C compiler
- `curl` to generate the wordlist during the build (can be removed after first build)

### Build from source

```bash
make
sudo make install
```

## Usage

bipass supports two modes: **BIP39 mode** for cryptocurrency wallets and **Password mode** for secure passphrases.

### BIP39 Mode (Cryptocurrency Wallets)

Generate BIP39-compliant mnemonic phrases with checksum validation:

```bash
bipass <word_count>
```

Where `word_count` is 12, 15, 18, 21, or 24.

**Examples:**

```bash
# Generate a 12-word BIP39 mnemonic
bipass 12

# Generate a 24-word BIP39 mnemonic
bipass 24
```

### Password Mode (Secure Passphrases)

Generate random word-based passwords without BIP39 checksum (1-100 words):

```bash
bipass --password <word_count>
```

**Examples:**

```bash
# Generate a 5-word password
bipass --password 5

# Generate a 10-word passphrase
bipass --password 10
```

**Note:** Password mode generates cryptographically secure random words but does NOT include BIP39 checksum validation. Do not use password mode for cryptocurrency wallets.

### Validation

Validate existing BIP39 mnemonic phrases:

```bash
bipass --validate "word1 word2 word3..."
```

**Example:**

```bash
# Validate a 12-word mnemonic
bipass --validate "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about"
```

### Help

Display usage information:

```bash
bipass --help
```

## Security Notes

- This tool generates real cryptographic material. Use it responsibly.
- Generated phrases should be stored securely and never shared.
- The entropy source (`/dev/urandom`) is cryptographically secure on modern Linux systems.
- Memory containing sensitive data is cleared using `explicit_bzero()`.

## License

MIT
