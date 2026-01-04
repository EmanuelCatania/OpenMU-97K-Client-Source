#include "stdafx.h"
#include "ProtectionScheme.h"
#include <cstring>
#include <algorithm>
#include <memory> // For std::unique_ptr

// ============================================================================
// OpenMU Protection Scheme Implementation
// ============================================================================

// The 20-byte XOR key used by InfoEncoder
static const uint8_t ENCRYPTION_KEY[20] = {
    0x9E, 0x98, 0x12, 0x2B, 0xB6, 0x76, 0xF6, 0x5B,
    0x76, 0x4D, 0x08, 0xF2, 0xAB, 0xDF, 0x91, 0xB6,
    0x4C, 0x1C, 0x93, 0x2F
};

uint32_t OpenMUProtectionScheme::CalculateChecksum(const uint8_t* data, uint32_t size, uint16_t key)
{
    if (!data || size == 0)
        return 0;

    // Use standard CRC-like checksum (simple rotating XOR checksum)
    uint32_t checksum = 0;
    
    for (uint32_t i = 0; i < size; ++i)
    {
        checksum = ((checksum << 1) | (checksum >> 31)) + data[i];
    }

    return checksum;
}

bool OpenMUProtectionScheme::ValidateChecksum(const uint8_t* data, uint32_t size, uint32_t expectedChecksum, uint16_t key)
{
    uint32_t calculated = CalculateChecksum(data, size, key);
    return calculated == expectedChecksum;
}

void OpenMUProtectionScheme::EncryptDecryptData(uint8_t* buffer, uint32_t size)
{
    if (!buffer || size == 0)
        return;

    // Decrypt using the same algorithm as InfoEncoder encryption (reverse operations)
    // InfoEncoder does:
    //   1. ^= (XorKey[n % 20] ^ LOBYTE(n))
    //   2. -= (XorKey[n % 20] ^ HIBYTE(n))
    //   3. += (XorKey[n % 20] ^ HIBYTE(n))
    //
    // To decrypt, we reverse (LIFO):
    //   1. -= (XorKey[n % 20] ^ HIBYTE(n))
    //   2. += (XorKey[n % 20] ^ HIBYTE(n))
    //   3. ^= (XorKey[n % 20] ^ LOBYTE(n))
    //
    // But note: step 1 and 2 cancel out, so we only need step 3

    for (uint32_t i = 0; i < size; ++i)
    {
        uint8_t xorKey = ENCRYPTION_KEY[i % 20];
        uint8_t lowByte = (uint8_t)(i & 0xFF);        // LOBYTE(i)
        uint8_t highByte = (uint8_t)((i >> 8) & 0xFF); // HIBYTE(i)
        
        // Reverse the operations in reverse order
        // The -= and += operations cancel out, so we skip them
        buffer[i] ^= (uint8_t)(xorKey ^ lowByte);
    }
}

// ============================================================================
// Protection Scheme Factory Implementation
// ============================================================================

std::unique_ptr<IProtectionScheme> ProtectionSchemeFactory::CreateScheme()
{
    return std::make_unique<OpenMUProtectionScheme>();
}
