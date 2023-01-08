
// TODO:
//@ Why do we get white border around the board when we resize the window?

#include <stdio.h>
#include <math.h>
#include <assert.h>

#include "lib.h"
#include "lib.cpp"
#include "chesss.h"
#include "renderer.cpp"
#include "chesss.cpp"

#include <windows.h>
#include <dsound.h>
#include <x86intrin.h>

// Initial dimensions
int ClientWidth = 60 * 8;
int ClientHeight = 60 * 8;
int WindowWidth = ClientWidth + 16;
int WindowHeight = ClientHeight + 38;

static BITMAPINFO GlobalBitmapInfo;
userInput GlobalGameInput;
image GlobalWindowBuffer;

i64 GlobalPerfFrequency;

typedef HRESULT directSoundCreate(LPGUID lpGuid, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter);
LPDIRECTSOUNDBUFFER GlobalSoundBuffer;

void InitDirectSound(HWND Window, i32 BufferSize, WORD NumChannels, WORD BitsPerSample, DWORD SamplesPerSec)
{
	HMODULE DSoundLib;
	if ((DSoundLib = LoadLibraryA("dsound.dll")) == NULL)
	{
		printf("LoadLibraryA() error!\n");
		return;
	}

	directSoundCreate *DirectSoundCreate = (directSoundCreate *) GetProcAddress(DSoundLib, "DirectSoundCreate");
	if (!DirectSoundCreate)
	{
		printf("GetProcAddress() error!\n");
		return;
	}

	LPDIRECTSOUND DirectSound;
	if (DirectSoundCreate(NULL, &DirectSound, NULL) != DS_OK)
	{
		printf("DirectSoundCreate() error!\n");
		return;
	}

	if (DirectSound->SetCooperativeLevel(Window, DSSCL_PRIORITY) != DS_OK)
	{
		printf("SetCooperativeLevel() error!\n");
		return;
	}

	WAVEFORMATEX WaveFormat;
	WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
	WaveFormat.nChannels = NumChannels;
	WaveFormat.nSamplesPerSec = SamplesPerSec;
	WaveFormat.wBitsPerSample = BitsPerSample;
	WaveFormat.nBlockAlign = WaveFormat.nChannels * WaveFormat.wBitsPerSample / 8;
	WaveFormat.nAvgBytesPerSec = WaveFormat.nSamplesPerSec * WaveFormat.nBlockAlign; // is this correct?
	WaveFormat.cbSize = 0;
	
	DSBUFFERDESC PrimaryBufferDescription = {};
	PrimaryBufferDescription.dwSize = sizeof(PrimaryBufferDescription);
	PrimaryBufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;

	LPDIRECTSOUNDBUFFER PrimaryBuffer;
	if (SUCCEEDED(DirectSound->CreateSoundBuffer(&PrimaryBufferDescription, &PrimaryBuffer, NULL)))
	{
		if (PrimaryBuffer->SetFormat(&WaveFormat) != DS_OK)
		{
			printf("SetFormat() error!\n");
			return;
		}
	}
	else
	{
		printf("CreateSoundBuffer() error! (1)\n");
		return;
	}

	DSBUFFERDESC SecondaryBufferDescription = {};
	SecondaryBufferDescription.dwSize = sizeof(SecondaryBufferDescription);
	SecondaryBufferDescription.dwFlags = 0;
	SecondaryBufferDescription.dwBufferBytes = BufferSize;
	SecondaryBufferDescription.lpwfxFormat = &WaveFormat;

	LPDIRECTSOUNDBUFFER SecondarySoundBuffer;
	HRESULT Result = DirectSound->CreateSoundBuffer(&SecondaryBufferDescription, &SecondarySoundBuffer, NULL);
	if (Result != DS_OK)
	{
		printf("CreateSoundBuffer() error: %d! (2)\n", Result);
		return;
	}
	GlobalSoundBuffer = SecondarySoundBuffer;

//	LPDIRECTSOUNDBUFFER SecondaryBuffer = NULL;
//	printf("SecondaryBuffer (before): %p\n", SecondaryBuffer);
//	if (DirectSound->CreateSoundBuffer(&SecondaryBufferDescription, &SecondaryBuffer, NULL) != DS_OK)
//	{
//		printf("CreateSoundBuffer() error! (2)\n"); // why do we get this???
//		printf("SecondaryBuffer (after): %p\n", SecondaryBuffer);
//		return;
//	}

	printf("direct sound OK\n");
}

static void ResizeWindowBuffer(image *Buffer, int Width, int Height, BITMAPINFO *Info)
{
	if (Buffer->Data)
	{
		VirtualFree(Buffer->Data, 0, MEM_RELEASE);
	}

	Info->bmiHeader.biSize = sizeof(Info->bmiHeader);
	Info->bmiHeader.biWidth = Width;
	Info->bmiHeader.biHeight = -Height;
	Info->bmiHeader.biPlanes = 1; // "This value must be set to 1"
	Info->bmiHeader.biBitCount = 32;
	Info->bmiHeader.biCompression = BI_RGB;

	Buffer->Data = (u8 *) VirtualAlloc(NULL, 4 * Width * Height, MEM_COMMIT, PAGE_READWRITE);
	Buffer->Width = Width;
	Buffer->Height = Height;
}

static void DisplayWindowBuffer(HDC DC, image *Buffer, BITMAPINFO *Info)
{
	StretchDIBits(
		DC,
		0, 0, Buffer->Width, Buffer->Height, // dest (bitmap dimensions are equal to window dimensions)
		0, 0, Buffer->Width, Buffer->Height, // src
		Buffer->Data,
		Info,
		DIB_RGB_COLORS, SRCCOPY);
}

LRESULT CALLBACK WindowCallback(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = 0;

	switch (message)
	{
		case WM_SIZE:
		{
			printf("WM_SIZE\n");

			static int Count = 0;
			if (Count == 0)
			{
				RECT client_rect;
				GetClientRect(window, &client_rect);
				int Width = client_rect.right - client_rect.left;
				int Height = client_rect.bottom - client_rect.top;
				printf("Width: %d, Height: %d\n", Width, Height);
				ResizeWindowBuffer(&GlobalWindowBuffer, Width, Height, &GlobalBitmapInfo);
				Count += 1;
			}
		}
		break;
		case WM_PAINT:
		{
			printf("WM_PAINT\n");

			PAINTSTRUCT PS;
			HDC DC = BeginPaint(window, &PS);
//			DisplayWindowBuffer(DC, &WindowBuffer, &BitmapInfo);
			EndPaint(window, &PS);
		}
		break;
		case WM_CLOSE:
		{
			printf("WM_CLOSE\n");
			PostQuitMessage(0);
		}
		break;
		case WM_DESTROY:
		{
			printf("WM_DESTROY\n");
		}
		break;
		case WM_ACTIVATEAPP:
		{
			printf("WM_ACTIVATEAPP\n");
		}
		break;
//		case WM_KEYDOWN:
//		{
//			printf("WM_KEYDOWN\n");
//
//			if (wParam == 'A')
//			{
//				printf("A\n");
//			}
//			else if (wParam == 'S')
//			{
//				printf("S\n");
//			}
//			else if (wParam == 'D')
//			{
//				printf("D\n");
//			}
//		}
//		break;
//		case WM_KEYUP:
//		{
//			printf("WM_KEYUP\n");
//		}
//		break;
//		case WM_SYSKEYDOWN:
//		{
//			printf("WM_SYSKEYDOWN\n");
//		}
//		break;
//		case WM_SYSKEYUP:
//		{
//			printf("WM_SYSKEYUP\n");
//		}
//		break;
		case WM_LBUTTONDOWN:
		{
//			printf("WM_LBUTTONDOWN\n");
			event Event;
			Event.Type = LBUTTONDOWN;
			Event.X = lParam & 0x0000ffff;
			Event.Y = (lParam & 0xffff0000) >> 16;
			ArrayAdd(&GlobalGameInput.Events, Event);
		}
		break;
		case WM_LBUTTONUP:
		{
//			printf("WM_LBUTTONUP\n");
			event Event;
			Event.Type = LBUTTONUP;
			Event.X = lParam & 0x0000ffff;
			Event.Y = (lParam & 0xffff0000) >> 16;
			ArrayAdd(&GlobalGameInput.Events, Event);
		}
		break;
		case WM_RBUTTONDOWN:
		{
//			printf("WM_RBUTTONDOWN\n");
		}
		break;
		case WM_RBUTTONUP:
		{
//			printf("WM_RBUTTONUP\n");
		}
		break;
		case WM_MOUSEMOVE:
		{
			int X = lParam & 0x0000ffff;
			int Y = (lParam & 0xffff0000) >> 16;
			GlobalGameInput.LastMouseX = X;
			GlobalGameInput.LastMouseY = Y;
//			printf("WM_MOUSEMOVE: X: %d, Y: %d\n", X, Y);
		}
		break;
		default:
		{
			result = DefWindowProc(window, message, wParam, lParam);
		}
	}

	return result;
}

i64 GetPerfCounterTicks()
{
	LARGE_INTEGER PerfCounter;
	QueryPerformanceCounter(&PerfCounter);
	return PerfCounter.QuadPart;
}

r64 GetElapsedSecs(i64 NumTicks1, i64 NumTicks2, i64 PerfFrequency)
{
	i64 ElapsedTicks = NumTicks2 - NumTicks1;
	r64 ElapsedSecs = (r64) ElapsedTicks / (r64) PerfFrequency;
	return ElapsedSecs;
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR pCmdLine, int nCmdShow)
{
	MessageBox(NULL, "message", "title", MB_OK);
	// To get the console window for debug messages:
	{
		AllocConsole();
		freopen("CONOUT$", "w", stdout);
	}

	LARGE_INTEGER PerfFrequency;
	if (!QueryPerformanceFrequency(&PerfFrequency))
	{
		// installed hardware does not support high resolution performance counter
		return 1;
	}
	GlobalPerfFrequency = PerfFrequency.QuadPart;
//	DebugDisplayValue(PerfFrequency.QuadPart); // 2 143 613

// That doesnt seem to have any effect on scheduler granularity.
//	if (timeBeginPeriod(1) != TIMERR_NOERROR)
//	{
//		return 1;
//	}

	WNDCLASS WndClass = {};
	WndClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW; // CS_HREDRAW | CS_VREDRAW -- always invalidate the whole window in WM_PAINT.
	WndClass.lpfnWndProc = WindowCallback;
	WndClass.hInstance = instance;
	WndClass.lpszClassName = "WindowClass";

	if (!RegisterClassA(&WndClass))
	{
		fprintf(stderr, "RegisterClassA(): error!\n");
		return 1;
	}

	HWND Window = CreateWindowExA(
		0,
		WndClass.lpszClassName,
		"Chesss",
		WS_OVERLAPPEDWINDOW|WS_VISIBLE,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		WindowWidth,
		WindowHeight,
		NULL,
		NULL,
		instance,
		NULL);

	if (!Window)
	{
		fprintf(stderr, "CreateWindowExA(): error!\n");
		return 1;
	}

	// Initialize sound
	// We configure the sound device to support 2 channels, always. When playing 1-channel audio, we just duplicate for both channels, and I see nothing wrong with this.
//	i32 Hz = 220;
//	u32 SoundAmplitude = 32000;
	int NumChannels = 2;
	u32 BytesPerSample = 2;
	u32 SamplesPerSecond = 48000;
//	u32 BufferSize = SamplesPerSecond * BytesPerSample * NumChannels;
//	u32 BufferSize = 1000 * BytesPerSample * NumChannels; // no good
	u32 BufferSize = 4000 * BytesPerSample * NumChannels; // for 60 fps
//	u32 BufferSize = 6000 * BytesPerSample * NumChannels; // for 30 fps
	InitDirectSound(Window, BufferSize, NumChannels, BytesPerSample * 8, SamplesPerSecond);
	GlobalSoundBuffer->Play(0, 0, DSBPLAY_LOOPING);
//	printf("buffer size: %u\n", BufferSize);

//	i32 SquareWavePeriod = SamplesPerSecond / Hz;
//	i32 HalfSquareWavePeriod = SquareWavePeriod / 2;
	u32 RunningSampleIndex = 0;

	ArrayInit(&GlobalGameInput.Events);

	GameInit();

//	int64 prev_counter;
//	{
//		LARGE_INTEGER perf_counter;
//		QueryPerformanceCounter(&perf_counter);
//		prev_counter = perf_counter.QuadPart;
//	}

//	uint64 cycle_count = __rdtsc();
//	printf("cycle count: %lu\n", cycle_count);

	i64 PreviousTicks;
	u64 FrameCount = 0; // when does this "roll over" back to 0 again?
	bool Running = true;
	while (Running)
	{
		if (FrameCount > 0)
		{
			i64 CurrentTicks = GetPerfCounterTicks();
			r64 ElapsedMS = 1000 * GetElapsedSecs(PreviousTicks, CurrentTicks, GlobalPerfFrequency);
//			printf("%.9f\n", ElapsedMS);
			printf("%ld ms\n", (i64) (ElapsedMS + 0.5));
			PreviousTicks = CurrentTicks;
		}
		else
		{
			PreviousTicks = GetPerfCounterTicks();
		}
		FrameCount += 1;

		MSG Message;
		while (PeekMessage(&Message, NULL, 0, 0, PM_REMOVE))
		{
			if (Message.message == WM_QUIT)
			{
				printf("Got WM_QUIT\n");
				Running = false;
			}

			TranslateMessage(&Message);
			DispatchMessage(&Message);
		}

		// sound test
		DWORD PlayCursor;
		DWORD WriteCursor;
		if (SUCCEEDED(GlobalSoundBuffer->GetCurrentPosition(&PlayCursor, &WriteCursor)))
		{
//			if (FrameCount < 21)
//			{
//				printf("play: %u, write: %u\n", PlayCursor, WriteCursor);
//			}
			DWORD ByteToLock = RunningSampleIndex * BytesPerSample * NumChannels % BufferSize;

			DWORD BytesToWrite;
			if (ByteToLock > PlayCursor)
			{
				BytesToWrite = BufferSize - ByteToLock;
				BytesToWrite += PlayCursor;
			}
			else
			{
				BytesToWrite = PlayCursor - ByteToLock;
			}

			soundBuffer SoundBuffer = {};
			SoundBuffer.NumSamplesWanted = BytesToWrite / (BytesPerSample * NumChannels);
			SoundBuffer.Samples = malloc(BytesToWrite);
			assert((BytesToWrite % (BytesPerSample * NumChannels)) == 0);

			GameUpdate(&GlobalWindowBuffer, &SoundBuffer, &GlobalGameInput);
			GlobalGameInput.Events.Count = 0;

			VOID *Region1;
			DWORD Region1Size;
			VOID *Region2;
			DWORD Region2Size;
			GlobalSoundBuffer->Lock(ByteToLock, BytesToWrite,
				&Region1, &Region1Size, &Region2, &Region2Size,
				0); // flags
			assert(BytesToWrite == Region1Size + Region2Size);

//			if (FrameCount < 21)
//			{
//				printf("play: %u\n", PlayCursor);
//				printf("size1: %u, size2: %u\n", Region1Size, Region2Size);
//			}

			i16 *PtrDevice = (i16 *) Region1;
			i16 *PtrGame = (i16 *) SoundBuffer.Samples;
			assert((Region1Size % (BytesPerSample * NumChannels)) == 0);
			int Region1SampleCount = Region1Size / (BytesPerSample * NumChannels);
			int Count = 0;
			while (Count < Region1SampleCount)
			{
				*PtrDevice++ = *PtrGame++;
				*PtrDevice++ = *PtrGame++;
				Count += 1;
				RunningSampleIndex += 1;
			}
			PtrDevice = (i16 *) Region2;
			PtrGame = (i16 *) SoundBuffer.Samples + Count * NumChannels;
			assert((Region2Size % (BytesPerSample * NumChannels)) == 0);
			int Region2SampleCount = Region2Size / (BytesPerSample * NumChannels);
			while (Count < Region1SampleCount + Region2SampleCount)
			{
				*PtrDevice++ = *PtrGame++;
				*PtrDevice++ = *PtrGame++;
				Count += 1;
				RunningSampleIndex += 1;
			}

			free(SoundBuffer.Samples);
			GlobalSoundBuffer->Unlock(Region1, Region1Size, Region2, Region2Size);
		}

//		GameUpdate(&GlobalWindowBuffer, &SoundBuffer, &GlobalGameInput);
////		GameUpdate(&GameMemory, &GameUserInput, &VideoBuffer, &SoundBuffer);
//		GlobalGameInput.Events.Count = 0;

		// If we dont hit the screen refresh rate (or a multiple of it) exactly (which is not going to happen), we are going to be "in shift" relative to the screen refresh rate which means that at some point we are going to miss a frame (?)
		i64 TargetFrameTimeMS = 17; // ~ 60 fps
//		i64 TargetFrameTimeMS = 33; // ~ 30 fps
		i64 ElapsedMS;
//		do
//		{
//			i64 CurrentTicks = GetPerfCounterTicks();
//			ElapsedMS = (i64) (1000 * GetElapsedSecs(PreviousTicks, CurrentTicks, GlobalPerfFrequency) + 0.5);
//		}
//		while (ElapsedMS < TargetFrameTimeMS);
		i64 CurrentTicks = GetPerfCounterTicks();
		ElapsedMS = (i64) (1000 * GetElapsedSecs(PreviousTicks, CurrentTicks, GlobalPerfFrequency) + 0.5);
		if (TargetFrameTimeMS - ElapsedMS > 0)
		{
			Sleep(TargetFrameTimeMS - ElapsedMS);
		}

		HDC DC = GetDC(Window);
		DisplayWindowBuffer(DC, &GlobalWindowBuffer, &GlobalBitmapInfo);
		ReleaseDC(Window, DC);
	}

	return 0;
}