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

```bash
bipass <word_count>
```

Where `word_count` is 12, 15, 18, 21, or 24.

### Examples

```bash
# Generate a 12-word seed phrase
bipass 12

# Generate a 24-word seed phrase
bipass 24
```

## Security Notes

- This tool generates real cryptographic material. Use it responsibly.
- Generated phrases should be stored securely and never shared.
- The entropy source (`/dev/urandom`) is cryptographically secure on modern Linux systems.
- Memory containing sensitive data is cleared using `explicit_bzero()`.

## License

MIT
