// Copyright 2022 Maximilien (Synock) Guislain


#include "Definitions.h"

FAttributeStruct& FAttributeStruct::operator+=(const FAttributeStruct& OtherAttribute)
{
	Str += OtherAttribute.Str;
	Sta += OtherAttribute.Sta;
	Agi += OtherAttribute.Agi;
	Dex += OtherAttribute.Dex;
	Int += OtherAttribute.Int;
	Wis += OtherAttribute.Wis;
	Cha += OtherAttribute.Cha;

	return *this;
}

int FAttributeStruct::Sum() const
{
	return Str + Sta + Agi + Dex + Int + Wis + Cha;
}

bool FAttributeStruct::IsNotNull() const
{
	return Str != 0 || Sta != 0 ||Agi != 0 ||Int != 0 ||Wis != 0 ||Cha != 0;
}
