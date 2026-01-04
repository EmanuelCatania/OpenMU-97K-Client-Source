#include "stdafx.h"
#include "Protect.h"
#include "ProtectionManager.h"
#include <cstring>
#include <cctype>

CProtect gProtect;

// ============================================================================
// Helper Functions for Filename Validation
// ============================================================================

/**
 * @brief Check if a character is valid for a filename
 */
static bool IsValidFileNameChar(unsigned char ch)
{
    // Allow alphanumeric, dots, underscores, hyphens, and backslashes
    if (isalnum(ch) || ch == '.' || ch == '_' || ch == '-' || ch == '\\' || ch == '/')
        return true;
    return false;
}

/**
 * @brief Validate and sanitize filename
 * Removes invalid characters and checks for proper null termination
 */
static bool ValidateFileName(char* filename, size_t maxLen)
{
    if (!filename)
        return false;

    // Check for proper null termination
    bool hasNullTerminator = false;
    for (size_t i = 0; i < maxLen; i++)
    {
        if (filename[i] == '\0')
        {
            hasNullTerminator = true;
            break;
        }
    }

    if (!hasNullTerminator)
        return false;

    // Check actual length
    size_t len = strlen(filename);
    if (len == 0 || len >= maxLen)
        return false;

    // Validate characters
    for (size_t i = 0; i < len; i++)
    {
        unsigned char ch = (unsigned char)filename[i];
        if (!IsValidFileNameChar(ch))
        {
            return false; // Invalid character found
        }
    }

    return true;
}

/**
 * @brief Safe wrapper for file operations with validation
 */
static bool SafeFileOperation(const char* filename, const char* operationType)
{
    if (!ValidateFileName((char*)filename, MAX_PATH))
    {
        char errorMsg[512];
        snprintf(errorMsg, sizeof(errorMsg),
            "Invalid filename detected for %s operation.\n\nFile: %s\n\n"
            "The filename contains invalid or corrupted characters.\n"
            "Please verify your configuration file.",
            operationType, filename);
        MessageBox(NULL, errorMsg, "Filename Validation Error", MB_OK | MB_ICONERROR);
        return false;
    }

    return true;
}

// ============================================================================
// CProtect Implementation (OpenMU - Refactored)
// ============================================================================

CProtect::CProtect()
    : m_ClientFileCRC(0)
{
    ZeroMemory(&m_MainInfo, sizeof(MAIN_FILE_INFO));
}

CProtect::~CProtect()
{
}

bool CProtect::ReadMainFile(char* name)
{
    if (!name || !g_ProtectionManager)
        return false;

    return g_ProtectionManager->ReadAndValidateConfigFile(
        name, 
        &m_MainInfo, 
        sizeof(MAIN_FILE_INFO)
    );
}

void CProtect::CheckLauncher()
{
    if (this->m_MainInfo.LauncherType == 0)
    {
        return;
    }

    if (this->m_MainInfo.LauncherType == 1)
    {
        HANDLE H = OpenMutex(MUTEX_ALL_ACCESS, 0, this->m_MainInfo.LauncherName);

        if (H == NULL)
        {
            MessageBox(NULL, "Open Game with launcher!", "Error", MB_OK | MB_ICONERROR);

            ExitProcess(0);
        }

        ReleaseMutex(H);

        CloseHandle(H);
    }
    else if (this->m_MainInfo.LauncherType == 2)
    {
        if (FindWindow(NULL, this->m_MainInfo.LauncherName) == NULL)
        {
            MessageBox(NULL, "Open Game with launcher!", "Error", MB_OK | MB_ICONERROR);

            ExitProcess(0);
        }
    }
}

void CProtect::CheckInstance()
{
    if (this->m_MainInfo.MultiInstanceBlock == 0)
    {
        return;
    }

    char buff[256];

    wsprintf(buff, "OPENMU_MAIN_09711_%s", this->m_MainInfo.IpAddress);

    if (OpenMutex(MUTEX_ALL_ACCESS, 0, buff) == NULL)
    {
        CreateMutex(0, 0, buff);
    }
    else
    {
        MessageBox(NULL, "You can only run 1 game at the same time!", "Error", MB_OK | MB_ICONERROR);

        ExitProcess(0);
    }
}

void CProtect::CheckClientFile()
{
    // Si ClientCRC32 es 0, no se requiere verificación
    if (this->m_MainInfo.ClientCRC32 == 0)
    {
        return;
    }

    if (!g_ProtectionManager)
    {
        MessageBox(NULL, "Protection Manager not initialized!", "Error", MB_OK | MB_ICONERROR);
        ExitProcess(0);
    }

    // Validar que el nombre de cliente no esté vacío
    if (strlen(this->m_MainInfo.ClientName) == 0)
    {
        MessageBox(NULL, "Client name is empty in configuration!", "Error", MB_OK | MB_ICONERROR);
        ExitProcess(0);
    }

    // Validar integridad del nombre de archivo
    if (!SafeFileOperation(this->m_MainInfo.ClientName, "client file read"))
    {
        ExitProcess(0);
    }

    // Intentar leer el archivo
    uint32_t clientChecksum = 0;
    if (!g_ProtectionManager->CalculateFileChecksum(this->m_MainInfo.ClientName, &clientChecksum))
    {
        // Error detallado: mostrar qué archivo no se pudo leer
        char errorMsg[512];
        snprintf(errorMsg, sizeof(errorMsg), 
            "Failed to read client file:\n%s\n\nPlease verify:\n1. File exists\n2. Path is correct\n3. Have read permissions", 
            this->m_MainInfo.ClientName);
        MessageBox(NULL, errorMsg, "File Read Error", MB_OK | MB_ICONERROR);

        ExitProcess(0);
    }

    // Validar checksum
    if (this->m_MainInfo.ClientCRC32 != clientChecksum)
    {
        char errorMsg[512];
        snprintf(errorMsg, sizeof(errorMsg), 
            "Client file checksum mismatch!\n\nFile: %s\nExpected: 0x%08X\nActual: 0x%08X", 
            this->m_MainInfo.ClientName,
            this->m_MainInfo.ClientCRC32, 
            clientChecksum);
        MessageBox(NULL, errorMsg, "Checksum Error", MB_OK | MB_ICONERROR);

        ExitProcess(0);
    }
}

void CProtect::CheckPluginFile()
{
    if (this->m_MainInfo.PluginCRC32 == 0)
    {
        return;
    }

    if (!g_ProtectionManager)
    {
        MessageBox(NULL, "Protection Manager not initialized!", "Error", MB_OK | MB_ICONERROR);
        ExitProcess(0);
    }

    // Validar que el nombre de plugin no esté vacío
    if (strlen(this->m_MainInfo.PluginName) == 0)
    {
        MessageBox(NULL, "Plugin name is empty in configuration!", "Error", MB_OK | MB_ICONERROR);
        ExitProcess(0);
    }

    // Validar integridad del nombre de archivo
    if (!SafeFileOperation(this->m_MainInfo.PluginName, "plugin file read"))
    {
        ExitProcess(0);
    }

    uint32_t pluginChecksum = 0;
    if (!g_ProtectionManager->CalculateFileChecksum(this->m_MainInfo.PluginName, &pluginChecksum))
    {
        char errorMsg[512];
        snprintf(errorMsg, sizeof(errorMsg), 
            "Failed to read plugin file:\n%s\n\nPlease verify:\n1. File exists\n2. Path is correct\n3. Have read permissions", 
            this->m_MainInfo.PluginName);
        MessageBox(NULL, errorMsg, "File Read Error", MB_OK | MB_ICONERROR);

        ExitProcess(0);
    }

    if (this->m_MainInfo.PluginCRC32 != pluginChecksum)
    {
        char errorMsg[512];
        snprintf(errorMsg, sizeof(errorMsg), 
            "Plugin file checksum mismatch!\n\nFile: %s\nExpected: 0x%08X\nActual: 0x%08X", 
            this->m_MainInfo.PluginName,
            this->m_MainInfo.PluginCRC32, 
            pluginChecksum);
        MessageBox(NULL, errorMsg, "Checksum Error", MB_OK | MB_ICONERROR);

        ExitProcess(0);
    }

    HMODULE module = GetModuleHandle(this->m_MainInfo.PluginName);

    if (module)
    {
        MessageBox(NULL, "Plugin file already loaded!", "Error", MB_OK | MB_ICONERROR);

        ExitProcess(0);
    }

    module = LoadLibrary(this->m_MainInfo.PluginName);

    if (module == NULL)
    {
        char errorMsg[512];
        snprintf(errorMsg, sizeof(errorMsg), 
            "Failed loading the plugin:\n%s\n\nError code: %lu", 
            this->m_MainInfo.PluginName,
            GetLastError());
        MessageBox(NULL, errorMsg, "Plugin Load Error", MB_OK | MB_ICONERROR);

        ExitProcess(0);
    }

    char filename[MAX_PATH];

    GetModuleFileName(module, filename, MAX_PATH);

    if (!g_ProtectionManager->CalculateFileChecksum(filename, &pluginChecksum))
    {
        MessageBox(NULL, "Failed to read loaded plugin checksum!", "Error", MB_OK | MB_ICONERROR);

        ExitProcess(0);
    }

    if (this->m_MainInfo.PluginCRC32 != pluginChecksum)
    {
        MessageBox(NULL, "Loaded plugin file CRC doesn't match!", "Error", MB_OK | MB_ICONERROR);

        ExitProcess(0);
    }

    void(*EntryProc)() = (void(*)())GetProcAddress(module, "EntryProc");

    if (EntryProc != 0)
    {
        EntryProc();
    }
}