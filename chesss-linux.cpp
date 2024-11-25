
//@ chess-linux: chesss-linux.cpp:355: int main(): Assertion `LoopCount < 2' failed.

// OS-agnostic stuff (general):

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#define LIB_INCLUDE_IMPLEMENTATION
#include "lib/lib.hpp"

#define LINUX_LIB_INCLUDE_IMPLEMENTATION
#include "lib/linux_lib.hpp"

#include "lib/array.hpp"
#include "lib/linked_list.hpp"

#include "chesss.h"
#include "chesss-helper.cpp"
#include "chesss-renderer.cpp"


// OS-specific stuff (available to the game):
//...

// OS-agnostic stuff (game):

#include "chesss.cpp" // can further divide into smaller files


// OS-specific stuff:

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <alsa/asoundlib.h>


// 8 squares, each square 60*60 pixels because 60*60 are the dimensions of our piece images.
i32 WindowWidth = 60 * 8;
i32 WindowHeight = 60 * 8;

// Messages sent to us by Window Manager to notify us about events we might want to handle ourselves.
enum {
	DeleteWindowMessage = 0,
	TakeFocusMessage = 1
};

bool Running = true;

//bool ReadFile(const char *FilePath, unsigned char **Contents, unsigned long *Size)
//{
//	int F = open(FilePath, O_RDONLY);
//	if (F == -1)
//	{
//		fprintf(stderr, "Failed to open file \"%s\"!\n", FilePath);
//		return false;
//	}
//
//	struct stat Stat;
//	if (fstat(F, &Stat) == -1)
//	{
//		fprintf(stderr, "Failed to stat file \"%s\"!\n", FilePath);
//		if (close(F) == -1)
//		{
//			fprintf(stderr, "Failed to close file \"%s\"!\n", FilePath);
//		}
//		return false;
//	}
//	*Size = Stat.st_size;
//
//	*Contents = (unsigned char *) malloc(*Size); //@ do we depend on standard library or not? Or depend on it minimally? How mmap() works?
//
//	if (read(F, ((void *) (*Contents)), ((size_t) (*Size))) == -1)
//	{
//		fprintf(stderr, "Failed to read from file \"%s\"!\n", FilePath);
//		if (close(F) == -1)
//		{
//			fprintf(stderr, "Failed to close file \"%s\"!\n", FilePath);
//		}
//		return false;
//	}
//
//	if (close(F) == -1)
//	{
//		fprintf(stderr, "Failed to close file \"%s\"!\n", FilePath);
//		return false;
//	}
//
//	return true;
//}

int64_t GetTimeUS()
{
	// (long int) time_t tv_sec
	// (long int) suseconds_t tv_usec
	timeval Time;
	gettimeofday(&Time, NULL);
	return Time.tv_sec * 1000000 + Time.tv_usec;
}

// struct square
// {
// 	int Num, Square, PiDigits, N1, N2, N3;
// };

// struct myTest
// {
// 	bool One, Two, Three;
// };

int main()
{
	// // test array
	// array<square *> Numbers; ArrayInit(&Numbers);
	// // array<square> Numbers; ArrayInit(&Numbers);

	// int64_t Time1 = GetTimeUS();

	// // ArrayAdd(&Numbers, 1);
	// // ArrayAdd(&Numbers, 2);
	// // ArrayAdd(&Numbers, 3);
	// // ArrayAdd(&Numbers, 5);
	// // ArrayAdd(&Numbers, 7);

	// // int *Pointer = ArrayAdd(&Numbers); *Pointer = 7;
	// // Pointer = ArrayAdd(&Numbers); *Pointer = 5;
	// // Pointer = ArrayAdd(&Numbers); *Pointer = 3;
	// // Pointer = ArrayAdd(&Numbers); *Pointer = 2;
	// // Pointer = ArrayAdd(&Numbers); *Pointer = 1;

	// for(int i = 0; i < 1000000; ++i)
	// {
	// 	// square Sq;
	// 	// Sq.Num = i;
	// 	// Sq.Square = i*i;
	// 	// Sq.PiDigits = 3141;
	// 	// Sq.N1 = 1*i;
	// 	// Sq.N2 = 2*i;
	// 	// Sq.N3 = 3*i;
	// 	// ArrayAdd(&Numbers, Sq);

	// 	// square *Sq = ArrayAdd(&Numbers);
	// 	// Sq->Num = i;
	// 	// Sq->Square = i*i;
	// 	// Sq->PiDigits = 3141;
	// 	// Sq->N1 = 1*i;
	// 	// Sq->N2 = 2*i;
	// 	// Sq->N3 = 3*i;

	// 	square *Sq = (square *)malloc(sizeof(square));
	// 	Sq->Num = i;
	// 	Sq->Square = i*i;
	// 	Sq->PiDigits = 3141;
	// 	Sq->N1 = 1*i;
	// 	Sq->N2 = 2*i;
	// 	Sq->N3 = 3*i;
	// 	ArrayAdd(&Numbers, Sq);
	// }

	// int64_t Time2 = GetTimeUS();
	// printf("elapsed: %ld\n", Time2 - Time1);

	// for(int i = 0; i < 9; ++i)
	// {
	// 	// printf("%d: %d, %d\n", i, Numbers.Data[i].Num, Numbers.Data[i].Square);
	// 	printf("%d: %d, %d\n", i, Numbers.Data[i]->Num, Numbers.Data[i]->Square);
	// }

	// return 0;

	/*
		In C++ you can do this:
			char *get_some_string()
			void get_some_string(char *Buffer, unsigned int BufferLength)
		Former allocates memory, the latter does not.

		void parent_path(char *path)
			-- changes the path in place

		void parent_path(const char *path, char *parent_path, unsigned int parent_path_len)
			-- put 'parent path' into a new buffer

		char *parent_path(const char *path)
			-- allocates memory for 'parent path'

		Problem is that the first and third have the same signature, there is no way to tell the compiler which one is being called. i dont think

		void parent_path(char *path)
		char *parent_path(char *path)
		Is it going to change the path in place or return a new copy?
	*/

	/*
	"/home/janedoe/jane'sfile" (absolute path)
	"./jane'sfile" (relative path)
	-- how long can the file(and directory)(-base-)names be? how many levels of nesting can there be?

	"../janedoe/../janedoe/.." (path with '..'s) -- there is no limit to how long this can be

	filesystem has to keep structured data on disk which describes the filesystem, so that file-data can be accessed and direcotries can be explored. more directories you have, more data they need to store. ultimately, as your filesystem data structures grow larger, you run out of disk space. so, given that any given disk can only store up to certain amount of data and no more, then the size of the disk does set an actual limit to how long a pathname can be. but nothing stops us from stacking multiple disks together and using some technology to make one filesystem span across multiple physical disks. how many disks at white storage capacity can be produced? how much storage cpacity can be produced and formatted using a single file system? what is this number?

	How long a path can be depends on the directory structure. But as far as I can see, there is no limit. There is no limit other than what is set by the filesystem. Or filename is very long.
	Windows limits file paths to 256 characters? D:\[256 chars]<NUL>
	*/
	// const unsigned int MaxPathWeUse = 1024;

	// char ExecutablePath[MaxPathWeUse];
	// if(get_executable_path(ExecutablePath, MaxPathWeUse))
	// {
	// 	path_make_parent(ExecutablePath);
	// 	if(chdir(ExecutablePath) == 0)
	// 	{
	// 		// we successfully changed the current working directory
	// 	}
	// 	else
	// 	{
	// 		// we failed to change the current working directory
	// 		assert(false);
	// 	}
	// }
	// else
	// {
	// 	// we failed to get the executable path
	// 	assert(false);
	// }

	// Make sure current working directory is the directory which contains the executable. (This might not be the case when the executable is executed through a symbolic link or through a Bash-shell from a different directory)
	const int exe_path_size = 1024; // allegedly MAX_PATH is not particularly trustworthy
	char exe_path[exe_path_size];
	if (!get_executable_path(exe_path, exe_path_size)) {
		//@ error handling
		fprintf(stderr, "failed\n");
		return 1;
	}
	char *parent_path = get_parent_path_noalloc(exe_path); //@
	chdir(parent_path);
	printf("Changed CWD to \"%s\"\n", parent_path);

	Display *Connection        = XOpenDisplay       (NULL);
	Window   DefaultRootWindow = XDefaultRootWindow (Connection);
	int      DefaultScreen     = XDefaultScreen     (Connection);
	Visual  *DefaultVisual     = XDefaultVisual     (Connection, DefaultScreen);
	int      DefaultDepth      = XDefaultDepth      (Connection, DefaultScreen);
	// int DisplayPlanes = XDisplayPlanes(Connection, DefaultScreen);

//	u8 *Data = (u8 *) malloc(4 * WindowWidth * WindowHeight);
//	image VideoBuffer = {WindowWidth, WindowHeight, Data};
//	XImage *Pixels = XCreateImage(Connection, DefaultVisual, DefaultDepth, ZPixmap, 0,
//		(char *) Data, VideoBuffer.Width, VideoBuffer.Height, 32, 0 );

	// To prevent the window from flickering when resized we change the "bit_gravity" value from its default "ForgetGravity" to "NorthWestGravity". "ForgetGravity" will cause X to repaint the whole window when the window is resized. What seems to be happening is that once X detects that the window was resized by the user, it repaints the whole window's client region with whatever background color was set for the window. Only after that, when we receive ConfigureNotify and Expose events, do we get to update the window's client region ourselves and if it takes us too long to get our own image onto the screen, the background color set by X will become noticeable and manifests itself as flickering.
	XSetWindowAttributes Attributes = {};
	Attributes.background_pixel = WhitePixel(Connection, DefaultScreen);
	Attributes.bit_gravity = NorthWestGravity; // If the window is resized, dont discard window's old content and keep it "attached" to the upper-left corner of the window. If the window is made larger, paint the newly appeared regions near the right and bottom edges of the window with the background color set for the window.
	Window W = XCreateWindow(
		Connection,
		DefaultRootWindow,
		0, 0, WindowWidth, WindowHeight,
		0,
		DefaultDepth,
		InputOutput,
		DefaultVisual,
		CWBackPixel|CWBitGravity,
		&Attributes
	);
	XStoreName(Connection, W, "Chesss");
	XMapWindow(Connection, W);
//	XFlush(Connection); // Make the window immediately visible.

	GC GraphicsContext = XCreateGC(Connection, W, 0, NULL);

	XSelectInput(Connection, W, ExposureMask|KeyPressMask|KeyReleaseMask|ButtonPressMask|ButtonReleaseMask|PointerMotionMask);

	// Tell Window Manager to notify us when the user wants to close the window or the window receives focus.
	Atom Protocols[2];
	Protocols[DeleteWindowMessage] = XInternAtom(Connection, "WM_DELETE_WINDOW", False);
	Protocols[TakeFocusMessage] = XInternAtom(Connection, "WM_TAKE_FOCUS", False);
	XSetWMProtocols(Connection, W, Protocols, 2);

	// Initialize sound
	i32 Latency = 6 * 16 * 1000; // for 30 fps
	// i32 Latency = 3 * 16 * 1000; // for 60 fps
	i32 NumChannels = 2;
	i32 SamplesPerSecond = 48000;

	snd_pcm_t *SoundHandle;
	if (snd_pcm_open(&SoundHandle, "default", SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK))
	{
		fprintf(stderr, "Error: snd_pcm_open()\n");
		return 1;
	}

	/*@ On my Arch Linux machine, on first try, I got an error: "ALSA lib pcm.c:8924:(snd_pcm_set_params) Unable to set hw params for PLAYBACK: Input/output error", but it worked fine the following times. Maybe do multiple tries before we give up?*/
	if (snd_pcm_set_params(SoundHandle, SND_PCM_FORMAT_S16_LE, SND_PCM_ACCESS_RW_INTERLEAVED, NumChannels, SamplesPerSecond, 0, Latency))
	{
		fprintf(stderr, "Error: snd_pcm_set_params()\n");
		return 1;
	}

	snd_pcm_status_t *SoundStatus;
	snd_pcm_status_alloca(&SoundStatus);

//	//@ what about 24-bit samples?
//	soundBuffer SoundBuffer = {}; //@ is this initialized to 0?
//	SoundBuffer.BytesInFrame = 2;
//	SoundBuffer.TotalNumFrames = SamplesPerSecond; //@ 'frames', 'samples' ...
//	SoundBuffer.Data = malloc(SoundBuffer.TotalNumFrames * SoundBuffer.BytesInFrame);
////	SoundBuffer.FirstUncopiedFrame = (u8 *) SoundBuffer.Data;
//	SoundBuffer.CurrentFrameIndex = 0;
//	SoundBuffer.PreviousFrameIndex = 0;

	InitApp();

	i64 PreviousTimeUS;
	u64 FrameCount = 0; // when does this "roll over" back to 0 again?
	while (Running)
	{
		if (FrameCount > 0)
		{
			i64 CurrentTimeUS = GetTimeUS();
			// i64 ElapsedUS = CurrentTimeUS - PreviousTimeUS;
			// i64 ElapsedMS = (i64)(((r64)ElapsedUS/1000)+0.5);
			// printf("%ld ms\n", ElapsedMS);
			PreviousTimeUS = CurrentTimeUS;
		}
		else
		{
			PreviousTimeUS = GetTimeUS();
		}
		FrameCount += 1;

		userInput GameInput; ArrayInit(&GameInput.Events);

		// once in a while take long time to generate a frame to test our capability to recover from EPIPE (underrun) error
//		if (FrameCount % 30 == 0)
//		{
//			usleep(50*1000); // 50 milliseconds
//		}

		while (XPending(Connection))
		{
			XEvent e;
			XNextEvent(Connection, &e);
			switch (e.type)
			{
				case Expose:
				{
					// printf("Expose event!\n");
					// XPutImage(Connection, W, GraphicsContext, Pixels, 0, 0, 0, 0, Pixels->width, Pixels->height);
					break;
				}
				case ClientMessage:
				{
					XClientMessageEvent Message = e.xclient;
					if (Message.data.l[0] == Lib::SafeUnsignedToSigned(Protocols[DeleteWindowMessage]))
					{
						printf("ClientMessage (WM_DELETE_WINDOW) event!\n");
						Running = false;
					}
					else if (Message.data.l[0] == Lib::SafeUnsignedToSigned(Protocols[TakeFocusMessage]))
					{
						printf("ClientMessage (WM_TAKE_FOCUS) event!\n");
					}
					break;
				}
				case KeyPress:
				{
					XKeyEvent KeyEvent = e.xkey;
					printf("KeyPress [keycode: %u]\n", KeyEvent.keycode);

					// // UP: 111, DOWN: 116, LEFT: 113, RIGHT: 114
					// if (KeyEvent.keycode == 111) Y -= 10;
					// if (KeyEvent.keycode == 116) Y += 10;
					// if (KeyEvent.keycode == 113) X -= 10;
					// if (KeyEvent.keycode == 114) X += 10;

					event Event;
					Event.Type = KEYPRESS;
					Event.KeyCode = KeyEvent.keycode;
					ArrayAdd(&GameInput.Events, Event);

					break;
				}
//				case KeyRelease:
//				{
//					XKeyEvent KeyEvent = e.xkey;
//					printf("KeyRelease [keycode: %u]\n", KeyEvent.keycode);
//					break;
//				}
				case ButtonPress:
				{
					XButtonEvent *ButtonEvent = (XButtonEvent *)&e.xbutton;
					if (ButtonEvent->button == 1)
					{
						event Event;
						Event.Type = LBUTTONDOWN;
						Event.X = ButtonEvent->x;
						Event.Y = ButtonEvent->y;
						ArrayAdd(&GameInput.Events, Event);
					}
					break;
				}
				case ButtonRelease:
				{
					XButtonEvent ButtonEvent = e.xbutton;
					if (ButtonEvent.button == 1)
					{
						event Event;
						Event.Type = LBUTTONUP;
						Event.X = ButtonEvent.x;
						Event.Y = ButtonEvent.y;
						ArrayAdd(&GameInput.Events, Event);
					}
					break;
				}
				case MotionNotify:
				{
					XMotionEvent MotionEvent = e.xmotion;
//					printf("MotionNotify [x: %d, y: %d]\n", MotionEvent.x, MotionEvent.y);
					GameInput.LastMouseX = MotionEvent.x;
					GameInput.LastMouseY = MotionEvent.y;
					break;
				}
			}
		}

		u8 *Data = (u8 *) malloc(4 * WindowWidth * WindowHeight);
		XImage *Pixels = XCreateImage(
			Connection,
			DefaultVisual,
			DefaultDepth,
			ZPixmap,
			0,
			(char *) Data,
			WindowWidth,
			WindowHeight,
			32,
			0);
		image VideoBuffer = {WindowWidth, WindowHeight, Data};

		snd_pcm_status(SoundHandle, SoundStatus);
		snd_pcm_uframes_t NumAvailableFrames = snd_pcm_status_get_avail(SoundStatus); //?
//		printf("NumAvailableFrames: %lu\n", NumAvailableFrames);

		soundBuffer SoundBuffer = {};
		SoundBuffer.NumSamplesWanted = NumAvailableFrames;
//		SoundBuffer.NumSamplesWanted = 1600;
		SoundBuffer.Samples = malloc(SoundBuffer.NumSamplesWanted * 2 * NumChannels);

		UpdateApp(&VideoBuffer, &SoundBuffer, &GameInput);
//		GameInput.Events.Count = 0;
		free(GameInput.Events.Data);
		
		XPutImage(Connection, W, GraphicsContext, Pixels, 0, 0, 0, 0, Pixels->width, Pixels->height);
		XDestroyImage(Pixels);

		int FramesCopied = 0;
		int LoopCount = 0;
		while (FramesCopied < SoundBuffer.NumSamplesWanted)
		{
			int BytesInFrame = 2 * NumChannels;
			void *PtrFirstUncopied = (void *) (((i8 *) SoundBuffer.Samples) + FramesCopied * BytesInFrame); 
			snd_pcm_sframes_t Result = snd_pcm_writei(SoundHandle, PtrFirstUncopied, SoundBuffer.NumSamplesWanted - FramesCopied);
			LoopCount += 1;
			if (Result < 0)
			{
				// error
				if (Result == -EAGAIN)
				{
					printf("EAGAIN\n");
					continue;
				}
				else if (Result == -EPIPE)
				{
					Result = snd_pcm_recover(SoundHandle, Result, 0);
					printf("EPIPE\n");
					if (Result == 0)
					{
						// success
						printf("Successfully recovered from EPIPE\n");
						continue;
					}
					else
					{
						// fail
						printf("Failed to recover from EPIPE\n");
						assert(false);
					}
				}
				else
				{
					assert(false);
				}
			}
			FramesCopied += Result;
		}
//		printf("LoopCount: %d\n", LoopCount);
		assert(LoopCount < 2);

		free(SoundBuffer.Samples);

//		snd_pcm_sframes_t FramesCopied = snd_pcm_writei(
//			PCMHandle,
////			SoundBuffer.FirstUncopiedFrame,
//			((u8 *) SoundBuffer.Data) + SoundBuffer.CurrentFrameIndex * SoundBuffer.BytesInFrame,
//			SoundBuffer.TotalNumFrames - SoundBuffer.CopiedNumFrames);
//		if (FramesCopied == -EAGAIN)
//		{
//			// nothing written
//			FramesCopied = 0;
//		}
//		else if (FramesCopied < 0)
//		{
//			if (FramesCopied == -EPIPE)
//			{
//				fprintf(stderr, "error: EPIPE\n");
//			}
//			else
//			{
//				fprintf(stderr, "error: snd_pcm_writei()\n");
//			}
//			return 1;
//		}
//////		Ptr += SamplesCopied * Sound.BytesPerSampleInMemory * Sound.NumChannels;
////		Ptr += SamplesCopied * 2;
////		SamplesToCopy -= SamplesCopied;
////		assert(SamplesToCopy >= 0);
////		if (SamplesToCopy == 0)
////		{
////			Ptr = (u8 *) Sound.Samples;
////			SamplesToCopy = Sound.NumSamples;
//////			printf("SamplesToCopy: %d\n", SamplesToCopy);
////		}
//		SoundBuffer.CopiedNumFrames += FramesCopied;
////		SoundBuffer.FirstUncopiedFrame += SoundBuffer.BytesInFrame * FramesCopied;
//		SoundBuffer.PreviousFrameIndex = SoundBuffer.CurrentFrameIndex;
//		SoundBuffer.CurrentFrameIndex += FramesCopied;
////		printf("Frames copied: %d\n", SoundBuffer.CopiedNumFrames);
////		printf("current: %d, previous: %d\n", SoundBuffer.CurrentFrameIndex, SoundBuffer.PreviousFrameIndex);
//		if (SoundBuffer.CopiedNumFrames == SoundBuffer.TotalNumFrames)
//		{
//			SoundBuffer.CopiedNumFrames = 0;
////			SoundBuffer.FirstUncopiedFrame = (u8 *) SoundBuffer.Data;
//			SoundBuffer.CurrentFrameIndex = 0;
//		}

		i64 TargetFrameTimeUS = 33000; // ~ 30 fps
		// i64 TargetFrameTimeUS = 17000; // ~ 60 fps
		// i64 TargetFrameTimeUS = 0;
		i64 CurrentTimeUS = GetTimeUS();
		i64 ElapsedUS = CurrentTimeUS - PreviousTimeUS;
		// if (TargetFrameTimeUS - ElapsedUS > 0)
		// {
		// 	unsigned int SleepForUS = TargetFrameTimeUS - ElapsedUS; // usleep takes unsigned int
		// 	usleep(SleepForUS);
		// }
		i64 Diff = TargetFrameTimeUS - ElapsedUS;
		if (Diff > 0)
		{
			unsigned int SleepForUS = Diff; //@ usleep takes unsigned int
			usleep(SleepForUS);
		}
	}

	XDestroyWindow(Connection, W);
	XCloseDisplay(Connection);

	return 0;
}