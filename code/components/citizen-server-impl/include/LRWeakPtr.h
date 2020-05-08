#include <memory>
#include <xenium/left_right.hpp>

template<typename T>
struct LRWeakPtr
{
	using TWeakPtr = std::weak_ptr<T>;

	void reset()
	{
		_ptr.update([](auto& weakPtr)
			{
				weakPtr.reset();
			});
	}

	void update(const TWeakPtr& newPtr)
	{
		_ptr.update([&](auto& weakPtr)
			{
				weakPtr = newPtr;
			});
	}

	std::shared_ptr<T> lock()
	{
		return _ptr.read([](auto& weakPtr)
			{
				return weakPtr.lock();
			});
	}

private:
	xenium::left_right<TWeakPtr> _ptr;
};
