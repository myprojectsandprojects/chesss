
//@ does 'sample' include all channels or not?
// yeah, I am beginning to think it shouldnt. if one channel, there is one sample and if two channels, there are two samples (left sample and right sample). So, we are currently wrong.
struct loadedSound
{
	int NumSamples;
	void *Samples;
	i32 NumChannels;
	i32 BytesPerSample;
	i32 SamplesPerSecond;
//	i32 BytesPerSampleInMemory;
};

struct playingSound
{
//	int FrameIndex;
	playingSound *Next;
	playingSound *Previous;
	int NumSamplesPlayed;
	loadedSound *Sound;
	bool IsLooping;
};

struct image
{
	i32 Width;
	i32 Height;
	u8 *Data;
};

//@ data needs to be freed somehow
bool LoadBMP(const char *FilePath, image *BMPImage)
{
	size_t Size;
	u8 *Contents;
	if (!ReadFile(FilePath, &Contents, &Size))
	{
		return false;
	}

	u8 *p = Contents;
	
	// First 2 bytes should be ASCII characters 'B' and 'M'
	// u8 b1 = *p;
	++p;
	// u8 b2 = *p;
	++p;
//	printf("%c%c\n", b1, b2);

	// Size of the image file in bytes
	u32 BMPSize = *((u32 *) p);
	p += 4;
	assert(BMPSize == Size);

	p += 4; // Skip reserved bytes

	// Offset of the image data
	u32 Offset = *((unsigned int *) p);
	p += 4;

	// DIB (Device Independent Bitmap) size (version of the DIB header)
	// u32 DIBSize = *((unsigned int *) p);
	p += 4;

	// Image width and height
	u32 Width = *((u32 *) p);
	p += 4;

	u32 Height = *((u32 *) p);
	p += 4;

	// u16 Planes = *((u16 *) p);
	p += 2;
	
	// u16 BitsPerPixel = *((u16 *) p);
	p += 2;

	BMPImage->Width = Width;
	BMPImage->Height = Height;
	BMPImage->Data = Contents + Offset;

	return true;
}

// Would love to do this, then have pointer to this type of struct and simply point it to the data that we read from the file, but sadly not all WAV files have the same format. The ones I am dealing with currently have an additional "LIST" chunk between "fmt" chunk and "data" chunk.
// This seemed to work on GCC and clang
// #pragma pack(push)
// #pragma pack(2)
// struct wavFile
// {
// 	u8 ChunkId[4]; // "RIFF"
// 	u32 ChunkSize;
// 	u32 Format; // big endian??? is it a string?

// 	u8 Subchunk1Id[4]; // "fmt "
// 	u32 Subchunk1Size;
// 	u16 AudioFormat;
// 	u16 NumChannels;
// 	u32 SampleRate;
// 	u32 ByteRate;
// 	u16 BlockAlign;
// 	u16 BitsPerSample;

// 	u8 Sunbchunk2Id[4]; // "data"
// 	u32 Subchunk2Size;
// 	// ... data
// };
// #pragma pack(pop) // restore default alignment for other structs

bool LoadWAVFile(const char *FilePath, loadedSound *Sound)
{
	size_t NumBytes;
	u8 *FileContents;
	if (!ReadFile(FilePath, &FileContents, &NumBytes))
	{
		fprintf(stderr, "error: LoadWAVFile: ReadFile\n");
		return false;
	}
	printf("read %s (%lu bytes)\n", FilePath, NumBytes);

	int32_t offset;
	// if(!find_bytes((uint8_t const *)"fmt ", 4, FileContents, NumBytes, &offset))
	// {
	// 	fprintf(stderr, "error: LoadWAVFile: couldnt find fmt-chunk!\n");
	// 	return false;
	// }
	if((offset = forward_find_bm(FileContents, NumBytes, (const uint8_t *)"fmt", c_str_len("fmt"))) == -1) {
		fprintf(stderr, "error: LoadWAVFile: couldnt find fmt-chunk!\n");
		return false;
	}
	printf("found fmt-chunk at %d\n", offset);

	u8 *Ptr = FileContents + offset;
	Ptr += 4;

	u32 FmtChunkSize = *((u32 *)Ptr); Ptr += sizeof(u32);
	u16 AudioFormat = *((u16 *)Ptr); Ptr += sizeof(u16);
	u16 NumChannels = *((u16 *)Ptr); Ptr += sizeof(u16);
	u32 SampleRate = *((u32 *)Ptr); Ptr += sizeof(u32);
	u32 ByteRate = *((u32 *)Ptr); Ptr += sizeof(u32);
	u16 BlockAlign = *((u16 *)Ptr); Ptr += sizeof(u16);
	u16 BitsPerSample = *((u16 *)Ptr); Ptr += sizeof(u16);

	printf("FmtChunkSize: %u\n", FmtChunkSize);
	printf("AudioFormat: %u\n", AudioFormat);
	printf("NumChannels: %u\n", NumChannels);
	printf("SampleRate: %u\n", SampleRate);
	printf("ByteRate: %u\n", ByteRate);
	printf("BlockAlign: %u\n", BlockAlign);
	printf("BitsPerSample: %u\n", BitsPerSample);

	// if(!find_bytes((uint8_t const *)"data", 4, FileContents, NumBytes, &offset))
	// {
	// 	fprintf(stderr, "error: LoadWAVFile: couldnt find data-chunk!\n");
	// 	return false;
	// }
	if((offset = forward_find_bm(FileContents, NumBytes, (const uint8_t *)"data", c_str_len("data"))) == -1) {
		fprintf(stderr, "error: LoadWAVFile: couldnt find fmt-chunk!\n");
		return false;
	}
	printf("found data-chunk at %d\n", offset);

	Ptr = FileContents + offset;
	Ptr += 4;

	u32 DataChunkSize = *((u32 *)Ptr); // the rest of it (samples)
	Ptr += 4;
	printf("DataChunkSize: %u\n", DataChunkSize);

	int BytesPerSample = BitsPerSample / 8; // 32-bit unsigned / 32-bit signed. this is no good. in reality BitsPerSample / 8 is not going to overflow BytesPerSample because it would have to be greater than 2,147,483,647. but in some other situations it might matter and I dont really know what the right thing to do is.
	Sound->NumSamples = DataChunkSize / BytesPerSample / NumChannels; //@ here, for example, we might actually exceed the capacity of NumSamples which is signed 32-bit.
	Sound->Samples = Ptr;
	Sound->NumChannels = NumChannels;
	Sound->BytesPerSample = BytesPerSample;
	Sound->SamplesPerSecond = SampleRate;

	return true;
}