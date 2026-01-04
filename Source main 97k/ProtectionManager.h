#pragma once

class IProtectionScheme {
public:
	virtual ~IProtectionScheme() = default;
	virtual DWORD CalculateChecksum(const BYTE* data, DWORD size) = 0;
	virtual bool ValidateChecksum(const BYTE* data, DWORD size, DWORD expectedChecksum) = 0;
};

class OpenMUProtectionScheme : public IProtectionScheme {
public:
	DWORD CalculateChecksum(const BYTE* data, DWORD size) override;
	bool ValidateChecksum(const BYTE* data, DWORD size, DWORD expectedChecksum) override;
};

class ProtectionManager {
private:
	IProtectionScheme* m_scheme;
public:
	ProtectionManager(IProtectionScheme* scheme) : m_scheme(scheme) {}
	~ProtectionManager() { delete m_scheme; }
	
	DWORD GetChecksum(const BYTE* data, DWORD size) {
		return m_scheme->CalculateChecksum(data, size);
	}
};