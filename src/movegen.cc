#include "movegen.h"
#include "movepicker.h"
#include "eval.h"


MoveGen::MoveGen(const Board &board, bool isCaptureGenerated) {
  setBoard(board, isCaptureGenerated);
}

MoveGen::MoveGen() {
  setBoard(Board(), false);
}

void MoveGen::setBoard(const Board &board, bool isCaptureGenerated) {
  _moves = MoveList();
  _legalMoves = MoveList();
  if (!isCaptureGenerated){
     _genMoves(board); 
  } else{
    _genCaptures(board);
  }
}


MoveList MoveGen::getMoves() {
  return _moves;
}

MoveList MoveGen::getLegalMoves() {
  return _legalMoves;
}

void MoveGen::_genMoves(const Board &board) {
  _moves.reserve(MOVELIST_RESERVE_SIZE);
  switch (board.getActivePlayer()) {
    case WHITE: _genWhiteMoves(board);
      break;
    case BLACK: _genBlackMoves(board);
      break;
  }
}

void MoveGen::_genCaptures(const Board &board) {
  _moves.reserve(MOVELIST_RESERVE_SIZE);
  switch (board.getActivePlayer()) {
    case WHITE: _genWhiteCaps(board);
      break;
    case BLACK: _genBlackCaps(board);
      break;
  }
}

void MoveGen::_genWhiteMoves(const Board &board) {
  _genWhitePawnMoves(board);
  _genWhiteRookMoves(board);
  _genWhiteKnightMoves(board);
  _genWhiteBishopMoves(board);
  _genWhiteKingMoves(board);
  _genWhiteQueenMoves(board);
}

void MoveGen::_genWhiteCaps(const Board &board) {
  _genWhitePawnCaps(board);
  _genWhiteRookCaps(board);
  _genWhiteKnightCaps(board);
  _genWhiteBishopCaps(board);
  _genWhiteKingCaps(board);
  _genWhiteQueenCaps(board);
}

void MoveGen::_genBlackMoves(const Board &board) {
  _genBlackPawnMoves(board);
  _genBlackRookMoves(board);
  _genBlackKnightMoves(board);
  _genBlackBishopMoves(board);
  _genBlackKingMoves(board);
  _genBlackQueenMoves(board);
}

void MoveGen::_genBlackCaps(const Board &board) {
  _genBlackPawnCaps(board);
  _genBlackRookCaps(board);
  _genBlackKnightCaps(board);
  _genBlackBishopCaps(board);
  _genBlackKingCaps(board);
  _genBlackQueenCaps(board);
}

void MoveGen::_genPawnPromotions(unsigned int from, unsigned int to, unsigned int flags, PieceType capturedPieceType) {
  Move promotionBase = Move(from, to, PAWN, flags | PROMOTION);
  if (flags & CAPTURE) {
    promotionBase.move = MoveUtils::setCapturedPieceType(capturedPieceType, promotionBase.move);
  }

  Move queenPromotion = promotionBase;
  queenPromotion.move = MoveUtils::setPromotionPieceType(QUEEN, queenPromotion.move);
  _moves.push_back(queenPromotion);

  Move rookPromotion = promotionBase;
  rookPromotion.move = MoveUtils::setPromotionPieceType(ROOK, rookPromotion.move);
  _moves.push_back(rookPromotion);

  Move bishopPromotion = promotionBase;
  bishopPromotion.move = MoveUtils::setPromotionPieceType(BISHOP, bishopPromotion.move);
  _moves.push_back(bishopPromotion);

  Move knightPromotion = promotionBase;
  knightPromotion.move = MoveUtils::setPromotionPieceType(KNIGHT, knightPromotion.move);
  _moves.push_back(knightPromotion);
}

void MoveGen::_genWhitePawnSingleMoves(const Board &board) {
  U64 movedPawns = board.getPieces(WHITE, PAWN) << 8;
  movedPawns &= board.getNotOccupied();

  U64 promotions = movedPawns & RANK_8;
  movedPawns &= ~RANK_8;

  // Generate single non promotion moves
  while (movedPawns) {
    int to = _popLsb(movedPawns);
    _moves.push_back(Move(to - 8, to, PAWN, 0));
  }

  // Generate promotions
  while (promotions) {
    int to = _popLsb(promotions);
    _genPawnPromotions(to - 8, to);
  }
}

void MoveGen::_genWhitePawnDoubleMoves(const Board &board) {
  U64 singlePushes = (board.getPieces(WHITE, PAWN) << 8) & board.getNotOccupied();
  U64 doublePushes = (singlePushes << 8) & board.getNotOccupied() & RANK_4;

  while (doublePushes) {
    int to = _popLsb(doublePushes);
    _moves.push_back(Move(to - 16, to, PAWN, DOUBLE_PAWN_PUSH));
  }
}

void MoveGen::_genWhitePawnLeftAttacks(const Board &board) {
  U64 leftRegularAttacks = (board.getPieces(WHITE, PAWN) << 7) & board.getAttackable(BLACK) & ~FILE_H;

  U64 leftAttackPromotions = leftRegularAttacks & RANK_8;
  leftRegularAttacks &= ~RANK_8;

  U64 leftEnPassant = (board.getPieces(WHITE, PAWN) << 7) & board.getEnPassant() & ~FILE_H;

  // Add regular attacks (Not promotions or en passants)
  while (leftRegularAttacks) {
    int to = _popLsb(leftRegularAttacks);

    Move move = Move(to - 7, to, PAWN, CAPTURE);
    move.move = MoveUtils::setCapturedPieceType(board.getPieceAtSquare(BLACK, to), move.move);

    _moves.push_back(move);
  }

  // Add promotion attacks
  while (leftAttackPromotions) {
    int to = _popLsb(leftAttackPromotions);
    _genPawnPromotions(to - 7, to, CAPTURE, board.getPieceAtSquare(BLACK, to));
  }

  // Add en passant attacks
  // There can only be one en passant square at a time, so no need for loop
  if (leftEnPassant) {
    int to = _popLsb(leftEnPassant);
    _moves.push_back(Move(to - 7, to, PAWN, EN_PASSANT));
  }
}

void MoveGen::_genWhitePawnRightAttacks(const Board &board) {
  U64 rightRegularAttacks = (board.getPieces(WHITE, PAWN) << 9) & board.getAttackable(BLACK) & ~FILE_A;

  U64 rightAttackPromotions = rightRegularAttacks & RANK_8;
  rightRegularAttacks &= ~RANK_8;

  U64 rightEnPassant = (board.getPieces(WHITE, PAWN) << 9) & board.getEnPassant() & ~FILE_A;

  // Add regular attacks (Not promotions or en passants)
  while (rightRegularAttacks) {
    int to = _popLsb(rightRegularAttacks);

    Move move = Move(to - 9, to, PAWN, CAPTURE);
    move.move = MoveUtils::setCapturedPieceType(board.getPieceAtSquare(BLACK, to), move.move);

    _moves.push_back(move);
  }

  // Add promotion attacks
  while (rightAttackPromotions) {
    int to = _popLsb(rightAttackPromotions);
    _genPawnPromotions(to - 9, to, CAPTURE, board.getPieceAtSquare(BLACK, to));
  }

  // Add en passant attacks
  // There can only be one en passant square at a time, so no need for loop
  if (rightEnPassant) {
    int to = _popLsb(rightEnPassant);
    _moves.push_back(Move(to - 9, to, PAWN, EN_PASSANT));
  }
}

void MoveGen::_genBlackPawnSingleMoves(const Board &board) {
  U64 movedPawns = board.getPieces(BLACK, PAWN) >> 8;
  movedPawns &= board.getNotOccupied();

  U64 promotions = movedPawns & RANK_1;
  movedPawns &= ~RANK_1;

  // Generate single non promotion moves'
  while (movedPawns) {
    int to = _popLsb(movedPawns);
    _moves.push_back(Move(to + 8, to, PAWN, 0));
  }

  // Generate promotions
  while (promotions) {
    int to = _popLsb(promotions);
    _genPawnPromotions(to + 8, to);
  }
}

void MoveGen::_genBlackPawnDoubleMoves(const Board &board) {
  U64 singlePushes = (board.getPieces(BLACK, PAWN) >> 8) & board.getNotOccupied();
  U64 doublePushes = (singlePushes >> 8) & board.getNotOccupied() & RANK_5;

  while (doublePushes) {
    int to = _popLsb(doublePushes);
    _moves.push_back(Move(to + 16, to, PAWN, DOUBLE_PAWN_PUSH));
  }
}

void MoveGen::_genBlackPawnLeftAttacks(const Board &board) {
  U64 leftRegularAttacks = (board.getPieces(BLACK, PAWN) >> 9) & board.getAttackable(WHITE) & ~FILE_H;

  U64 leftAttackPromotions = leftRegularAttacks & RANK_1;
  leftRegularAttacks &= ~RANK_1;

  U64 leftEnPassant = (board.getPieces(BLACK, PAWN) >> 9) & board.getEnPassant() & ~FILE_H;

  // Add regular attacks (Not promotions or en passants)
  while (leftRegularAttacks) {
    int to = _popLsb(leftRegularAttacks);

    Move move = Move(to + 9, to, PAWN, CAPTURE);
    move.move = MoveUtils::setCapturedPieceType(board.getPieceAtSquare(WHITE, to), move.move);

    _moves.push_back(move);
  }

  // Add promotion attacks
  while (leftAttackPromotions) {
    int to = _popLsb(leftAttackPromotions);
    _genPawnPromotions(to + 9, to, CAPTURE, board.getPieceAtSquare(WHITE, to));
  }

  // Add en passant attacks
  // There can only be one en passant square at a time, so no need for loop
  if (leftEnPassant) {
    int to = _popLsb(leftEnPassant);
    _moves.push_back(Move(to + 9, to, PAWN, EN_PASSANT));
  }
}

void MoveGen::_genBlackPawnRightAttacks(const Board &board) {
  U64 rightRegularAttacks = (board.getPieces(BLACK, PAWN) >> 7) & board.getAttackable(WHITE) & ~FILE_A;

  U64 rightAttackPromotions = rightRegularAttacks & RANK_1;
  rightRegularAttacks &= ~RANK_1;

  U64 rightEnPassant = (board.getPieces(BLACK, PAWN) >> 7) & board.getEnPassant() & ~FILE_A;

  // Add regular attacks (Not promotions or en passants)
  while (rightRegularAttacks) {
    int to = _popLsb(rightRegularAttacks);

    Move move = Move(to + 7, to, PAWN, CAPTURE);
    move.move = MoveUtils::setCapturedPieceType(board.getPieceAtSquare(WHITE, to), move.move);

    _moves.push_back(move);
  }

  // Add promotion attacks
  while (rightAttackPromotions) {
    int to = _popLsb(rightAttackPromotions);
    _genPawnPromotions(to + 7, to, CAPTURE, board.getPieceAtSquare(WHITE, to));
  }

  // Add en passant attacks
  // There can only be one en passant square at a time, so no need for loop
  if (rightEnPassant) {
    int to = _popLsb(rightEnPassant);
    _moves.push_back(Move(to + 7, to, PAWN, EN_PASSANT));
  }
}

void MoveGen::_genWhitePawnMoves(const Board &board) {
  _genWhitePawnSingleMoves(board);
  _genWhitePawnDoubleMoves(board);
  _genWhitePawnLeftAttacks(board);
  _genWhitePawnRightAttacks(board);
}

void MoveGen::_genWhitePawnCaps(const Board &board) {
  _genWhitePawnLeftAttacks(board);
  _genWhitePawnRightAttacks(board);
}

void MoveGen::_genBlackPawnMoves(const Board &board) {
  _genBlackPawnSingleMoves(board);
  _genBlackPawnDoubleMoves(board);
  _genBlackPawnLeftAttacks(board);
  _genBlackPawnRightAttacks(board);
}

void MoveGen::_genBlackPawnCaps(const Board &board) {
  _genBlackPawnLeftAttacks(board);
  _genBlackPawnRightAttacks(board);
}

void MoveGen::_genWhiteKingMoves(const Board &board) {
  _genKingMoves(board, board.getPieces(WHITE, KING), board.getAttackable(BLACK));

  if (board.whiteCanCastleKs()) {
    _moves.push_back(Move(e1, g1, KING, KSIDE_CASTLE));
  }
  if (board.whiteCanCastleQs()) {
    _moves.push_back(Move(e1, c1, KING, QSIDE_CASTLE));
  }
}

void MoveGen::_genWhiteKingCaps(const Board &board) {
  _genKingCaps(board, board.getPieces(WHITE, KING), board.getAttackable(BLACK));
}

void MoveGen::_genBlackKingMoves(const Board &board) {
  _genKingMoves(board, board.getPieces(BLACK, KING), board.getAttackable(WHITE));

  if (board.blackCanCastleKs()) {
    _moves.push_back(Move(e8, g8, KING, KSIDE_CASTLE));
  }
  if (board.blackCanCastleQs()) {
    _moves.push_back(Move(e8, c8, KING, QSIDE_CASTLE));
  }
}

void MoveGen::_genBlackKingCaps(const Board &board) {
  _genKingCaps(board, board.getPieces(BLACK, KING), board.getAttackable(WHITE));
}

void MoveGen::_genKingMoves(const Board &board, U64 king, U64 attackable) {
  if (king == 0) {
    return;
  }

  int kingIndex = _bitscanForward(king);

  U64 moves = board.getAttacksForSquare(KING, board.getActivePlayer(), kingIndex);

  _addMoves(board, kingIndex, KING, moves, attackable);
}

void MoveGen::_genKingCaps(const Board &board, U64 king, U64 attackable) {
  if (king == 0) {
    return;
  }

  int kingIndex = _bitscanForward(king);

  U64 moves = board.getAttacksForSquare(KING, board.getActivePlayer(), kingIndex);

  _addCaps(board, kingIndex, KING, moves, attackable);
}

void MoveGen::_genWhiteKnightMoves(const Board &board) {
  _genKnightMoves(board, board.getPieces(WHITE, KNIGHT), board.getAttackable(BLACK));
}

void MoveGen::_genWhiteKnightCaps(const Board &board) {
  _genKnightCaps(board, board.getPieces(WHITE, KNIGHT), board.getAttackable(BLACK));
}

void MoveGen::_genBlackKnightMoves(const Board &board) {
  _genKnightMoves(board, board.getPieces(BLACK, KNIGHT), board.getAttackable(WHITE));
}

void MoveGen::_genBlackKnightCaps(const Board &board) {
  _genKnightCaps(board, board.getPieces(BLACK, KNIGHT), board.getAttackable(WHITE));
}

void MoveGen::_genKnightMoves(const Board &board, U64 knights, U64 attackable) {
  while (knights) {
    int from = _popLsb(knights);

    U64 moves = board.getAttacksForSquare(KNIGHT, board.getActivePlayer(), from);

    _addMoves(board, from, KNIGHT, moves, attackable);
  }
}

void MoveGen::_genKnightCaps(const Board &board, U64 knights, U64 attackable) {
  while (knights) {
    int from = _popLsb(knights);

    U64 moves = board.getAttacksForSquare(KNIGHT, board.getActivePlayer(), from);

    _addCaps(board, from, KNIGHT, moves, attackable);
  }
}

void MoveGen::_genWhiteBishopMoves(const Board &board) {
  _genBishopMoves(board, board.getPieces(WHITE, BISHOP), board.getAttackable(BLACK));
}

void MoveGen::_genWhiteBishopCaps(const Board &board) {
  _genBishopCaps(board, board.getPieces(WHITE, BISHOP), board.getAttackable(BLACK));
}

void MoveGen::_genBlackBishopMoves(const Board &board) {
  _genBishopMoves(board, board.getPieces(BLACK, BISHOP), board.getAttackable(WHITE));
}

void MoveGen::_genBlackBishopCaps(const Board &board) {
  _genBishopCaps(board, board.getPieces(BLACK, BISHOP), board.getAttackable(WHITE));
}

void MoveGen::_genBishopMoves(const Board &board, U64 bishops, U64 attackable) {
  while (bishops) {
    int from = _popLsb(bishops);

    U64 moves = board.getAttacksForSquare(BISHOP, board.getActivePlayer(), from);

    _addMoves(board, from, BISHOP, moves, attackable);
  }
}

void MoveGen::_genBishopCaps(const Board &board, U64 bishops, U64 attackable) {
  while (bishops) {
    int from = _popLsb(bishops);

    U64 moves = board.getAttacksForSquare(BISHOP, board.getActivePlayer(), from);

    _addCaps(board, from, BISHOP, moves, attackable);
  }
}

void MoveGen::_genWhiteRookMoves(const Board &board) {
  _genRookMoves(board, board.getPieces(WHITE, ROOK), board.getAttackable(BLACK));
}

void MoveGen::_genWhiteRookCaps(const Board &board) {
  _genRookCaps(board, board.getPieces(WHITE, ROOK), board.getAttackable(BLACK));
}

void MoveGen::_genBlackRookMoves(const Board &board) {
  _genRookMoves(board, board.getPieces(BLACK, ROOK), board.getAttackable(WHITE));
}

void MoveGen::_genBlackRookCaps(const Board &board) {
  _genRookCaps(board, board.getPieces(BLACK, ROOK), board.getAttackable(WHITE));
}

void MoveGen::_genRookMoves(const Board &board, U64 rooks, U64 attackable) {
  while (rooks) {
    int from = _popLsb(rooks);

    U64 moves = board.getAttacksForSquare(ROOK, board.getActivePlayer(), from);

    _addMoves(board, from, ROOK, moves, attackable);
  }
}

void MoveGen::_genRookCaps(const Board &board, U64 rooks, U64 attackable) {
  while (rooks) {
    int from = _popLsb(rooks);

    U64 moves = board.getAttacksForSquare(ROOK, board.getActivePlayer(), from);

    _addCaps(board, from, ROOK, moves, attackable);
  }
}

void MoveGen::_genWhiteQueenMoves(const Board &board) {
  _genQueenMoves(board, board.getPieces(WHITE, QUEEN), board.getAttackable(BLACK));
}

void MoveGen::_genWhiteQueenCaps(const Board &board) {
  _genQueenCaps(board, board.getPieces(WHITE, QUEEN), board.getAttackable(BLACK));
}

void MoveGen::_genBlackQueenMoves(const Board &board) {
  _genQueenMoves(board, board.getPieces(BLACK, QUEEN), board.getAttackable(WHITE));
}

void MoveGen::_genBlackQueenCaps(const Board &board) {
  _genQueenCaps(board, board.getPieces(BLACK, QUEEN), board.getAttackable(WHITE));
}

void MoveGen::_genQueenMoves(const Board &board, U64 queens, U64 attackable) {
  while (queens) {
    int from = _popLsb(queens);

    U64 moves = board.getAttacksForSquare(QUEEN, board.getActivePlayer(), from);

    _addMoves(board, from, QUEEN, moves, attackable);
  }
}

void MoveGen::_genQueenCaps(const Board &board, U64 queens, U64 attackable) {
  while (queens) {
    int from = _popLsb(queens);

    U64 moves = board.getAttacksForSquare(QUEEN, board.getActivePlayer(), from);

    _addCaps(board, from, QUEEN, moves, attackable);
  }
}

void MoveGen::_addMoves(const Board &board, int from, PieceType pieceType, U64 moves, U64 attackable) {
  // Ignore all moves/attacks to kings
  moves &= ~(board.getPieces(board.getInactivePlayer(), KING));

  // Generate non attacks
  U64 nonAttacks = moves & ~attackable;
  while (nonAttacks) {
    int to = _popLsb(nonAttacks);
    _moves.push_back(Move(from, to, pieceType, 0));
  }

  // Generate attacks
  U64 attacks = moves & attackable;
  while (attacks) {
    int to = _popLsb(attacks);

    Move move(from, to, pieceType, CAPTURE);
    move.move = MoveUtils::setCapturedPieceType(board.getPieceAtSquare(board.getInactivePlayer(), to), move.move);

    _moves.push_back(move);
  }
}

void MoveGen::_addCaps(const Board &board, int from, PieceType pieceType, U64 moves, U64 attackable) {
  // Ignore all moves/attacks to kings
  moves &= ~(board.getPieces(board.getInactivePlayer(), KING));

  // Generate attacks
  U64 attacks = moves & attackable;
  while (attacks) {
    int to = _popLsb(attacks);

    Move move(from, to, pieceType, CAPTURE);
    move.move = MoveUtils::setCapturedPieceType(board.getPieceAtSquare(board.getInactivePlayer(), to), move.move);
    _moves.push_back(move);
  }
}
