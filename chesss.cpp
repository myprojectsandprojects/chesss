//@ dragging a piece outside of the window and then releasing the mouse button causes some weird behaviour.

/*
Todo:
- play an animation when a computer makes a move
- if there is no animation, there is really no need to render frames (?)
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
*/

enum appMode
{
	appMode_PREGAME,
	appMode_GAME,
	appMode_POSTGAME
	// appMode_LEARNCHESS
};

enum playerType
{
	playerType_HUMAN,
	playerType_COMPUTER
};

enum color
{
	WHITE = 0,
	BLACK = 1
};

struct player
{
	playerType PlayerType;

	bool ComputerCalculateMove;
};

struct move
{
	int FromX;
	int FromY;
	int ToX;
	int ToY;
	// int X0, Y0, X1, Y1;
};

//loadedSound TestSound;
loadedSound MoveSound;
loadedSound CaptureSound;
loadedSound CheckSound;

//@ could store these in such a way that we could figure out the color by comparing memory addresses (?)
// "The C++ standard guarantees that the members of a class or struct appear in memory in the same order as they are declared."
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
// playingSound *FirstPlayingSound;

lib::linkedList<playingSound *> PlayingSounds; // The only reason its a linked list is because I wanted to try out a linked list.

// bool IsDraggedPiece;
// int DraggedPieceX;
// int DraggedPieceY;
// // int DraggedPiecePixelX;
// // int DraggedPiecePixelY;

struct animation
{
	int DurationFrames; // both initialized to 0, so not playing
	int ProgressFrames;

	int StartX;
	int StartY;
	int EndX;
	int EndY;
	image *Piece;

	double X, Y;
	double DX, DY;

	move Move;
};

// image *Board[8 * 8];

// enum castlingType {
// 	WHITE_KINGSIDE = 0,
// 	WHITE_QUEENSIDE = 1,
// 	BLACK_KINGSIDE = 2,
// 	BLACK_QUEENSIDE = 3
// };

// directly translated from fen
struct gameState {
	image *Board[8][8];

	color ActiveColor; // who's move

	// // bool CastlingOptions[4]; // use 'castlingType' to access
	// struct {
	// 	bool WhiteKingside;
	// 	bool WhiteQueenside;
	// 	bool BlackKingside;
	// 	bool BlackQueenside;
	// } CastlingOptions;

	// // this is a square behind a pawn that has moved 2 squares
	// // if any of the opponents pawns can capture at this square, then they can capture the pawn
	// // but the capture can only happen in the next move
	// // this square only exists during the opponents move
	// struct {
	// 	bool Exists;
	// 	int X, Y;
	// } EnPassantSquare;

	// int HalfmoveCounter; // 50-move rule
	// int CurrentMoveNumber;
};

struct app
{
	appMode Mode;

	gameState GameState;
	player Players[2];

	int TileWidth, TileHeight;

	bool IsDraggedPiece;
	struct {
		int BoardX, BoardY;
	} DraggedPiece;

	animation *PlayingAnimation; // just 1 for now
	animation Animation;
} App;

void MakeAnimation(app *App, animation *A, move Move, image *Piece)
{
	// A->DurationFrames = 64;
	A->DurationFrames = 4;
	A->ProgressFrames = 0;

	A->StartX = Move.FromX * App->TileWidth;
	A->StartY = Move.FromY * App->TileHeight;
	A->EndX = Move.ToX * App->TileWidth;
	A->EndY = Move.ToY * App->TileHeight;

	A->DX = (A->EndX - A->StartX) / (double)A->DurationFrames;
	A->DY = (A->EndY - A->StartY) / (double)A->DurationFrames;

	A->X = A->StartX;
	A->Y = A->StartY;

	A->Piece = Piece;
	A->Move = Move;
}

move *NewMove(int FromX, int FromY, int ToX, int ToY)
{
	move *Move = (move *) malloc(sizeof(move));
	Move->FromX = FromX;
	Move->FromY = FromY;
	Move->ToX = ToX;
	Move->ToY = ToY;
	return Move;
}

// more exotic moves: en passant, castling, pawn promotion
// later on, should updating move counters, castling rights etc. also be done here?
image *MakeMove(gameState *GameState, move *Move)
{
	image *PossibleCapturedPiece = GameState->Board[Move->ToY][Move->ToX];
	GameState->Board[Move->ToY][Move->ToX] = GameState->Board[Move->FromY][Move->FromX];
	GameState->Board[Move->FromY][Move->FromX] = NULL;

	return PossibleCapturedPiece;
}

void UnmakeMove(gameState *GameState, move *Move, image *PossibleCapturedPiece)
{
	GameState->Board[Move->FromY][Move->FromX] = GameState->Board[Move->ToY][Move->ToX];
	GameState->Board[Move->ToY][Move->ToX] = PossibleCapturedPiece;
}

void SwitchTurn(gameState *GameState)
{
	GameState->ActiveColor = (color)(~GameState->ActiveColor & 1);
}

// FEN: [PLACEMENT] [MOVE] [CASTLING] [EN PASSANT] [# of HALFMOVES] [# of FULLMOVES]
// [PLACEMENT of the pieces] "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR"
// [who MOVEs next] w/b
// [CASTLING rights]: KQkq/-
// [EN PASSANT] square in algebraic notation (e3)
// [# of HALFMOVES]: Halfmoves made since the last pawn advance or the piece capture. When this counter reaches 100 the game ends in a draw
void SetUpBoard(image *Board[], const char *Fen)
{
	for (int Y = 0; Y < 8; ++Y)
	{
		for (int X = 0; X < 8; ++X)
		{
			Board[Y * 8 + X] = NULL;
		}
	}

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

color GetPieceColor(image *Piece)
{
	assert(Piece == &WKingImage
		|| Piece == &BKingImage
		|| Piece == &WQueenImage
		|| Piece == &BQueenImage
		|| Piece == &WRookImage
		|| Piece == &BRookImage
		|| Piece == &WBishopImage
		|| Piece == &BBishopImage
		|| Piece == &WKnightImage
		|| Piece == &BKnightImage
		|| Piece == &WPawnImage
		|| Piece == &BPawnImage);

	return (Piece == &WKingImage
		|| Piece == &WQueenImage
		|| Piece == &WRookImage
		|| Piece == &WBishopImage
		|| Piece == &WKnightImage
		|| Piece == &WPawnImage)
		? WHITE : BLACK;
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

	array<move *> OpponentsPossibleMoves; ArrayInit(&OpponentsPossibleMoves);

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

bool IsInStalemate(/*image *Board[], color Color*/)
{
	return false;
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

void StartGame(app *App)
{
	printf("%s called\n", __FUNCTION__);

	gameState *GameState = &App->GameState;

//	SetUpBoard(Board, "rbBqkn2/8/8/4p3/3P4/8/8/QK4NR");
	// SetUpBoard((image **)GameState->Board, "k7/8/8/8/8/8/8/1RQ5");
	SetUpBoard((image **)GameState->Board, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");
	GameState->ActiveColor = WHITE;

	// white
	player Player1 = {playerType_COMPUTER, true}; App->Players[0] = Player1;
	// black
	player Player2 = {playerType_COMPUTER, false}; App->Players[1] = Player2;

	// // white
	// player Player1 = {playerType_HUMAN, true}; App->Players[0] = Player1;
	// // black
	// player Player2 = {playerType_HUMAN, false}; App->Players[1] = Player2;

	// // white
	// player Player1 = {playerType_HUMAN, true}; App->Players[0] = Player1;
	// // black
	// player Player2 = {playerType_COMPUTER, false}; App->Players[1] = Player2;

	// // white
	// player Player1 = {playerType_COMPUTER, true}; App->Players[0] = Player1;
	// // black
	// player Player2 = {playerType_HUMAN, false}; App->Players[1] = Player2;
}

void UpdatePreGame(app *App, image *WindowBuffer, userInput *Input)
{
	array<event> Events = Input->Events;
	for (int I = 0; I < Events.Count; ++I)
	{
		event Event = Events.Data[I];
		switch (Event.Type)
		{
			case LBUTTONDOWN:
			{
				printf("pregame: LBUTTONDOWN: X: %d, Y: %d\n", Event.X, Event.Y);
			}
			break;
			case LBUTTONUP:
			{
				printf("pregame: LBUTTONUP: X: %d, Y: %d\n", Event.X, Event.Y);
			}
			break;
			case KEYPRESS:
			{
				printf("pregame: KEYPRESS: keycode: %u\n", Event.KeyCode);

				App->Mode = appMode_GAME;
				StartGame(App);
			}
			break;
			default:
				assert(false);
		}
	}

	uint32_t Color = 0xffff0000;
	RenderRectangle(WindowBuffer, 0, 0, WindowBuffer->Width, WindowBuffer->Height, Color);
}

void UpdateGame(app *App, image *WindowBuffer, userInput *Input)
{
	// printf("ActiveColor: %d\n", (int)App->GameState.ActiveColor);

	gameState *GameState = &App->GameState;

	array<event> Events = Input->Events;
	for (int I = 0; I < Events.Count; ++I)
	{
		event Event = Events.Data[I];
		switch (Event.Type)
		{
			case LBUTTONDOWN:
			{
				// printf("LBUTTONDOWN: X: %d, Y: %d\n", Event.X, Event.Y);
				int DownX = Event.X / App->TileWidth;
				int DownY = Event.Y / App->TileHeight;
				if (GameState->Board[DownY][DownX])
				{
					App->IsDraggedPiece = true;
					App->DraggedPiece.BoardX = DownX;
					App->DraggedPiece.BoardY = DownY;

//					GetMovesForPiece(Board, PressedX, PressedY, &DraggedPieceAvailableMoves);
				}
			}
			break;
			case LBUTTONUP:
			{
				// printf("LBUTTONUP X: %d, Y: %d\n", Event.X, Event.Y);

				if (App->IsDraggedPiece)
				{
//					DraggedPieceAvailableMoves.Count = 0;
//					for (int I = 0; I < DraggedPieceAvailableMoves.Count; ++I)
//					{
//						free(DraggedPieceAvailableMoves.Data[I]);
//					}

					int UpX = Event.X / App->TileWidth;
					int UpY = Event.Y / App->TileHeight;
					// assert(UpX < 8 && UpY < 8);
					// assert(UpX >= 0 && UpY >= 0);

					move AttemptedMove;
					AttemptedMove.FromX = App->DraggedPiece.BoardX;
					AttemptedMove.FromY = App->DraggedPiece.BoardY;
					AttemptedMove.ToX = UpX;
					AttemptedMove.ToY = UpY;

					// Player put the piece back to its original square.
					//@ should probably be handled by GetMoves(), its just not one of the possible moves
					if (UpX == App->DraggedPiece.BoardX && UpY == App->DraggedPiece.BoardY)
					{
						App->IsDraggedPiece = false;
						break;
					}

					// Human attempted to make a move, but the move should be made by a computer
					if(App->Players[GameState->ActiveColor].PlayerType == playerType_COMPUTER)
					{
						printf("Next move should be made by a computer!\n");
						App->IsDraggedPiece = false;
						break;
					}

					image *DraggedPiece = GameState->Board[App->DraggedPiece.BoardY][App->DraggedPiece.BoardX];

					// Player moved a piece, but its not their turn (it was of wrong color)
					if(GetPieceColor(DraggedPiece) != GameState->ActiveColor)
					{
						const char *Color = (App->GameState.ActiveColor == WHITE) ? "WHITE" : "BLACK";
						printf("%s's move!\n", Color);

						App->IsDraggedPiece = false;
						break;
					}

					// What are the possible moves?
					array<move *> PossibleMoves; ArrayInit(&PossibleMoves);
					GetMovesForPiece((image **)GameState->Board, App->DraggedPiece.BoardX, App->DraggedPiece.BoardY, &PossibleMoves); //@ free things
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
						if (PossibleMove->ToX == AttemptedMove.ToX && PossibleMove->ToY == AttemptedMove.ToY)
						{
							assert(PossibleMove->FromX == AttemptedMove.FromX);
							assert(PossibleMove->FromY == AttemptedMove.FromY);
							CanMakeMove = true;
							break;
						}
					}
					if (!CanMakeMove)
					{
						printf("not a legal move!\n");
						App->IsDraggedPiece = false;
						break;
					}

					// // Alternative way to get moves:
					//@ I think that's better
					// unless there is some reason we want an array
					// moveInfoSomething Moves[8][8]; // y, x
					// GetMoveInfo(&GameState, PieceX, PieceY, Moves)
					// maybe we want more info about the move
					// also moves we cant make might also include why
					// if(Moves[UpY][UpX]) {
					// 	// can make the move
					// } else {
					// 	// nope
					// }

					image *PossibleCapturedPiece = MakeMove(GameState, &AttemptedMove);

					if (IsInCheck((image **)GameState->Board, GameState->ActiveColor))
					{
						// The user attempted a move that leaves their king in check.
						// Play some sound / animation maybe.

						UnmakeMove(GameState, &AttemptedMove, PossibleCapturedPiece);

						const char *Color = (GameState->ActiveColor == WHITE) ? "WHITE" : "BLACK";
						printf("this move would leave %s in check!\n", Color);

						App->IsDraggedPiece = false;
						break;
					}

					App->IsDraggedPiece = false;

					SwitchTurn(GameState);

					// gameOver GameOver = IsGameOver(App->GameState)
					// if(GameOver)
					// {
					// 	if(GameOver == gameOver_CHECKMATE)
					// 	{
					// 		//
					// 	}
					// 	// if(GameOver == gameOver_STALEMATE) {}
					// 	// if(GameOver == gameOver_INSUFFICIENT_MATERIAL) {}
					// 	// if(GameOver == gameOver_FIFTYTH_MOVE_RULE) {}
					// 	// if(GameOver == gameOver_THREEFOLD_REPETITION_RULE) {}
					// }
					// else
					// {
					// 	//
					// }

					// The opponent could be a computer or a human. If human, then this will be ignored:
					App->Players[GameState->ActiveColor].ComputerCalculateMove = true;

					// check if it was a check / checkmate
					bool Check = false;
					if (IsInCheck((image **)GameState->Board, GameState->ActiveColor))
					{
						printf("%s is in check!\n", (GameState->ActiveColor == WHITE) ? "WHITE" : "BLACK");
						Check = true;
					}
					printf("%s's move...\n", (GameState->ActiveColor == WHITE) ? "WHITE" : "BLACK");

					// Play sound
					playingSound *PlayingSound = (playingSound *) malloc(sizeof(playingSound));
					PlayingSound->Sound = Check ? &CheckSound : (PossibleCapturedPiece ? &CaptureSound : &MoveSound);
					PlayingSound->NumSamplesPlayed = 0;
					PlayingSound->IsLooping = false;
					PlayingSounds.append(PlayingSound);

					if (IsCheckmated((image **)GameState->Board, GameState->ActiveColor))
					{
						printf("%s is checkmated!\n", (GameState->ActiveColor == WHITE) ? "WHITE" : "BLACK");
						App->Mode = appMode_POSTGAME;
						break;
					}
				}
				else
				{
					printf("MOUSE BUTTON UP, no dragged piece\n");
				}
			}
			break;
			default:
			{
				assert(false);
			}
		}
	}

	// Computer's move.
	player *ActivePlayer = &App->Players[GameState->ActiveColor];
	if(ActivePlayer->PlayerType == playerType_COMPUTER)
	{
		if(ActivePlayer->ComputerCalculateMove)
		{
			// Pick random move.
			move *ComputerMove = NULL;
			image *MovedPiece  = NULL;

			// Get computer move
			array<move *> ComputerPossibleMoves; ArrayInit(&ComputerPossibleMoves); //@ free things

			for (int Y = 0; Y < 8; ++Y)
			{
				for (int X = 0; X < 8; ++X)
				{
					image *Piece = GameState->Board[Y][X];
					if (Piece && (GetPieceColor(Piece) == GameState->ActiveColor))
					{
						GetMovesForPiece((image **)GameState->Board, X, Y, &ComputerPossibleMoves);
					}
				}
			}

			// Generate random index
			assert(ComputerPossibleMoves.Count > 0); // mod by 0 is undefined
			int RandIndex = ((unsigned int)rand()) % (ComputerPossibleMoves.Count);
			printf("RandIndex: %d, ComputerPossibleMoves.Count: %d\n", RandIndex, ComputerPossibleMoves.Count);
			assert(0 <= RandIndex && RandIndex < ComputerPossibleMoves.Count);

			for(int i = 0; i < ComputerPossibleMoves.Count; ++i)
			{
				int Index = (RandIndex + i) % ComputerPossibleMoves.Count;
				move *PlausableMove = ComputerPossibleMoves.Data[Index];

				image *PossibleCapturedPiece = MakeMove(GameState, PlausableMove);
				if (IsInCheck((image **)GameState->Board, GameState->ActiveColor))
				{
					UnmakeMove(GameState, PlausableMove, PossibleCapturedPiece);
					
					continue;
				}

				UnmakeMove(GameState, PlausableMove, PossibleCapturedPiece);
				
				ComputerMove = PlausableMove;
				MovedPiece = GameState->Board[PlausableMove->FromY][PlausableMove->FromX];

				break;
			}

			// assert(ComputerMove && MovedPiece);

			ActivePlayer->ComputerCalculateMove = false;

			App->PlayingAnimation = &App->Animation;
			MakeAnimation(App, App->PlayingAnimation, *ComputerMove, MovedPiece);
			GameState->Board[ComputerMove->FromY][ComputerMove->FromX] = NULL;
		}


		// // time ->
		// // [calc move] -> [simulate thinking] -> [animate move] -> and finally make the move
	}

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
			if (GameState->Board[Y][X] && !(App->IsDraggedPiece && App->DraggedPiece.BoardX == X && App->DraggedPiece.BoardY == Y))
			{
				RenderImage(WindowBuffer, GameState->Board[Y][X], X * 60, Y * 60);
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

	i32 MouseX = Input->LastMouseX;
	i32 MouseY = Input->LastMouseY;
//	printf("MouseX: %d, MouseY: %d\n", MouseX, MouseY);
	// If we have a dragged piece, render it
	if (App->IsDraggedPiece)
	{
		RenderImage(WindowBuffer, GameState->Board[App->DraggedPiece.BoardY][App->DraggedPiece.BoardX], MouseX - 30, MouseY - 30);
	}

	if (App->PlayingAnimation)
	{
		animation* A = App->PlayingAnimation;
		assert(A->ProgressFrames < A->DurationFrames);

		// printf("animation in progress (%d, x: %f, y: %f)\n", AnimFramesDone, AnimCurrentX, AnimCurrentY);
		RenderImage(WindowBuffer, A->Piece, (int)A->X, (int)A->Y);

		
		A->ProgressFrames += 1;

		if (A->ProgressFrames < A->DurationFrames)
		{
			A->X += A->DX;
			A->Y += A->DY;
		}
		else
		{
			assert(A->ProgressFrames == A->DurationFrames);

			App->PlayingAnimation = NULL;

			GameState->Board[A->Move.FromY][A->Move.FromX] = A->Piece;
			image *PossibleCapturedPiece = MakeMove(GameState, &A->Move);

			SwitchTurn(GameState);

			player *Opponent = &App->Players[GameState->ActiveColor];
			Opponent->ComputerCalculateMove = true; // if human, then ignored

			// check if it was a check
			bool Check = false;
			if (IsInCheck((image **)GameState->Board, GameState->ActiveColor))
			{
				printf("%s is in check!\n", (GameState->ActiveColor == WHITE) ? "WHITE" : "BLACK");
				Check = true;
			}

			printf("%s's move...\n", (GameState->ActiveColor == WHITE) ? "WHITE" : "BLACK");

			// Play sound
			playingSound *PlayingSound = (playingSound *) malloc(sizeof(playingSound));
			PlayingSound->Sound = Check ? &CheckSound : (PossibleCapturedPiece ? &CaptureSound : &MoveSound);
			PlayingSound->NumSamplesPlayed = 0;
			PlayingSound->IsLooping = false;
			PlayingSounds.append(PlayingSound);

			if (IsCheckmated((image **)GameState->Board, GameState->ActiveColor))
			{
				printf("%s is in checkmate!\n", (GameState->ActiveColor == WHITE) ? "WHITE" : "BLACK");

				App->Mode = appMode_POSTGAME;
			}
		}
	}
}

void UpdatePostGame(app *App, image *WindowBuffer, userInput *Input)
{
	array<event> Events = Input->Events;
	for (int I = 0; I < Events.Count; ++I)
	{
		event Event = Events.Data[I];
		switch (Event.Type)
		{
			case LBUTTONDOWN:
			{
				printf("pregame: LBUTTONDOWN: X: %d, Y: %d\n", Event.X, Event.Y);
			}
			break;
			case LBUTTONUP:
			{
				printf("pregame: LBUTTONUP: X: %d, Y: %d\n", Event.X, Event.Y);
			}
			break;
			case KEYPRESS:
			{
				printf("pregame: KEYPRESS: keycode: %u\n", Event.KeyCode);

				App->Mode = appMode_PREGAME;
			}
			break;
			default:
				assert(false);
		}
	}

	uint32_t Color = 0xff00ff00;
	RenderRectangle(WindowBuffer, 0, 0, WindowBuffer->Width, WindowBuffer->Height, Color);
}

void InitApp()
{
	App.TileWidth = 60;
	App.TileHeight = 60;

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
	const char *ThemeDir = "standard";
	// const char *ThemeDir = "chess-dot-com";
	const int MAX_PATH_LEN = 1024; //@ something reasonable here?
	const int BUF_LEN = MAX_PATH_LEN+1;
	char SoundFilePath[BUF_LEN];
	snprintf(SoundFilePath, BUF_LEN, "sounds/%s/Move.wav", ThemeDir); LoadSound(SoundFilePath, &MoveSound);
	snprintf(SoundFilePath, BUF_LEN, "sounds/%s/Capture.wav", ThemeDir); LoadSound(SoundFilePath, &CaptureSound);
	snprintf(SoundFilePath, BUF_LEN, "sounds/%s/Check.wav", ThemeDir); LoadSound(SoundFilePath, &CheckSound);
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

//	player Players[2] = {{HUMAN}, {HUMAN}};
//	color WhosMove = WHITE;
//	//@Perspective =; // From whos perpective are we looking at the board.
//	bool GameOver = false;
//	array<move *> DraggedPieceAvailableMoves;
//	ArrayInit(&DraggedPieceAvailableMoves);
////	ArrayAdd(&DraggedPieceAvailableMoves, new_move(0, 0, 0, 2));

	// random number generator
	// otherwise we get the same set of random numbers every time
	srand(time(NULL));

	App.Mode = appMode_PREGAME;
}

void UpdateApp(image *WindowBuffer, soundBuffer *SoundBuffer, userInput *Input)
{
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

	switch(App.Mode)
	{
		case appMode_PREGAME:
			UpdatePreGame(&App, WindowBuffer, Input);
			break;
		case appMode_GAME:
			UpdateGame(&App, WindowBuffer, Input);
			break;
		case appMode_POSTGAME:
			UpdatePostGame(&App, WindowBuffer, Input);
			break;
		// case appMode_LEARNCHESS:
		// 	//...
		// 	break;
		default:
			assert(false);
	}
}