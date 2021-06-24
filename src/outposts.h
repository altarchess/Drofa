#ifndef OUTPOSTS_H
#define OUTPOSTS_H

#include "eval.h"

//various OUTPOSTS PSQT are defined here

/**
 * @brief [color][sqv] 
 */ 
const int KNIGHT_PROT_OUTPOST_BLACK[64] = {
           gS(0,0), gS(0,0), gS(0,0), gS(0,0), gS(0,0), gS(0,0), gS(0,0), gS(0,0),
           gS(0,3), gS(7,5), gS(0,4), gS(0,-1), gS(17,0), gS(0,3), gS(0,0), gS(0,0),
           gS(24,17), gS(6,34), gS(23,18), gS(21,21), gS(9,30), gS(36,23), gS(28,37), gS(31,11),
           gS(12,12), gS(24,17), gS(25,22), gS(26,27), gS(35,33), gS(34,28), gS(41,22), gS(15,18),
           gS(10,12), gS(19,13), gS(26,14), gS(35,16), gS(35,15), gS(29,8), gS(21,11), gS(12,9),
           gS(-4,3), gS(0,-1), gS(-1,-2), gS(3,-2), gS(4,-1), gS(-4,0), gS(-5,0), gS(-8,2),
           gS(0,0), gS(0,0), gS(0,0), gS(0,0), gS(0,0), gS(0,0), gS(0,0), gS(0,0),
           gS(0,0), gS(0,0), gS(0,0), gS(0,0), gS(0,0), gS(0,0), gS(0,0), gS(0,0),
};

const int BISHOP_PROT_OUTPOST_BLACK[64] = {
           gS(0,0), gS(0,0), gS(0,0), gS(0,0), gS(0,-5), gS(0,0), gS(0,0), gS(0,0),
           gS(0,1), gS(22,0), gS(10,-4), gS(0,-5), gS(1,-12), gS(2,0), gS(25,0), gS(0,-1),
           gS(-5,0), gS(29,1), gS(19,0), gS(37,0), gS(33,0), gS(62,-3), gS(34,10), gS(-10,0),
           gS(0,0), gS(27,6), gS(36,0), gS(41,5), gS(43,3), gS(42,6), gS(31,13), gS(0,6),
           gS(-6,0), gS(33,6), gS(38,2), gS(43,3), gS(44,7), gS(36,4), gS(33,6), gS(-8,0),
           gS(0,5), gS(9,11), gS(6,10), gS(11,11), gS(10,4), gS(1,0), gS(11,7), gS(-4,0),
           gS(0,0), gS(0,0), gS(0,0), gS(0,0), gS(0,0), gS(0,0), gS(0,0), gS(0,0),
           gS(0,0), gS(0,0), gS(0,0), gS(0,0), gS(0,0), gS(0,0), gS(0,0), gS(0,0),
};

const int KNIGHT_OUTPOST_BLACK[64] = {
           gS(0,0), gS(0,0), gS(0,0), gS(0,0), gS(0,0), gS(0,0), gS(0,0), gS(0,0),
           gS(0,0), gS(0,0), gS(0,0), gS(0,0), gS(0,0), gS(0,0), gS(0,0), gS(0,0),
           gS(8,-5), gS(9,0), gS(10,0), gS(18,2), gS(8,0), gS(30,-3), gS(21,1), gS(-8,0),
           gS(3,0), gS(10,0), gS(23,2), gS(40,2), gS(44,2), gS(38,5), gS(26,0), gS(11,7),
           gS(1,-3), gS(9,-2), gS(17,0), gS(20,8), gS(29,2), gS(22,-4), gS(10,-2), gS(6,0),
           gS(-3,0), gS(4,5), gS(5,9), gS(3,10), gS(3,14), gS(2,13), gS(8,0), gS(0,4),
           gS(-15,-1), gS(-2,3), gS(-10,18), gS(-5,9), gS(-1,2), gS(-9,12), gS(0,2), gS(-13,0),
           gS(-3,0), gS(-8,1), gS(-9,7), gS(-8,2), gS(-7,0), gS(-5,0), gS(-1,0), gS(-7,0),
};

const int BISHOP_OUTPOST_BLACK[64] = {
           gS(0,0), gS(0,0), gS(0,0), gS(0,0), gS(0,0), gS(0,0), gS(0,0), gS(0,0),
           gS(0,0), gS(0,0), gS(0,0), gS(0,0), gS(0,0), gS(0,0), gS(0,0), gS(0,0),
           gS(1,-9), gS(12,-13), gS(14,0), gS(18,0), gS(6,0), gS(26,0), gS(9,-1), gS(-10,0),
           gS(12,-3), gS(14,-4), gS(22,0), gS(32,0), gS(26,0), gS(27,0), gS(20,0), gS(9,0),
           gS(5,-2), gS(11,-6), gS(18,0), gS(25,1), gS(28,4), gS(24,0), gS(18,0), gS(3,0),
           gS(3,4), gS(2,8), gS(3,4), gS(6,7), gS(5,7), gS(0,4), gS(0,5), gS(-3,3),
           gS(-6,4), gS(-3,3), gS(-2,9), gS(-1,8), gS(1,8), gS(-2,2), gS(2,0), gS(-5,2),
           gS(5,6), gS(-4,17), gS(-2,3), gS(0,8), gS(0,4), gS(-8,0), gS(-2,6), gS(1,3),
};
#endif