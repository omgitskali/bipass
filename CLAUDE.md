# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

**bipass** is a BIP39 passphrase generator written in C. BIP39 (Bitcoin Improvement Proposal 39) defines a standard for generating mnemonic phrases that can be used to derive cryptographic keys.

## Development Commands

### Prerequisites
- libcurl development headers (`libcurl4-openssl-dev` on Debian/Ubuntu)
- OpenSSL development headers (`libssl-dev` on Debian/Ubuntu)
- GCC or compatible C compiler

### Build
```bash
make
```

### Usage
```bash
./bipass <word_count>
```
Where `word_count` is 12, 15, 18, 21, or 24.

Example:
```bash
./bipass 12    # Generate 12-word seed phrase
./bipass 24    # Generate 24-word seed phrase
```

### Install
```bash
sudo make install    # Installs to /usr/local/bin/
```

### Uninstall
```bash
sudo make uninstall
```

### Clean
```bash
make clean
```

## Architecture Notes

### BIP39 Standard
- BIP39 generates mnemonic phrases from entropy (128-256 bits)
- Standard wordlist contains 2048 words
- Mnemonic length: 12, 15, 18, 21, or 24 words
- Includes checksum validation

### Implementation Details
- Wordlist is fetched remotely from GitHub at runtime (reduces binary size)
- Uses libcurl for HTTP requests to fetch the official BIP39 English wordlist
- Entropy is generated using `/dev/urandom` for cryptographic security
- SHA256 checksum calculation uses OpenSSL
- Sensitive data (mnemonic) is cleared from memory using `explicit_bzero()`

### Security Considerations
- This is cryptographic code dealing with key generation
- Entropy source must be cryptographically secure (use `/dev/urandom` or equivalent)
- Never log or expose generated mnemonics
- Memory containing sensitive data should be zeroed after use
- Be mindful of timing attacks when validating checksums or passphrases

### C Implementation Considerations
- Avoid buffer overflows when handling word arrays
- Use constant-time comparison for checksum validation
- Properly handle memory allocation/deallocation
- Consider using `explicit_bzero()` or `memset_s()` to clear sensitive data

## Important Resources

- BIP39 Specification: https://github.com/bitcoin/bips/blob/master/bip-0039.mediawiki
- Standard wordlist: https://github.com/bitcoin/bips/blob/master/bip-0039/english.txt

---

## Using Gemini CLI for Large Codebase Analysis

When analyzing large codebases or multiple files that might exceed context limits, use the Gemini CLI with its massive context window. Use `gemini -p` to leverage Google Gemini's large context capacity.

### File and Directory Inclusion Syntax

Use the `@` syntax to include files and directories in your Gemini prompts. The paths should be relative to WHERE you run the gemini command:

#### Examples:

**Single file analysis:**
```bash
gemini -p "@src/main.py Explain this file's purpose and structure"
```

**Multiple files:**
```bash
gemini -p "@package.json @src/index.js Analyze the dependencies used in the code"
```

**Entire directory:**
```bash
gemini -p "@src/ Summarize the architecture of this codebase"
```

**Multiple directories:**
```bash
gemini -p "@src/ @tests/ Analyze test coverage for the source code"
```

**Current directory and subdirectories:**
```bash
gemini -p "@./ Give me an overview of this entire project"
```

**Or use --all_files flag:**
```bash
gemini --all_files -p "Analyze the project structure and dependencies"
```

### Implementation Verification Examples

**Check if a feature is implemented:**
```bash
gemini -p "@src/ @lib/ Has dark mode been implemented in this codebase? Show me the relevant files and functions"
```

**Verify authentication implementation:**
```bash
gemini -p "@src/ @middleware/ Is JWT authentication implemented? List all auth-related endpoints and middleware"
```

**Check for specific patterns:**
```bash
gemini -p "@src/ Are there any React hooks that handle WebSocket connections? List them with file paths"
```

**Verify error handling:**
```bash
gemini -p "@src/ @api/ Is proper error handling implemented for all API endpoints? Show examples of try-catch blocks"
```

**Check for rate limiting:**
```bash
gemini -p "@backend/ @middleware/ Is rate limiting implemented for the API? Show the implementation details"
```

**Verify caching strategy:**
```bash
gemini -p "@src/ @lib/ @services/ Is Redis caching implemented? List all cache-related functions and their usage"
```

**Check for specific security measures:**
```bash
gemini -p "@src/ @api/ Are SQL injection protections implemented? Show how user inputs are sanitized"
```

**Verify test coverage for features:**
```bash
gemini -p "@src/payment/ @tests/ Is the payment processing module fully tested? List all test cases"
```

### When to Use Gemini CLI

Use `gemini -p` when:
- Analyzing entire codebases or large directories
- Comparing multiple large files
- Need to understand project-wide patterns or architecture
- Current context window is insufficient for the task
- Working with files totaling more than 100KB
- Verifying if specific features, patterns, or security measures are implemented
- Checking for the presence of certain coding patterns across the entire codebase

### Important Notes

- Paths in `@` syntax are relative to your current working directory when invoking gemini
- The CLI will include file contents directly in the context
- No need for --yolo flag for read-only analysis
- Gemini's context window can handle entire codebases that would overflow Claude's context
- When checking implementations, be specific about what you're looking for to get accurate results
