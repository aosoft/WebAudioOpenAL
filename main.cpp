#include <cstdint>
#include <cmath>
#include <memory>

#include <emscripten.h>
#include <emscripten/bind.h>
#include <AL/al.h>
#include <AL/alc.h>

class StreamingPlayer
{
private:
	static const int BufferCount = 2;

	ALCdevice *_device;
	ALCcontext *_context;
	ALuint _buffers[BufferCount];
	ALuint _source;
	int _sampleRate;
	int64_t _sampleNumber;

	bool _isPlaying;
public:
	StreamingPlayer() :
		_device(nullptr),
		_context(nullptr),
		_source(0),
		_sampleRate(0),
		_sampleNumber(0),
		_isPlaying(false)
	{
		_device = alcOpenDevice(nullptr);
		_context = alcCreateContext(_device, nullptr);
		alcMakeContextCurrent(_context);

		alGetError();

		alGenBuffers(2, _buffers);
	}

	~StreamingPlayer()
	{
		Stop();
		if (_buffers[0] != 0)
		{
			alDeleteBuffers(2, _buffers);
		}
		if (_context != nullptr)
		{
			alcDestroyContext(_context);
		}
		if (_device != nullptr)
		{
			alcCloseDevice(_device);
		}
	}

	bool IsPlaying() const
	{
		return _isPlaying;
	}

	void Play(int sampleRate)
	{
		if (!_isPlaying)
		{
			if (_source != 0)
			{
				alDeleteSources(1, &_source);
			}
			alGenSources(1, &_source);
			alSourcef(_source, AL_GAIN, 1);
			alSource3f(_source, AL_POSITION, 0, 0, 0);
			_sampleRate = sampleRate;
			_sampleNumber = 0;

			for (auto buf : _buffers)
			{
				InternalProcess(buf);
			}
			alSourcePlay(_source);
			_isPlaying = true;
		}
	}

	void Stop()
	{
		if (_isPlaying)
		{
			alSourceStop(_source);
			if (_source != 0)
			{
				alDeleteSources(1, &_source);
			}

			_isPlaying = false;
		}
	}

	void Process()
	{
		if (_isPlaying)
		{
			int processed;
			alGetSourcei(_source, AL_BUFFERS_PROCESSED, &processed);
			while (processed > 0)
			{
				ALuint buffer;
				alSourceUnqueueBuffers(_source, 1, &buffer);
				InternalProcess(buffer);
				processed--;
			}
		}
	}

private:
	void InternalProcess(ALuint buffer)
	{
		_sampleNumber += ProcessAudio(buffer, _sampleRate, _sampleNumber);
		alSourceQueueBuffers(_source, 1, &buffer);
	}

private:
	int ProcessAudio(ALuint buffer, int sampleRate, int64_t sampleNumber)
	{
		static const size_t BufLen = 16000;
		short buf[BufLen];
		for (auto& r : buf)
		{
			r = std::sinf(static_cast<float>(sampleNumber) * 440 * 2 * 3.14159f / sampleRate) * 32767;
			sampleNumber++;
		}

		alBufferData(buffer, AL_FORMAT_MONO16, buf, sizeof(buf), sampleRate);

		return BufLen;
	}
};

std::unique_ptr<StreamingPlayer> g_player;


void MainLoop()
{
	if (g_player != nullptr)
	{
		g_player->Process();
	}
}

int main()
{
	emscripten_set_main_loop(MainLoop, 60, 1);
	g_player.release();
	return 0;
}

void StartPlay()
{
	g_player.release();
	g_player = std::make_unique<StreamingPlayer>();
	g_player->Play(44100);
}

void StopPlay()
{
	g_player.release();
}

EMSCRIPTEN_BINDINGS(WebAudioOpenAL)
{
	emscripten::function("startPlay", &StartPlay);
	emscripten::function("stopPlay", &StopPlay);
}

