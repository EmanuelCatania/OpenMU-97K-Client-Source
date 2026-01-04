#pragma once

#include <cstdint>
#include <memory>
#include <windows.h>

class IProtectionScheme;

/**
 * @brief Main protection manager for file integrity and security checks
 * Designed specifically for OpenMU compatibility
 */
class ProtectionManager
{
public:
    /**
     * @brief Constructor - initializes with OpenMU protection scheme
     */
    ProtectionManager();

    ~ProtectionManager();

    // Prevent copying
    ProtectionManager(const ProtectionManager&) = delete;
    ProtectionManager& operator=(const ProtectionManager&) = delete;

    // Allow moving
    ProtectionManager(ProtectionManager&&) = default;
    ProtectionManager& operator=(ProtectionManager&&) = default;

    /**
     * @brief Read and validate configuration file
     * @param filename Path to configuration file
     * @param outData Output buffer for configuration data
     * @param maxSize Maximum size of output buffer
     * @return True if file was successfully read and validated
     */
    bool ReadAndValidateConfigFile(const char* filename, void* outData, uint32_t maxSize);

    /**
     * @brief Calculate file checksum
     * @param filename File path
     * @param outChecksum Output parameter for calculated checksum
     * @return True if calculation was successful
     */
    bool CalculateFileChecksum(const char* filename, uint32_t* outChecksum);

    /**
     * @brief Validate file integrity
     * @param filename File path
     * @param expectedChecksum Expected checksum value
     * @return True if file checksum matches expected value
     */
    bool ValidateFileIntegrity(const char* filename, uint32_t expectedChecksum);

    /**
     * @brief Encrypt/Decrypt data buffer using OpenMU scheme
     * @param buffer Data to process
     * @param size Size of data
     */
    void ProcessBuffer(uint8_t* buffer, uint32_t size);

    /**
     * @brief Get current protection scheme name (always "OpenMU")
     * @return Human-readable scheme name
     */
    const char* GetCurrentScheme() const;

private:
    std::unique_ptr<IProtectionScheme> m_scheme;
};

/**
 * @brief Global protection manager singleton for OpenMU
 * Usage: g_ProtectionManager->ValidateFileIntegrity("client.exe", expectedCRC);
 */
extern std::unique_ptr<ProtectionManager> g_ProtectionManager;

/**
 * @brief Initialize global protection manager with OpenMU scheme
 */
void InitializeProtectionManager();

/**
 * @brief Shutdown global protection manager
 */
void ShutdownProtectionManager();
