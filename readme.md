Fork or clone your this chess project into a new GitHub repository.

Add support for FEN stringsLinks to an external site. to your game setup so that instead of the current way you are setting up your game board you are setting it up with a call similar to the following call.

FENtoBoard("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");

Your routine should be able to take just the board position portion of a FEN string, or the entire FEN string like so:

FENtoBoard("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

(you can ignore the end for now)

This will allow you to quickly check that your castling, promotion and en passant code is working.

## Peter Dobbins - Changes

Implemented `FENtoBoard()` in `Chess.cpp` to parse FEN strings and place pieces on the board. The function extracts the piece placement field (everything before the first space), then steps through the string character by character: `/` moves to the next rank, digits skip that many files, and letters create the corresponding piece. Uppercase letters are white pieces, lowercase are black.

Also fixed `PieceForPlayer()` to set each piece's `gameTag` so that piece color and type are properly encoded (white = 1-6, black = 129-134). This is required for turn-based color validation and the existing `pieceNotation()` function to work correctly.

### Chess Movement

Implemented move validation for pawns, knights, and kings in `canBitMoveFromTo()`. It identifies the piece type from its `gameTag`, rejects moves onto friendly pieces, then dispatches to piece-specific logic:

- **Pawns**: Can move forward one square, forward two squares from their starting rank (rank 2 for white, rank 7 for black), and capture diagonally. Forward moves are blocked if the destination or intermediate square is occupied. Diagonal moves are only allowed when an enemy piece is present.
- **Knights**: Uses the provided `BitboardElement` class to generate an L-shaped move mask from the source square, then checks if the destination bit is set. Knights can jump over other pieces.
- **King**: Uses `BitboardElement` to generate a mask of all 8 adjacent squares, then checks if the destination bit is set.
