#include <StdInc.h>
#include <VFSWin32.h>

#include <VFSManager.h>

#include <wrl.h>

class VfsStream : public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>, IStream>
{
private:
	fwRefContainer<vfs::Stream> m_stream;

public:
	VfsStream(fwRefContainer<vfs::Stream> stream)
	{
		m_stream = stream;
	}

	// Inherited via RuntimeClass
	virtual HRESULT Read(void* pv, ULONG cb, ULONG* pcbRead) override
	{
		*pcbRead = m_stream->Read(pv, cb);

		return S_OK;
	}
	virtual HRESULT Write(const void* pv, ULONG cb, ULONG* pcbWritten) override
	{
		*pcbWritten = m_stream->Write(pv, cb);
		return S_OK;
	}
	virtual HRESULT Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER* plibNewPosition) override
	{
		auto p = m_stream->Seek(dlibMove.QuadPart, dwOrigin);

		if (plibNewPosition)
		{
			plibNewPosition->QuadPart = p;
		}

		return S_OK;
	}

	virtual HRESULT SetSize(ULARGE_INTEGER libNewSize) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT CopyTo(IStream* pstm, ULARGE_INTEGER cb, ULARGE_INTEGER* pcbRead, ULARGE_INTEGER* pcbWritten) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT Commit(DWORD grfCommitFlags) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT Revert(void) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT Stat(STATSTG* pstatstg, DWORD grfStatFlag) override
	{
		pstatstg->cbSize.QuadPart = m_stream->GetLength();
		pstatstg->type = STGTY_STREAM;
		pstatstg->grfMode = STGM_READ;

		return S_OK;
	}
	virtual HRESULT Clone(IStream** ppstm) override
	{
		return E_NOTIMPL;
	}
};

namespace vfs
{
	DLL_EXPORT Microsoft::WRL::ComPtr<IStream> CreateComStream(fwRefContainer<vfs::Stream> stream)
	{
		return Microsoft::WRL::Make<VfsStream>(stream);
	}
}
