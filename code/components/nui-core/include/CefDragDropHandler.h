#pragma once

// When generating projects with CMake the CEF_USE_ATL value will be defined
// automatically if using a supported Visual Studio version. Pass -DUSE_ATL=OFF
// to the CMake command-line to disable use of ATL.
// Uncomment this line to manually enable ATL support.
#define CEF_USE_ATL 1

#if defined(CEF_USE_ATL)

#if __has_include(<atlcomcli.h>)
#include <atlcomcli.h>
#else
#include <cassert>
#include <utility>

template<class T>
class CComPtr {
public:
	T* p;

	constexpr CComPtr() noexcept : p(nullptr) {}
	CComPtr(T* lp) : p(lp) { if (p) p->AddRef(); }
	CComPtr(const CComPtr<T>& lp) : p(lp.p) { if (p) p->AddRef(); }
	CComPtr(CComPtr<T>&& lp) noexcept : p(lp.p) { lp.p = nullptr; }

	~CComPtr() {
		if (p) {
			p->Release();
			p = nullptr;
		}
	}

	operator T*() const noexcept { return p; }
	T& operator*() const noexcept { return *p; }
	T* operator->() const noexcept { return p; }
	bool operator!() const noexcept { return p == nullptr; }

	T** operator&() noexcept {
		assert(p == nullptr && "Address of non-null CComPtr requested; potential COM reference leak");
		return &p;
	}

	CComPtr<T>& operator=(T* lp) {
		if (p != lp) {
			if (lp) lp->AddRef();
			if (p) p->Release();
			p = lp;
		}
		return *this;
	}

	CComPtr<T>& operator=(const CComPtr<T>& lp) {
		return operator=(lp.p);
	}

	CComPtr<T>& operator=(CComPtr<T>&& lp) noexcept {
		if (p != lp.p) {
			if (p) p->Release();
			p = lp.p;
			lp.p = nullptr;
		}
		return *this;
	}

	void Attach(T* lp) noexcept {
		if (p) p->Release();
		p = lp;
	}

	T* Detach() noexcept {
		T* pt = p;
		p = nullptr;
		return pt;
	}

	void Release() noexcept {
		if (p) {
			T* pt = p;
			p = nullptr;
			pt->Release();
		}
	}

	bool operator==(T* lp) const noexcept { return p == lp; }
	bool operator!=(T* lp) const noexcept { return p != lp; }
	bool operator==(const CComPtr<T>& lp) const noexcept { return p == lp.p; }
	bool operator!=(const CComPtr<T>& lp) const noexcept { return p != lp.p; }
};
#endif
#include <objidl.h>
#include <stdio.h>

class OsrDragEvents {
public:
	virtual CefBrowserHost::DragOperationsMask OnDragEnter(
		CefRefPtr<CefDragData> drag_data,
		CefMouseEvent ev,
		CefBrowserHost::DragOperationsMask effect) = 0;

	virtual CefBrowserHost::DragOperationsMask OnDragOver(
		CefMouseEvent ev,
		CefBrowserHost::DragOperationsMask effect) = 0;

	virtual void OnDragLeave() = 0;

	virtual CefBrowserHost::DragOperationsMask OnDrop(
		CefMouseEvent ev,
		CefBrowserHost::DragOperationsMask effect) = 0;

protected:
	virtual ~OsrDragEvents() {}
};

#define DEFAULT_QUERY_INTERFACE(__Class)                            \
  HRESULT __stdcall QueryInterface(const IID& iid, void** object) { \
    *object = NULL;                                                 \
    if (IsEqualIID(iid, IID_IUnknown)) {                            \
      IUnknown* obj = this;                                         \
      *object = obj;                                                \
    } else if (IsEqualIID(iid, IID_##__Class)) {                    \
      __Class* obj = this;                                          \
      *object = obj;                                                \
    } else {                                                        \
      return E_NOINTERFACE;                                         \
    }                                                               \
    AddRef();                                                       \
    return S_OK;                                                    \
  }
#define IUNKNOWN_IMPLEMENTATION                     \
  ULONG __stdcall AddRef() { return ++ref_count_; } \
  ULONG __stdcall Release() {                       \
    if (--ref_count_ == 0) {                        \
      delete this;                                  \
      return 0U;                                    \
    }                                               \
    return ref_count_;                              \
  }                                                 \
                                                    \
 protected:                                         \
  ULONG ref_count_;

	class DropTargetWin : public IDropTarget {
	public:
		static CComPtr<DropTargetWin> Create(OsrDragEvents* callback, HWND hWnd);

		CefBrowserHost::DragOperationsMask StartDragging(
			CefRefPtr<CefBrowser> browser,
			CefRefPtr<CefDragData> drag_data,
			CefRenderHandler::DragOperationsMask allowed_ops,
			int x,
			int y);

		// IDropTarget implementation:
		HRESULT __stdcall DragEnter(IDataObject* data_object,
			DWORD key_state,
			POINTL cursor_position,
			DWORD* effect);

		HRESULT __stdcall DragOver(DWORD key_state,
			POINTL cursor_position,
			DWORD* effect);

		HRESULT __stdcall DragLeave();

		HRESULT __stdcall Drop(IDataObject* data_object,
			DWORD key_state,
			POINTL cursor_position,
			DWORD* effect);

		HRESULT __stdcall CancelCallback();

		DEFAULT_QUERY_INTERFACE(IDropTarget)
			IUNKNOWN_IMPLEMENTATION

	protected:
		DropTargetWin(OsrDragEvents* callback, HWND hWnd)
			: ref_count_(0), callback_(callback), hWnd_(hWnd) {}
		virtual ~DropTargetWin() {}

	private:
		OsrDragEvents * callback_;
		HWND hWnd_;

		CefRefPtr<CefDragData> current_drag_data_;
	};

	class DropSourceWin : public IDropSource {
	public:
		static CComPtr<DropSourceWin> Create();

		// IDropSource implementation:
		HRESULT __stdcall GiveFeedback(DWORD dwEffect);

		HRESULT __stdcall QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState);

		DEFAULT_QUERY_INTERFACE(IDropSource)
			IUNKNOWN_IMPLEMENTATION

	protected:
		explicit DropSourceWin() : ref_count_(0) {}
		virtual ~DropSourceWin() {}
	};

	class DragEnumFormatEtc : public IEnumFORMATETC {
	public:
		static HRESULT CreateEnumFormatEtc(UINT cfmt,
			FORMATETC* afmt,
			IEnumFORMATETC** ppEnumFormatEtc);

		//
		// IEnumFormatEtc members
		//
		HRESULT __stdcall Next(ULONG celt,
			FORMATETC* pFormatEtc,
			ULONG* pceltFetched);
		HRESULT __stdcall Skip(ULONG celt);
		HRESULT __stdcall Reset(void);
		HRESULT __stdcall Clone(IEnumFORMATETC** ppEnumFormatEtc);

		//
		// Construction / Destruction
		//
		DragEnumFormatEtc(FORMATETC* pFormatEtc, int nNumFormats);
		virtual ~DragEnumFormatEtc();

		static void DeepCopyFormatEtc(FORMATETC* dest, FORMATETC* source);

		DEFAULT_QUERY_INTERFACE(IEnumFORMATETC)
			IUNKNOWN_IMPLEMENTATION

	private:
		ULONG m_nIndex;           // current enumerator index
		ULONG m_nNumFormats;      // number of FORMATETC members
		FORMATETC* m_pFormatEtc;  // array of FORMATETC objects
	};

	class DataObjectWin : public IDataObject {
	public:
		static CComPtr<DataObjectWin> Create(FORMATETC* fmtetc,
			STGMEDIUM* stgmed,
			int count);

		// IDataObject memberS
		HRESULT __stdcall GetDataHere(FORMATETC* pFormatEtc, STGMEDIUM* pmedium);
		HRESULT __stdcall QueryGetData(FORMATETC* pFormatEtc);
		HRESULT __stdcall GetCanonicalFormatEtc(FORMATETC* pFormatEct,
			FORMATETC* pFormatEtcOut);
		HRESULT __stdcall SetData(FORMATETC* pFormatEtc,
			STGMEDIUM* pMedium,
			BOOL fRelease);
		HRESULT __stdcall DAdvise(FORMATETC* pFormatEtc,
			DWORD advf,
			IAdviseSink*,
			DWORD*);
		HRESULT __stdcall DUnadvise(DWORD dwConnection);
		HRESULT __stdcall EnumDAdvise(IEnumSTATDATA** ppEnumAdvise);

		HRESULT __stdcall EnumFormatEtc(DWORD dwDirection,
			IEnumFORMATETC** ppEnumFormatEtc);
		HRESULT __stdcall GetData(FORMATETC* pFormatEtc, STGMEDIUM* pMedium);

		DEFAULT_QUERY_INTERFACE(IDataObject)
			IUNKNOWN_IMPLEMENTATION

	protected:
		int m_nNumFormats;
		FORMATETC* m_pFormatEtc;
		STGMEDIUM* m_pStgMedium;

		static HGLOBAL DupGlobalMem(HGLOBAL hMem);

		int LookupFormatEtc(FORMATETC* pFormatEtc);

		explicit DataObjectWin(FORMATETC* fmtetc, STGMEDIUM* stgmed, int count);
		virtual ~DataObjectWin() {}
	};


#endif  // defined(CEF_USE_ATL)
