# PC Principal PGP

A secure, portable PGP encryption application designed for privacy-conscious users who need powerful encryption on the go.

![PC Principal PGP Icon](resources/icon.png)

## Features

### Core PGP Functionality
- **Generate PGP Keys** - Create RSA key pairs (2048, 3072, or 4096 bits)
- **Import/Export Keys** - Support for `.asc`, `.gpg`, `.pgp`, `.pub`, `.key` formats
- **Encrypt/Decrypt Messages** - Secure text encryption with ASCII armor support
- **Sign & Verify** - Create and verify digital signatures
- **File Operations** - Encrypt, decrypt, sign, and verify files

### Profile Management
- **Contact Profiles** - Create profiles for friends/contacts with their public keys
- **Mass Import** - Import entire folders of keys with automatic pair matching
- **Trust Management** - Set trust levels for contacts
- **Quick Select** - Easy profile selection for encryption operations

### Security Features
- **Post-Quantum Encryption** - XChaCha20-Poly1305 with Argon2id key derivation
- **Password Protection** - Secure your keys with strong passwords (24+ chars recommended)
- **Encrypted Storage** - Option to encrypt all data or just private keys
- **Secure Memory** - libsodium secure memory for sensitive operations

### Portability
- **100% Portable** - Runs from USB without installation
- **Directory Mode** - Keys stored next to executable (perfect for USB)
- **Partial Portable** - Choose custom locations for public/private keys
- **Self-Contained** - All dependencies included

## Quick Start - Just Double Click!

### Prerequisites
1. **Install MSYS2** (one-time setup)
   - Download from https://www.msys2.org/
   - Run the installer
   - Follow the setup instructions

### Build Instructions (Super Simple!)

1. **Double-click `compile.bat`**
   - That's it! The script will:
     - Find your MSYS2 installation automatically
     - Install all required dependencies
     - Build the application
     - Deploy all necessary files
     - Open the output folder when done

2. **Get your app**
   - After building, you'll find `PCPrincipalPGP_Portable` folder
   - Copy this folder anywhere (USB, external drive, etc.)
   - Run `PCPrincipalPGP.exe` - no installation needed!

### Alternative Build Methods

If you prefer to build manually in MSYS2:

```bash
# Open MSYS2 MinGW 64-bit terminal
cd /path/to/PCPrincipalPGP
./build_internal.sh
```

Or use the simpler batch (if already in MSYS2):
```bash
./build_simple.bat
```

## Usage

### First Run

1. **Launch** `PCPrincipalPGP.exe`
2. **Setup Wizard** will guide you through:
   - Choose portable mode (Directory or Partial)
   - Set key storage locations
   - Configure post-quantum encryption
   - Set password protection (optional)

### Basic Operations

#### Encrypt a Message
1. Select recipient profile from dropdown
2. Type or paste message in Notepad
3. Click **Encrypt** button
4. Encrypted message appears - copy and send

#### Decrypt a Message
1. Paste encrypted message in Notepad
2. Click **Decrypt** button
3. Enter passphrase if required
4. Decrypted message appears

#### Sign a Message
1. Select your signing profile
2. Type message in Notepad
3. Click **Sign** button
4. Signature is appended to message

#### Encrypt a File
1. Go to **Encryption** → **Encrypt File...**
2. Select file to encrypt
3. Choose recipient profile
4. Save encrypted file

### Key Management

#### Generate New Key
1. Go to **Keys** → **Manage Keys...**
2. Click **Generate New Key**
3. Enter name, email, and optional passphrase
4. Select key length (4096 recommended)
5. Click **Generate**

#### Import Keys
1. Go to **Keys** → **Manage Keys...**
2. Click **Import Key** for single key
3. Or **Mass Import** for entire folders
4. Keys are validated and imported

#### Export Keys
1. Select key in Key Manager
2. Click **Export Public** or **Export Private**
3. Choose save location
4. **WARNING**: Keep private keys secure!

### Profile Management

#### Create Profile
1. Go to **Keys** → **New Profile...**
2. Enter contact information
3. Associate with their public key
4. Set trust level

#### Mass Import Profiles
1. Go to **Keys** → **Mass Import...**
2. Select public keys directory
3. Select private keys directory (optional)
4. Enable "Match key pairs" for validation
5. Click **Start Import**

## Security Recommendations

### Passwords
- Use **24+ character passwords** for post-quantum security
- Include uppercase, lowercase, numbers, and symbols
- Use a password manager
- Never reuse passwords

### Key Management
- Keep private keys on encrypted storage
- Backup keys regularly
- Use expiration dates on keys
- Revoke compromised keys immediately

### Portable Usage
- Use **Directory Mode** for USB drives
- Encrypt the entire USB drive if possible
- Never leave USB unattended
- Use password protection

## File Structure

```
PCPrincipalPGP_Portable/
├── PCPrincipalPGP.exe      # Main executable
├── icon.png                # Application icon
├── config.json             # Configuration file
├── secure_storage.bin      # Encrypted storage (if enabled)
├── keys/
│   ├── public/             # Public keys
│   └── private/            # Private keys
├── profiles/               # Contact profiles
├── gpg/                    # GPG home directory
└── temp/                   # Temporary files
```

## Technical Details

### Cryptography
- **PGP**: GPGME library (OpenPGP standard)
- **Symmetric**: XChaCha20-Poly1305 (post-quantum)
- **Key Derivation**: Argon2id
- **Random**: libsodium secure random

### Algorithms
- **RSA**: 2048, 3072, 4096 bit keys
- **Hashing**: SHA-256, SHA-512
- **Encryption**: AES-256, ChaCha20

### Security Features
- Constant-time operations where possible
- Secure memory clearing with sodium_memzero
- Password strength indicator
- Automatic key validation

## Troubleshooting

### Build Issues

**"MSYS2 not found"**
- Install MSYS2 from https://www.msys2.org/
- Restart your computer after installation
- Try running compile.bat again

**"CMake not found"**
```bash
pacman -S mingw-w64-x86_64-cmake
```

**"Qt6 not found"**
```bash
pacman -S mingw-w64-x86_64-qt6-base
```

**"GPGME errors"**
```bash
pacman -S mingw-w64-x86_64-gpgme
```

### Runtime Issues

**"Failed to initialize PGP manager"**
- Ensure `gpg.exe` is in the same directory
- Check that GPG home directory is writable

**"Decryption failed"**
- Verify you have the correct private key
- Check passphrase is correct
- Ensure key is not expired

**"Encryption failed"**
- Verify recipient's public key is imported
- Check recipient key is valid and not expired

## Command Line Usage

```bash
PCPrincipalPGP.exe [options]

Options:
  -e, --encrypt <file>     Encrypt a file
  -d, --decrypt <file>     Decrypt a file
  -s, --sign <file>        Sign a file
  -v, --verify <file>      Verify a signature
  -r, --recipient <keyid>  Recipient for encryption
  -k, --key <keyid>        Key for signing
  -o, --output <file>      Output file
  -i, --import <file>      Import a key file
  -p, --profile <name>     Use profile for operation
  --setup                  Run setup wizard
  --nogui                  Run without GUI
  -h, --help               Show help
  --version                Show version
```

## Build Scripts Explained

| Script | Purpose |
|--------|---------|
| `compile.bat` | **MAIN SCRIPT** - Double-click this! Auto-finds MSYS2 and builds everything |
| `build_internal.sh` | The actual build script that runs inside MSYS2 |
| `build_simple.bat` | Simple wrapper if you're already in MSYS2 |
| `build_msys2.bat` | Full-featured build with all options |

## License

This project is provided as-is for educational and personal use.

## Credits

- **Qt6** - GUI Framework
- **GPGME** - PGP/GPG integration
- **libsodium** - Modern cryptography library
- **PC Principal** - The icon (South Park character)

## Support

For issues, questions, or contributions:
- Check the troubleshooting section
- Review the documentation
- Submit issues with detailed information

---

**Stay secure. Encrypt everything.**
