#pragma once

template <typename T, uint16_t BufferSize>
class CRingBuffer
{
public:
	using value_type = T;
	static constexpr uint16_t Size = BufferSize;

	CRingBuffer()
	{
		memset(_buffer, 0, BufferSize * sizeof(T));
	}

	~CRingBuffer() = default;

	CRingBuffer(const CRingBuffer& other) = delete;
	CRingBuffer& operator=(const CRingBuffer& other) = delete;

	void pushValue(const T value)
	{
		_newestValueIndex = incrementIndex(_newestValueIndex, 1);
		_oldestValueIndex = incrementIndex(_oldestValueIndex, 1);

		_buffer[_newestValueIndex] = value;
	}


	// The items are stored from the oldest to the newest, so [0] == front() == oldest; [BufferSize-1] == back() == newwst.
	const T operator[] (const uint16_t index) const
	{
		return _buffer[incrementIndex(_oldestValueIndex, index)];
	}

	// Oldest value
	const T front() const
	{
		return _buffer[_oldestValueIndex];
	}

	// Newest value
	const T back() const
	{
		return _buffer[_newestValueIndex];
	}

	static uint16_t size()
	{
		return BufferSize;
	}

private:
	// This is really just modulo BufferSize addition
	static uint16_t incrementIndex(uint16_t index, const uint16_t increment)
	{
		index += increment;
		if (index >= BufferSize)
			index -= BufferSize;

		return index;
	}

private:
	T _buffer[BufferSize];
	uint16_t _newestValueIndex = BufferSize - 1, _oldestValueIndex = 0;
};