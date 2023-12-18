//@ dragging a piece outside of the window and then releasing the mouse button causes some weird behaviour.

/*
Todo:
- play sound when a move is made
- play an animation when a computer makes a move
- highlight possible moves
- add castling move
- add en passant move
- add pawn promotion move
- recognize a draw
	- stalemate
	- insufficient material
	- repetition
	- move number (?)
- make computer play better
- computer moves should be evaluated in a separate thread (?)
- possibility to start a new game and choose a color (also choose perspective?)
- click-move a piece in addition to drag-move
- display taken pieces somewhere
- dont forget the Windows version!
- add notation to the chess board (a - h and 1 - 8)
- display an interactive list of moves next to the board
- make window/board resizeable (?)

- Should reading files and loading assets be a game thing though? The only reason why reading a file can be platform independent currently is because we use standard library for this and we expect standard library to be cross platform. The underlying api's (which are used by the standard library) differ across OS's. The file formats, like wav and bmp, are not platform dependent, so the code that parses these file formats isnt also, but its also not really part of the game logic, so maybe we should at least divide our platform independent code into game code and other code or ulility code or helper code or something?
*/

enum playerType
{
	HUMAN,
	MACHINE
};

enum color
{
	WHITE = 0,
	BLACK = 1
};

struct player
{
	playerType PlayerType;
};

struct move
{
	int FromX;
	int FromY;
	int ToX;
	int ToY;
};

//loadedSound TestSound;
loadedSound MoveSound;
loadedSound CaptureSound;
loadedSound CheckSound;

image WKingImage;
image BKingImage;
image WQueenImage;
image BQueenImage;
image WRookImage;
image BRookImage;
image WBishopImage;
image BBishopImage;
image BKnightImage;
image WPawnImage;
image WKnightImage;
image BPawnImage;

//playingSound PlayingSound;
//array<playingSound> PlayingSounds;
playingSound *FirstPlayingSound;

#include "lib/linked_list.hpp"
lib::linkedList<playingSound *> PlayingSounds; // The only reason its a linked list is because I wanted to try out a linked list.

image *Board[8 * 8];

bool IsDraggedPiece;
int DraggedPieceX;
int DraggedPieceY;
int DraggedPiecePixelX;
int DraggedPiecePixelY;

color WhosMove;
bool GameOver;

move *NewMove(int FromX, int FromY, int ToX, int ToY)
{
	move *Move = (move *) malloc(sizeof(move));
	Move->FromX = FromX;
	Move->FromY = FromY;
	Move->ToX = ToX;
	Move->ToY = ToY;
	return Move;
}

// FEN: [PLACEMENT] [MOVE] [CASTLING] [EN PASSANT] [# of HALFMOVES] [# of FULLMOVES]
// [PLACEMENT of the pieces] "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR"
// [who MOVEs next] w/b
// [CASTLING rights]: KQkq/-
// [EN PASSANT] square in algebraic notation (e3)
// [# of HALFMOVES]: Halfmoves made since the last pawn advance or the piece capture. When this counter reaches 100 the game ends in a draw
void SetUpBoard(image *Board[], const char *Fen)
{
//	for (Y = 0; Y < 8; ++Y)
//	{
//		for (X = 0; X < 8; ++X)
//		{
//			Board[Y * 8 + X] = NULL;
//		}
//	}
	int I = 0;
	for (const char *P = Fen; *P; ++P)
	{
//		printf("%c, %d\n", *P, I);

		switch (*P)
		{
			case 'r': Board[I] = &BRookImage; ++I; break;
			case 'R': Board[I] = &WRookImage; ++I; break;
			case 'n': Board[I] = &BKnightImage; ++I; break;
			case 'N': Board[I] = &WKnightImage; ++I; break;
			case 'b': Board[I] = &BBishopImage; ++I; break;
			case 'B': Board[I] = &WBishopImage; ++I; break;
			case 'q': Board[I] = &BQueenImage; ++I; break;
			case 'Q': Board[I] = &WQueenImage; ++I; break;
			case 'k': Board[I] = &BKingImage; ++I; break;
			case 'K': Board[I] = &WKingImage; ++I; break;
			case 'p': Board[I] = &BPawnImage; ++I; break;
			case 'P': Board[I] = &WPawnImage; ++I; break;
			case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': I += *P - 48; break;
			default:
			{
				if (*P != '/')
				{
					printf("DO WE EVER DO THIS?"); // nope
					++I;
				}
			}
		}
	}
}

bool IsWhitePiece(image *Piece)
{
	return (Piece == &WPawnImage || Piece == &WRookImage || Piece == &WKnightImage || Piece == &WBishopImage || Piece == &WQueenImage || Piece == &WKingImage);
}

//@ what if its an empty square?
bool IsBlackPiece(image *Piece)
{
	return !IsWhitePiece(Piece);
}

void GetMovesForPiece(image *Board[], int PieceX, int PieceY, array<move *> *Moves)
{
	image *Piece = Board[PieceY * 8 + PieceX];

	if (Piece == &WRookImage || Piece == &BRookImage || Piece == &WQueenImage || Piece == &BQueenImage)
	{
		// scan horizontally to the right
		for (int X = PieceX + 1; X < 8; ++X)
		{
			int I = PieceY * 8 + X;
			if (Board[I])
			{
				// is this an ally or an enemy piece?
				if ((IsWhitePiece(Piece) && IsWhitePiece(Board[I]))
					|| (IsBlackPiece(Piece) && IsBlackPiece(Board[I])))
				{
					break;
				}
				move *Move = (move *) malloc(sizeof(move));
				Move->FromX = PieceX;
				Move->FromY = PieceY;
				Move->ToX = X;
				Move->ToY = PieceY;
				ArrayAdd(Moves, Move);
//				ArrayAdd(Moves, MoveNew(PieceX, PieceY, X, PieceY));
				break;
			}
			else
			{
				move *Move = (move *) malloc(sizeof(move));
				Move->FromX = PieceX;
				Move->FromY = PieceY;
				Move->ToX = X;
				Move->ToY = PieceY;
				ArrayAdd(Moves, Move);
			}
		}

		// scan horizontally to the left
		for (int X = PieceX - 1; X >= 0; --X)
		{
			int I = PieceY * 8 + X;
			if (Board[I])
			{
				// is this an ally or an enemy piece?
				if ((IsWhitePiece(Piece) && IsWhitePiece(Board[I])) || (IsBlackPiece(Piece) && IsBlackPiece(Board[I])))
				{
					break;
				}
				move *Move = (move *) malloc(sizeof(move));
				Move->FromX = PieceX;
				Move->FromY = PieceY;
				Move->ToX = X;
				Move->ToY = PieceY;
				ArrayAdd(Moves, Move);
				break;
			}
			else
			{
				move *Move = (move *) malloc(sizeof(move));
				Move->FromX = PieceX;
				Move->FromY = PieceY;
				Move->ToX = X;
				Move->ToY = PieceY;
				ArrayAdd(Moves, Move);
			}
		}

		// scan vertically to the up
		for (int Y = PieceY - 1; Y >= 0; --Y)
		{
			int I = Y * 8 + PieceX;
			if (Board[I])
			{
				// is this an ally or an enemy piece?
				if ((IsWhitePiece(Piece) && IsWhitePiece(Board[I])) || (IsBlackPiece(Piece) && IsBlackPiece(Board[I])))
				{
					break;
				}
				move *Move = (move *) malloc(sizeof(move));
				Move->FromX = PieceX;
				Move->FromY = PieceY;
				Move->ToX = PieceX;
				Move->ToY = Y;
				ArrayAdd(Moves, Move);
				break;
			}
			else
			{
				move *Move = (move *) malloc(sizeof(move));
				Move->FromX = PieceX;
				Move->FromY = PieceY;
				Move->ToX = PieceX;
				Move->ToY = Y;
				ArrayAdd(Moves, Move);
			}
		}
		// scan vertically to the down
		for (int Y = PieceY + 1; Y < 8; ++Y)
		{
			int I = Y * 8 + PieceX;
			if (Board[I])
			{
				// is this an ally or an enemy piece?
				if ((IsWhitePiece(Piece) && IsWhitePiece(Board[I])) || (IsBlackPiece(Piece) && IsBlackPiece(Board[I])))
				{
					break;
				}
				move *Move = (move *) malloc(sizeof(move));
				Move->FromX = PieceX;
				Move->FromY = PieceY;
				Move->ToX = PieceX;
				Move->ToY = Y;
				ArrayAdd(Moves, Move);
				break;
			}
			else
			{
				move *Move = (move *) malloc(sizeof(move));
				Move->FromX = PieceX;
				Move->FromY = PieceY;
				Move->ToX = PieceX;
				Move->ToY = Y;
				ArrayAdd(Moves, Move);
			}
		}
	}

	if (Piece == &WBishopImage || Piece == &BBishopImage || Piece == &WQueenImage || Piece == &BQueenImage)
	{
		// scan diagonally in down-right direction
		for (int X = PieceX + 1, Y = PieceY + 1; X < 8 && Y < 8; ++X, ++Y)
		{
			int I = Y * 8 + X;
			if (Board[I])
			{
				// is this an ally or an enemy piece?
				if ((IsWhitePiece(Piece) && IsWhitePiece(Board[I])) || (IsBlackPiece(Piece) && IsBlackPiece(Board[I])))
				{
					break;
				}
				move *Move = (move *) malloc(sizeof(move));
				Move->FromX = PieceX;
				Move->FromY = PieceY;
				Move->ToX = X;
				Move->ToY = Y;
				ArrayAdd(Moves, Move);
				break;
			}
			else
			{
				move *Move = (move *) malloc(sizeof(move));
				Move->FromX = PieceX;
				Move->FromY = PieceY;
				Move->ToX = X;
				Move->ToY = Y;
				ArrayAdd(Moves, Move);
			}
		}

		// scan diagonally in down-left direction
		for (int X = PieceX - 1, Y = PieceY + 1; X >= 0 && Y < 8; --X, ++Y)
		{
			int I = Y * 8 + X;
			if (Board[I])
			{
				// is this an ally or an enemy piece?
				if ((IsWhitePiece(Piece) && IsWhitePiece(Board[I])) || (IsBlackPiece(Piece) && IsBlackPiece(Board[I])))
				{
					break;
				}
				move *Move = (move *) malloc(sizeof(move));
				Move->FromX = PieceX;
				Move->FromY = PieceY;
				Move->ToX = X;
				Move->ToY = Y;
				ArrayAdd(Moves, Move);
				break;
			}
			else
			{
				move *Move = (move *) malloc(sizeof(move));
				Move->FromX = PieceX;
				Move->FromY = PieceY;
				Move->ToX = X;
				Move->ToY = Y;
				ArrayAdd(Moves, Move);
			}
		}

		// scan diagonally in top-left direction
		for (int X = PieceX - 1, Y = PieceY - 1; X >= 0 && Y >= 0; --X, --Y)
		{
			int I = Y * 8 + X;
			if (Board[I])
			{
				// is this an ally or an enemy piece?
				if ((IsWhitePiece(Piece) && IsWhitePiece(Board[I])) || (IsBlackPiece(Piece) && IsBlackPiece(Board[I])))
				{
					break;
				}
				move *Move = (move *) malloc(sizeof(move));
				Move->FromX = PieceX;
				Move->FromY = PieceY;
				Move->ToX = X;
				Move->ToY = Y;
				ArrayAdd(Moves, Move);
				break;
			}
			else
			{
				move *Move = (move *) malloc(sizeof(move));
				Move->FromX = PieceX;
				Move->FromY = PieceY;
				Move->ToX = X;
				Move->ToY = Y;
				ArrayAdd(Moves, Move);
			}
		}

		// scan diagonally in top-right direction
		for (int X = PieceX + 1, Y = PieceY - 1; X < 8 && Y >= 0; ++X, --Y)
		{
			int I = Y * 8 + X;
			if (Board[I])
			{
				// is this an ally or an enemy piece?
				if ((IsWhitePiece(Piece) && IsWhitePiece(Board[I])) || (IsBlackPiece(Piece) && IsBlackPiece(Board[I])))
				{
					break;
				}
				move *Move = (move *) malloc(sizeof(move));
				Move->FromX = PieceX;
				Move->FromY = PieceY;
				Move->ToX = X;
				Move->ToY = Y;
				ArrayAdd(Moves, Move);
				break;
			}
			else
			{
				move *Move = (move *) malloc(sizeof(move));
				Move->FromX = PieceX;
				Move->FromY = PieceY;
				Move->ToX = X;
				Move->ToY = Y;
				ArrayAdd(Moves, Move);
			}
		}
	}

	if (Piece == &WKingImage || Piece == &BKingImage)
	{
		double Pi = 3.14159;
		double PiOverFour = Pi / 4.0;
		double TwoPi = 2.0 * Pi;
//		printf("%f\n", PiOverFour);
		for (double A = 0; A < TwoPi; A += PiOverFour)
		{
			int DX = ((int) (cos(A) + 1.0 + 0.5)) - 1;
			int DY = ((int) (sin(A) + 1.0 + 0.5)) - 1;
			int X = PieceX + DX;
			int Y = PieceY + DY;
			if (X < 0 || X > 7 || Y < 0 || Y > 7)
			{
				continue;
			}
//			printf("X: %d, Y: %d\n", X, Y);

			int BoardIndex = Y * 8 + X;
			if (Board[BoardIndex] && ((IsWhitePiece(Piece) && IsWhitePiece(Board[BoardIndex])) || (IsBlackPiece(Piece) && IsBlackPiece(Board[BoardIndex]))))
			{
				continue;
			}
			else
			{
				move *Move = (move *) malloc(sizeof(move));
				Move->FromX = PieceX;
				Move->FromY = PieceY;
				Move->ToX = X;
				Move->ToY = Y;
				ArrayAdd(Moves, Move);
			}
		}
	}

	if (Piece == &WKnightImage || Piece == &BKnightImage)
	{
		int XDeltas[] = {-1, 1, -2, 2, -2, 2, -1, 1};
		int YDeltas[] = {-2, -2, -1, -1, 1, 1, 2, 2};
		for (int I = 0; I < 8; ++I)
		{
			int X = PieceX + XDeltas[I];
			int Y = PieceY + YDeltas[I];
			if (X < 0 || X > 7 || Y < 0 || Y > 7)
			{
				continue;
			}
//			printf("X: %d, Y: %d\n", X, Y);

			int BoardIndex = Y * 8 + X;
			if (Board[BoardIndex] && ((IsWhitePiece(Piece) && IsWhitePiece(Board[BoardIndex])) || (IsBlackPiece(Piece) && IsBlackPiece(Board[BoardIndex]))))
			{
				continue;
			}
			else
			{
				move *Move = (move *) malloc(sizeof(move));
				Move->FromX = PieceX;
				Move->FromY = PieceY;
				Move->ToX = X;
				Move->ToY = Y;
				ArrayAdd(Moves, Move);
			}
		}
	}

	if (Piece == &WPawnImage)
	{
		{
			int X = PieceX;
			int Y = PieceY - 1;
			int BoardIndex = Y * 8 + X;
			if (!Board[BoardIndex] && (X < 8 && X >= 0 && Y < 8 && Y >= 0))
			{
				move *Move = (move *) malloc(sizeof(move));
				Move->FromX = PieceX;
				Move->FromY = PieceY;
				Move->ToX = X;
				Move->ToY = Y;
				ArrayAdd(Moves, Move);
			}
		}
		{
			int X = PieceX - 1;
			int Y = PieceY - 1;
			int BoardIndex = Y * 8 + X;
			if (Board[BoardIndex] && (IsBlackPiece(Board[BoardIndex])) && (X < 8 && X >= 0 && Y < 8 && Y >= 0))
			{
				move *Move = (move *) malloc(sizeof(move));
				Move->FromX = PieceX;
				Move->FromY = PieceY;
				Move->ToX = X;
				Move->ToY = Y;
				ArrayAdd(Moves, Move);
			}
		}
		{
			int X = PieceX + 1;
			int Y = PieceY - 1;
			int BoardIndex = Y * 8 + X;
			if (Board[BoardIndex] && (IsBlackPiece(Board[BoardIndex])) && (X < 8 && X >= 0 && Y < 8 && Y >= 0))
			{
				move *Move = (move *) malloc(sizeof(move));
				Move->FromX = PieceX;
				Move->FromY = PieceY;
				Move->ToX = X;
				Move->ToY = Y;
				ArrayAdd(Moves, Move);
			}
		}
		if (PieceY == 6)
		{
			int X = PieceX;
			int Y = PieceY;
			int BoardIndex1 = (Y - 1) * 8 + X;
			int BoardIndex2 = (Y - 2) * 8 + X;
			if (!Board[BoardIndex1] && !Board[BoardIndex2])
			{
				move *Move = (move *) malloc(sizeof(move));
				Move->FromX = PieceX;
				Move->FromY = PieceY;
				Move->ToX = X;
				Move->ToY = Y - 2;
				ArrayAdd(Moves, Move);
			}
		}
	}
	if (Piece == &BPawnImage)
	{
		{
			int X = PieceX;
			int Y = PieceY + 1;
			int BoardIndex = Y * 8 + X;
			if (!Board[BoardIndex] && (X < 8 && X >= 0 && Y < 8 && Y >= 0))
			{
				move *Move = (move *) malloc(sizeof(move));
				Move->FromX = PieceX;
				Move->FromY = PieceY;
				Move->ToX = X;
				Move->ToY = Y;
				ArrayAdd(Moves, Move);
			}
		}
		{
			int X = PieceX - 1;
			int Y = PieceY + 1;
			int BoardIndex = Y * 8 + X;
			if (Board[BoardIndex] && (IsWhitePiece(Board[BoardIndex])) && (X < 8 && X >= 0 && Y < 8 && Y >= 0))
			{
				move *Move = (move *) malloc(sizeof(move));
				Move->FromX = PieceX;
				Move->FromY = PieceY;
				Move->ToX = X;
				Move->ToY = Y;
				ArrayAdd(Moves, Move);
			}
		}
		{
			int X = PieceX + 1;
			int Y = PieceY + 1;
			int BoardIndex = Y * 8 + X;
			if (Board[BoardIndex] && (IsWhitePiece(Board[BoardIndex])) && (X < 8 && X >= 0 && Y < 8 && Y >= 0))
			{
				move *Move = (move *) malloc(sizeof(move));
				Move->FromX = PieceX;
				Move->FromY = PieceY;
				Move->ToX = X;
				Move->ToY = Y;
				ArrayAdd(Moves, Move);
			}
		}
		if (PieceY == 1)
		{
			int X = PieceX;
			int Y = PieceY;
			int BoardIndex1 = (Y + 1) * 8 + X;
			int BoardIndex2 = (Y + 2) * 8 + X;
			if (!Board[BoardIndex1] && !Board[BoardIndex2])
			{
				move *Move = (move *) malloc(sizeof(move));
				Move->FromX = PieceX;
				Move->FromY = PieceY;
				Move->ToX = X;
				Move->ToY = Y + 2;
				ArrayAdd(Moves, Move);
			}
		}
	}
}

bool IsInCheck(image *Board[], color Color)
{
	bool IsCheck = false;

	array<move *> OpponentsPossibleMoves;
	ArrayInit(&OpponentsPossibleMoves);

	bool (*IsOpponentsPiece) (image *) = (Color == WHITE) ? IsBlackPiece : IsWhitePiece;
	image *OwnKing = (Color == WHITE) ? &WKingImage : &BKingImage;

	for (int Y = 0; Y < 8; ++Y)
	{
		for (int X = 0; X < 8; ++X)
		{
			int BoardIndex = Y * 8 + X;
			if (Board[BoardIndex] && IsOpponentsPiece(Board[BoardIndex]))
			{
				GetMovesForPiece(Board, X, Y, &OpponentsPossibleMoves);
			}
		}
	}

	for (int I = 0; I < OpponentsPossibleMoves.Count; ++I)
	{
		move *Move = OpponentsPossibleMoves.Data[I];
		if (Board[Move->ToY * 8 + Move->ToX] == OwnKing)
		{
			IsCheck = true;
			break;
		}
	}

	return IsCheck;
}

bool IsCheckmated(image *Board[], color Color)
{
	bool IsCheckmated = false;
	// determine if 'Color' is in check
	if (IsInCheck(Board, Color))
	{
		// if so, get all its possible moves
		IsCheckmated = true;

		array<move *> PossibleMoves;
		ArrayInit(&PossibleMoves);
	
		bool (*IsColor) (image *) = (Color == WHITE) ? IsWhitePiece : IsBlackPiece;
	
		for (int Y = 0; Y < 8; ++Y)
		{
			for (int X = 0; X < 8; ++X)
			{
				int BoardIndex = Y * 8 + X;
				if (Board[BoardIndex] && IsColor(Board[BoardIndex]))
				{
					GetMovesForPiece(Board, X, Y, &PossibleMoves);
				}
			}
		}
	
		// and try each one to see if it stops the check
		// if none of them stops the check, then 'Color' is checkmated
		for (int I = 0; I < PossibleMoves.Count; ++I)
		{
			move *Move = PossibleMoves.Data[I];
			// make the move
			image *CapturedPiece = Board[Move->ToY * 8 + Move->ToX];
			Board[Move->ToY * 8 + Move->ToX] = Board[Move->FromY * 8 + Move->FromX];
			Board[Move->FromY * 8 + Move->FromX] = NULL;
			// are we in check
			bool NotInCheck = !IsInCheck(Board, Color);
			// undo the move
			Board[Move->FromY * 8 + Move->FromX] = Board[Move->ToY * 8 + Move->ToX];
			Board[Move->ToY * 8 + Move->ToX] = CapturedPiece;
			if (NotInCheck)
			{
				IsCheckmated = false;
				break;
			}
		}
	}
	return IsCheckmated;
}

bool IsStalemated(image *Board[], color Color)
{
	return false;
}

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
	u8 b1 = *p; ++p;
	u8 b2 = *p; ++p;
//	printf("%c%c\n", b1, b2);

	// Size of the image file in bytes
	u32 BMPSize = *((u32 *) p); p += 4;
	assert(BMPSize == Size);

	p += 4; // Skip reserved bytes

	// Offset of the image data
	u32 Offset = *((unsigned int *) p); p += 4;

	// DIB (Device Independent Bitmap) size (version of the DIB header)
	u32 DIBSize = *((unsigned int *) p); p += 4;

	// Image width and height
	u32 Width = *((u32 *) p); p += 4;
	u32 Height = *((u32 *) p); p += 4;
	u16 Planes = *((u16 *) p); p += 2;
	u16 BitsPerPixel = *((u16 *) p); p += 2;
	BMPImage->Width = Width;
	BMPImage->Height = Height;

	BMPImage->Data = Contents + Offset;

	return true;
}

bool LoadWAVFile(const char *SoundFilePath, loadedSound *Sound)
{
	size_t NumBytes;
	u8 *FileContents;
	if (!ReadFile(SoundFilePath, &FileContents, &NumBytes))
	{
		return false;
	}

	int ContentsIndex = 0;
	{
		bool Found = false;
		const char *StrToFind = "fmt";
		int StrToFindIndex = 0;
		while (ContentsIndex < NumBytes)
		{
			while (FileContents[ContentsIndex] == StrToFind[StrToFindIndex] && ContentsIndex < NumBytes)
			{
				ContentsIndex += 1;
				StrToFindIndex += 1;
			}
			if (StrToFind[StrToFindIndex] == '\0')
			{
				Found = true;
				break;
			}
			else
			{
				StrToFindIndex = 0;
			}
			ContentsIndex += 1;
		}
		if (!Found)
		{
//			printf("Error: couldnt find fmt-chunk!\n");
			return false;
		}
	
//		printf("found fmt-chunk at %d\n", I);
		ContentsIndex += 1; // trailing zero
	}

	// Length of format data
	u32 Length = *((u32 *) &FileContents[ContentsIndex]);
//	printf("Length of format data: %u\n", Length);
	ContentsIndex += sizeof(u32);

	// Type of format (1 is PCM)
	u16 Type = *((u16 *) &FileContents[ContentsIndex]);
//	printf("Type of format: %u\n", Type);
	ContentsIndex += sizeof(u16);

	// Number of channels
	u16 NumChannels = *((u16 *) &FileContents[ContentsIndex]);
//	printf("Number of channels: %u\n", NumChannels);
	ContentsIndex += sizeof(u16);

	// Samples per second
	u32 SamplesPerSecond = *((u32 *) &FileContents[ContentsIndex]);
//	printf("Samples per second: %u\n", SamplesPerSecond);
	ContentsIndex += sizeof(u32);

	ContentsIndex += 6; // jump over some uninteresting values

	// Bits per sample
	u16 BitsPerSample = *((u16 *) &FileContents[ContentsIndex]);
//	printf("Bits per sample: %u\n", BitsPerSample);
	ContentsIndex += sizeof(u16);

	Sound->NumChannels = NumChannels;
	Sound->BytesPerSample = BitsPerSample / 8;
	Sound->SamplesPerSecond = SamplesPerSecond;

	{
		bool Found = false;
		const char *StrToFind = "data";
		int J = 0;
		for (; ContentsIndex < NumBytes; ++ContentsIndex)
		{
			while (FileContents[ContentsIndex] == StrToFind[J] && ContentsIndex < NumBytes)
			{
				++ContentsIndex;
				++J;
			}
			if (StrToFind[J] == '\0')
			{
				Found = true;
				break;
			}
			else
			{
				J = 0;
			}
		}
		if (!Found)
		{
//			printf("Error: couldnt find data-chunk!\n");
			return false;
		}
	
//		printf("found data-chunk at %d\n", I);
	}

	// File size (data)
	u32 DataSize = *((u32 *) &FileContents[ContentsIndex]);
//	printf("File size (data): %u\n", DataSize);
	ContentsIndex += sizeof(u32);

//	Sound->BytesPerSampleInMemory = 4;
//
//	printf("DataSize mod 3 = %d\n", DataSize % 3);
//	u8 *Ptr = (u8 *) malloc(DataSize / 3 * 4);
//	Sound->Samples = Ptr;
//	for (int K = 0; K < DataSize / 3; ++K)
//	{
//		*Ptr++ = FileContents[ContentsIndex++];
//		*Ptr++ = FileContents[ContentsIndex++];
//		*Ptr++ = FileContents[ContentsIndex++];
//		*Ptr++ = 0;
//	}
	Sound->Samples = &FileContents[ContentsIndex];

	Sound->NumSamples = DataSize / (Sound->BytesPerSample * Sound->NumChannels);

	return true;
}

void LoadPieceImage(const char *ImagePath, image *Image)
{
	if (!LoadBMP(ImagePath, Image))
	{
		fprintf(stderr, "Failed to load image data from file \"%s\"\n", ImagePath);
		exit(EXIT_FAILURE);
	}
}

void LoadSound(const char *SoundPath, loadedSound *Sound)
{
	if (!LoadWAVFile(SoundPath, Sound))
	{
		fprintf(stderr, "Failed to load sound data from file \"%s\"\n", SoundPath);
		exit(EXIT_FAILURE);
	}
}

void GameInit()
{
	// const char *SoundExt = "wav";
	// const char *SoundPrefix = "sounds/";
	// const char *SoundPostfix = "chess-dot-com/";

	// char ExeDir[MAX_PATH_LEN];
	// PlatformGetExeDir(ExeDir, MAX_PATH_LEN);
	// char SoundDir[MAX_PATH_LEN];
	// snprintf(SoundDir, MAX_PATH_LEN, "%s/%s/%s", ExeDir, SoundPrefix, SoundPostfix);
	// const char *SoundFileNames = {"move", "capture", "check"};
	// for(int i = 0; i < sizeof(SoundFileNames) / sizeof(SoundFileNames[0]); ++i)
	// {
	// 	char SoundFilePath[MAX_PATH_LEN];
	// 	snprintf(SoundFilePath, MAX_PATH_LEN, "%s/%s.%s", SoundDir, SoundFileNames[i], SoundExt);

	// 	loadedSound Loaded;
	// 	LoadSound(SoundFilePath, &Loaded);
	// 	LoadedSoundsTable.put(SoundName, Loaded);
	// }

	{
	const char *ThemeDir = "chess-dot-com";
	const int MAX_PATH_LEN = 1024; //@ something reasonable here?
	const int BUF_LEN = MAX_PATH_LEN+1;
	char SoundFilePath[BUF_LEN];
	snprintf(SoundFilePath, BUF_LEN, "sounds/%s/move.wav", ThemeDir); LoadSound(SoundFilePath, &MoveSound);
	snprintf(SoundFilePath, BUF_LEN, "sounds/%s/capture.wav", ThemeDir); LoadSound(SoundFilePath, &CaptureSound);
	snprintf(SoundFilePath, BUF_LEN, "sounds/%s/check.wav", ThemeDir); LoadSound(SoundFilePath, &CheckSound);
	}

	// LoadSound("sounds/chess-dot-com/move.wav", &MoveSound);
	// LoadSound("sounds/chess-dot-com/capture.wav", &CaptureSound);
	// LoadSound("sounds/chess-dot-com/check.wav", &CheckSound);

	{
	// const char *ThemeDir = "alpha";
	// const char *ThemeDir = "anarcandy";
	// const char *ThemeDir = "tatiana";
	// const char *ThemeDir = "pixel";
	// const char *ThemeDir = "pirouetti";
	// const char *ThemeDir = "letter";
	const char *ThemeDir = "chessnut";
	const int MAX_PATH_LEN = 1024; //@ something reasonable here?
	const int BUF_LEN = MAX_PATH_LEN+1;
	char ImageFilePath[BUF_LEN];
	snprintf(ImageFilePath, BUF_LEN, "images/%s/wK.bmp", ThemeDir); LoadPieceImage(ImageFilePath, &WKingImage);
	snprintf(ImageFilePath, BUF_LEN, "images/%s/wQ.bmp", ThemeDir); LoadPieceImage(ImageFilePath, &WQueenImage);
	snprintf(ImageFilePath, BUF_LEN, "images/%s/wN.bmp", ThemeDir); LoadPieceImage(ImageFilePath, &WKnightImage);
	snprintf(ImageFilePath, BUF_LEN, "images/%s/wB.bmp", ThemeDir); LoadPieceImage(ImageFilePath, &WBishopImage);
	snprintf(ImageFilePath, BUF_LEN, "images/%s/wR.bmp", ThemeDir); LoadPieceImage(ImageFilePath, &WRookImage);
	snprintf(ImageFilePath, BUF_LEN, "images/%s/wP.bmp", ThemeDir); LoadPieceImage(ImageFilePath, &WPawnImage);
	snprintf(ImageFilePath, BUF_LEN, "images/%s/bK.bmp", ThemeDir); LoadPieceImage(ImageFilePath, &BKingImage);
	snprintf(ImageFilePath, BUF_LEN, "images/%s/bQ.bmp", ThemeDir); LoadPieceImage(ImageFilePath, &BQueenImage);
	snprintf(ImageFilePath, BUF_LEN, "images/%s/bN.bmp", ThemeDir); LoadPieceImage(ImageFilePath, &BKnightImage);
	snprintf(ImageFilePath, BUF_LEN, "images/%s/bB.bmp", ThemeDir); LoadPieceImage(ImageFilePath, &BBishopImage);
	snprintf(ImageFilePath, BUF_LEN, "images/%s/bR.bmp", ThemeDir); LoadPieceImage(ImageFilePath, &BRookImage);
	snprintf(ImageFilePath, BUF_LEN, "images/%s/bP.bmp", ThemeDir); LoadPieceImage(ImageFilePath, &BPawnImage);
	}

	// LoadPieceImage("images/lking.bmp", &WKingImage);
	// LoadPieceImage("images/dking.bmp", &BKingImage);
	// LoadPieceImage("images/lqueen.bmp", &WQueenImage);
	// LoadPieceImage("images/dqueen.bmp", &BQueenImage);
	// LoadPieceImage("images/lrook.bmp", &WRookImage);
	// LoadPieceImage("images/drook.bmp", &BRookImage);
	// LoadPieceImage("images/lbishop.bmp", &WBishopImage);
	// LoadPieceImage("images/dbishop.bmp", &BBishopImage);
	// LoadPieceImage("images/lknight.bmp", &WKnightImage);
	// LoadPieceImage("images/dknight.bmp", &BKnightImage);
	// LoadPieceImage("images/lpawn.bmp", &WPawnImage);
	// LoadPieceImage("images/dpawn.bmp", &BPawnImage);

	SetUpBoard(Board, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");
//	SetUpBoard(Board, "rbBqkn2/8/8/4p3/3P4/8/8/QK4NR");
	// SetUpBoard(Board, "k7/8/8/8/8/8/8/1RQ5");

//	player Players[2] = {{HUMAN}, {HUMAN}};
//	color WhosMove = WHITE;
//	//@Perspective =; // From whos perpective are we looking at the board.
//	bool GameOver = false;
//	array<move *> DraggedPieceAvailableMoves;
//	ArrayInit(&DraggedPieceAvailableMoves);
////	ArrayAdd(&DraggedPieceAvailableMoves, new_move(0, 0, 0, 2));

	GameOver = false;
	WhosMove = WHITE;

	// random number generator
	// otherwise we get the same set of random numbers every time
	srand(time(NULL));

	// DEBUG:
	// printf("%dx%d\n", BBishopImage.Width, BBishopImage.Height);
	// unsigned int *Pixels = (unsigned int *)BBishopImage.Data;
	// int NumPixels = BBishopImage.Width * BBishopImage.Height;
	// for(int i = 0; i < NumPixels; ++i) {
	// 	printf("%X ", Pixels[i]);
	// }
}

void GameUpdate(image *WindowBuffer, soundBuffer *SoundBuffer, userInput *Input)
{
//	RenderHandmadeheroGradient2(WindowBuffer, 0, 0);
//	return;

//	if (SoundBuffer)
//	{
//		int FramesToWrite = 0;
//	//	i16 *Dest = (i16 *) ((i32 *) SoundBuffer->Data) + SoundBuffer->PreviousFrameIndex;
//		i16 *Dest = ((i16 *) SoundBuffer->Data) + SoundBuffer->PreviousFrameIndex;
//		if (SoundBuffer->CurrentFrameIndex > SoundBuffer->PreviousFrameIndex)
//		{
//			FramesToWrite = SoundBuffer->CurrentFrameIndex - SoundBuffer->PreviousFrameIndex;
//		}
//		else if (SoundBuffer->CurrentFrameIndex < SoundBuffer->PreviousFrameIndex)
//		{
//			FramesToWrite = SoundBuffer->TotalNumFrames - SoundBuffer->PreviousFrameIndex;
//		}
//		else
//		{
//			// no new frames to write
//			assert(SoundBuffer->CurrentFrameIndex == SoundBuffer->PreviousFrameIndex);
//		}
//	
//		for (int FrameIndex = 0; FrameIndex < FramesToWrite; ++FrameIndex)
//		{
//			r32 SampleValue1 = 0.0;
//	//		r32 SampleValue2 = 0.0;
//			for (int SoundIndex = 0; SoundIndex < PlayingSounds.Count; ++SoundIndex)
//			{
//				playingSound *PlayingSound = &(PlayingSounds.Data[SoundIndex]);
//				i16 *Ptr = (i16 *) PlayingSound->Sound->Samples;
//				Ptr += PlayingSound->FrameIndex * PlayingSound->Sound->NumChannels;
//				SampleValue1 += (r32) *Ptr;
//	//			Ptr += 1;
//	//			SampleValue2 += (r32) *Ptr;
//				PlayingSound->FrameIndex += 1;
//				if (PlayingSound->FrameIndex == PlayingSound->Sound->NumSamples)
//				{
//					PlayingSound->FrameIndex = 0;
//				}
//			}
//			*Dest = (i16) (SampleValue1 + 0.5);
//			Dest += 1;
//	//		*Dest = (i16) (SampleValue2 + 0.5);
//	//		Dest += 1;
//		}
//	}

// SIMPLE WINDOWS SOUND TEST
//	assert(SoundBuffer);
//	static int NumSamplesPlayed = 0;
//	i16 *Src  = ((i16 *) TestSound.Samples) + NumSamplesPlayed * TestSound.NumChannels;
//	i16 *Dest = (i16 *) SoundBuffer->Samples;
//	for (int Counter = 0; Counter < SoundBuffer->NumSamplesWanted; ++Counter)
//	{
////		r64 SampleValue = 0.0;
////		if (NumSamplesPlayed < BowlSound.NumSamples)
////		{
////			SampleValue += (r64) *Src++;
////			NumSamplesPlayed += 1;
////		}
////		else
////		{
////			// looping sound
////			NumSamplesPlayed = 0;
////			Src = ((i16 *) BowlSound.Samples);
////		}
////		*Dest++ = (i16) SampleValue;
//
//		assert(NumSamplesPlayed <= TestSound.NumSamples);
//		if (NumSamplesPlayed == TestSound.NumSamples)
//		{
//			// looping sound
//			NumSamplesPlayed = 0;
//			Src = ((i16 *) TestSound.Samples);
//		}
//
//		if (TestSound.NumChannels == 1)
//		{
//			// if one channels, duplicate
//			i16 SampleValue = *Src++;
//			*Dest++ = SampleValue;
//			*Dest++ = SampleValue;
//		}
//		else if (TestSound.NumChannels == 2)
//		{
//			*Dest++ = *Src++;
//			*Dest++ = *Src++;
//		}
//		else
//		{
//			// unsupported # of channels
//			assert(false);
//		}
//
//		NumSamplesPlayed += 1;
//	}

	// MIXER THAT WORKS WITH WINDOWS AND LINUX

	i16 *Dest = (i16 *) SoundBuffer->Samples;
	for (int Counter = 0; Counter < SoundBuffer->NumSamplesWanted; ++Counter)
	{
		r32 SampleValueLeft = 0.0f;
		r32 SampleValueRight = 0.0f;
		for (lib::link<playingSound *> *Ptr = PlayingSounds.Head; Ptr;)
		{
			playingSound *PtrSound = Ptr->Data;
			if (PtrSound->NumSamplesPlayed == PtrSound->Sound->NumSamples)
			{
				if (PtrSound->IsLooping)
				{
					PtrSound->NumSamplesPlayed = 0;
				}
				else
				{
					// playingSound *Previous = Ptr->Previous;
					// playingSound *Next = Ptr->Next;
					// if (Previous) Previous->Next = Next;
					// if (Next) Next->Previous = Previous;
					// free(Ptr);
					// Ptr = Next;
					// if (!Previous) FirstPlayingSound = Ptr;
					free(PtrSound); //@ is that all?
					lib::link<playingSound *> *OldLink = Ptr;
					Ptr = Ptr->Prev;
					PlayingSounds.remove(OldLink);
					continue;
				}
			}
			if (PtrSound->Sound->NumChannels == 1)
			{
				r32 SampleValue = (r32) *(((i16 *) PtrSound->Sound->Samples) + PtrSound->NumSamplesPlayed * 1);
				SampleValueLeft += SampleValue;
				SampleValueRight += SampleValue;
			}
			else if (PtrSound->Sound->NumChannels == 2)
			{
				SampleValueLeft += (r32) *(((i16 *) PtrSound->Sound->Samples) + PtrSound->NumSamplesPlayed * 2);
				SampleValueRight += (r32) *(((i16 *) PtrSound->Sound->Samples) + PtrSound->NumSamplesPlayed * 2 + 1);
			}
			PtrSound->NumSamplesPlayed += 1;

			Ptr = Ptr->Prev;
		}
		*Dest++ = (i16) SampleValueLeft; //@ cast?
		*Dest++ = (i16) SampleValueRight; //@ cast?
	}

	// i16 *Dest = (i16 *) SoundBuffer->Samples;
	// for (int Counter = 0; Counter < SoundBuffer->NumSamplesWanted; ++Counter)
	// {
	// 	r32 SampleValueLeft = 0.0f;
	// 	r32 SampleValueRight = 0.0f;
	// 	for (playingSound *Ptr = FirstPlayingSound; Ptr;)
	// 	{
	// 		if (Ptr->NumSamplesPlayed == Ptr->Sound->NumSamples)
	// 		{
	// 			if (Ptr->IsLooping)
	// 			{
	// 				Ptr->NumSamplesPlayed = 0;
	// 			}
	// 			else
	// 			{
	// 				playingSound *Previous = Ptr->Previous;
	// 				playingSound *Next = Ptr->Next;
	// 				if (Previous) Previous->Next = Next;
	// 				if (Next) Next->Previous = Previous;
	// 				free(Ptr);
	// 				Ptr = Next;
	// 				if (!Previous) FirstPlayingSound = Ptr;
	// 				continue;
	// 			}
	// 		}
	// 		if (Ptr->Sound->NumChannels == 1)
	// 		{
	// 			r32 SampleValue = (r32) *(((i16 *) Ptr->Sound->Samples) + Ptr->NumSamplesPlayed * 1);
	// 			SampleValueLeft += SampleValue;
	// 			SampleValueRight += SampleValue;
	// 		}
	// 		else if (Ptr->Sound->NumChannels == 2)
	// 		{
	// 			SampleValueLeft += (r32) *(((i16 *) Ptr->Sound->Samples) + Ptr->NumSamplesPlayed * 2);
	// 			SampleValueRight += (r32) *(((i16 *) Ptr->Sound->Samples) + Ptr->NumSamplesPlayed * 2 + 1);
	// 		}
	// 		Ptr->NumSamplesPlayed += 1;

	// 		Ptr = Ptr->Next;
	// 	}
	// 	*Dest++ = (i16) SampleValueLeft; //@ cast?
	// 	*Dest++ = (i16) SampleValueRight; //@ cast?
	// }

	array<event> Events = Input->Events;
	for (int I = 0; I < Events.Count; ++I)
	{
		event Event = Events.Data[I];
		switch (Event.Type)
		{
			case LBUTTONDOWN:
			{
				// printf("LBUTTONDOWN: X: %d, Y: %d\n", Event.X, Event.Y);
				int DownX = Event.X / 60;
				int DownY = Event.Y / 60;
				int BoardIndex = DownY * 8 + DownX;
				if (Board[BoardIndex])
				{
					IsDraggedPiece = true;
					DraggedPieceX = DownX;
					DraggedPieceY = DownY;
//					DraggedPiecePixelX = ButtonEvent.x - 60 / 2;
//					DraggedPiecePixelY = ButtonEvent.y - 60 / 2;

//					GetMovesForPiece(Board, PressedX, PressedY, &DraggedPieceAvailableMoves);
				}
			}
			break;
			case LBUTTONUP:
			{
				// printf("LBUTTONUP X: %d, Y: %d\n", Event.X, Event.Y);
				if (IsDraggedPiece)
				{
//					DraggedPieceAvailableMoves.Count = 0;
//					for (int I = 0; I < DraggedPieceAvailableMoves.Count; ++I)
//					{
//						free(DraggedPieceAvailableMoves.Data[I]);
//					}

					int BoardIndex = DraggedPieceY * 8 + DraggedPieceX;
					image *DraggedPiece = Board[BoardIndex];
					// image *DraggedPiece = Board[DraggedPieceY][DraggeedPieceX];

					int ReleasedX = Event.X / 60;
					int ReleasedY = Event.Y / 60;
					assert(ReleasedX < 8 && ReleasedY < 8);
					assert(ReleasedX >= 0 && ReleasedY >= 0);

					if (GameOver)
					{
						printf("Game is over!\n");
						IsDraggedPiece = false;
						break;
					}

					// Player put the piece back to its original square.
					if (ReleasedX == DraggedPieceX && ReleasedY == DraggedPieceY)
					{
						IsDraggedPiece = false;
						break;
					}

					// Player moved a piece, but its not their turn (it was of wrong color)
					if ((IsWhitePiece(DraggedPiece) && WhosMove == BLACK) || (IsBlackPiece(DraggedPiece) && WhosMove == WHITE))
					{
						const char *Color = (WhosMove == WHITE) ? "WHITE" : "BLACK";
						printf("%s's move!\n", Color);

						IsDraggedPiece = false;
						break;
					}

					// What are the possible moves?
					array<move *> PossibleMoves; ArrayInit(&PossibleMoves);
					GetMovesForPiece(Board, DraggedPieceX, DraggedPieceY, &PossibleMoves); //@ free things
//					for (int I = 0; I < PossibleMoves.Count; ++I)
//					{
//						move *PossibleMove = PossibleMoves.Data[I];
//						printf("POSSIBLE MOVE: %d, %d\n", PossibleMove->ToX, PossibleMove->ToY);
//					}

					// Is our move a possible move?
					bool CanMakeMove = false;
					for (int I = 0; I < PossibleMoves.Count; ++I)
					{
						move *PossibleMove = PossibleMoves.Data[I];
						if (PossibleMove->ToX == ReleasedX && PossibleMove->ToY == ReleasedY)
						{
							CanMakeMove = true;
							break;
						}
					}

					if (!CanMakeMove)
					{
						IsDraggedPiece = false;
						break;
					}

					// make the move
					image *CapturedPiece = Board[ReleasedY * 8 + ReleasedX];
					Board[ReleasedY * 8 + ReleasedX] = Board[DraggedPieceY * 8 + DraggedPieceX];
					Board[DraggedPieceY * 8 + DraggedPieceX] = NULL;

					if (IsInCheck(Board, WhosMove))
					{
						// take the move back
						Board[DraggedPieceY * 8 + DraggedPieceX] = Board[ReleasedY * 8 + ReleasedX];
						Board[ReleasedY * 8 + ReleasedX] = CapturedPiece; // if there was a captured piece, put it back

						const char *Color = (WhosMove == WHITE) ? "WHITE" : "BLACK";
						printf("%s is in check!\n", Color);

						IsDraggedPiece = false;
						break;
					}

					IsDraggedPiece = false;

					WhosMove = (color) !((bool) WhosMove);
					// WhosMove = (WhosMove == BLACK) ? WHITE : BLACK;
					// WhosMove = BLACK;

					// check if it was a check / checkmate
					bool Check = false;
					if (IsInCheck(Board, WhosMove))
					{
						printf("%s is in check!\n", (WhosMove == WHITE) ? "WHITE" : "BLACK");
						Check = true;
					}
					printf("%s's move...\n", (WhosMove == WHITE) ? "WHITE" : "BLACK");

					// Play sound
					playingSound *PlayingSound = (playingSound *) malloc(sizeof(playingSound));
					PlayingSound->Sound = Check ? &CheckSound : (CapturedPiece ? &CaptureSound : &MoveSound);
					PlayingSound->NumSamplesPlayed = 0;
					PlayingSound->IsLooping = false;
					PlayingSounds.append(PlayingSound);

					if (IsCheckmated(Board, WhosMove))
					{
						printf("%s is checkmated!\n", (WhosMove == WHITE) ? "WHITE" : "BLACK");
						GameOver = true;
						break;
					}

					// Get computer move
					array<move *> ComputerPossibleMoves; ArrayInit(&ComputerPossibleMoves); //@ free things
				
					for (int Y = 0; Y < 8; ++Y)
					{
						for (int X = 0; X < 8; ++X)
						{
							int BoardIndex = Y * 8 + X;
							if (Board[BoardIndex] && IsBlackPiece(Board[BoardIndex]))
							{
								GetMovesForPiece(Board, X, Y, &ComputerPossibleMoves);
							}
						}
					}

					// Pick random move.
					// Generate random index
					int RandIndex = ((unsigned int)rand()) % (ComputerPossibleMoves.Count);
					printf("RandIndex: %d, ComputerPossibleMoves.Count: %d\n", RandIndex, ComputerPossibleMoves.Count);
					assert(0 <= RandIndex && RandIndex < ComputerPossibleMoves.Count);

					bool FoundMove = false;
					for(int I = RandIndex; I < ComputerPossibleMoves.Count; ++I)
					{
						move *PlausableMove = ComputerPossibleMoves.Data[I];

						// actually make the move
						image *CapturedPiece = Board[PlausableMove->ToY * 8 + PlausableMove->ToX];
						Board[PlausableMove->ToY * 8 + PlausableMove->ToX] = Board[PlausableMove->FromY * 8 + PlausableMove->FromX];
						Board[PlausableMove->FromY * 8 + PlausableMove->FromX] = NULL;

						if (IsInCheck(Board, WhosMove))
						{
							// take the move back
							Board[PlausableMove->FromY * 8 + PlausableMove->FromX] = Board[PlausableMove->ToY * 8 + PlausableMove->ToX];
							Board[PlausableMove->ToY * 8 + PlausableMove->ToX] = CapturedPiece; // if there was a captured piece, put it back
							
							continue;
						}
						else
						{
							FoundMove = true;
							break;
						}
					}

					if(!FoundMove)
					{
						for(int I = 0; I < RandIndex; ++I)
						{
							move *PlausableMove = ComputerPossibleMoves.Data[I];

							// actually make the move
							image *CapturedPiece = Board[PlausableMove->ToY * 8 + PlausableMove->ToX];
							Board[PlausableMove->ToY * 8 + PlausableMove->ToX] = Board[PlausableMove->FromY * 8 + PlausableMove->FromX];
							Board[PlausableMove->FromY * 8 + PlausableMove->FromX] = NULL;

							if (IsInCheck(Board, WhosMove))
							{
								// take the move back
								Board[PlausableMove->FromY * 8 + PlausableMove->FromX] = Board[PlausableMove->ToY * 8 + PlausableMove->ToX];
								Board[PlausableMove->ToY * 8 + PlausableMove->ToX] = CapturedPiece; // if there was a captured piece, put it back
								
								continue;
							}
							else
							{
								FoundMove = true;
								break;
							}
						}
					}
					assert(FoundMove); // If there are no moves, then the player is checkmated and we should have detected that earlier. Actually it could be a stalemate as well.

					WhosMove = (color) !((bool) WhosMove);
				}
			}
			break;
			default:
			{
				assert(false);
			}
		}
	}
	i32 MouseX = Input->LastMouseX;
	i32 MouseY = Input->LastMouseY;
//	printf("MouseX: %d, MouseY: %d\n", MouseX, MouseY);

//	static uint32 OffsetX = 0;
//	static uint32 OffsetY = 0;
//	RenderHandmadeheroGradient(WindowBuffer, OffsetX, OffsetY);
//	OffsetX += 1;
//	OffsetY += 1;

//	RenderHandmadeheroGradient(WindowBuffer, 0, 0);
//RenderRectangle(WindowBuffer, 0, 0, WindowBuffer->Width, WindowBuffer->Height, 0x00ff0000);

//	SetUpBoard(Board, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");

	// Render background
	u32 Light = 0x00efd9b7;
	u32 Dark = 0x00b58965;
	u32 TileWidth = WindowBuffer->Width / 8;
	u32 TileHeight = WindowBuffer->Height / 8;
	for (i32 Y = 0; Y < 8; ++Y)
	{
		for (i32 X = 0; X < 8; ++X)
		{
			u32 TileColor = ((X + Y) % 2) ? Dark : Light;
			RenderRectangle(WindowBuffer, X * TileWidth, Y * TileHeight, TileWidth, TileHeight, TileColor);
		}
	}

	// Render pieces
	for (int Y = 0; Y < 8; ++Y)
	{
		for (int X = 0; X < 8; ++X)
		{
			if (Board[Y * 8 + X] && !(IsDraggedPiece && DraggedPieceX == X && DraggedPieceY == Y))
			{
				RenderImage(WindowBuffer, Board[Y * 8 + X], X * 60, Y * 60);
			}
		}
	}

//		// highlight available squares for dragged piece
//		for (int I = 0; I < DraggedPieceAvailableMoves.Count; ++I)
//		{
//			move *Move = DraggedPieceAvailableMoves.Data[I];
//			int SquareWidth = 20;
//			int SquareHeight = 20;
//			int X = Move->ToX * TileWidth + TileWidth / 2 - SquareWidth / 2;
//			int Y = Move->ToY * TileHeight + TileHeight / 2 - SquareHeight / 2;
//			RenderRectangle(Pixels, X, Y, SquareWidth, SquareHeight, 0x00ff0000);
//		}

		// If we have a dragged piece, render it
		if (IsDraggedPiece)
		{
			RenderImage(WindowBuffer, Board[DraggedPieceY * 8 + DraggedPieceX], MouseX - 30, MouseY - 30);
		}
}