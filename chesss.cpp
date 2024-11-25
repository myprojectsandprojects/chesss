/*
Todo:
- example: if white's kingside rook is captured, white can manouver its queenside rook to its kingside rook's initial position and castle. I doubt this should happen though???

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

// image *Board[8 * 8];

// enum castlingType {
// 	WHITE_KINGSIDE = 0,
// 	WHITE_QUEENSIDE = 1,
// 	BLACK_KINGSIDE = 2,
// 	BLACK_QUEENSIDE = 3
// };

struct castlingRights
{
	bool Kingside;
	bool Queenside;
};

enum moveType
{
	moveType_REGULAR,
	moveType_KINGSIDE_CASTLING,
	moveType_QUEENSIDE_CASTLING,
	moveType_PAWN_PROMOTION,
	moveType_EN_PASSANT
};

struct regularMove
{
	int FromX, FromY, ToX, ToY;
};

struct pawnPromotionMove
{
	int FromX, ToX;
	image *PromoteTo;
};

struct enPassantMove
{
	int FromX; // a pawn moves diagonally to en-passant square, so the only question is: from which column, as there can be 2
};

struct move
{
	moveType Type;
	color Side; // have it or not have it? i dont know

	union
	{
		regularMove Regular;
		pawnPromotionMove PawnPromotion;
		enPassantMove EnPassant;
	};
};

struct enPassantSquare
{
	bool Exists;
	int X, Y;
};

// struct move
// {
// 	int FromX;
// 	int FromY;
// 	int ToX;
// 	int ToY;
// 	// int X0, Y0, X1, Y1;

// 	bool IsPromotion;

// 	// castlingRights CastlingRightsLost;
// };

// directly translated from fen
struct gameState
{
	image *Board[8][8];

	color ActiveColor; // who's move

	castlingRights CastlingRights[2]; // use color to access

	// // this is a square behind a pawn that has moved 2 squares
	// // if any of the opponents pawns can capture at this square, then they can capture the pawn
	// // but the capture can only happen in the next move
	// // this square only exists during the opponents move
	enPassantSquare EnPassantSquare;

	// int HalfmoveCounter; // 50-move rule
	// int CurrentMoveNumber;
};

struct animation
{
	int DurationFrames;
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

enum promotionDialogPiece
{
	promotionDialogPiece_NO_PIECE = -1,
	promotionDialogPiece_QUEEN,
	promotionDialogPiece_KNIGHT,
	promotionDialogPiece_BISHOP,
	promotionDialogPiece_ROOK
};

struct promotionDialog
{
	int X;
	int Y;
	int Width;
	int Height;

	// color Color;

	promotionDialogPiece PieceIndex;
	image *SelectablePieces[2][4];

	move MoveToMake;
};

struct app
{
	appMode Mode;

	gameState GameState;
	player Players[2];

	int TileWidth, TileHeight;
	int BoardWidth, BoardHeight;

	bool IsDraggedPiece;
	struct {
		int X, Y;
		array<move> AvailableMoves;
	} DraggedPiece;

	animation *PlayingAnimation; // just 1 for now
	animation Animation;

	bool DisplayPromotionDialog;
	promotionDialog PromotionDialog;

	bool PreviousStateExists;
	gameState PreviousGameState;

	bool IsGameDebugMode;
} App;

enum gameOver
{
	gameOver_NOT_OVER = 0,
	gameOver_CHECKMATE,
	gameOver_STALEMATE
};

// struct unmakeMove
// {
// 	image *CapturedPiece;
// 	castlingRights CastlingRightsLost; // castling rights to restore
// 	bool SetEnPassantSquare;
// };

//loadedSound TestSound;
loadedSound MoveSound;
loadedSound CaptureSound;
loadedSound CheckSound;

// "The C++ standard guarantees that the members of a class or struct appear in memory in the same order as they are declared."
//@ if we store all white piece images before all black piece images, for example, then, given that we have an image pointer, we could just compare it with the memory address of the first black piece image to determine the color. and that would be just one comparsion. (?)
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

#if 0
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
#endif

#if 0
move *NewMove(int FromX, int FromY, int ToX, int ToY)
{
	move *Move = (move *) malloc(sizeof(move));
	Move->FromX = FromX;
	Move->FromY = FromY;
	Move->ToX = ToX;
	Move->ToY = ToY;
	return Move;
}
#endif

// void Make_move(move *Move, int FromX, int FromY, int ToX, int ToY, bool IsPromotion = false)
// {
// 	Move->FromX = FromX;
// 	Move->FromY = FromY;
// 	Move->ToX = ToX;
// 	Move->ToY = ToY;
// 	Move->IsPromotion = IsPromotion;
// }

void Make_regularMove(move *Move, color Side, int FromX, int FromY, int ToX, int ToY)
{
	Move->Type = moveType_REGULAR;
	Move->Side = Side;

	Move->Regular.FromX = FromX;
	Move->Regular.FromY = FromY;
	Move->Regular.ToX = ToX;
	Move->Regular.ToY = ToY;
}

void Make_kingsideCastlingMove(move *Move, color Side)
{
	Move->Type = moveType_KINGSIDE_CASTLING;
	Move->Side = Side;
}

void Make_queensideCastlingMove(move *Move, color Side)
{
	Move->Type = moveType_QUEENSIDE_CASTLING;
	Move->Side = Side;
}

void Make_pawnPromotionMove(move *Move, color Side, int FromX, int ToX)
{
	Move->Type = moveType_PAWN_PROMOTION;
	Move->Side = Side;

	Move->PawnPromotion.FromX = FromX;
	Move->PawnPromotion.ToX = ToX;
	Move->PawnPromotion.PromoteTo = NULL;
}

void Make_enPassantMove(move *Move, color Side, int FromX)
{
	Move->Type = moveType_EN_PASSANT;
	Move->Side = Side;

	Move->EnPassant.FromX = FromX;
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

// // easily unmakeable
// image *MakeMove(gameState *GameState, move *Move)
// {
// 	// make the move on board, so that we can check for checks
// }

// // you cant unmake this
// void MakeFinalMove(gameState *GameState, move *Move)
// {
// 	// update everything else around the move

// 	MakeMove(gameState *GameState, move *Move);
// }

// later on, should updating move counters, castling rights etc. also be done here?
void MakeMove(app *App, gameState *GameState, move *Move)
// unmakeMove MakeMove(gameState *GameState, move *Move)
{
	// // unmakeMove UnmakeMove = {};
	// unmakeMove Unmake;
	// Unmake.CapturedPiece = NULL;
	// Unmake.CastlingRightsLost.Kingside = false;
	// Unmake.CastlingRightsLost.Queenside = false;
	// Unmake.SetEnPassantSquare = false; //@ incomplete

	App->PreviousGameState = App->GameState;
	App->PreviousStateExists = true;

	enPassantSquare EnPassantSquare = GameState->EnPassantSquare;
	GameState->EnPassantSquare.Exists = false;

	switch(Move->Type)
	{
		case moveType_REGULAR:
		{
			regularMove *Regular = (regularMove *)&Move->Regular;

			image *CapturedPiece = GameState->Board[Regular->ToY][Regular->ToX];
			GameState->Board[Regular->ToY][Regular->ToX] = GameState->Board[Regular->FromY][Regular->FromX];
			GameState->Board[Regular->FromY][Regular->FromX] = NULL;

			// Unmake.CapturedPiece = CapturedPiece;

			image *MovedPiece = GameState->Board[Regular->ToY][Regular->ToX];

			// if pawn moved two squares, set en-passant square
			if(GameState->ActiveColor == WHITE)
			{
				if(MovedPiece == &WPawnImage && Regular->FromY == 6 && Regular->ToY == 4)
				{
					assert(Regular->FromX == Regular->ToX);
					
					GameState->EnPassantSquare.Exists = true;
					GameState->EnPassantSquare.X = Regular->ToX;
					GameState->EnPassantSquare.Y = Regular->ToY + 1;

					// Unmake.SetEnPassantSquare = true;
				}
			}
			else
			{
				assert(GameState->ActiveColor == BLACK);

				if(MovedPiece == &BPawnImage && Regular->FromY == 1 && Regular->ToY == 3)
				{
					assert(Regular->FromX == Regular->ToX);

					GameState->EnPassantSquare.Exists = true;
					GameState->EnPassantSquare.X = Regular->ToX;
					GameState->EnPassantSquare.Y = Regular->ToY - 1;

					// Unmake.SetEnPassantSquare = true;
				}
			}
		}
		break;
		case moveType_KINGSIDE_CASTLING:
		{
			assert(GameState->CastlingRights[GameState->ActiveColor].Kingside);
			if(Move->Side == WHITE)
			{
				// king
				GameState->Board[7][6] = GameState->Board[7][4];
				GameState->Board[7][4] = NULL;

				// rook
				GameState->Board[7][5] = GameState->Board[7][7];
				GameState->Board[7][7] = NULL;
			}
			else
			{
				assert(Move->Side == BLACK);

				// king
				GameState->Board[0][4] = NULL;
				GameState->Board[0][6] = &BKingImage;

				// rook
				GameState->Board[0][7] = NULL;
				GameState->Board[0][5] = &BRookImage;
			}
		}
		break;
		case moveType_QUEENSIDE_CASTLING:
		{
			assert(GameState->CastlingRights[GameState->ActiveColor].Queenside);
			if(Move->Side == WHITE)
			{
				// castling

				// king
				GameState->Board[7][2] = GameState->Board[7][4];
				GameState->Board[7][4] = NULL;

				// rook
				GameState->Board[7][3] = GameState->Board[7][0];
				GameState->Board[7][0] = NULL;
			}
			else
			{
				assert(Move->Side == BLACK);

				// king
				GameState->Board[0][4] = NULL;
				GameState->Board[0][2] = &BKingImage;

				// rook
				GameState->Board[0][0] = NULL;
				GameState->Board[0][3] = &BRookImage;
			}

			// castling rights
			GameState->CastlingRights[GameState->ActiveColor].Kingside = false;
			GameState->CastlingRights[GameState->ActiveColor].Queenside = false;
		}
		break;
		case moveType_PAWN_PROMOTION:
		{
			if(Move->Side == WHITE)
			{
				GameState->Board[0][Move->PawnPromotion.ToX] = Move->PawnPromotion.PromoteTo;
				GameState->Board[1][Move->PawnPromotion.FromX] = NULL;
			}
			else
			{
				assert(Move->Side == BLACK);
				GameState->Board[7][Move->PawnPromotion.ToX] = Move->PawnPromotion.PromoteTo;
				GameState->Board[6][Move->PawnPromotion.FromX] = NULL;
			}
		}
		break;
		case moveType_EN_PASSANT:
		{
			assert(EnPassantSquare.Exists);
			if(Move->Side == WHITE)
			{
				GameState->Board[EnPassantSquare.Y][EnPassantSquare.X] = GameState->Board[3][Move->EnPassant.FromX];
				GameState->Board[3][Move->EnPassant.FromX] = NULL;
				GameState->Board[EnPassantSquare.Y + 1][EnPassantSquare.X] = NULL;
			}
			else
			{
				assert(Move->Side == BLACK);

				GameState->Board[EnPassantSquare.Y][EnPassantSquare.X] = GameState->Board[4][Move->EnPassant.FromX];
				GameState->Board[4][Move->EnPassant.FromX] = NULL;
				GameState->Board[EnPassantSquare.Y - 1][EnPassantSquare.X] = NULL;
			}
		}
		break;

		default: assert(false);
	}

	// castling rights (castling, king, rook)
	if(Move->Type == moveType_REGULAR)
	{
		image *MovingPiece = GameState->Board[Move->Regular.ToY][Move->Regular.ToX];

		if(MovingPiece == &WKingImage || MovingPiece == &BKingImage)
		{
			GameState->CastlingRights[GameState->ActiveColor].Kingside = false;
			GameState->CastlingRights[GameState->ActiveColor].Queenside = false;
		}

		if(MovingPiece == &WRookImage)
		{
			if(Move->Regular.FromY == 7 && Move->Regular.FromX == 7)
			{
				// if(GameState->CastlingRights[WHITE].Kingside)
				// {
				// 	// printf("WHITES'S KINGSIDE ROOK MOVED FIRST TIME\n");
				// 	GameState->CastlingRights[WHITE].Kingside = false;
				// }
				GameState->CastlingRights[WHITE].Kingside = false;
			}
			else if(Move->Regular.FromY == 7 && Move->Regular.FromX == 0)
			{
				GameState->CastlingRights[WHITE].Queenside = false;
			}
		}
		else if(MovingPiece == &BRookImage)
		{
			if(Move->Regular.FromY == 0 && Move->Regular.FromX == 7)
			{
				GameState->CastlingRights[BLACK].Kingside = false;
			}
			else if(Move->Regular.FromY == 0 && Move->Regular.FromX == 0)
			{
				GameState->CastlingRights[BLACK].Queenside = false;
			}
		}
	}
	else if(Move->Type == moveType_KINGSIDE_CASTLING || Move->Type == moveType_QUEENSIDE_CASTLING)
	{
			GameState->CastlingRights[GameState->ActiveColor].Kingside = false;
			GameState->CastlingRights[GameState->ActiveColor].Queenside = false;
	}

	// return Unmake;
}

void UnmakeLastMove(app *App)
{
	assert(App->PreviousStateExists);

	App->GameState = App->PreviousGameState;
	App->PreviousStateExists = false;
}

// void UnmakeMove(gameState *GameState, move *Move, unmakeMove Unmake)
// // void UnmakeMove(gameState *GameState, move *Move, image *PossibleCapturedPiece)
// {
// 	// restore castling rights
// 	if(Unmake.CastlingRightsLost.Kingside)
// 	{
// 		color OurColor = GetPieceColor(GameState->Board[Move->ToY][Move->ToX]);
// 		GameState->CastlingRights[OurColor].Kingside = true;
// 	}

// 	if(GameState->Board[Move->ToY][Move->ToX] == &WKingImage
// 		&& Move->FromX == 4 && Move->FromY == 7
// 		&& Move->ToX == 6 && Move->ToY == 7)
// 	{
// 		// castling

// 		// king
// 		GameState->Board[Move->FromY][Move->FromX] = GameState->Board[Move->ToY][Move->ToX];
// 		GameState->Board[Move->ToY][Move->ToX] = NULL;

// 		// rook
// 		GameState->Board[7][7] = GameState->Board[7][5];
// 		GameState->Board[7][5] = NULL;

// 		return;
// 	}
// 	else if(GameState->Board[Move->ToY][Move->ToX] == &BKingImage
// 		&& Move->FromX == 4 && Move->FromY == 0
// 		&& Move->ToX == 6 && Move->ToY == 0)
// 	{
// 		// castling

// 		// king
// 		GameState->Board[Move->FromY][Move->FromX] = GameState->Board[Move->ToY][Move->ToX];
// 		GameState->Board[Move->ToY][Move->ToX] = NULL;

// 		// rook
// 		GameState->Board[0][7] = GameState->Board[0][5];
// 		GameState->Board[0][5] = NULL;

// 		return;
// 	}

// 	GameState->Board[Move->FromY][Move->FromX] = GameState->Board[Move->ToY][Move->ToX];
// 	GameState->Board[Move->ToY][Move->ToX] = Unmake.CapturedPiece;
// }

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
void SetUpGameState(gameState *GameState, const char *Fen)
{
	//@ implement this
}

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

//@ en-passant
void GetMovesForPawn(gameState *GameState, int PieceX, int PieceY, array<move> *LegalMoves)
{
	image *Piece = GameState->Board[PieceY][PieceX];
	assert(Piece == &WPawnImage || Piece == &BPawnImage);

	color OurColor = GetPieceColor(Piece);
	color OpponentsColor = (OurColor == WHITE) ? BLACK : WHITE;

	int OneStepForward = (OurColor == WHITE) ? -1 : +1;
	int FirstRow = (OurColor == WHITE) ? 7 : 0;
	int LastRow = FirstRow + OneStepForward * 7;

	int X, Y;

	X = PieceX; Y = PieceY + OneStepForward;
	if(X >= 0 && Y >= 0 && X < 8 && Y < 8)
	{
		if(GameState->Board[Y][X] == NULL)
		{
			move *Move = ArrayAdd(LegalMoves);

			if(Y == LastRow)
			{
				// Move->IsPromotion = true;
				Make_pawnPromotionMove(Move, OurColor, PieceX, X);
			}
			else
			{
				Make_regularMove(Move, OurColor, PieceX, PieceY, X, Y);
			}
		}
	}

	if(PieceY == FirstRow + OneStepForward)
	{
		X = PieceX; Y = PieceY + OneStepForward * 2;
		if(X >= 0 && Y >= 0 && X < 8 && Y < 8)
		{
			if(GameState->Board[Y][X] == NULL && GameState->Board[Y - OneStepForward][X] == NULL)
			{
				// LegalMoves[Y][X] = (moveBits)(LegalMoves[Y][X] | moveBits_LEGAL_MOVE);

				move *Move = ArrayAdd(LegalMoves);
				// Make_move(Move, PieceX, PieceY, X, Y);
				Make_regularMove(Move, OurColor, PieceX, PieceY, X, Y);

				// promotion cant happen here
				assert(Y != LastRow);
			}
		}
	}

	X = PieceX - 1; Y = PieceY + OneStepForward;
	if(X >= 0 && Y >= 0 && X < 8 && Y < 8)
	{
		if(GameState->Board[Y][X] && GetPieceColor(GameState->Board[Y][X]) == OpponentsColor)
		{
			// LegalMoves[Y][X] = (moveBits)(LegalMoves[Y][X] | moveBits_LEGAL_MOVE);
			// // LegalMoves[Y][X] = (moveBits)(LegalMoves[Y][X] | moveBits_CAPTURE);

			move *Move = ArrayAdd(LegalMoves);

			if(Y == LastRow)
			{
				// Move->IsPromotion = true;
				Make_pawnPromotionMove(Move, OurColor, PieceX, X);
			}
			else
			{
				Make_regularMove(Move, OurColor, PieceX, PieceY, X, Y);
			}
		}
		else if(GameState->Board[Y][X] == NULL && GameState->EnPassantSquare.Exists && GameState->EnPassantSquare.X == X && GameState->EnPassantSquare.Y == Y)
		{
			move *Move = ArrayAdd(LegalMoves);
			Make_enPassantMove(Move, OurColor, PieceX);
		}
	}

	X = PieceX + 1; Y = PieceY + OneStepForward;
	if(X >= 0 && Y >= 0 && X < 8 && Y < 8)
	{
		if(GameState->Board[Y][X] && GetPieceColor(GameState->Board[Y][X]) == OpponentsColor)
		{
			// // LegalMoves[Y][X] = (moveBits)(LegalMoves[Y][X] | moveBits_CAPTURE);

			move *Move = ArrayAdd(LegalMoves);

			if(Y == LastRow)
			{
				// Move->IsPromotion = true;
				Make_pawnPromotionMove(Move, OurColor, PieceX, X);
			}
			else
			{
				Make_regularMove(Move, OurColor, PieceX, PieceY, X, Y);
			}
		}
		else if(GameState->Board[Y][X] == NULL && GameState->EnPassantSquare.Exists && GameState->EnPassantSquare.X == X && GameState->EnPassantSquare.Y == Y)
		{
			move *Move = ArrayAdd(LegalMoves);
			Make_enPassantMove(Move, OurColor, PieceX);
		}
	}
}

void GetMovesForKing(gameState *GameState, int PieceX, int PieceY, array<move> *LegalMoves)
{
	color OurColor = GetPieceColor(GameState->Board[PieceY][PieceX]);

// 	double Pi = 3.14159;
// 	double PiOverFour = Pi / 4.0;
// 	double TwoPi = 2.0 * Pi;
// //		printf("%f\n", PiOverFour);

// 	for (double A = 0; A < TwoPi; A += PiOverFour)
// 	{
// 		int DX = ((int) (cos(A) + 1.0 + 0.5)) - 1;
// 		int DY = ((int) (sin(A) + 1.0 + 0.5)) - 1;
// 		int X = PieceX + DX;
// 		int Y = PieceY + DY;
// 		if (X < 0 || X > 7 || Y < 0 || Y > 7)
// 		{
// 			continue;
// 		}

// 		if(GameState->Board[Y][X] == NULL || GetPieceColor(GameState->Board[Y][X]) != OurColor)
// 		{
// 			move *Move = ArrayAdd(LegalMoves);
// 			Make_move(Move, PieceX, PieceY, X, Y);
// 		}
// 		else
// 		{
// 			continue;
// 		}
// 	}

	int Deltas[8][2] = {
		// x, y
		{0, -1}, // up
		{1, -1}, // up-right
		{1, 0}, // right
		{1, 1}, // down-right
		{0, 1}, // down
		{-1, 1}, // down-left
		{-1, 0}, // left
		{-1, -1} // up-left
	};

	for(int i = 0; i < 8; ++i)
	{
		int X = PieceX + Deltas[i][0];
		int Y = PieceY + Deltas[i][1];

		if (X < 0 || X > 7 || Y < 0 || Y > 7)
		{
			continue;
		}

		if(GameState->Board[Y][X] == NULL)
		{
			move *Move = ArrayAdd(LegalMoves);
			// Make_move(Move, PieceX, PieceY, X, Y);
			Make_regularMove(Move, OurColor, PieceX, PieceY, X, Y);
		}
		else if(GetPieceColor(GameState->Board[Y][X]) != OurColor)
		{
			// capture
			move *Move = ArrayAdd(LegalMoves);
			// Make_move(Move, PieceX, PieceY, X, Y);
			Make_regularMove(Move, OurColor, PieceX, PieceY, X, Y);
		}
	}

	// castling
	if(OurColor == WHITE)
	{
		if(GameState->CastlingRights[WHITE].Kingside
			&& GameState->Board[7][4] == &WKingImage
			&& GameState->Board[7][5] == NULL
			&& GameState->Board[7][6] == NULL
			&& GameState->Board[7][7] == &WRookImage)
		{
			move *Move = ArrayAdd(LegalMoves);
			// Make_move(Move, PieceX, PieceY, 6, 7);
			Make_kingsideCastlingMove(Move, OurColor);
		}

		if(GameState->CastlingRights[WHITE].Queenside
			&& GameState->Board[7][4] == &WKingImage
			&& GameState->Board[7][1] == NULL
			&& GameState->Board[7][2] == NULL
			&& GameState->Board[7][3] == NULL
			&& GameState->Board[7][0] == &WRookImage)
		{
			move *Move = ArrayAdd(LegalMoves);
			// Make_move(Move, PieceX, PieceY, 6, 7);
			Make_queensideCastlingMove(Move, OurColor);
		}
	}
	else
	{
		assert(OurColor == BLACK);

		if(GameState->CastlingRights[BLACK].Kingside
			&& GameState->Board[0][4] == &BKingImage
			&& GameState->Board[0][5] == NULL
			&& GameState->Board[0][6] == NULL
			&& GameState->Board[0][7] == &BRookImage)
		{
			move *Move = ArrayAdd(LegalMoves);
			Make_kingsideCastlingMove(Move, OurColor);
		}

		if(GameState->CastlingRights[BLACK].Queenside
			&& GameState->Board[0][4] == &BKingImage
			&& GameState->Board[0][1] == NULL
			&& GameState->Board[0][2] == NULL
			&& GameState->Board[0][3] == NULL
			&& GameState->Board[0][0] == &BRookImage)
		{
			move *Move = ArrayAdd(LegalMoves);
			// Make_move(Move, PieceX, PieceY, 6, 7);
			Make_queensideCastlingMove(Move, OurColor);
		}
	}
}

void GetMovesForKnight(gameState *GameState, int PieceX, int PieceY, array<move> *LegalMoves)
{
	color OurColor = GetPieceColor(GameState->Board[PieceY][PieceX]);

	int XDeltas[] = {-1, +1, -2, +2, -2, +2, -1, +1};
	int YDeltas[] = {-2, -2, -1, -1, +1, +1, +2, +2};
	
	for (int I = 0; I < 8; ++I)
	{
		int X = PieceX + XDeltas[I];
		int Y = PieceY + YDeltas[I];
		if (X < 0 || X > 7 || Y < 0 || Y > 7)
		{
			continue;
		}
//			printf("X: %d, Y: %d\n", X, Y);

		// int BoardIndex = Y * 8 + X;
		// if (Board[BoardIndex] && ((IsWhitePiece(Piece) && IsWhitePiece(Board[BoardIndex])) || (IsBlackPiece(Piece) && IsBlackPiece(Board[BoardIndex]))))
		// {
		// 	continue;
		// }
		// else
		// {
		// 	move *Move = (move *) malloc(sizeof(move));
		// 	Move->FromX = PieceX;
		// 	Move->FromY = PieceY;
		// 	Move->ToX = X;
		// 	Move->ToY = Y;
		// 	ArrayAdd(Moves, Move);
		// }

		if(GameState->Board[Y][X] == NULL || GetPieceColor(GameState->Board[Y][X]) != OurColor)
		{
			move *Move = ArrayAdd(LegalMoves);
			// Make_move(Move, PieceX, PieceY, X, Y);
			Make_regularMove(Move, OurColor, PieceX, PieceY, X, Y);
		}
		else
		{
			continue;
		}
	}
}

void GetMovesForBishop(gameState *GameState, int PieceX, int PieceY, array<move> *LegalMoves)
{
	color OurColor = GetPieceColor(GameState->Board[PieceY][PieceX]);

	// scan diagonally in down-right direction
	for (int X = PieceX + 1, Y = PieceY + 1; X < 8 && Y < 8; ++X, ++Y)
	{
		if(GameState->Board[Y][X])
		{
			if(GetPieceColor(GameState->Board[Y][X]) != OurColor)
			{
				move *Move = ArrayAdd(LegalMoves);
				// Make_move(Move, PieceX, PieceY, X, Y);
				Make_regularMove(Move, OurColor, PieceX, PieceY, X, Y);
			}
			break;
		}
		else
		{
			move *Move = ArrayAdd(LegalMoves);
			// Make_move(Move, PieceX, PieceY, X, Y);
			Make_regularMove(Move, OurColor, PieceX, PieceY, X, Y);
		}
	}

	// scan diagonally in down-left direction
	for (int X = PieceX - 1, Y = PieceY + 1; X >= 0 && Y < 8; --X, ++Y)
	{
		if(GameState->Board[Y][X])
		{
			if(GetPieceColor(GameState->Board[Y][X]) != OurColor)
			{
				move *Move = ArrayAdd(LegalMoves);
				// Make_move(Move, PieceX, PieceY, X, Y);
				Make_regularMove(Move, OurColor, PieceX, PieceY, X, Y);
			}
			break;
		}
		else
		{
			move *Move = ArrayAdd(LegalMoves);
			// Make_move(Move, PieceX, PieceY, X, Y);
			Make_regularMove(Move, OurColor, PieceX, PieceY, X, Y);
		}
	}

	// scan diagonally in top-left direction
	for (int X = PieceX - 1, Y = PieceY - 1; X >= 0 && Y >= 0; --X, --Y)
	{
		if(GameState->Board[Y][X])
		{
			if(GetPieceColor(GameState->Board[Y][X]) != OurColor)
			{
				move *Move = ArrayAdd(LegalMoves);
				// Make_move(Move, PieceX, PieceY, X, Y);
				Make_regularMove(Move, OurColor, PieceX, PieceY, X, Y);
			}
			break;
		}
		else
		{
			move *Move = ArrayAdd(LegalMoves);
			// Make_move(Move, PieceX, PieceY, X, Y);
			Make_regularMove(Move, OurColor, PieceX, PieceY, X, Y);
		}
	}

	// scan diagonally in top-right direction
	for (int X = PieceX + 1, Y = PieceY - 1; X < 8 && Y >= 0; ++X, --Y)
	{
		if(GameState->Board[Y][X])
		{
			if(GetPieceColor(GameState->Board[Y][X]) != OurColor)
			{
				move *Move = ArrayAdd(LegalMoves);
				// Make_move(Move, PieceX, PieceY, X, Y);
				Make_regularMove(Move, OurColor, PieceX, PieceY, X, Y);
			}
			break;
		}
		else
		{
			move *Move = ArrayAdd(LegalMoves);
			// Make_move(Move, PieceX, PieceY, X, Y);
			Make_regularMove(Move, OurColor, PieceX, PieceY, X, Y);
		}
	}
}

void GetMovesForRook(gameState *GameState, int PieceX, int PieceY, array<move> *LegalMoves)
{
	color OurColor = GetPieceColor(GameState->Board[PieceY][PieceX]);

	// scan horizontally to the right
	for (int X = PieceX + 1, Y = PieceY; X < 8; ++X)
	{
		if(GameState->Board[Y][X])
		{
			if(GetPieceColor(GameState->Board[Y][X]) != OurColor)
			{
				move *Move = ArrayAdd(LegalMoves);
				// Make_move(Move, PieceX, PieceY, X, Y);
				Make_regularMove(Move, OurColor, PieceX, PieceY, X, Y);
			}
			break;
		}
		else
		{
			move *Move = ArrayAdd(LegalMoves);
			// Make_move(Move, PieceX, PieceY, X, Y);
			Make_regularMove(Move, OurColor, PieceX, PieceY, X, Y);
		}
	}

	// scan horizontally to the left
	for (int X = PieceX - 1, Y = PieceY; X >= 0; --X)
	{
		if(GameState->Board[Y][X])
		{
			if(GetPieceColor(GameState->Board[Y][X]) != OurColor)
			{
				move *Move = ArrayAdd(LegalMoves);
				// Make_move(Move, PieceX, PieceY, X, Y);
				Make_regularMove(Move, OurColor, PieceX, PieceY, X, Y);
			}
			break;
		}
		else
		{
			move *Move = ArrayAdd(LegalMoves);
			// Make_move(Move, PieceX, PieceY, X, Y);
			Make_regularMove(Move, OurColor, PieceX, PieceY, X, Y);
		}
	}

	// scan vertically to the up
	for (int Y = PieceY - 1, X = PieceX; Y >= 0; --Y)
	{
		if(GameState->Board[Y][X])
		{
			if(GetPieceColor(GameState->Board[Y][X]) != OurColor)
			{
				move *Move = ArrayAdd(LegalMoves);
				// Make_move(Move, PieceX, PieceY, X, Y);
				Make_regularMove(Move, OurColor, PieceX, PieceY, X, Y);
			}
			break;
		}
		else
		{
			move *Move = ArrayAdd(LegalMoves);
			// Make_move(Move, PieceX, PieceY, X, Y);
			Make_regularMove(Move, OurColor, PieceX, PieceY, X, Y);
		}
	}

	// scan vertically to the down
	for (int Y = PieceY + 1, X = PieceX; Y < 8; ++Y)
	{
		if(GameState->Board[Y][X])
		{
			if(GetPieceColor(GameState->Board[Y][X]) != OurColor)
			{
				move *Move = ArrayAdd(LegalMoves);
				// Make_move(Move, PieceX, PieceY, X, Y);
				Make_regularMove(Move, OurColor, PieceX, PieceY, X, Y);
			}
			break;
		}
		else
		{
			move *Move = ArrayAdd(LegalMoves);
			// Make_move(Move, PieceX, PieceY, X, Y);
			Make_regularMove(Move, OurColor, PieceX, PieceY, X, Y);
		}
	}
}

void GetMovesForPiece(gameState *GameState, int PieceX, int PieceY, array<move> *LegalMoves)
{
	image *Piece = GameState->Board[PieceY][PieceX];
	// assert(GetPieceColor(Piece) == GameState->ActiveColor);

	if(Piece == &WPawnImage || Piece == &BPawnImage)
	{
		GetMovesForPawn(GameState, PieceX, PieceY, LegalMoves);
	}
	else if(Piece == &WKingImage || Piece == &BKingImage)
	{
		GetMovesForKing(GameState, PieceX, PieceY, LegalMoves);
	}
	else if(Piece == &WQueenImage || Piece == &BQueenImage)
	{
		GetMovesForBishop(GameState, PieceX, PieceY, LegalMoves);
		GetMovesForRook(GameState, PieceX, PieceY, LegalMoves);
	}
	else if(Piece == &WKnightImage || Piece == &BKnightImage)
	{
		GetMovesForKnight(GameState, PieceX, PieceY, LegalMoves);
	}
	else if(Piece == &WBishopImage || Piece == &BBishopImage)
	{
		GetMovesForBishop(GameState, PieceX, PieceY, LegalMoves);
	}
	else if(Piece == &WRookImage || Piece == &BRookImage)
	{
		GetMovesForRook(GameState, PieceX, PieceY, LegalMoves);
	}
	else
	{
		assert(false);
	}
}

bool IsActivePlayerInCheck(gameState *GameState)
{
	bool IsCheck = false;

	array<move> OpponentsPossibleMoves; ArrayInit(&OpponentsPossibleMoves);

	bool (*IsOpponentsPiece) (image *) = (GameState->ActiveColor == WHITE) ? IsBlackPiece : IsWhitePiece;
	image *OwnKing = (GameState->ActiveColor == WHITE) ? &WKingImage : &BKingImage;

	for (int Y = 0; Y < 8; ++Y)
	{
		for (int X = 0; X < 8; ++X)
		{
			if (GameState->Board[Y][X] && IsOpponentsPiece(GameState->Board[Y][X]))
			{
				GetMovesForPiece(GameState, X, Y, &OpponentsPossibleMoves);
			}
		}
	}

	for (int i = 0; i < OpponentsPossibleMoves.Count; ++i)
	{
		move *PossibleMove = &OpponentsPossibleMoves.Data[i];
		switch(PossibleMove->Type)
		{
			case moveType_REGULAR:
			{
				if(GameState->Board[PossibleMove->Regular.ToY][PossibleMove->Regular.ToX] == OwnKing)
				{
					IsCheck = true;
					goto SearchFinished;
				}
			}
			break;
			case moveType_KINGSIDE_CASTLING:
			{
				//?
			}
			break;
			case moveType_QUEENSIDE_CASTLING:
			{
				//?
			}
			break;
			case moveType_PAWN_PROMOTION:
			{
				int Row = (PossibleMove->Side == WHITE) ? 0 : 7;
				if(GameState->Board[Row][PossibleMove->PawnPromotion.ToX] == OwnKing)
				{
					IsCheck = true;
					goto SearchFinished;
				}
			}
			break;
			case moveType_EN_PASSANT:
			{
				//?
				// its not possible that our king is on en-passant square
			}
			break;

			default: assert(false);
		}
	}
	SearchFinished:

	return IsCheck;
}

void GetLegalMovesForPiece(app* App, gameState *GameState, int PieceX, int PieceY, array<move> *LegalMoves)
{
	assert(GetPieceColor(GameState->Board[PieceY][PieceX]) == GameState->ActiveColor);

	GetMovesForPiece(GameState, PieceX, PieceY, LegalMoves);
	//@ why not check for checks in MakeMove()? simply not make a move, when leaves us in check
	// we have to do it here because we need a way to get legal moves for a piece when we highlight legal moves
	// it doesnt mean we shouldnt check for checks in MakeMove() though

	// remove moves that leave us in check
	int i = 0;
	while(i < LegalMoves->Count)
	{
		move Move = LegalMoves->Data[i]; // cant be a pointer (as long as we use UnmakeLastMove() we could though)
		// printf("%d, %d -> %d, %d -- ", Move.FromX, Move.FromY, Move.ToX, Move.ToY);

		if(Move.Type == moveType_KINGSIDE_CASTLING || Move.Type == moveType_QUEENSIDE_CASTLING)
		{
			if(IsActivePlayerInCheck(GameState))
			{
				ArrayRemove(LegalMoves, i);
				continue;
			}

			// figure out opponents color
			color OurColor = GameState->ActiveColor;
			color OpponentsColor = (OurColor == WHITE) ? BLACK : WHITE;

			// get opponents possible moves (ignore checks)
			array<move> OpponentsPossibleMoves; ArrayInit(&OpponentsPossibleMoves);
			for (int Y = 0; Y < 8; ++Y)
			{
				for (int X = 0; X < 8; ++X)
				{
					if (GameState->Board[Y][X] && GetPieceColor(GameState->Board[Y][X]) == OpponentsColor)
					{
						GetMovesForPiece(GameState, X, Y, &OpponentsPossibleMoves);
					}
				}
			}

			// iterate over opponents possible moves and see if any one of them has destination square 5,7 (if we are white) or 5,0 (if we are black)
			bool FoundMove = false;
			for (int j = 0; j < OpponentsPossibleMoves.Count; ++j)
			{
				move *PossibleMove = &OpponentsPossibleMoves.Data[j];
				switch(PossibleMove->Type)
				{
					case moveType_REGULAR:
					{
						if(
							(Move.Type == moveType_KINGSIDE_CASTLING && OurColor == WHITE
								&& PossibleMove->Regular.ToY == 7 && PossibleMove->Regular.ToX == 5)
							|| (Move.Type == moveType_KINGSIDE_CASTLING && OurColor == BLACK
								&& PossibleMove->Regular.ToY == 0 && PossibleMove->Regular.ToX == 5)
							|| (Move.Type == moveType_QUEENSIDE_CASTLING && OurColor == WHITE
								&& PossibleMove->Regular.ToY == 7 && PossibleMove->Regular.ToX == 3)
							|| (Move.Type == moveType_QUEENSIDE_CASTLING && OurColor == BLACK
								&& PossibleMove->Regular.ToY == 0 && PossibleMove->Regular.ToX == 3)
							)
						{
							FoundMove = true;
							goto SearchFinished;
						}
					}
					break;
					case moveType_KINGSIDE_CASTLING:
					{
						//?
					}
					break;
					case moveType_QUEENSIDE_CASTLING:
					{
						//?
					}
					break;
					case moveType_PAWN_PROMOTION:
					{
						int Row = (OpponentsColor == WHITE) ? 0 : 7;
						if(Row == 7 && PossibleMove->PawnPromotion.ToX == 5)
						{
							FoundMove = true;
							goto SearchFinished;
						}
					}
					break;
					case moveType_EN_PASSANT:
					{
						// do we even get this?
						assert(false);
					}
					break;

					default: assert(false);
				}
			}
			SearchFinished:

			// if so, we cant castle kingside
			if(FoundMove)
			{
				ArrayRemove(LegalMoves, i);
				continue;
			}
		}

		MakeMove(App, GameState, &Move);
		// unmakeMove Unmake = MakeMove(GameState, &Move);
		if(IsActivePlayerInCheck(GameState))
		{
			// printf("check!\n");
			ArrayRemove(LegalMoves, i);
		}
		else
		{
			// printf("ok\n");
			i += 1;
		}
		// UnmakeMove(GameState, &Move, Unmake);
		UnmakeLastMove(App);
	}
}

#if 0
bool IsCheckmated(image *Board[], color Color)
{
	bool IsCheckmated = false;
	// determine if 'Color' is in check
	if (IsInCheck(Board, Color))
	{
		// if so, get all its possible moves
		IsCheckmated = true;

		array<move *> PossibleMoves; ArrayInit(&PossibleMoves);
	
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
#endif

gameOver IsGameOver(app *App, gameState *GameState)
{
	gameOver GameResult = gameOver_NOT_OVER;

	array<move> LegalMoves; ArrayInit(&LegalMoves);
	for (int Y = 0; Y < 8; ++Y)
	{
		for (int X = 0; X < 8; ++X)
		{
			if (GameState->Board[Y][X] && GetPieceColor(GameState->Board[Y][X]) == GameState->ActiveColor)
			{
				GetLegalMovesForPiece(App, GameState, X, Y, &LegalMoves);
			}
		}
	}

	if(LegalMoves.Count == 0)
	{
		if(IsActivePlayerInCheck(GameState))
		{
			GameResult = gameOver_CHECKMATE;
		}
		else
		{
			GameResult = gameOver_STALEMATE;
		}
	}

	return GameResult;
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
	// SetUpBoard((image **)GameState->Board, "pp1p1p2/4P3/8/8/8/8/8/6PP");
	SetUpBoard((image **)GameState->Board, "r1b1kb1r/8/8/8/8/8/8/R1B1KB1R");
	// SetUpBoard((image **)GameState->Board, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");

	GameState->ActiveColor = WHITE;

	GameState->CastlingRights[WHITE].Kingside = true;
	GameState->CastlingRights[WHITE].Queenside = true;
	GameState->CastlingRights[BLACK].Kingside = true;
	GameState->CastlingRights[BLACK].Queenside = true;

	GameState->EnPassantSquare.Exists = false;

	App->PreviousStateExists = false;

	// // white
	// player Player1 = {playerType_COMPUTER, true}; App->Players[0] = Player1;
	// // black
	// player Player2 = {playerType_COMPUTER, false}; App->Players[1] = Player2;

	// white
	player Player1 = {playerType_HUMAN, true}; App->Players[0] = Player1;
	// black
	player Player2 = {playerType_HUMAN, false}; App->Players[1] = Player2;

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
				if(App->DisplayPromotionDialog)
				{
					promotionDialog *Dialog = &App->PromotionDialog;

					int LeftEdge = Dialog->X;
					int RightEdge = Dialog->X + Dialog->Width;
					int TopEdge = Dialog->Y;
					int BottomEdge = Dialog->Y + Dialog->Height;
					if(Event.X > LeftEdge && Event.X < RightEdge && Event.Y > TopEdge && Event.Y < BottomEdge)
					{
						int DistanceIntoDialog = Event.Y - TopEdge;
						Dialog->PieceIndex = (promotionDialogPiece) (DistanceIntoDialog / App->TileHeight);
					}
					else
					{
						Dialog->PieceIndex = promotionDialogPiece_NO_PIECE;
					}

					break;
				}

				// printf("LBUTTONDOWN: X: %d, Y: %d\n", Event.X, Event.Y);
				int DownX = Event.X / App->TileWidth;
				int DownY = Event.Y / App->TileHeight;
				if (GameState->Board[DownY][DownX])
				{
					App->IsDraggedPiece = true;
					App->DraggedPiece.X = DownX;
					App->DraggedPiece.Y = DownY;

					// highlight available squares for dragged piece
					if(GetPieceColor(GameState->Board[DownY][DownX]) == GameState->ActiveColor)
					{
						GetLegalMovesForPiece(App, GameState, DownX, DownY, &App->DraggedPiece.AvailableMoves);
					}
				}
			}
			break;
			case LBUTTONUP:
			{
				// printf("LBUTTONUP X: %d, Y: %d\n", Event.X, Event.Y);

				if(App->DisplayPromotionDialog)
				{
					promotionDialogPiece PieceIndex = promotionDialogPiece_NO_PIECE;

					promotionDialog *Dialog = &App->PromotionDialog;

					int LeftEdge = Dialog->X;
					int RightEdge = Dialog->X + Dialog->Width;
					int TopEdge = Dialog->Y;
					int BottomEdge = Dialog->Y + Dialog->Height;
					if(Event.X > LeftEdge && Event.X < RightEdge && Event.Y > TopEdge && Event.Y < BottomEdge)
					{
						int DistanceIntoDialog = Event.Y - Dialog->Y;
						PieceIndex = (promotionDialogPiece) (DistanceIntoDialog / App->TileHeight);
					}

					if(PieceIndex == Dialog->PieceIndex)
					{
						if(PieceIndex != promotionDialogPiece_NO_PIECE)
						{
							// user selected a piece
							image *PromoteTo = Dialog->SelectablePieces[GameState->ActiveColor][PieceIndex];
							Dialog->MoveToMake.PawnPromotion.PromoteTo = PromoteTo;
							MakeMove(App, GameState, &App->PromotionDialog.MoveToMake);

							SwitchTurn(GameState);
						}

						App->DisplayPromotionDialog = false;
					}
					else
					{
						// nothing should happen
					}

					break;
				}

				if (App->IsDraggedPiece)
				{
					App->IsDraggedPiece = false;
					App->DraggedPiece.AvailableMoves.Count = 0;

					if(Event.X < 0 || Event.Y < 0 || Event.X > App->BoardWidth || Event.Y > App->BoardHeight)
					{
						printf("piece outside the board\n");
						// App->IsDraggedPiece = false;
						break;
					}

					int UpX = Event.X / App->TileWidth;
					int UpY = Event.Y / App->TileHeight;

					// // Human attempted to make a move, but the move should be made by a computer
					// if(App->Players[GameState->ActiveColor].PlayerType == playerType_COMPUTER)
					// {
					// 	printf("Next move should be made by a computer!\n");
					// 	App->IsDraggedPiece = false;
					// 	break;
					// }

					/*
						moveResult MoveMade = MakeMove(GameState, AttemptedMove);
						switch(MoveMade.Status)
						{
							case moveStatus_UNMADE:
							{
								unmadeMove *Unmade = (unmadeMove *)&MoveInfo;
								if(Unmade->WouldBeCheck)
								{
									// move would put/leave us in check
								}
								else
								{
									// illegal for other reasons
								}
							}
							break;
							case moveStatus_MADE:
							{
								madeMove *Made = (madeMove *)&MoveInfo;
								if(Made->GameOver)
								{
									switch(GameOver)
									{
										case gameOver_CHECKMATE:
										case gameOver_STALEMATE:
										case gameOver_INSUFFICIENT_MATERIAL:
										case gameOver_THREEFOLD_REPETITION:
										case gameOver_FIFTY_MOVE_RULE:
									}
								}
								else
								{
									// play sounds
									Made->IsCapture, Made->IsCheck
								}
							}
							break;
							case moveStatus_INCOMPLETE:
							{
								// pawn promotion
								incompleteMove *PromotionMove = (incompleteMove *)&MoveInfo;

								CompletePromotion(pawnPromotion *Promotion, image *PromoteTo);
								CancelPromotion(pawnPromotion *Promotion)

								MakeMoveWithPromotion(gameState *GameState, move Move, image *PromoteTo)
							}
							break;
						}
					*/

					image *DraggedPiece = GameState->Board[App->DraggedPiece.Y][App->DraggedPiece.X];
					color DraggedPieceColor = GetPieceColor(DraggedPiece);

					// Player moved a piece, but its not their turn (it was of wrong color)
					if(DraggedPieceColor != GameState->ActiveColor)
					{
						const char *Color = (GameState->ActiveColor == WHITE) ? "WHITE" : "BLACK";
						printf("%s's move!\n", Color);

						break;
					}

					/*
					struct userMove
					{
						int FromX, FromY, ToX, ToY;
					};
					userMove GetUserMove(move *LegalMove)
					?
					*/
					move AttemptedMove;
					if
					(
						(DraggedPiece == &WKingImage
						&& App->DraggedPiece.X == 4 && App->DraggedPiece.Y == 7
						&& UpX == 6 && UpY == 7)
						||
						(DraggedPiece == &BKingImage
						&& App->DraggedPiece.X == 4 && App->DraggedPiece.Y == 0
						&& UpX == 6 && UpY == 0)
					)
					{
						Make_kingsideCastlingMove(&AttemptedMove, DraggedPieceColor);
					}
					else if((DraggedPiece == &WKingImage && App->DraggedPiece.X == 4 && App->DraggedPiece.Y == 7 && UpX == 2 && UpY == 7) || (DraggedPiece == &BKingImage && App->DraggedPiece.X == 4 && App->DraggedPiece.Y == 0 && UpX == 2 && UpY == 0))
					{
						Make_queensideCastlingMove(&AttemptedMove, DraggedPieceColor);
					}
					else if((DraggedPiece == &WPawnImage && UpY == 0) || (DraggedPiece == &BPawnImage && UpY == 7))
					{
						Make_pawnPromotionMove(&AttemptedMove, DraggedPieceColor, App->DraggedPiece.X, UpX);
					}
					else if(GameState->EnPassantSquare.Exists && (DraggedPiece == &WPawnImage
							&& UpX == GameState->EnPassantSquare.X
							&& UpY == GameState->EnPassantSquare.Y
							&& ((App->DraggedPiece.X == GameState->EnPassantSquare.X + 1) || (App->DraggedPiece.X == GameState->EnPassantSquare.X - 1))
							&& App->DraggedPiece.Y == GameState->EnPassantSquare.Y + 1))
					{
						Make_enPassantMove(&AttemptedMove, DraggedPieceColor, App->DraggedPiece.X);
					}
					else if(GameState->EnPassantSquare.Exists && (DraggedPiece == &BPawnImage
							&& UpX == GameState->EnPassantSquare.X
							&& UpY == GameState->EnPassantSquare.Y
							&& ((App->DraggedPiece.X == GameState->EnPassantSquare.X + 1) || (App->DraggedPiece.X == GameState->EnPassantSquare.X - 1))
							&& App->DraggedPiece.Y == GameState->EnPassantSquare.Y - 1))
					{
						Make_enPassantMove(&AttemptedMove, DraggedPieceColor, App->DraggedPiece.X);
					}
					else
					{
						Make_regularMove(&AttemptedMove, DraggedPieceColor, App->DraggedPiece.X, App->DraggedPiece.Y, UpX, UpY);
					}

					// What are the possible moves?
					array<move> LegalMoves; ArrayInit(&LegalMoves);
					//@ This doesnt seem to be the most efficient and useful function to use here.
					// efficient: we dont really need all possible moves for a piece
					// useful: we also want to know if the move would leave us in check
					GetLegalMovesForPiece(App, GameState, App->DraggedPiece.X, App->DraggedPiece.Y, &LegalMoves);
					// if(!IsLegalMove(GameState, &AttemptedMove)){}else{}

					// Is our move a possible move?
					bool IsFoundMove = false; move FoundMove;
					for(int i = 0; i < LegalMoves.Count; ++i)
					{
						move *LegalMove = &LegalMoves.Data[i];
						// regularMove *Regular = (regularMove *)&PossibleMove->Regular;

						switch(LegalMove->Type)
						{
							case moveType_REGULAR:
							{
								if(AttemptedMove.Type == moveType_REGULAR && AttemptedMove.Regular.ToX == LegalMove->Regular.ToX && AttemptedMove.Regular.ToY == LegalMove->Regular.ToY)
								{
									// equal
									assert(AttemptedMove.Regular.FromX == LegalMove->Regular.FromX);
									assert(AttemptedMove.Regular.FromY == LegalMove->Regular.FromY);

									FoundMove = *LegalMove;
									IsFoundMove = true;
									goto SearchDone;
								}
							}
							break;
							case moveType_KINGSIDE_CASTLING:
							{
								if(AttemptedMove.Type == moveType_KINGSIDE_CASTLING)
								{
									// equal
									assert(AttemptedMove.Side == LegalMove->Side);

									FoundMove = *LegalMove;
									IsFoundMove = true;
									goto SearchDone;
								}
							}
							break;
							case moveType_QUEENSIDE_CASTLING:
							{
								if(AttemptedMove.Type == moveType_QUEENSIDE_CASTLING)
								{
									// equal
									assert(AttemptedMove.Side == LegalMove->Side);

									FoundMove = *LegalMove;
									IsFoundMove = true;
									goto SearchDone;
								}
							}
							break;
							case moveType_PAWN_PROMOTION:
							{
								if(AttemptedMove.Type == moveType_PAWN_PROMOTION && AttemptedMove.PawnPromotion.ToX == LegalMove->PawnPromotion.ToX)
								{
									// equal
									assert(AttemptedMove.Side == LegalMove->Side);
									assert(AttemptedMove.PawnPromotion.FromX == LegalMove->PawnPromotion.FromX);

									FoundMove = *LegalMove;
									IsFoundMove = true;
									goto SearchDone;
								}
							}
							break;
							case moveType_EN_PASSANT:
							{
								if(AttemptedMove.Type == moveType_EN_PASSANT && AttemptedMove.EnPassant.FromX == LegalMove->EnPassant.FromX)
								{
									// equal
									assert(AttemptedMove.Side == LegalMove->Side);

									FoundMove = *LegalMove;
									IsFoundMove = true;
									goto SearchDone;
								}
							}
							break;

							default: assert(false);
						}
					}
					SearchDone:
					ArrayFree(&LegalMoves);
					if (!IsFoundMove)
					{
						printf("not a legal move!\n");

						break;
					}

					if(FoundMove.Type == moveType_PAWN_PROMOTION)
					{
						App->DisplayPromotionDialog = true;

						promotionDialog *Dialog = &App->PromotionDialog;
						if(GameState->ActiveColor == WHITE)
						{
							Dialog->X = FoundMove.PawnPromotion.ToX * App->TileWidth;
							// Dialog->Y = AttemptedMove.ToY * App->TileHeight;
							Dialog->Y = 0;
						}
						else
						{
							assert(GameState->ActiveColor == BLACK);
							Dialog->X = FoundMove.PawnPromotion.ToX * App->TileWidth;
							Dialog->Y = (8 - 4) * App->TileHeight;
							// Dialog->Y = (AttemptedMove.ToY - 3) * App->TileHeight;
						}
						Dialog->Width = App->TileWidth;
						Dialog->Height = App->TileHeight * 4;
						// Dialog->Color = GameState->ActiveColor;
						Dialog->MoveToMake = FoundMove;

						break;
					}

					// // bool PossibleMovesForPiece[8][8];
					// moveBits LegalMoves[8][8];
					// GetLegalMovesForPiece(GameState, AttemptedMove.FromX, AttemptedMove.FromY, LegalMoves);
					// moveBits MoveInfo = LegalMoves[AttemptedMove.ToY][AttemptedMove.ToX];
					// if(!(MoveInfo & moveBits_LEGAL_MOVE))
					// {
					// 	printf("illegal move!\n");

					// 	if(MoveInfo & moveBits_WOULD_BE_CHECK)
					// 	{
					// 		// play sound, animation?
					// 	}

					// 	App->IsDraggedPiece = false;
					// 	break;
					// }
					// printf("legal move\n");

					MakeMove(App, GameState, &FoundMove);
					image *CapturedPiece = NULL;
					// UnmakeLastMove(App);
					// // UnmakeLastMove(App);
					// MakeMove(App, GameState, MoveWeCanMake);

					// if (IsInCheck((image **)GameState->Board, GameState->ActiveColor))
					// {
					// 	// The user attempted a move that leaves their king in check.
					// 	// Play some sound / animation maybe.

					// 	UnmakeMove(GameState, &AttemptedMove, PossibleCapturedPiece);

					// 	const char *Color = (GameState->ActiveColor == WHITE) ? "WHITE" : "BLACK";
					// 	printf("this move would leave %s in check!\n", Color);

					// 	App->IsDraggedPiece = false;
					// 	break;
					// }

					SwitchTurn(GameState);

					bool Check = false;
					if(IsActivePlayerInCheck(GameState))
					{
						printf("CHECK!\n");
						Check = true;
					}

					gameOver GameOver = IsGameOver(App, GameState);
					if(GameOver)
					{
						if(GameOver == gameOver_CHECKMATE)
						{
							printf("game over: checkmate\n");
						}
						if(GameOver == gameOver_STALEMATE)
						{
							printf("game over: stalemate\n");
						}
						// if(GameOver == gameOver_INSUFFICIENT_MATERIAL) {}
						// if(GameOver == gameOver_FIFTYTH_MOVE_RULE) {}
						// if(GameOver == gameOver_THREEFOLD_REPETITION_RULE) {} // number of possible board configurations is larger than possible bit configurations in 64-bits.

						App->Mode = appMode_POSTGAME;
						break;
					}

					// // The opponent could be a computer or a human. If human, then this will be ignored:
					// App->Players[GameState->ActiveColor].ComputerCalculateMove = true;

					// // check if it was a check / checkmate
					// bool Check = false;
					// if (IsInCheck((image **)GameState->Board, GameState->ActiveColor))
					// {
					// 	printf("%s is in check!\n", (GameState->ActiveColor == WHITE) ? "WHITE" : "BLACK");
					// 	Check = true;
					// }
					// printf("%s's move...\n", (GameState->ActiveColor == WHITE) ? "WHITE" : "BLACK");

					// Play sound
					playingSound *PlayingSound = (playingSound *) malloc(sizeof(playingSound));
					PlayingSound->Sound = Check ? &CheckSound : (CapturedPiece ? &CaptureSound : &MoveSound);
					PlayingSound->NumSamplesPlayed = 0;
					PlayingSound->IsLooping = false;
					PlayingSounds.append(PlayingSound);

					// if (IsCheckmated((image **)GameState->Board, GameState->ActiveColor))
					// {
					// 	printf("%s is checkmated!\n", (GameState->ActiveColor == WHITE) ? "WHITE" : "BLACK");
					// 	App->Mode = appMode_POSTGAME;
					// 	break;
					// }
				}
				else
				{
					printf("MOUSE BUTTON UP, no dragged piece\n");
				}
			}
			break;
			default:
			{
				// assert(false);
			}
		}
	}

#if 0
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

				unmakeMove Unmake = MakeMove(GameState, PlausableMove);
				if (IsInCheck((image **)GameState->Board, GameState->ActiveColor))
				{
					UnmakeMove(GameState, PlausableMove, Unmake);
					
					continue;
				}

				UnmakeMove(GameState, PlausableMove, Unmake);
				
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
#endif
	// Render background
	u32 Light = 0x00efd9b7;
	u32 Dark = 0x00b58965;
	// u32 Light = 0x00ffffff;
	// u32 Dark = 0x00000000;
	// u32 TileWidth = WindowBuffer->Width / 8;
	// u32 TileHeight = WindowBuffer->Height / 8;
	uint32_t TileWidth = App->TileWidth;
	uint32_t TileHeight = App->TileHeight;
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
			if (GameState->Board[Y][X] && !(App->IsDraggedPiece && App->DraggedPiece.X == X && App->DraggedPiece.Y == Y))
			{
				// RenderImage(WindowBuffer, GameState->Board[Y][X], X * 60, Y * 60);
				RenderImage(WindowBuffer, GameState->Board[Y][X], X * TileWidth, Y * TileHeight);
			}
		}
	}

	if(App->DisplayPromotionDialog)
	{
		promotionDialog *Dialog = &App->PromotionDialog;

		RenderRectangle(WindowBuffer, Dialog->X, Dialog->Y, Dialog->Width, Dialog->Height, 0xffffffff);
		// RenderRectangle(WindowBuffer, App->PromotionDialogX, App->PromotionDialogY, App->TileWidth, 4 * App->TileHeight, 0xff000000);
		if(GameState->ActiveColor == WHITE)
		{
			RenderImage(WindowBuffer, &WQueenImage, Dialog->X, Dialog->Y);
			RenderImage(WindowBuffer, &WKnightImage, Dialog->X, Dialog->Y + TileHeight);
			RenderImage(WindowBuffer, &WBishopImage, Dialog->X, Dialog->Y + 2 * TileHeight);
			RenderImage(WindowBuffer, &WRookImage, Dialog->X, Dialog->Y + 3 * TileHeight);
		}
		else
		{
			RenderImage(WindowBuffer, &BQueenImage, Dialog->X, Dialog->Y);
			RenderImage(WindowBuffer, &BKnightImage, Dialog->X, Dialog->Y + TileHeight);
			RenderImage(WindowBuffer, &BBishopImage, Dialog->X, Dialog->Y + 2 * TileHeight);
			RenderImage(WindowBuffer, &BRookImage, Dialog->X, Dialog->Y + 3 * TileHeight);
		}
	}

	// highlight available squares for dragged piece
	for (int I = 0; I < App->DraggedPiece.AvailableMoves.Count; ++I)
	{
		move *Move = &App->DraggedPiece.AvailableMoves.Data[I];

		int MoveX, MoveY;
		uint32_t Color = 0x00ff0000;
		switch(Move->Type)
		{
			case moveType_REGULAR:
			{
				MoveX = Move->Regular.ToX;
				MoveY = Move->Regular.ToY;
			}
			break;
			case moveType_KINGSIDE_CASTLING:
			{
				if(Move->Side == WHITE)
				{
					MoveX = 6;
					MoveY = 7;
				}
				else
				{
					assert(Move->Side == BLACK);
					MoveX = 6;
					MoveY = 0;
				}
			}
			break;
			case moveType_QUEENSIDE_CASTLING:
			{
				if(Move->Side == WHITE)
				{
					MoveX = 2;
					MoveY = 7;
				}
				else
				{
					assert(Move->Side == BLACK);
					MoveX = 2;
					MoveY = 0;
				}
			}
			break;
			case moveType_PAWN_PROMOTION:
			{
				if(Move->Side == WHITE)
				{
					MoveX = Move->PawnPromotion.ToX;
					MoveY = 0;
				}
				else
				{
					assert(Move->Side == BLACK);
					MoveX = Move->PawnPromotion.ToX;
					MoveY = 7;
				}
				Color = 0x0000ff00;
			}
			break;
			case moveType_EN_PASSANT:
			{
				MoveX = GameState->EnPassantSquare.X;
				MoveY = GameState->EnPassantSquare.Y;
			}
			break;

			default: assert(false);
		}
		int SquareWidth = 20;
		int SquareHeight = 20;
		int X = MoveX * TileWidth + TileWidth / 2 - SquareWidth / 2;
		int Y = MoveY * TileHeight + TileHeight / 2 - SquareHeight / 2;
		RenderRectangle(WindowBuffer, X, Y, SquareWidth, SquareHeight, Color);
	}

	if(App->IsGameDebugMode)
	{
		int SquareWidth = 10;
		int SquareHeight = 10;
		uint32_t Color = 0x0000ff00;
		int X, Y;
		if(GameState->EnPassantSquare.Exists)
		{
			X = GameState->EnPassantSquare.X * TileWidth + TileWidth / 2 - SquareWidth / 2;
			Y = GameState->EnPassantSquare.Y * TileHeight + TileHeight / 2 - SquareHeight / 2;
			RenderRectangle(WindowBuffer, X, Y, SquareWidth, SquareHeight, 0x00330033);
		}
		if(GameState->CastlingRights[WHITE].Kingside)
		{
			X = 6 * TileWidth + TileWidth / 2 - SquareWidth / 2;
			Y = 7 * TileHeight + TileHeight / 2 - SquareHeight / 2;
			RenderRectangle(WindowBuffer, X, Y, SquareWidth, SquareHeight, Color);
		}
		if(GameState->CastlingRights[WHITE].Queenside)
		{
			X = 2 * TileWidth + TileWidth / 2 - SquareWidth / 2;
			Y = 7 * TileHeight + TileHeight / 2 - SquareHeight / 2;
			RenderRectangle(WindowBuffer, X, Y, SquareWidth, SquareHeight, Color);
		}
		if(GameState->CastlingRights[BLACK].Kingside)
		{
			X = 6 * TileWidth + TileWidth / 2 - SquareWidth / 2;
			Y = 0 * TileHeight + TileHeight / 2 - SquareHeight / 2;
			RenderRectangle(WindowBuffer, X, Y, SquareWidth, SquareHeight, Color);
		}
		if(GameState->CastlingRights[BLACK].Queenside)
		{
			X = 2 * TileWidth + TileWidth / 2 - SquareWidth / 2;
			Y = 0 * TileHeight + TileHeight / 2 - SquareHeight / 2;
			RenderRectangle(WindowBuffer, X, Y, SquareWidth, SquareHeight, Color);
		}
	}

	i32 MouseX = Input->LastMouseX;
	i32 MouseY = Input->LastMouseY;
//	printf("MouseX: %d, MouseY: %d\n", MouseX, MouseY);
	// If we have a dragged piece, render it
	if (App->IsDraggedPiece)
	{
		assert(GameState->Board[App->DraggedPiece.Y][App->DraggedPiece.X]);
		RenderImage(WindowBuffer, GameState->Board[App->DraggedPiece.Y][App->DraggedPiece.X], MouseX - 30, MouseY - 30);
	}
#if 0
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
			unmakeMove Unmake = MakeMove(GameState, &A->Move);
			image *PossibleCapturedPiece = Unmake.CapturedPiece;

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
#endif
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
	App.BoardWidth = App.TileWidth * 8;
	App.BoardHeight = App.TileHeight * 8;

	App.DisplayPromotionDialog = false;

	App.PromotionDialog.SelectablePieces[WHITE][0] = &WQueenImage;
	App.PromotionDialog.SelectablePieces[WHITE][1] = &WKnightImage;
	App.PromotionDialog.SelectablePieces[WHITE][2] = &WBishopImage;
	App.PromotionDialog.SelectablePieces[WHITE][3] = &WRookImage;
	App.PromotionDialog.SelectablePieces[BLACK][0] = &BQueenImage;
	App.PromotionDialog.SelectablePieces[BLACK][1] = &BKnightImage;
	App.PromotionDialog.SelectablePieces[BLACK][2] = &BBishopImage;
	App.PromotionDialog.SelectablePieces[BLACK][3] = &BRookImage;

	ArrayInit(&App.DraggedPiece.AvailableMoves);

	App.IsGameDebugMode = true;

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