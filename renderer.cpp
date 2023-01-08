void RenderHandmadeheroGradient(image *Buffer, i32 OffsetX, i32 OffsetY)
{
//	u32 *Row = (u32 *) Buffer->Data;
	u32 *Pixel = (u32 *) Buffer->Data;
	for (i32 Y = 0; Y < Buffer->Height; ++Y)
	{
//		u32 *Pixel = Row;
		for (i32 X = 0; X < Buffer->Width; ++X)
		{
			u8 Red = X + OffsetX;
			u8 Green = Y + OffsetY;
			u8 Blue = 0;
			*Pixel = (0 << 24) | (Red << 16) | (Green << 8) | (Blue); // prgb
			++Pixel;
		}
//		Row += Buffer->Width;
	}
}

void RenderHandmadeheroGradient2(image *Buffer, i32 XOffset, i32 YOffset)
{
	for (int Y = 0; Y < Buffer->Height; ++Y)
	{
		for (int X = 0; X < Buffer->Width; ++X)
		{
//			// 1: [blue] 2: [green] 3: [red] 4: [padding] ...
//			char *Pixel = Data + X * 4 + Y * WindowWidth * 4;
//			*Pixel = BlueValue; Pixel += 1;
//			*Pixel = GreenValue; Pixel += 1;
//			*Pixel = RedValue; Pixel += 1;
//			*Pixel = 0;

			i32 *Pixel = ((int *) Buffer->Data) + X + Y * Buffer->Width;
			u8 Red = X + XOffset;
			u8 Green = 0;
			u8 Blue = Y + YOffset;
			*Pixel = (Red << 16) | (Green << 8) | (Blue);
		}
	}
}

float Lerp(r32 a, r32 b, r32 t)
{
	return a * (1 - t) + b * t;
}

void RenderRectangle(image *WindowBuffer, i32 XPos, i32 YPos, i32 Width, i32 Height, u32 Color)
{
	if (XPos > (WindowBuffer->Width - 1))
	{
//		X = Pixels->width - 1;
		return;
	}
	if (YPos > (WindowBuffer->Height - 1))
	{
//		Y = Pixels->height - 1;
		return;
	}
	if (XPos < 0)
	{
		if (XPos + Width > 0)
		{
			Width = XPos + Width;
			XPos = 0;
		}
		else
		{
			return;
		}
	}
	if (YPos < 0)
	{
		if (YPos + Height > 0)
		{
			Height = YPos + Height;
			YPos = 0;
		}
		else
		{
			return;
		}
	}
	if ((XPos + Width) > WindowBuffer->Width)
	{
		Width = WindowBuffer->Width - XPos;
	}
	if ((YPos + Height) > WindowBuffer->Height)
	{
		Height = WindowBuffer->Height - YPos;
	}

	for (int Y = 0; Y < Height; ++Y)
	{
		u32 *P1 = ((u32 *) WindowBuffer->Data) + ((YPos + Y) * WindowBuffer->Width + XPos);
		for (int X = 0; X < Width; ++X)
		{
			u32 *P2 = P1 + X;
			*P2 = Color;
		}
	}
}

//void RenderImage(XImage *Dest, image *Src, int XPos, int YPos)
void RenderImage(image *WindowBuffer, image *Image, i32 XPos, i32 YPos)
{
	int RenderX = 0;
	int RenderY = 0;
	int RenderWidth = Image->Width;
	int RenderHeight = Image->Height;
	if (YPos < 0)
	{
		if (-YPos < Image->Height)
		{
			RenderY = -YPos;
		}
		else
		{
			return;
		}
	}
	if (YPos + Image->Height > WindowBuffer->Height)
	{
		if (YPos < WindowBuffer->Height)
		{
			RenderHeight = WindowBuffer->Height - YPos;
		}
		else
		{
			return;
		}
	}
	if (XPos < 0)
	{
		if (-XPos < Image->Width)
		{
			RenderX = -XPos;
		}
		else
		{
			return;
		}
	}
	if (XPos + Image->Width > WindowBuffer->Width)
	{
		if (XPos < WindowBuffer->Width)
		{
			RenderWidth = WindowBuffer->Width - XPos;
		}
		else
		{
			return;
		}
	}
	for (int Y = RenderY; Y < RenderHeight; ++Y)
	{
		for (int X = RenderX; X < RenderWidth; ++X)
		{
			int ImageY = (Image->Height - 1) - Y; // To flip the image
			unsigned int *ImagePixel = ((unsigned int *) Image->Data) + ImageY * Image->Width + X;
			unsigned int *WindowBufferPixel = ((unsigned int *) WindowBuffer->Data) + (Y + YPos) * WindowBuffer->Width + (X + XPos);

			unsigned char ImageR = (*ImagePixel & 0x0000ff00) >> 8;
			unsigned char ImageG = (*ImagePixel & 0x00ff0000) >> 16;
			unsigned char ImageB = (*ImagePixel & 0xff000000) >> 24;
			
			unsigned char WindowBufferR = (*WindowBufferPixel & 0x00ff0000) >> 16;
			unsigned char WindowBufferG = (*WindowBufferPixel & 0x0000ff00) >> 8;
			unsigned char WindowBufferB = (*WindowBufferPixel & 0x000000ff) >> 0;
					
			float Alpha = ((float) (*ImagePixel & 0x000000ff)) / 255.0f;
			unsigned char BlendedRed = (unsigned char) Lerp((float) WindowBufferR, (float) ImageR, Alpha);
			unsigned char BlendedGreen = (unsigned char) Lerp((float) WindowBufferG, (float) ImageG, Alpha);
			unsigned char BlendedBlue = (unsigned char) Lerp((float) WindowBufferB, (float) ImageB, Alpha);
			
			*WindowBufferPixel = BlendedRed << 16 | BlendedGreen << 8 | BlendedBlue;
		}
	}
}