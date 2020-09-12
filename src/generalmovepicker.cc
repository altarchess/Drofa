#include "movepicker.h"
#include "generalmovepicker.h"
#include "eval.h"

int g_TT_MO_hit;
extern HASH myHASH;

GeneralMovePicker::GeneralMovePicker(const OrderingInfo *orderingInfo, const Board *board, MoveList *moveList)
    : MovePicker(moveList) {
  _orderingInfo = orderingInfo;
  _moves = moveList;
  _board = board;
  _currHead = 0;
  _scoreMoves();
}

void GeneralMovePicker::_scoreMoves() {
  int pvMove;
  HASH_Entry pvEntry = myHASH.HASH_Get(_board->getZKey().getValue());
  if (pvEntry.Flag != NONE){
    pvMove = pvEntry.move;
  }else{
    pvMove = 0;
  }

  for (auto &move : *_moves) {
    int m = move.move;
    if (m == pvMove) {
      move.score = INF;
    } else if (getTypeFlag(m) & CAPTURE) {
      move.score = (CAPTURE_BONUS + _mvvLvaTable[getCapPiece(m)][getPieceType(m)]);
    } else if (getTypeFlag(m) & PROMOTION) {
      move.score = (PROMOTION_BONUS + Eval::getMaterialValue(getPromoteTo(m)));
    } else if (m == _orderingInfo->getKiller1(_orderingInfo->getPly()).move) {
      move.score = (KILLER1_BONUS);
    } else if (m == _orderingInfo->getKiller2(_orderingInfo->getPly()).move) {
      move.score = (KILLER2_BONUS);
    } else { // Quiet
      move.score = (QUIET_BONUS + _orderingInfo->getHistory(_board->getActivePlayer(), getSQVFrom(m), getSQVTo(m)));
    }
  }
}

bool GeneralMovePicker::hasNext() const {
  return _currHead < _moves->size();
}

Move GeneralMovePicker::getNext() {
  size_t bestIndex;
  int bestScore = -INF;

  for (size_t i = _currHead; i < _moves->size(); i++) {
    if (_moves->at(i).score > bestScore) {
      bestScore = _moves->at(i).score;
      bestIndex = i;
    }
  }

  std::swap(_moves->at(_currHead), _moves->at(bestIndex));
  return _moves->at(_currHead++);
}