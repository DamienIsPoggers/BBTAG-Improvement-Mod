#pragma once
#include "PaletteManager/impl_format.h"
#include "utils.h"

#define MAX_PAL_INDEX 15
#define TOTAL_PALETTE_FILES 8

extern char* palFileNames[TOTAL_PALETTE_FILES];

enum PaletteFile
{
	PaletteFile_Character,
	PaletteFile_Effect1,
	PaletteFile_Effect2,
	PaletteFile_Effect3,
	PaletteFile_Effect4,
	PaletteFile_Effect5,
	PaletteFile_Effect6,
	PaletteFile_Effect7
};

class CharPaletteHandle
{
	friend class PaletteManager;

	int* m_pCurPalIndex;
	int m_origPalIndex;
	const char* m_pPalBaseAddr;
	IMPL_data_t m_origPalBackup;
	IMPL_data_t m_CurrentPalData;
	int m_switchPalIndex1;
	int m_switchPalIndex2;
	int m_selectedCustomPalIndex;
	bool m_updateLocked;

public:
	void SetPointerPalIndex(int* pPalIdx);
	void SetPointerBasePal(char* pPalBaseAddr);
	bool IsNullPointerPalIndex();
	int& GetPalIndexRef();

private:
	void SetPaletteIndex(int palIndex);
	int GetOrigPalIndex() const;
	void ReplaceAllPalFiles(IMPL_data_t *newPaletteData);
	void ReplaceSinglePalFile(const char* newPalData, PaletteFile palFile);
	void OnMatchInit();
	void OnMatchEnd();
	void LockUpdate();
	void UnlockUpdate();
	int GetSelectedCustomPalIndex();
	void SetSelectedCustomPalIndex(int index);
	const char* GetCurPalFileAddr(PaletteFile palFile);
	const char* GetOrigPalFileAddr(PaletteFile palFile);
	const IMPL_data_t& GetCurrentPalData();
	char* GetPalFileAddr(const char* base, int palIdx, int fileIdx);
	void ReplacePalArrayInMemory(char* Dst, const void* Src);
	void ReplaceAllPalFiles(IMPL_data_t *newPaletteData, int palIdx);
	void BackupCurrentPal();
	void UpdatePalette();
};