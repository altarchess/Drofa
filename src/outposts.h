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
           gS(24,17), gS(4,38), gS(25,21), gS(21,24), gS(11,32), gS(36,27), gS(26,42), gS(31,16),
           gS(17,15), gS(29,15), gS(31,23), gS(32,29), gS(44,31), gS(39,28), gS(47,18), gS(18,19),
           gS(11,14), gS(20,14), gS(28,15), gS(37,18), gS(38,17), gS(30,9), gS(22,11), gS(10,12),
           gS(-4,7), gS(3,-1), gS(1,0), gS(6,-3), gS(8,-4), gS(3,3), gS(0,0), gS(-8,2),
           gS(0,0), gS(0,0), gS(0,0), gS(0,0), gS(0,0), gS(0,0), gS(0,0), gS(0,0),
           gS(0,0), gS(0,0), gS(0,0), gS(0,0), gS(0,0), gS(0,0), gS(0,0), gS(0,0),
};

const int BISHOP_PROT_OUTPOST_BLACK[64] = {
           gS(0,0), gS(0,0), gS(0,-1), gS(0,0), gS(0,-5), gS(0,0), gS(0,0), gS(0,0),
           gS(0,1), gS(22,0), gS(10,-11), gS(0,-13), gS(1,-13), gS(2,-2), gS(24,0), gS(0,-1),
           gS(-5,2), gS(30,0), gS(22,-5), gS(38,-4), gS(34,-2), gS(62,-8), gS(34,8), gS(-10,0),
           gS(4,0), gS(28,6), gS(39,0), gS(45,4), gS(48,2), gS(45,5), gS(33,14), gS(0,6),
           gS(-6,0), gS(33,7), gS(40,0), gS(47,4), gS(47,6), gS(37,4), gS(33,8), gS(-8,0),
           gS(0,5), gS(9,12), gS(5,10), gS(9,12), gS(12,4), gS(4,0), gS(14,8), gS(-7,0),
           gS(0,0), gS(0,0), gS(0,0), gS(0,0), gS(0,0), gS(0,0), gS(0,0), gS(0,0),
           gS(0,0), gS(0,0), gS(0,0), gS(0,0), gS(0,0), gS(0,0), gS(0,0), gS(0,0),
};

const int KNIGHT_OUTPOST_BLACK[64] = {
           gS(0,0), gS(0,0), gS(0,0), gS(0,0), gS(0,0), gS(0,0), gS(0,0), gS(0,0),
           gS(0,0), gS(0,0), gS(0,0), gS(0,0), gS(0,0), gS(0,0), gS(0,0), gS(0,0),
           gS(9,-5), gS(13,1), gS(11,0), gS(20,3), gS(15,0), gS(30,-3), gS(21,5), gS(-9,0),
           gS(7,0), gS(15,-4), gS(27,0), gS(40,2), gS(48,-2), gS(42,3), gS(31,-4), gS(13,8),
           gS(3,-7), gS(9,-4), gS(17,-6), gS(18,3), gS(30,-2), gS(24,-8), gS(11,-4), gS(7,0),
           gS(-3,0), gS(4,4), gS(6,8), gS(3,7), gS(2,11), gS(7,11), gS(8,0), gS(0,4),
           gS(-15,0), gS(-2,4), gS(-10,20), gS(-5,10), gS(-3,5), gS(-9,12), gS(0,3), gS(-17,0),
           gS(-3,0), gS(-9,2), gS(-9,7), gS(-8,2), gS(-8,0), gS(-6,0), gS(-2,0), gS(-8,0),
};

const int BISHOP_OUTPOST_BLACK[64] = {
           gS(0,0), gS(0,0), gS(0,0), gS(0,0), gS(0,0), gS(0,0), gS(0,0), gS(0,0),
           gS(0,0), gS(0,0), gS(0,0), gS(0,0), gS(0,0), gS(0,0), gS(0,0), gS(0,0),
           gS(3,-9), gS(13,-11), gS(15,0), gS(19,1), gS(11,0), gS(26,0), gS(12,-1), gS(-10,0),
           gS(14,-3), gS(17,-4), gS(23,1), gS(32,4), gS(30,-2), gS(29,1), gS(24,0), gS(12,0),
           gS(7,-1), gS(13,-3), gS(18,0), gS(23,3), gS(29,5), gS(26,2), gS(20,1), gS(4,3),
           gS(2,7), gS(2,13), gS(1,7), gS(4,9), gS(4,9), gS(0,6), gS(1,8), gS(-5,6),
           gS(-7,8), gS(-4,6), gS(-5,13), gS(-4,11), gS(0,10), gS(-3,5), gS(5,2), gS(-7,3),
           gS(5,10), gS(-4,19), gS(-1,8), gS(0,13), gS(0,10), gS(-3,1), gS(-1,7), gS(0,5),
};

#endif