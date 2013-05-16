//								-*- c++ -*-

#ifndef VDR_TEXT2SKIN_XML_TYPE_H
#define VDR_TEXT2SKIN_XML_TYPE_H

#include <cstdlib>
#include <string>
#include <vdr/tools.h>
#include <stdio.h>

class cxType {
public:
	enum eType {
		string,
		number,
		boolean
	};

	friend class cxFunction;

private:
	eType       mType;
	std::string mString;
	int         mNumber;
	uint        mUpdateIn;

public:
	cxType(void): mType(boolean), mNumber(0), mUpdateIn(0) {}
	cxType(const char *String): mType(string), mString(String ?: ""), mUpdateIn(0) {}
	cxType(std::string String): mType(string), mString(String), mUpdateIn(0) {}
	cxType(int Number): mType(number), mNumber(Number), mUpdateIn(0) {}
	cxType(time_t Number): mType(number), mNumber(Number), mUpdateIn(0) {}
	cxType(bool Value): mType(boolean), mNumber(Value ? 1 : 0), mUpdateIn(0) {}

	std::string String(void) const;
	int         Number(void) const { return mType == number ? mNumber : atoi(mString.c_str()); }

	void SetUpdate(uint UpdateIn) { mUpdateIn = UpdateIn; }
	uint UpdateIn(void) const { return mUpdateIn; }

	operator std::string () const { return String(); }
	operator int         () const { return Number(); }
	operator bool        () const;

	friend bool operator== (const cxType &a, const cxType &b);
	friend bool operator!= (const cxType &a, const cxType &b);
	friend bool operator<  (const cxType &a, const cxType &b);
	friend bool operator>  (const cxType &a, const cxType &b);
	friend bool operator<= (const cxType &a, const cxType &b);
	friend bool operator>= (const cxType &a, const cxType &b);
};

inline std::string cxType::String(void) const {
	switch (mType) {
	case number:  return (const char*)itoa(mNumber);
	case boolean: return mNumber != 0 ? "1" : "";
	default:      return mString;
	}
}

inline cxType::operator bool () const
{
	switch (mType) {
	case string: return mString != "";
	default:     return mNumber != 0;
	}
}

inline bool operator== (const cxType &a, const cxType &b)
{
	if (a.mType == cxType::string || b.mType == cxType::string)
		return a.String() == b.String();
	return a.mNumber == b.mNumber;
}

inline bool operator!= (const cxType &a, const cxType &b)
{
	if (a.mType == cxType::string || b.mType == cxType::string)
		return a.String() != b.String();
	return a.mNumber != b.mNumber;
}

inline bool operator< (const cxType &a, const cxType &b)
{
	if (a.mType == cxType::string || b.mType == cxType::string)
		return a.String() < b.String();
	return a.mNumber < b.mNumber;
}

inline bool operator> (const cxType &a, const cxType &b)
{
	if (a.mType == cxType::string || b.mType == cxType::string)
		return a.String() > b.String();
	return a.mNumber > b.mNumber;
}

inline bool operator<= (const cxType &a, const cxType &b)
{
	if (a.mType == cxType::string || b.mType == cxType::string)
		return a.String() <= b.String();
	return a.mNumber <= b.mNumber;
}

inline bool operator>= (const cxType &a, const cxType &b)
{
	if (a.mType == cxType::string || b.mType == cxType::string)
		return a.String() >= b.String();
	return a.mNumber >= b.mNumber;
}

#endif // VDR_TEXT2SKIN_XML_TYPE_H
