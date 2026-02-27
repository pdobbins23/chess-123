#include "Chess.h"
#include <cmath>
#include <limits>

Chess::Chess() { _grid = new Grid(8, 8); }

Chess::~Chess() { delete _grid; }

char Chess::pieceNotation(int x, int y) const {
  const char *wpieces = {"0PNBRQK"};
  const char *bpieces = {"0pnbrqk"};
  Bit *bit = _grid->getSquare(x, y)->bit();
  char notation = '0';
  if (bit) {
    notation = bit->gameTag() < 128 ? wpieces[bit->gameTag()]
                                    : bpieces[bit->gameTag() - 128];
  }
  return notation;
}

Bit *Chess::PieceForPlayer(const int playerNumber, ChessPiece piece) {
  const char *pieces[] = {"pawn.png", "knight.png", "bishop.png",
                          "rook.png", "queen.png",  "king.png"};

  Bit *bit = new Bit();
  const char *pieceName = pieces[piece - 1];
  std::string spritePath =
      std::string("") + (playerNumber == 0 ? "w_" : "b_") + pieceName;
  bit->LoadTextureFromFile(spritePath.c_str());
  bit->setOwner(getPlayerAt(playerNumber));
  bit->setSize(pieceSize, pieceSize);
  bit->setGameTag(piece + playerNumber * 128);

  return bit;
}

void Chess::setUpBoard() {
  setNumberOfPlayers(2);
  _gameOptions.rowX = 8;
  _gameOptions.rowY = 8;

  _grid->initializeChessSquares(pieceSize, "boardsquare.png");
  FENtoBoard("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");

  startGame();
}

void Chess::FENtoBoard(const std::string &fen) {
  std::string placement = fen.substr(0, fen.find(' '));

  int row = 7;
  int col = 0;

  for (char c : placement) {
    if (c == '/') {
      row--;
      col = 0;
    } else if (c >= '1' && c <= '8') {
      col += c - '0';
    } else {
      int player = islower(c) ? 1 : 0;
      ChessPiece piece = NoPiece;

      switch (tolower(c)) {
      case 'p':
        piece = Pawn;
        break;
      case 'n':
        piece = Knight;
        break;
      case 'b':
        piece = Bishop;
        break;
      case 'r':
        piece = Rook;
        break;
      case 'q':
        piece = Queen;
        break;
      case 'k':
        piece = King;
        break;
      }

      if (piece != NoPiece) {
        Bit *bit = PieceForPlayer(player, piece);
        bit->setPosition(_grid->getSquare(col, row)->getPosition());
        _grid->getSquare(col, row)->setBit(bit);
      }
      col++;
    }
  }
}

bool Chess::actionForEmptyHolder(BitHolder &holder) { return false; }

bool Chess::canBitMoveFrom(Bit &bit, BitHolder &src) {
  // need to implement friendly/unfriendly in bit so for now this hack
  int currentPlayer = getCurrentPlayer()->playerNumber() * 128;
  int pieceColor = bit.gameTag() & 128;
  if (pieceColor == currentPlayer)
    return true;
  return false;
}

bool Chess::canBitMoveFromTo(Bit &bit, BitHolder &src, BitHolder &dst) {
  ChessSquare &srcSquare = static_cast<ChessSquare &>(src);
  ChessSquare &dstSquare = static_cast<ChessSquare &>(dst);

  int pieceType = bit.gameTag() & 127;
  int player = (bit.gameTag() & 128) ? 1 : 0;

  Bit *dstBit = dstSquare.bit();
  if (dstBit) {
    int dstPlayer = (dstBit->gameTag() & 128) ? 1 : 0;
    if (dstPlayer == player)
      return false;
  }

  switch (pieceType) {
  case Pawn:
    return canPawnMove(srcSquare, dstSquare, player);
  case Knight:
    return canKnightMove(srcSquare, dstSquare);
  case King:
    return canKingMove(srcSquare, dstSquare);
  default:
    return false;
  }
}

bool Chess::canPawnMove(ChessSquare &src, ChessSquare &dst, int player) {
  int srcCol = src.getColumn();
  int srcRow = src.getRow();
  int dstCol = dst.getColumn();
  int dstRow = dst.getRow();

  int direction = (player == 0) ? 1 : -1;
  int startRank = (player == 0) ? 1 : 6;

  if (dstCol == srcCol && dst.bit() == nullptr) {
    if (dstRow == srcRow + direction)
      return true;
    if (srcRow == startRank && dstRow == srcRow + 2 * direction) {
      ChessSquare *between = _grid->getSquare(srcCol, srcRow + direction);
      if (between && between->bit() == nullptr)
        return true;
    }
  }

  if (abs(dstCol - srcCol) == 1 && dstRow == srcRow + direction) {
    if (dst.bit() != nullptr)
      return true;
  }

  return false;
}

BitboardElement Chess::knightMoves(int square) {
  BitboardElement board;
  int col = square % 8;
  int row = square / 8;

  const int offsets[8][2] = {
      {-2, -1}, {-2, 1}, {-1, -2}, {-1, 2},
      {1, -2},  {1, 2},  {2, -1},  {2, 1}};

  for (auto &offset : offsets) {
    int newCol = col + offset[0];
    int newRow = row + offset[1];
    if (newCol >= 0 && newCol < 8 && newRow >= 0 && newRow < 8)
      board |= (1ULL << (newRow * 8 + newCol));
  }

  return board;
}

bool Chess::canKnightMove(ChessSquare &src, ChessSquare &dst) {
  int srcIndex = src.getSquareIndex();
  int dstIndex = dst.getSquareIndex();

  BitboardElement moves = knightMoves(srcIndex);
  return moves.getData() & (1ULL << dstIndex);
}

BitboardElement Chess::kingMoves(int square) {
  BitboardElement board;
  int col = square % 8;
  int row = square / 8;

  for (int dr = -1; dr <= 1; dr++) {
    for (int dc = -1; dc <= 1; dc++) {
      if (dr == 0 && dc == 0)
        continue;
      int newCol = col + dc;
      int newRow = row + dr;
      if (newCol >= 0 && newCol < 8 && newRow >= 0 && newRow < 8)
        board |= (1ULL << (newRow * 8 + newCol));
    }
  }

  return board;
}

bool Chess::canKingMove(ChessSquare &src, ChessSquare &dst) {
  int srcIndex = src.getSquareIndex();
  int dstIndex = dst.getSquareIndex();

  BitboardElement moves = kingMoves(srcIndex);
  return moves.getData() & (1ULL << dstIndex);
}

void Chess::stopGame() {
  _grid->forEachSquare(
      [](ChessSquare *square, int x, int y) { square->destroyBit(); });
}

Player *Chess::ownerAt(int x, int y) const {
  if (x < 0 || x >= 8 || y < 0 || y >= 8) {
    return nullptr;
  }

  auto square = _grid->getSquare(x, y);
  if (!square || !square->bit()) {
    return nullptr;
  }
  return square->bit()->getOwner();
}

Player *Chess::checkForWinner() { return nullptr; }

bool Chess::checkForDraw() { return false; }

std::string Chess::initialStateString() { return stateString(); }

std::string Chess::stateString() {
  std::string s;
  s.reserve(64);
  _grid->forEachSquare(
      [&](ChessSquare *square, int x, int y) { s += pieceNotation(x, y); });
  return s;
}

void Chess::setStateString(const std::string &s) {
  _grid->forEachSquare([&](ChessSquare *square, int x, int y) {
    int index = y * 8 + x;
    char playerNumber = s[index] - '0';
    if (playerNumber) {
      square->setBit(PieceForPlayer(playerNumber - 1, Pawn));
    } else {
      square->setBit(nullptr);
    }
  });
}
