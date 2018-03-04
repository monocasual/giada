#ifndef G_AUDIO_BUFFER_H
#define G_AUDIO_BUFFER_H


namespace giada {
namespace m
{
class AudioBuffer
{
public:

	AudioBuffer();
	~AudioBuffer();

	/* operator []
	Given a frame 'offset', returns a pointer to it. This is useful for digging 
	inside a frame, i.e. parsing each channel. How to use it:

		for (int k=0; k<buffer->countFrames(), k++)
			for (int i=0; i<buffer->countChannels(); i++)
				... buffer[k][i] ...

	Also note that buffer[0] will give you a pointer to the whole internal data
	array. */

	float* operator [](int offset) const;

	int countFrames() const;
	int countSamples() const;
	int countChannels() const;
	bool isAllocd() const;

	bool alloc(int size, int channels) noexcept;
	void free();

	/* copyData
	Copies 'frames' frames from the new 'data' into m_data, and fills m_data 
	starting from frame 'offset'. It takes for granted that the new data contains 
	the same number of channels than m_channels. */

	void copyData(float* data, int frames, int offset=0);

	/* copyFrame
	Copies data pointed by 'values' into m_data[frame]. It takes for granted that
	'values' contains the same number of channels than m_channels. */

	void copyFrame(int frame, float* values);

	/* setData
	Borrow 'data' as new m_data. Makes sure not to delete the data 'data' points
	to while using it. Set it back to nullptr when done. */

	void setData(float* data, int size, int channels);

	/* moveData
	Moves data held by 'b' into this buffer. Then 'b' becomes an empty buffer. */
	 
	void moveData(AudioBuffer& b);

	/* clear
	Clears the internal data by setting all bytes to 0.0f. Optional parameters
	'a' and 'b' set the range. */
	
	void clear(int a=0, int b=-1);

private:

	float* m_data;
	int    m_size;     // in frames    
	int    m_channels;
};

}} // giada::m::

#endif