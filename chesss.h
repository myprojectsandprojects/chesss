// struct image
// {
// 	i32 Width;
// 	i32 Height;
// 	u8 *Data;
// };

//// 'frame' includes all channels
//struct soundBuffer
//{
//	void *Data;
//	i32 BytesInFrame;
//	i32 TotalNumFrames;
//	i32 CopiedNumFrames;
////	u8 *FirstUncopiedFrame;
//	int CurrentFrameIndex;
//	int PreviousFrameIndex;
//};
struct soundBuffer
{
	int NumSamplesWanted;
	void *Samples;
};

// //@ does 'sample' include all channels or not?
// struct loadedSound
// {
// 	int NumSamples;
// 	void *Samples;
// 	i32 NumChannels;
// 	i32 BytesPerSample;
// 	i32 SamplesPerSecond;
// //	i32 BytesPerSampleInMemory;
// // };

// struct playingSound
// {
// //	int FrameIndex;
// 	playingSound *Next;
// 	playingSound *Previous;
// 	int NumSamplesPlayed;
// 	loadedSound *Sound;
// 	bool IsLooping;
// };

enum eventType
{
	LBUTTONDOWN,
	LBUTTONUP
};

struct event
{
	eventType Type;
	i32 X;
	i32 Y;
};

//event *EventNew(eventType Type, i32 X, i32 Y)
//{
//	event *Event = (event *) malloc(sizeof(event));
//	Event->Type = Type;
//	Event->X = X;
//	Event->Y = Y;
//	return Event;
//}

struct userInput
{
	array<event> Events;
	i32 LastMouseX;
	i32 LastMouseY;
};

// Game exports:
//void GameInit();
//void GameUpdate(image *WindowBuffer, userInput *Input);