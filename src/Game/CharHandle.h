#pragma once
#include "CharInfo.h"
#include "CharPaletteHandle.h"

class CharHandle
{
	CharInfo** m_charData;
	CharPaletteHandle m_charPalHandler;
	//const char* charName;
	//CharIndex charIndex;

public:
	void SetCharDataPtr(const void* addr);
	bool IsNullPtrCharData() const;
	const CharInfo* Data() const;
	CharPaletteHandle& PalHandle();
};