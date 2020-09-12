#ifndef MOVE_H
#define MOVE_H

#include "defs.h"
#include "board.h"
#include <string>
#include <vector>

// Firsst define some macros for
// fast move-processing

#define getPieceType(m) static_cast<PieceType>(m & 0x7)
#define getCapPiece(m)  static_cast<PieceType>((m >> 6) & 0x7)
#define getPromoteTo(m) static_cast<PieceType>((m >> 3) & 0x7)
#define getSQVFrom(m)   (uint)((m >> 9) & 0x3f)
#define getSQVTo(m)     (uint)(((m >> 15) & 0x3f))
#define getTypeFlag(m)  (uint)((m >> 21) & 0x7f)


  /**
   * @name Rank and file characters in algebraic notation
   *
   * @{
   */
  const char RANKS[] = {'1', '2', '3', '4', '5', '6', '7', '8'};
  const char FILES[] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'};
  /**@}*/

    /** @brief Notation that should be used to represent a null move */
  const std::string NULL_MOVE_NOTATION = "(none)";


  /**
   * @enum Flag
   * @brief Flags that indicate special moves.
   */
  enum Flag {
    NULL_MOVE = 1 << 0, /**< The move is a null move */
    CAPTURE = 1 << 1, /**< The move is a capture (Captured piece type must be set with setCapturedPieceType()) */
    DOUBLE_PAWN_PUSH = 1 << 2, /**< The move is a double pawn push */
    KSIDE_CASTLE = 1 << 3, /**< The move is a kingisde castle */
    QSIDE_CASTLE = 1 << 4, /**< The move is a queenside castle */
    EN_PASSANT = 1 << 5, /**< The move is an en passant capture (Do not set the CAPTURE flag additionally) */
    PROMOTION = 1 << 6 /**< The move is a promotion (Promotion piece type must be set with setPromotionPieceType()) */
  };

/**
 * @brief Represents our Move
 *
 * Move contains 2 parts:
 * 
 * move -  the move INT itself
 * score - score assigned to the move 
 */

struct Move
{
    /**
   * @brief A packed integer containing all of the move data.
   *
   * 28 bits are used in total to store move information. The format is as
   * follows:
   *
   * @code
   * MSB  |----6----|---5---|---4---|-3--|-2--|-1--|  LSB
   *      28        21      15      9    6    3    0
   * @endcode
   *
   * - 6 - Flags (7 bits)
   * - 5 - To square (6 bits)
   * - 4 - From square (6 bits)
   * - 3 - Captured PieceType (if applicable) (3 bits)
   * - 2 - Promotion PieceType (if applicable) (3 bits)
   * - 1 - PieceType of moving piece (3 bits)
   *
   * Moves are stored as a packed integer consisting of 28 bits total.
   */
    int move;
    int score;

    Move() : move(0), score(0) {}
    Move(int m) : move(m), score(0) {}
    Move(int m, int sc) :
        move(m), score(sc) {}

    Move(unsigned int from, unsigned int to, PieceType piece, unsigned int flags) {
      move = ((flags & 0x7f) << 21) | ((to & 0x3f) << 15) | ((from & 0x3f) << 9) | (piece & 0x7);
      score = 0;
    }
};


/**
 * @brief Represents a move.
 */
namespace MoveUtils {
 
  /**
   * @brief Sets the specified flag on this move.
   * @param flag Flag to set
   */
  int setFlag(Flag, int);

  /**
   * @brief Set the type of piece captured by this move.
   *
   * Note that the CAPTURE flag should be set if this method is to be called.
   *
   * @param pieceType The type of captured piece to set
   */
  int setCapturedPieceType(PieceType, int);

  /**
   * @brief Set the type of piece that this move promotes to.
   *
   * Note that the PROMOTION flag should be set if this method is to be called.
   *
   * @param pieceType The type of promotion piece to set
   */
  int setPromotionPieceType(PieceType, int);

  /**
   * @brief Return a UCI compliant string representation of this move.
   * @return A UCI compliant string representation of this move.
   */
  std::string getNotation(int);

  /**
   * @brief Returns the index of a square on the chess board given its algebraic notation.
   *
   * Eg. notationToIndex("g1") would return 6
   *
   * @param  notation The algebraic notation of the square on the chess board to get the index for.
   * @return The index of the square (little endian rank-file mapping)
   */
  unsigned int notationToIndex(std::string);

  /**
   * @brief Returns the algebraic notation of a square on the chess board given its index.
   *
   * Eg. indexToNotation(6) would return "g1"
   *
   * @param  index Index to get algebraic notation for
   * @return Algebraic notation of the square at the given index.
   */
  std::string indexToNotation(int);

};

#endif
