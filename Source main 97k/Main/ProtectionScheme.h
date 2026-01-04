#pragma once

#include <cstdint>
#include <memory>

/**
 * @brief Interface for protection schemes
 * Current implementation supports OpenMU standard protection
 */
class IProtectionScheme
{
public:
    virtual ~IProtectionScheme() = default;

    /**
     * @brief Calculate checksum for data validation
     * @param data Buffer containing data
     * @param size Size of buffer
     * @param key Optional key for encryption-aware checksums
     * @return Calculated checksum
     */
    virtual uint32_t CalculateChecksum(const uint8_t* data, uint32_t size, uint16_t key = 0) = 0;

    /**
     * @brief Validate data integrity
     * @param data Buffer containing data
     * @param size Size of buffer
     * @param expectedChecksum Expected checksum value
     * @param key Optional key for encryption-aware checksums
     * @return True if checksum matches, false otherwise
     */
    virtual bool ValidateChecksum(const uint8_t* data, uint32_t size, uint32_t expectedChecksum, uint16_t key = 0) = 0;

    /**
     * @brief Encrypt/Decrypt data with XOR operation
     * @param buffer Data to encrypt/decrypt
     * @param size Size of data
     */
    virtual void EncryptDecryptData(uint8_t* buffer, uint32_t size) = 0;

    /**
     * @brief Get scheme identifier
     * @return Human-readable name of the scheme
     */
    virtual const char* GetSchemeName() const = 0;
};

/**
 * @brief OpenMU standard protection scheme
 * Uses 3-byte XOR key (0xFC, 0xCF, 0xAB) as per OpenMU protocol
 */
class OpenMUProtectionScheme : public IProtectionScheme
{
public:
    uint32_t CalculateChecksum(const uint8_t* data, uint32_t size, uint16_t key = 0) override;
    bool ValidateChecksum(const uint8_t* data, uint32_t size, uint32_t expectedChecksum, uint16_t key = 0) override;
    void EncryptDecryptData(uint8_t* buffer, uint32_t size) override;
    const char* GetSchemeName() const override { return "OpenMU"; }

private:
    static constexpr uint8_t XOR_KEY[3] = { 0xFC, 0xCF, 0xAB };
};

/**
 * @brief Factory for creating protection schemes
 */
class ProtectionSchemeFactory
{
public:
    /**
     * @brief Create a new OpenMU protection scheme instance
     * @return Unique pointer to OpenMUProtectionScheme
     */
    static std::unique_ptr<IProtectionScheme> CreateScheme();
};
