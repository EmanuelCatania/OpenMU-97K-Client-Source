#include "stdafx.h"
#include "ProtectionManager.h"
#include "ProtectionScheme.h"
#include <cstring>
#include <algorithm>
#include <vector>

// Global singleton
std::unique_ptr<ProtectionManager> g_ProtectionManager;

// ============================================================================
// ProtectionManager Implementation
// ============================================================================

ProtectionManager::ProtectionManager()
    : m_scheme(ProtectionSchemeFactory::CreateScheme())
{
}

ProtectionManager::~ProtectionManager()
{
    // Explicit destructor for unique_ptr cleanup
}

bool ProtectionManager::ReadAndValidateConfigFile(const char* filename, void* outData, uint32_t maxSize)
{
    if (!filename || !outData || maxSize == 0)
        return false;

    HANDLE file = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE)
        return false;

    DWORD fileSize = GetFileSize(file, nullptr);
    if (fileSize == INVALID_FILE_SIZE || fileSize > maxSize)
    {
        CloseHandle(file);
        return false;
    }

    DWORD bytesRead = 0;
    if (!ReadFile(file, outData, fileSize, &bytesRead, nullptr) || bytesRead != fileSize)
    {
        CloseHandle(file);
        return false;
    }

    CloseHandle(file);

    // ? IMPORTANTE: Desencriptar los datos después de leer
    // El archivo está encriptado con XOR, necesitamos desencriptarlo
    m_scheme->EncryptDecryptData((uint8_t*)outData, fileSize);

    return true;
}

bool ProtectionManager::CalculateFileChecksum(const char* filename, uint32_t* outChecksum)
{
    if (!filename || !outChecksum)
        return false;

    // Validar que el string no esté vacío
    if (strlen(filename) == 0)
        return false;

    HANDLE file = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    
    if (file == INVALID_HANDLE_VALUE)
    {
        // Obtener error detallado para debugging
        DWORD error = GetLastError();
        // Descomentar para logging si es necesario:
        // printf("Failed to open file '%s': Error code %lu\n", filename, error);
        return false;
    }

    DWORD fileSize = GetFileSize(file, nullptr);
    if (fileSize == INVALID_FILE_SIZE || fileSize == 0)
    {
        CloseHandle(file);
        return false;
    }

    // Leer archivo completo en memoria
    std::vector<uint8_t> buffer;
    try
    {
        buffer.resize(fileSize);
    }
    catch (...)
    {
        CloseHandle(file);
        return false;  // Fallo al asignar memoria
    }

    DWORD bytesRead = 0;
    if (!ReadFile(file, buffer.data(), fileSize, &bytesRead, nullptr) || bytesRead != fileSize)
    {
        CloseHandle(file);
        return false;
    }

    CloseHandle(file);

    // Calcular checksum usando el esquema
    *outChecksum = m_scheme->CalculateChecksum(buffer.data(), fileSize);
    return true;
}

bool ProtectionManager::ValidateFileIntegrity(const char* filename, uint32_t expectedChecksum)
{
    uint32_t calculatedChecksum = 0;
    
    if (!CalculateFileChecksum(filename, &calculatedChecksum))
        return false;

    return calculatedChecksum == expectedChecksum;
}

void ProtectionManager::ProcessBuffer(uint8_t* buffer, uint32_t size)
{
    if (m_scheme)
        m_scheme->EncryptDecryptData(buffer, size);
}

const char* ProtectionManager::GetCurrentScheme() const
{
    return m_scheme ? m_scheme->GetSchemeName() : "OpenMU";
}

// ============================================================================
// Global Functions
// ============================================================================

void InitializeProtectionManager()
{
    g_ProtectionManager = std::make_unique<ProtectionManager>();
}

void ShutdownProtectionManager()
{
    g_ProtectionManager.reset();
}
