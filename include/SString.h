// SciTE - Scintilla based Text Editor
/** @file SString.h
 ** A simple string class.
 **/
// Copyright 1998-2001 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#ifndef SSTRING_H
#define SSTRING_H

// These functions are implemented because each platform calls them something different
int CompareCaseInsensitive(const char *a, const char *b);
int CompareNCaseInsensitive(const char *a, const char *b, int len);
bool EqualCaseInsensitive(const char *a, const char *b);

// Define another string class.
// While it would be 'better' to use std::string, that doubles the executable size.
// An SString may contain embedded nul characters.

/**
 * Duplicate a C string.
 * Allocate memory of the given size, or big enough to fit the string if length isn't given;
 * then copy the given string in the allocated memory.
 * @return the pointer to the new string
 */
inline char *StringDup(
	const char *s,	///< The string to duplicate
	int len=-1)		///< The length of memory to allocate. Optional.
{
	if (!s)
		return 0;
	if (len == -1)
		len = strlen(s);
	char *sNew = new char[len + 1];
	if (sNew) {
		strncpy(sNew, s, len);
		sNew[len] = '\0';
	}
	return sNew;
}

/**
 * @brief A simple string class.
 *
 * Hold the length of the string for quick operations,
 * can have a buffer bigger than the string to avoid too many memory allocations and copies.
 * May have embedded zeroes as a result of @a substitute, but rely too heavily on C string
 * functions to allow reliable manipulations of these strings.
 **/
class SString {
	char *s;		///< The C string
	int sSize;		///< The size of the buffer, less 1: ie. the maximum size of the string
	int sLen;		///< The size of the string in s
	int sizeGrowth;	///< Minimum growth size when appending strings
	enum { sizeGrowthDefault = 64 };
	bool grow(int lenNew) {
		while (sizeGrowth * 6 < lenNew)
			sizeGrowth *= 2;
		char *sNew = new char[lenNew + sizeGrowth + 1];
		if (sNew) {
			if (s) {
				memcpy(sNew, s, sLen);
				delete []s;
			}
			s = sNew;
			s[sLen] = '\0';
			sSize = lenNew + sizeGrowth;
		}
		return sNew != 0;
	}
public:
	typedef int size_type;

	SString() : s(0), sSize(0), sLen(0), sizeGrowth(sizeGrowthDefault) {
	}
	SString(const SString &source) : sizeGrowth(sizeGrowthDefault) {
		s = StringDup(source.s);
		sSize = sLen = (s) ? strlen(s) : 0;
	}
	SString(const char *s_) : sizeGrowth(sizeGrowthDefault) {
		s = StringDup(s_);
		sSize = sLen = (s) ? strlen(s) : 0;
	}
	SString(const char *s_, int first, int last) : sizeGrowth(sizeGrowthDefault) {
		s = StringDup(s_ + first, last - first);
		sSize = sLen = (s) ? strlen(s) : 0;
	}
	SString(int i) : sizeGrowth(sizeGrowthDefault) {
		char number[32];
		sprintf(number, "%0d", i);
		s = StringDup(number);
		sSize = sLen = (s) ? strlen(s) : 0;
	}
	SString(double d) : sizeGrowth(sizeGrowthDefault) {
		char number[32];
		sprintf(number, "%.6f", d);
		s = StringDup(number);
		sSize = sLen = (s) ? strlen(s) : 0;
	}
	~SString() {
		delete []s;
		s = 0;
		sSize = 0;
		sLen = 0;
	}
	void clear(void) {
		if (s) {
			*s = '\0';
		}
		sLen = 0;
	}
	/** Size of buffer. */
	size_type size(void) const {	///<
		if (s)
			return sSize;
		else
			return 0;
	}
	/** Size of string in buffer. */
	int length() const {
		return sLen;
	}
	SString &assign(const char *sOther, int sSize_ = -1) {
		if (!sOther) {
			sSize_ = 0;
		}
		if (sSize_ < 0) {
			sSize_ = strlen(sOther);
		}
		if (sSize > 0 && sSize_ <= sSize) {	// Does not allocate new buffer if the current is big enough
			if (s && sSize_) {
				strncpy(s, sOther, sSize_);
			}
			s[sSize_] = '\0';
			sLen = sSize_;
		} else {
			delete []s;
			s = StringDup(sOther, sSize_);
			if (s) {
				sSize = sSize_;	// Allow buffer bigger than real string, thus providing space to grow
				sLen = strlen(s);
			} else {
				sSize = sLen = 0;
			}
		}
		return *this;
	}
	SString &assign(const SString& sOther, int sSize_ = -1) {
		return assign(sOther.s, sSize_);
	}
	SString &operator=(const char *source) {
		return assign(source);
	}
	SString &operator=(const SString &source) {
		if (this != &source) {
			assign(source.c_str());
		}
		return *this;
	}
	bool operator==(const SString &sOther) const {
		if ((s == 0) && (sOther.s == 0))
			return true;
		if ((s == 0) || (sOther.s == 0))
			return false;
		return strcmp(s, sOther.s) == 0;
	}
	bool operator!=(const SString &sOther) const {
		return !operator==(sOther);
	}
	bool operator==(const char *sOther) const {
		if ((s == 0) && (sOther == 0))
			return true;
		if ((s == 0) || (sOther == 0))
			return false;
		return strcmp(s, sOther) == 0;
	}
	bool operator!=(const char *sOther) const {
		return !operator==(sOther);
	}
	bool contains(char ch) {
		if (s && *s)
			return strchr(s, ch) != 0;
		else
			return false;
	}
	void setsizegrowth(int sizeGrowth_) {
		sizeGrowth = sizeGrowth_;
	}
	const char *c_str() const {
		if (s)
			return s;
		else
			return "";
	}
	/** Give ownership of buffer to caller which must use delete[] to free buffer. */
	char *detach() {
		char *sRet = s;
		s = 0;
		sSize = 0;
		sLen = 0;
		return sRet;
	}
	char operator[](int i) const {
		if (s && i < sSize)	// Or < sLen? Depends on the use, both are OK
			return s[i];
		else
			return '\0';
	}
	SString &append(const char *sOther, int sLenOther=-1, char sep=0) {
		if (sLenOther < 0) {
			sLenOther = strlen(sOther);
		}
		int lenSep = 0;
		if (sLen && sep) {	// Only add a separator if not empty
			lenSep = 1;
		}
		int lenNew = sLen + sLenOther + lenSep;
		// Conservative about growing the buffer: don't do it, unless really needed
		if ((lenNew + 1 < sSize) || (grow(lenNew))) {
			if (lenSep) {
				s[sLen] = sep;
				sLen++;
			}
			strncpy(&s[sLen], sOther, sLenOther);
			sLen += sLenOther;
			s[sLen] = '\0';
		}
		return *this;
	}
	SString &operator+=(const char *sOther) {
		return append(sOther, -1);
	}
	SString &operator+=(const SString &sOther) {
		return append(sOther.s, sOther.sSize);
	}
	SString &operator+=(char ch) {
		return append(&ch, 1);
	}
	SString &appendwithseparator(const char *sOther, char sep) {
		return append(sOther, strlen(sOther), sep);
	}
	SString &insert(int pos, const char *sOther, int sLenOther=-1) {
		if (sLenOther < 0)
			sLenOther = strlen(sOther);
		int lenNew = sLen + sLenOther;
		// Conservative about growing the buffer: don't do it, unless really needed
		if ((lenNew + 1 < sSize) || (grow(lenNew))) {
			int moveChars = sLen-pos;
			for (int i=moveChars; i>=0; i--)
				s[lenNew-moveChars+i] = s[pos+i];
			memcpy(s + pos, sOther, sLenOther);
			sLen += sLenOther;
		}
		return *this;
	}
	void remove(int pos, int len=1) {
		for (int i=pos; i<sLen-len+1; i++)
			s[i] = s[i+len];
		sLen -= len;
	}
	int value() const {
		if (s)
			return atoi(s);
		else
			return 0;
	}
	int search(const char *sFind, int start=0) {
		if ((start >= 0) && (start < sLen)) {
			const char *sFound = strstr(s+start, sFind);
			if (sFound)
				return sFound - s;
		}
		return -1;
	}
	bool contains(const char *sFind) {
		return search(sFind) >= 0;
	}
	int substitute(char chFind, char chReplace) {
		int c = 0;
		char *t = s;
		while (t) {
			t = strchr(t, chFind);
			if (t) {
				*t = chReplace;
				t++;
				c++;
			}
		}
		return c;
	}
	int substitute(const char *sFind, const char *sReplace) {
		int c = 0;
		int lenFind = strlen(sFind);
		int lenReplace = strlen(sReplace);
		int posFound = search(sFind);
		while (posFound >= 0) {
			remove(posFound, lenFind);
			insert(posFound, sReplace, lenReplace);
			posFound = search(sFind, posFound + lenReplace);
			c++;
		}
		return c;
	}
	int remove(const char *sFind) {
		return substitute(sFind, "");
	}
};

#endif
