#include "move.h"
#include "eval.h"
#include <iostream>
#include <algorithm>

extern U64 PASSED_PAWN_MASKS[2][64];

int MoveUtils::setCapturedPieceType(PieceType pieceType, int _move) {
  unsigned int mask = 0x7 << 6;
  return (_move & ~mask) | ((pieceType << 6) & mask);
}

int MoveUtils::setPromotionPieceType(PieceType pieceType, int _move) {
  unsigned int mask = 0x7 << 3;
  return (_move & ~mask) | ((pieceType << 3) & mask);
}

int MoveUtils::setFlag(Flag flag, int _move) {
  return _move | (flag << 21);
}

std::string MoveUtils::getNotation(int _move) {
  // Special case for null moves
  if (getTypeFlag(_move) & NULL_MOVE) {
    return NULL_MOVE_NOTATION;
  }

  int fromIndex = getSQVFrom(_move);
  int toIndex = getSQVTo(_move);

  std::string moveNotation = indexToNotation(fromIndex) + indexToNotation(toIndex);

  if (getTypeFlag(_move) & PROMOTION) {
    switch (getPromoteTo(_move)) {
      case QUEEN: moveNotation += 'q';
        break;
      case ROOK: moveNotation += 'r';
        break;
      case KNIGHT: moveNotation += 'n';
        break;
      case BISHOP: moveNotation += 'b';
        break;
      default:break;
    }
  }

  return moveNotation;
}

std::string MoveUtils::indexToNotation(int index) {
  return std::string({FILES[index % 8], RANKS[index / 8]});
}

unsigned int MoveUtils::notationToIndex(std::string notation) {
  int file = std::find(FILES, FILES + 8, notation[0]) - std::begin(FILES);
  int rank = std::find(RANKS, RANKS + 8, notation[1]) - std::begin(RANKS);

  return rank * 8 + file;
}
