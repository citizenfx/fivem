#include "StdInc.h"
#include "SDK.h"

#include <wrl.h>
#include <shellapi.h>
#include <ShlObj_core.h>

namespace fxdk::ioUtils
{
	namespace WRL = Microsoft::WRL;

	// From https://github.com/electron/electron/blob/09d7b2bc91009ae794e45142d064b4e8c5462022/shell/common/platform_util_win.cc#L41
	// Required COM implementation of IFileOperationProgressSink so we can
	// precheck files before deletion to make sure they can be move to the
	// Recycle Bin.
	class DeleteFileProgressSink : public IFileOperationProgressSink {
	public:
		DeleteFileProgressSink();
		virtual ~DeleteFileProgressSink() = default;

	private:
		ULONG STDMETHODCALLTYPE AddRef(void) override;
		ULONG STDMETHODCALLTYPE Release(void) override;
		HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid,
			LPVOID* ppvObj) override;
		HRESULT STDMETHODCALLTYPE StartOperations(void) override;
		HRESULT STDMETHODCALLTYPE FinishOperations(HRESULT) override;
		HRESULT STDMETHODCALLTYPE PreRenameItem(DWORD, IShellItem*, LPCWSTR) override;
		HRESULT STDMETHODCALLTYPE
			PostRenameItem(DWORD, IShellItem*, LPCWSTR, HRESULT, IShellItem*) override;
		HRESULT STDMETHODCALLTYPE PreMoveItem(DWORD,
			IShellItem*,
			IShellItem*,
			LPCWSTR) override;
		HRESULT STDMETHODCALLTYPE PostMoveItem(DWORD,
			IShellItem*,
			IShellItem*,
			LPCWSTR,
			HRESULT,
			IShellItem*) override;
		HRESULT STDMETHODCALLTYPE PreCopyItem(DWORD,
			IShellItem*,
			IShellItem*,
			LPCWSTR) override;
		HRESULT STDMETHODCALLTYPE PostCopyItem(DWORD,
			IShellItem*,
			IShellItem*,
			LPCWSTR,
			HRESULT,
			IShellItem*) override;
		HRESULT STDMETHODCALLTYPE PreDeleteItem(DWORD, IShellItem*) override;
		HRESULT STDMETHODCALLTYPE PostDeleteItem(DWORD,
			IShellItem*,
			HRESULT,
			IShellItem*) override;
		HRESULT STDMETHODCALLTYPE PreNewItem(DWORD, IShellItem*, LPCWSTR) override;
		HRESULT STDMETHODCALLTYPE PostNewItem(DWORD,
			IShellItem*,
			LPCWSTR,
			LPCWSTR,
			DWORD,
			HRESULT,
			IShellItem*) override;
		HRESULT STDMETHODCALLTYPE UpdateProgress(UINT, UINT) override;
		HRESULT STDMETHODCALLTYPE ResetTimer(void) override;
		HRESULT STDMETHODCALLTYPE PauseTimer(void) override;
		HRESULT STDMETHODCALLTYPE ResumeTimer(void) override;

		ULONG m_cRef;
	};

	DeleteFileProgressSink::DeleteFileProgressSink() {
		m_cRef = 0;
	}

	HRESULT DeleteFileProgressSink::PreDeleteItem(DWORD dwFlags, IShellItem*) {
		if (!(dwFlags & TSF_DELETE_RECYCLE_IF_POSSIBLE)) {
			// TSF_DELETE_RECYCLE_IF_POSSIBLE will not be set for items that cannot be
			// recycled.  In this case, we abort the delete operation.  This bubbles
			// up and stops the Delete in IFileOperation.
			return E_ABORT;
		}
		// Returns S_OK if successful, or an error value otherwise. In the case of an
		// error value, the delete operation and all subsequent operations pending
		// from the call to IFileOperation are canceled.
		return S_OK;
	}

	HRESULT DeleteFileProgressSink::QueryInterface(REFIID riid, LPVOID* ppvObj) {
		// Always set out parameter to NULL, validating it first.
		if (!ppvObj)
			return E_INVALIDARG;
		*ppvObj = nullptr;
		if (riid == IID_IUnknown || riid == IID_IFileOperationProgressSink) {
			// Increment the reference count and return the pointer.
			*ppvObj = reinterpret_cast<IUnknown*>(this);
			AddRef();
			return NOERROR;
		}
		return E_NOINTERFACE;
	}

	ULONG DeleteFileProgressSink::AddRef() {
		InterlockedIncrement(&m_cRef);
		return m_cRef;
	}

	ULONG DeleteFileProgressSink::Release() {
		// Decrement the object's internal counter.
		ULONG ulRefCount = InterlockedDecrement(&m_cRef);
		if (0 == m_cRef) {
			delete this;
		}
		return ulRefCount;
	}

	HRESULT DeleteFileProgressSink::StartOperations() {
		return S_OK;
	}

	HRESULT DeleteFileProgressSink::FinishOperations(HRESULT) {
		return S_OK;
	}

	HRESULT DeleteFileProgressSink::PreRenameItem(DWORD, IShellItem*, LPCWSTR) {
		return S_OK;
	}

	HRESULT DeleteFileProgressSink::PostRenameItem(DWORD,
		IShellItem*,
		__RPC__in_string LPCWSTR,
		HRESULT,
		IShellItem*) {
		return E_NOTIMPL;
	}

	HRESULT DeleteFileProgressSink::PreMoveItem(DWORD,
		IShellItem*,
		IShellItem*,
		LPCWSTR) {
		return E_NOTIMPL;
	}

	HRESULT DeleteFileProgressSink::PostMoveItem(DWORD,
		IShellItem*,
		IShellItem*,
		LPCWSTR,
		HRESULT,
		IShellItem*) {
		return E_NOTIMPL;
	}

	HRESULT DeleteFileProgressSink::PreCopyItem(DWORD,
		IShellItem*,
		IShellItem*,
		LPCWSTR) {
		return E_NOTIMPL;
	}

	HRESULT DeleteFileProgressSink::PostCopyItem(DWORD,
		IShellItem*,
		IShellItem*,
		LPCWSTR,
		HRESULT,
		IShellItem*) {
		return E_NOTIMPL;
	}

	HRESULT DeleteFileProgressSink::PostDeleteItem(DWORD,
		IShellItem*,
		HRESULT,
		IShellItem*) {
		return S_OK;
	}

	HRESULT DeleteFileProgressSink::PreNewItem(DWORD dwFlags,
		IShellItem*,
		LPCWSTR) {
		return E_NOTIMPL;
	}

	HRESULT DeleteFileProgressSink::PostNewItem(DWORD,
		IShellItem*,
		LPCWSTR,
		LPCWSTR,
		DWORD,
		HRESULT,
		IShellItem*) {
		return E_NOTIMPL;
	}

	HRESULT DeleteFileProgressSink::UpdateProgress(UINT, UINT) {
		return S_OK;
	}

	HRESULT DeleteFileProgressSink::ResetTimer() {
		return S_OK;
	}

	HRESULT DeleteFileProgressSink::PauseTimer() {
		return S_OK;
	}

	HRESULT DeleteFileProgressSink::ResumeTimer() {
		return S_OK;
	}

	std::string DeleteItems(const std::vector<std::string>& items)
	{
		WRL::ComPtr<IFileOperation> pfo;

		if (FAILED(CoCreateInstance(CLSID_FileOperation, nullptr, CLSCTX_ALL, IID_PPV_ARGS(&pfo))))
		{
			return "Failed to create FileOperation instance";
		}

		if (FAILED(pfo->SetOperationFlags(FOF_NO_UI | FOFX_ADDUNDORECORD | FOF_NOERRORUI | FOF_SILENT |
			FOFX_SHOWELEVATIONPROMPT | FOFX_RECYCLEONDELETE)))
		{
			return "Failed to set operation flags";
		}

		WRL::ComPtr<IFileOperationProgressSink> delete_sink(new DeleteFileProgressSink);
		if (!delete_sink) {
			return "Failed to create delete sink";
		}

		for (auto item : items)
		{
			WRL::ComPtr<IShellItem> psi;
			if (FAILED(SHCreateItemFromParsingName(ToWide(item).c_str(), NULL, IID_PPV_ARGS(&psi))))
			{
				return fmt::sprintf("Failed to create ShellItem for %s", item);
			}

			if (FAILED(pfo->DeleteItem(psi.Get(), delete_sink.Get())))
			{
				return fmt::sprintf("Failed to add delete operation for %s", item);
			}
		}

		if (FAILED(pfo->PerformOperations()))
		{
			return "Failed to perform delete operations";
		}

		BOOL anyOperationsAborted;
		if (FAILED(pfo->GetAnyOperationsAborted(&anyOperationsAborted)))
		{
			return "Failed to check operation status";
		}

		if (anyOperationsAborted)
		{
			return "Some operations were aborted";
		}

		return {};
	}

	void RecycleShellItems(const std::vector<std::string> items, const RecycleShellItemsCallback cb)
	{
		std::thread([=]()
		{
			cb(DeleteItems(items));
		}).detach();
	}
}
