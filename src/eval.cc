#include "defs.h"
#include "rays.h"
#include "movegen.h"
#include "attacks.h"
#include "outposts.h"
#include "eval.h"
#include "transptable.h"

extern HASH myHASH;

int MATERIAL_VALUES_TUNABLE[2][6] = {
    [OPENING] = {
        [PAWN] = 100,
        [ROOK] = 465,
        [KNIGHT] = 304,
        [BISHOP] = 336,
        [QUEEN] = 1190,
        [KING] = 0
    },
    [ENDGAME] = {
        [PAWN] = 86,
        [ROOK] = 565,
        [KNIGHT] = 328,
        [BISHOP] = 357,
        [QUEEN] = 995,
        [KING] = 0
    }
};
int BISHOP_PAIR_TUNABLE [2] = {
    [OPENING] = 20,
    [ENDGAME] = 20
};

U64 Eval::detail::FILES[8] = {FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H};
U64 Eval::detail::DISTANCE[64][64];
U64 Eval::detail::NEIGHBOR_FILES[8]{
    FILE_B,
    FILE_A | FILE_C,
    FILE_B | FILE_D,
    FILE_C | FILE_E,
    FILE_D | FILE_F,
    FILE_E | FILE_G,
    FILE_F | FILE_H,
    FILE_G
};
U64 Eval::detail::PASSED_PAWN_MASKS[2][64];
U64 Eval::detail::BITS_SCAN_FORWARD[2][64];
U64 Eval::detail::OUTPOST_MASK[2][64];
U64 Eval::detail::OUTPOST_PROTECTION[2][64];
U64 Eval::detail::KINGZONE[2][64];
int Eval::detail::PHASE_WEIGHT_SUM = 0;
U64 Eval::detail::KING_OO_MASKS[2][2] = {
        [WHITE] = {
            [0] = 0xC0ull,
            [1] = 0x7ull
        },
        [BLACK] = {
            [0] = 0xC000000000000000ull,
            [1] = 0x700000000000000ull
        }
    };
U64 Eval::detail::KING_PAWN_MASKS[2][2][7] = {
        [WHITE] = {
          [0] = {
            [0] = 0xE000ull,
            [1] = 0x806000ull,
            [2] = 0x40A000ull,
            [3] = 0x80402000ull,
            [4] = 0xC02000ull,
            [5] = 0xC000ull,
            [6] = 0x804000ull
          },
          [1] = {
            [0] = 0x700ull,
            [1] = 0x10600ull,
            [2] = 0x20500ull,
            [3] = 0x1020400ull,
            [4] = 0x30400ull,
            [5] = 0,
            [6] = 0
          }
        },
        [BLACK] = {
          [0] = {
              [0] = 0xE0000000000000ull,
              [1] = 0x60800000000000ull,
              [2] = 0xA0400000000000ull,
              [3] = 0x20408000000000ull,
              [4] = 0x20C00000000000ull,
              [5] = 0xC0000000000000ull,
              [6] = 0x40800000000000ull
          },
          [1] = {
            [0] = 0x7000000000000ull,
            [1] = 0x6010000000000ull,
            [2] = 0x5020000000000ull,
            [3] = 0x4020100000000ull,
            [4] = 0x4030000000000ull,
            [5] = 0,
            [6] = 0
          }
        }
    };


void Eval::init() {
  // Initialize passed pawn masks
  for (int square = 0; square < 64; square++) {

    U64 currNorthRay = Rays::getRay(Rays::NORTH, square);
    U64 currSouthRay = Rays::getRay(Rays::SOUTH, square);

    detail::PASSED_PAWN_MASKS[WHITE][square] =
        currNorthRay | _eastN(currNorthRay, 1) | _westN(currNorthRay, 1);
    detail::PASSED_PAWN_MASKS[BLACK][square] =
        currSouthRay | _westN(currSouthRay, 1) | _eastN(currSouthRay, 1);

    detail::OUTPOST_MASK[WHITE][square] = detail::PASSED_PAWN_MASKS[WHITE][square] & detail::NEIGHBOR_FILES[_col(square)];  
    detail::OUTPOST_MASK[BLACK][square] = detail::PASSED_PAWN_MASKS[BLACK][square] & detail::NEIGHBOR_FILES[_col(square)];  

    detail::BITS_SCAN_FORWARD[WHITE][square] = detail::PASSED_PAWN_MASKS[WHITE][square] ^ detail::NEIGHBOR_FILES[_col(square)];
    detail::BITS_SCAN_FORWARD[BLACK][square] = detail::PASSED_PAWN_MASKS[BLACK][square] ^ detail::NEIGHBOR_FILES[_col(square)];

    U64 sqv = ONE << square;
    detail::OUTPOST_PROTECTION[WHITE][square] = ((sqv >> 9) & ~FILE_H) | ((sqv >> 7) & ~FILE_A);
    detail::OUTPOST_PROTECTION[BLACK][square] = ((sqv << 9) & ~FILE_A) | ((sqv << 7) & ~FILE_H);

    U64 kingAttack = Attacks::getNonSlidingAttacks(KING, square, WHITE); 
    detail::KINGZONE[WHITE][square] = sqv | kingAttack | (kingAttack << 8);
    detail::KINGZONE[BLACK][square] = sqv | kingAttack | (kingAttack >> 8);

    for (int i =0; i < 64; i++){
      detail::DISTANCE[square][i] = std::max(abs(_col(square) - _col(i)), abs(_row(square) - _row(i)));
    }

  }

  // Initialize PHASE_WEIGHT_SUM
  detail::PHASE_WEIGHT_SUM += detail::PHASE_WEIGHTS[PAWN] * 16;
  detail::PHASE_WEIGHT_SUM += detail::PHASE_WEIGHTS[KNIGHT] * 4;
  detail::PHASE_WEIGHT_SUM += detail::PHASE_WEIGHTS[BISHOP] * 4;
  detail::PHASE_WEIGHT_SUM += detail::PHASE_WEIGHTS[ROOK] * 4;
  detail::PHASE_WEIGHT_SUM += detail::PHASE_WEIGHTS[QUEEN] * 2;
}

evalBits Eval::Setupbits(const Board &board){
  evalBits eB;
  U64 pBB = board.getPieces(WHITE, PAWN);
  eB.EnemyPawnAttackMap[BLACK] = ((pBB << 9) & ~FILE_A) | ((pBB << 7) & ~FILE_H);
  pBB = board.getPieces(BLACK, PAWN);
  eB.EnemyPawnAttackMap[WHITE] = ((pBB >> 9) & ~FILE_H) | ((pBB >> 7) & ~FILE_A);

  U64 king = board.getPieces(WHITE, KING);
  eB.EnemyKingZone[BLACK] = detail::KINGZONE[WHITE][_bitscanForward(king)];
      king = board.getPieces(BLACK, KING);
  eB.EnemyKingZone[WHITE] = detail::KINGZONE[BLACK][_bitscanForward(king)];
  
  eB.RammedCount = _popCount((board.getPieces(BLACK, PAWN) >> 8) & board.getPieces(WHITE, PAWN));
  eB.OutPostedLines[0] = 0, eB.OutPostedLines[1] = 0;
  eB.KingAttackers[0] = 0, eB.KingAttackers[1] = 0;
  eB.KingAttackPower[0] = 0, eB.KingAttackPower[1] = 0;
  eB.Passers[0] = 0, eB.Passers[1] = 0;
  return eB;
}

void Eval::SetupTuning(int phase, PieceType piece, int value){
  MATERIAL_VALUES_TUNABLE [phase][piece] = value;
}

void Eval::SetupFeatureTuning(int phase, TuningFeature feature, int value){
  switch (feature)
  {
  case BISHOP_PAIR:
    BISHOP_PAIR_TUNABLE[phase] = value;
    break;
  
  default:
    break;
  }
}

int Eval::getMaterialValue(int phase, PieceType pieceType) {
  #ifdef _TUNE_
  return MATERIAL_VALUES_TUNABLE [phase][pieceType];
  #else
  if (phase == OPENING){
    return opS(MATERIAL_VALUES[pieceType]);
  }else{
    return egS(MATERIAL_VALUES[pieceType]);
  }
  #endif
}

bool Eval::IsItDeadDraw (int w_P, int w_N, int w_B, int w_R, int w_Q,
int b_P, int b_N, int b_B, int b_R, int b_Q){

if (w_P > 0 || b_P > 0){
  return false;           // если есть хоть 1 пешка, то ещё играем
}

if (w_Q > 0 || b_Q > 0 ){
  return false;           // есть есть хоть 1 королева, то ещё играем
} 

if (w_R == 0 && b_R == 0){ // нет пешек, нет королев, нет ладей
  if (w_B == 0 && b_B == 0){  // нет пешек, ладей, слонов.
      if (w_N < 3 && b_N < 3){ // меньше 2х коней = ничья
        return true;
      }
  } else if (w_N == 0 && b_N == 0){ // нет пешек, королев, ладей, коней
      if (abs( w_B - b_B) < 2){
        return true;
      }
  }else if (( w_N < 3 && w_B ==0) || (w_B == 1 &&  w_N == 0)){
    if ((b_N < 3 && b_B == 0)||(b_B == 1 && b_N == 0)){
      return true;
    }
  }
}else {                             // ладьи есть
  if (w_R == 1 && b_R == 1){
    if (w_N + w_B < 2 && b_N + b_B < 2){    // тут немного криво, так как BR vs R выигрывают
      return true;
    } 
  }else if (w_R == 1 && b_R == 0){
    if (w_N + w_B == 0 && b_N + b_B > 1){
      return true;
    }
  }else if (b_R == 1 && w_R == 0){
    if (b_N + b_B == 0 && w_N + w_B > 1){
      return true;
    }
  }
}

return false;

}

int Eval::kingSafety(const Board &board, Color color, int Q_count){
      //King safety - замена pawnsShieldingKing
    // идея в том, чтобы
    // а. Найти позицию короля
    // b. Для каждой выбранной позиции мы имеем некую маску и скор того, насколько она "безопасна"

    // Упрощённо будем считать, что если нет ферзя у врага, то мы в безопасности.
    if (Q_count == 0){
      return 0;
    }
    U64 pawnMap = board.getPieces(color, PAWN);
    //проверяем где наш король. Проверка по сути на то, куда сделали рокировку.
    //если король не рокирован-выгнан с рокировки, то король не в безопасности, начислить штраф

         //0 - kingSide; 1 - QueenSide

    if ((detail::KING_OO_MASKS[color][0] & board.getPieces(color, KING)) != 0){
      // если это верно, то мы находимся на королевском фланге
      // сравниваем с масками, возвращаем бонус если маска совпала
      if ((pawnMap & detail::KING_PAWN_MASKS[color][0][0]) == detail::KING_PAWN_MASKS[color][0][0]){
          return KING_SAFE;    
      }
      if ((pawnMap & detail::KING_PAWN_MASKS[color][0][1]) == detail::KING_PAWN_MASKS[color][0][1]){
          return KING_SAFE;
      }
      if ((pawnMap & detail::KING_PAWN_MASKS[color][0][2]) == detail::KING_PAWN_MASKS[color][0][2]){
          return KING_SAFE;
      }
      if ((pawnMap & detail::KING_PAWN_MASKS[color][0][3]) == detail::KING_PAWN_MASKS[color][0][3]){
          return KING_LOW_DANGER;     
      }
      if ((pawnMap & detail::KING_PAWN_MASKS[color][0][4]) == detail::KING_PAWN_MASKS[color][0][4]){
          return KING_LOW_DANGER;
      }
      if ((pawnMap & detail::KING_PAWN_MASKS[color][0][5]) == detail::KING_PAWN_MASKS[color][0][5]){
          return KING_LOW_DANGER;
      }
      if ((pawnMap & detail::KING_PAWN_MASKS[color][0][6]) == detail::KING_PAWN_MASKS[color][0][6]){
          return KING_LOW_DANGER;
      }
      // если не одна из масок не прошла, то король в опасности.
      // вернуть штраф к нашей позиции
      return KING_MED_DANGER;
    }      

    if ((detail::KING_OO_MASKS[color][1] & board.getPieces(color, KING)) != 0){
      // если это верно, то мы находимся на ферзевом
      // сравниваем с масками, возвращаем бонус если маска совпала
      if ((pawnMap & detail::KING_PAWN_MASKS[color][1][0]) == detail::KING_PAWN_MASKS[color][1][0]){
          return KING_SAFE;    
      }
      if ((pawnMap & detail::KING_PAWN_MASKS[color][1][1]) == detail::KING_PAWN_MASKS[color][1][1]){
          return KING_SAFE;
      }
      if ((pawnMap & detail::KING_PAWN_MASKS[color][1][2]) == detail::KING_PAWN_MASKS[color][1][2]){
          return KING_SAFE;
      }
      if ((pawnMap & detail::KING_PAWN_MASKS[color][1][3]) == detail::KING_PAWN_MASKS[color][1][3]){
          return KING_LOW_DANGER;     
      }
      if ((pawnMap & detail::KING_PAWN_MASKS[color][1][4]) == detail::KING_PAWN_MASKS[color][1][4]){
          return KING_LOW_DANGER;
      }
      // если не одна из масок не прошла, то король в опасности.
      // вернуть штраф к нашей позиции
      return KING_MED_DANGER;
    } 

      // мы не рокированы по факту при живом ферзе.
      // это очень опасно, вернуть штраф
    return KING_HIGH_DANGER;
}

int Eval::getPhase(const Board &board) {
  int phase = detail::PHASE_WEIGHT_SUM;

  for (auto pieceType : {ROOK, KNIGHT, BISHOP, QUEEN}) {
    phase -= _popCount(board.getPieces(WHITE, pieceType)) * detail::PHASE_WEIGHTS[pieceType];
    phase -= _popCount(board.getPieces(BLACK, pieceType)) * detail::PHASE_WEIGHTS[pieceType];
  }

  // Transform phase from the range 0 - PHASE_WEIGHT_SUM to 0 - PHASE_WEIGHT_MAX
  return ((phase * MAX_PHASE) + (detail::PHASE_WEIGHT_SUM / 2)) / detail::PHASE_WEIGHT_SUM;
}

inline int Eval::evaluateQUEEN(const Board & board, Color color, evalBits * eB){
  int s = 0;

  U64 pieces = board.getPieces(color, QUEEN);

  // Mobility

    while (pieces) {
      int square = _popLsb(pieces);
      U64 attackBitBoard = board.getMobilityForSquare(QUEEN, color, square, eB->EnemyPawnAttackMap[color]);
      s += QUEEN_MOBILITY[_popCount(attackBitBoard)];
      int kingAttack = _popCount(attackBitBoard & eB->EnemyKingZone[color]);
      if (kingAttack > 0){
        eB->KingAttackers[color]++;
        eB->KingAttackPower[color] += kingAttack * PIECE_ATTACK_POWER[QUEEN];
      }
    }

  return s;
}

inline int Eval::evaluateROOK(const Board & board, Color color, evalBits * eB){
  int s = 0;
  Color otherColor = getOppositeColor(color);

  U64 pieces = board.getPieces(color, ROOK);



    while (pieces) {

      // Mobility
      int square = _popLsb(pieces);
      U64 attackBitBoard = board.getMobilityForSquare(ROOK, color, square, eB->EnemyPawnAttackMap[color]);
      s += ROOK_MOBILITY[_popCount(attackBitBoard)];
      int kingAttack = _popCount(attackBitBoard & eB->EnemyKingZone[color]);
      if (kingAttack > 0){
        eB->KingAttackers[color]++;
        eB->KingAttackPower[color] += kingAttack * PIECE_ATTACK_POWER[ROOK];
      }

      U64 file = detail::FILES[_col(square)];

      if ( ((file & board.getPieces(color, PAWN)) == 0)
        && ((file & board.getPieces(otherColor, PAWN)) == 0)){
          s += ROOK_OPEN_FILE_BONUS[((file & eB->OutPostedLines[otherColor]) != 0)];
      }
      else if ((file & board.getPieces(color, PAWN)) == 0){
          s += ROOK_SEMI_FILE_BONUS[((file & eB->OutPostedLines[otherColor]) != 0)];
      }
    }

  return s;
}

inline int Eval::evaluateBISHOP(const Board & board, Color color, evalBits * eB){
  int s = 0;

  U64 pieces = board.getPieces(color, BISHOP);
  s += eB->RammedCount * _popCount(pieces) * BISHOP_RAMMED_PENALTY;
    while (pieces) {
      
      // Mobility
      int square = _popLsb(pieces);
      U64 attackBitBoard = board.getMobilityForSquare(BISHOP, color, square, eB->EnemyPawnAttackMap[color]);
      s += BISHOP_MOBILITY[_popCount(attackBitBoard)];
      int kingAttack = _popCount(attackBitBoard & eB->EnemyKingZone[color]);
      if (kingAttack > 0){
        eB->KingAttackers[color]++;
        eB->KingAttackPower[color] += kingAttack * PIECE_ATTACK_POWER[BISHOP];
      }

      // OUTPOSTED BISHOP
      if ((board.getPieces(getOppositeColor(color), PAWN) & detail::OUTPOST_MASK[color][square]) == ZERO){
        if (detail::OUTPOST_PROTECTION[color][square] & board.getPieces(color, PAWN)){
          s += BISHOP_PROT_OUTPOST[color][square];
          eB->OutPostedLines[color] = eB->OutPostedLines[color] | detail::FILES[_col(square)];
        }else{
          s += BISHOP_OUTPOST[color][square];          
        }

      }
    }

  return s;
}

inline int Eval::evaluateKNIGHT(const Board & board, Color color, evalBits * eB){
  int s = 0;

  U64 pieces = board.getPieces(color, KNIGHT);

    while (pieces) {
      
      // Mobility
      int square = _popLsb(pieces);
      U64 attackBitBoard = board.getMobilityForSquare(KNIGHT, color, square,eB->EnemyPawnAttackMap[color]);
      s += KNIGHT_MOBILITY[_popCount(attackBitBoard)];
      int kingAttack = _popCount(attackBitBoard & eB->EnemyKingZone[color]);
      if (kingAttack > 0){
        eB->KingAttackers[color]++;
        eB->KingAttackPower[color] += kingAttack * PIECE_ATTACK_POWER[KNIGHT];
      }

      // OUTPOSTED KNIGHT
      if ((board.getPieces(getOppositeColor(color), PAWN) & detail::OUTPOST_MASK[color][square]) == ZERO){
        if (detail::OUTPOST_PROTECTION[color][square] & board.getPieces(color, PAWN)){
          s += KNIGHT_PROT_OUTPOST[color][square];
          eB->OutPostedLines[color] = eB->OutPostedLines[color] | detail::FILES[_col(square)];
        }else{
          s += KNIGHT_OUTPOST[color][square];         
        }
      }
    }
  return s;
}

inline int Eval::evaluateKING(const Board & board, Color color, const evalBits & eB){
  int s = 0;

  U64 pieces = board.getPieces(color, KING);
  int square = _popLsb(pieces);
  // Mobility
  U64 attackBitBoard = board.getMobilityForSquare(KING, color, square, eB.EnemyPawnAttackMap[color]);
  s += KING_MOBILITY[_popCount(attackBitBoard)];
  
  U64 tmpPawns = eB.Passers[color];
  while (tmpPawns != ZERO) {

    int passerSquare = _popLsb(tmpPawns);
    s += KING_PASSER_DISTANCE_FRIENDLY[Eval::detail::DISTANCE[square][passerSquare]];
  }

  tmpPawns = eB.Passers[getOppositeColor(color)];
  while (tmpPawns != ZERO) {

    int passerSquare = _popLsb(tmpPawns);
    s += KING_PASSER_DISTANCE_ENEMY[Eval::detail::DISTANCE[square][passerSquare]];
  }

  return s;
}

inline int Eval::evaluatePAWNS(const Board & board, Color color, evalBits * eB){

  //passed
  int s = 0;

  U64 pawns = board.getPieces(color, PAWN);
  U64 tmpPawns = pawns;

  while (tmpPawns != ZERO) {

    // add bonuses if the pawn is passed

    int square = _popLsb(tmpPawns);
    int pawnCol = _col(square);
    if ((board.getPieces(getOppositeColor(color), PAWN) & detail::PASSED_PAWN_MASKS[color][square]) == ZERO){
      eB->Passers[color] = eB->Passers[color] | (ONE << square);
      int r = color == WHITE ? _row(square) : 8 - _row(square);
      s += PASSED_PAWN_RANKS[r] + PASSED_PAWN_FILES[pawnCol]; 
    }

    // add penalties for the doubled pawns
    if (_popCount(tmpPawns & detail::FILES[pawnCol]) > 0){
      s += DOUBLED_PAWN_PENALTY;

    // for the last pawn on the column determine if it is isolated.
    }
    else if (!(detail::NEIGHBOR_FILES[pawnCol] & pawns)){
      s += ISOLATED_PAWN_PENALTY;
    }
  }

  return s;
}

int Eval::evaluate(const Board &board, Color color) {

  int score = 0;

  Color otherColor = getOppositeColor(color);

  // Material value

  int w_P = _popCount(board.getPieces(color, PAWN));
  int b_P = _popCount(board.getPieces(otherColor, PAWN));
  int w_N = _popCount(board.getPieces(color, KNIGHT));
  int b_N = _popCount(board.getPieces(otherColor, KNIGHT));
  int w_B = _popCount(board.getPieces(color, BISHOP));
  int b_B = _popCount(board.getPieces(otherColor, BISHOP));
  int w_R = _popCount(board.getPieces(color, ROOK));
  int b_R = _popCount(board.getPieces(otherColor, ROOK));
  int w_Q = _popCount(board.getPieces(color, QUEEN));
  int b_Q = _popCount(board.getPieces(otherColor, QUEEN));

  if (IsItDeadDraw(w_P, w_N, w_B, w_R, w_Q, b_P, b_N, b_B, b_R, b_Q)){
    return 0;
  }

#ifdef _TUNE_
  openingScore += getMaterialValue(OPENING, PAWN) * ( w_P - b_P);
  openingScore += getMaterialValue(OPENING, KNIGHT) * (w_N - b_N);
  openingScore += getMaterialValue(OPENING, BISHOP) * ( w_B - b_B);
  openingScore += getMaterialValue(OPENING, ROOK) * ( w_R - b_R);
  openingScore += getMaterialValue(OPENING, QUEEN) * ( w_Q - b_Q);

  endgameScore += getMaterialValue(ENDGAME, PAWN) * ( w_P - b_P);
  endgameScore += getMaterialValue(ENDGAME, KNIGHT) * (w_N - b_N);
  endgameScore += getMaterialValue(ENDGAME, BISHOP) * ( w_B - b_B);
  endgameScore += getMaterialValue(ENDGAME, ROOK) * ( w_R - b_R);
  endgameScore += getMaterialValue(ENDGAME, QUEEN) * ( w_Q - b_Q);
#else
  score += MATERIAL_VALUES[PAWN] * ( w_P - b_P);
  score += MATERIAL_VALUES[KNIGHT] * (w_N - b_N);
  score += MATERIAL_VALUES[BISHOP] * ( w_B - b_B);
  score += MATERIAL_VALUES[ROOK] * ( w_R - b_R);
  score += MATERIAL_VALUES[QUEEN] * ( w_Q - b_Q);
#endif

  // Piece square tables
  score += board.getPSquareTable().getScore(color) - board.getPSquareTable().getScore(otherColor);

  // Create evalBits stuff
  evalBits eB = Eval::Setupbits(board);

  // Pawn structure
  int pScore = 0;
  pawn_HASH_Entry pENTRY  = myHASH.pHASH_Get(board.getPawnStructureZKey().getValue());
  
  if (pENTRY.posKey != 0){
    eB.Passers[WHITE] = pENTRY.wPassers;
    eB.Passers[BLACK] = pENTRY.bPassers;

    if (color == BLACK) {
      score -= pENTRY.score;
    } else {
      score += pENTRY.score;
    }
 
  }
  else
  {

  // PawnSupported  
  pScore += PAWN_SUPPORTED * _popCount(board.getPieces(WHITE, PAWN) & eB.EnemyPawnAttackMap[BLACK]);
  pScore -= PAWN_SUPPORTED * _popCount(board.getPieces(BLACK, PAWN) & eB.EnemyPawnAttackMap[WHITE]);

  // Passed pawns
  pScore += evaluatePAWNS(board, WHITE, &eB) - evaluatePAWNS(board, BLACK, &eB);

  myHASH.pHASH_Store(board.getPawnStructureZKey().getValue(), eB.Passers[WHITE], eB.Passers[BLACK], pScore);

    if (color == BLACK) {
      score -= pScore;
    } else {
      score += pScore;
    }
  }

  // Evaluate pieces
  int pieceS = evaluateBISHOP(board, color, &eB) - evaluateBISHOP(board, otherColor, &eB)
            + evaluateKNIGHT(board, color, &eB) - evaluateKNIGHT(board, otherColor, &eB)
            + evaluateROOK  (board, color, &eB) - evaluateROOK  (board, otherColor, &eB)
            + evaluateQUEEN (board, color, &eB) - evaluateQUEEN (board, otherColor, &eB)
            + evaluateKING (board, color, eB) - evaluateKING (board, otherColor, eB);

  score += pieceS;

  // evluate king danger

  int attackScore = eB.KingAttackPower[color] * COUNT_TO_POWER[std::min(7, eB.KingAttackers[color])] / 100;
  score += gS(std::max(0, attackScore), 0);
  attackScore = eB.KingAttackPower[otherColor] * COUNT_TO_POWER[std::min(7, eB.KingAttackers[otherColor])] / 100;
  score -= gS(std::max(0, attackScore), 0);

  // Bishop pair
  #ifdef _TUNE_
  openingScore += w_B > 1 ? BISHOP_PAIR_TUNABLE[OPENING] : 0;
  endgameScore += w_B > 1 ? BISHOP_PAIR_TUNABLE[ENDGAME] : 0;
  openingScore -= b_B > 1 ? BISHOP_PAIR_TUNABLE[OPENING] : 0;
  endgameScore -= b_B > 1 ? BISHOP_PAIR_TUNABLE[ENDGAME] : 0;
  #else
  score += w_B > 1 ? BISHOP_PAIR_BONUS : 0;
  score -= b_B > 1 ? BISHOP_PAIR_BONUS : 0;
  #endif

    // King pawn shield
  // Tapering is included in, so we count it in both phases
  // As of 5.08.20 400 game testing did not showed advantage for any king safety implementation.
  // Changes commitet for further use though
  score += kingSafety(board, color, b_Q) - kingSafety(board, otherColor, w_Q);


  // Calculation of the phase value
  int phase = getPhase(board);

  // Interpolate between opening/endgame scores depending on the phase

  int final_eval = ((opS(score) * (MAX_PHASE - phase)) + (egS(score) * phase)) / MAX_PHASE;

  if (w_Q == 0 && b_Q == 0 &&
      w_R == 0 && b_R == 0 &&
      w_N == 0 && b_N == 0 &&
      w_B == 1 && b_B == 1){
        U64 bothBishops = board.getPieces(color, BISHOP) | board.getPieces(otherColor, BISHOP);
        if (_popCount(bothBishops & WHITE_SQUARES) == 1){
          final_eval = final_eval / 2;
        }
      }

  return final_eval + TEMPO;
}

int Eval::evalTestSuite(const Board &board, Color color)
{
  return 0;
}