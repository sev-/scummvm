/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef BACKENDS_PRINTING_PRINTMAN_WIN32_HGLOBAL_H
#define BACKENDS_PRINTING_PRINTMAN_WIN32_HGLOBAL_H

#ifdef WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace Win32 {

template<class T>
class HGlobalPtr {
public:
	HGlobalPtr() : objPtr(nullptr) {}
	~HGlobalPtr() {
		if (objPtr) {
			GlobalUnlock(GlobalHandle(objPtr));
		}
	}

	HGlobalPtr(const HGlobalPtr &old) {
		if (old.objPtr) {
			HGLOBAL hGlob = GlobalHandle(old.objPtr);
			objPtr = (T *)GlobalLock(hGlob);
		} else {
			objPtr = nullptr;
		}
	}

	HGlobalPtr(HGlobalPtr &&old) : objPtr(old.objPtr) {
		old.objPtr = nullptr;
	}

	T* operator->() const {
		return objPtr;
	}
	T& operator*() const {
		return *objPtr;
	}

	HGlobalPtr &operator=(const HGlobalPtr& old) {
		if (objPtr) {
			GlobalUnlock(GlobalHandle(objPtr));
		}
		if (old.objPtr) {
			HGLOBAL hGlob = GlobalHandle(old.objPtr);
			objPtr = (T *)GlobalLock(hGlob);
		} else {
			objPtr = nullptr;
		}
		return *this;
	}

	HGlobalPtr &operator=(HGlobalPtr &&old) {
		if (objPtr) {
			GlobalUnlock(GlobalHandle(objPtr));
		}

		if (old.objPtr) {
			HGLOBAL hGlob = GlobalHandle(old.objPtr);
			objPtr = (T *)GlobalLock(hGlob);
		} else {
			objPtr = nullptr;
		}

		old.objPtr = nullptr;

		return *this;
	}

	bool operator==(const HGlobalPtr &other) const {
		return other.objPtr == objPtr;
	}
	bool operator!=(const HGlobalPtr &other) const {
		return other.objPtr != objPtr;
	}

	T *get() const { return objPtr; }

	operator T*() const { return objPtr; }

private:
	HGlobalPtr(HGLOBAL hGlob) {
		objPtr = (T *)GlobalLock(hGlob);
	}
	T *objPtr;

	template<class T>
	friend class HGlobalObject;
};

template <class T> class HGlobalObject {
public:
	HGlobalPtr<T> lock() { return HGlobalPtr<T>(hGlob); }

	explicit HGlobalObject(HGLOBAL hGlob, bool owned=true) : hGlob(hGlob), owned(owned) {};

	HGlobalObject(UINT flags, size_t newSize, bool owned = true) : owned(owned) {
		hGlob = GlobalAlloc(flags, newSize);
	}

	explicit HGlobalObject(UINT flags, bool owned = true) : owned(owned) {
		hGlob = GlobalAlloc(flags, sizeof(T));
	}

	HGlobalObject(HGlobalObject&& old) : hGlob(old.hGlob), owned(old.owned) {
		if (old.owned) {
			old.hGlob = NULL;
			old.owned = false;
		}
	}

	void operator=(HGlobalObject && old) {
		if (owned) {
			GlobalFree(hGlob);
		}

		hGlob = old.hGlob;
		owned = old.owned;

		if (old.owned) {
			old.hGlob = NULL;
			old.owned = false;
		}

	}

	void set(HGLOBAL newHGlob, bool takeOwnership = true) {
		if (owned) {
			GlobalFree(hGlob);
		}
		hGlob = newHGlob;
		owned = takeOwnership;
	}

	HGLOBAL get() const { return hGlob; }

	void realloc(size_t newSize, bool zeroInit = false) {
		HGLOBAL newHGlob = GlobalReAlloc(hGlob, newSize, zeroInit? GMEM_ZEROINIT : 0);

		if (newHGlob) {
			hGlob = newHGlob;
		}
	}

	HGlobalPtr<T> lock() const {
		return HGlobalPtr<T>(hGlob);
	}

	~HGlobalObject() {
		if (owned) {
			GlobalFree(hGlob);
		}
	}

private:
	HGLOBAL hGlob;
	bool owned;
};



} // namespace Win32

#endif
#endif
