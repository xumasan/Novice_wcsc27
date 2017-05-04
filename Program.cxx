#include <vector>
#include <cinttypes>
#include <chrono>
#include <algorithm>
#include <string>
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <vector> 
#include <unordered_map>
#include <cstring>
#include <thread>
#include <atomic>
#include <unistd.h>
#include <math.h>
#include <unordered_map> 
#include <tuple>
#include <random>
#include <emmintrin.h>
#include <mutex>
//#undef NDEBUG 
#define NDEBUG
#include <cassert>

typedef std::chrono::system_clock::time_point TIME;
#define TimerElapsed(start,end) std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count()
#define TimerNow() std::chrono::system_clock::now()

namespace color {
	
	enum { Black = 0, White = 1, size = 2, };
	int opp(int cl) { return cl ^ 1; }  
}

namespace piece {

	enum {
		NONE = 0, Empty = 0, PromoteBit = 8, WhiteBit = 16, Wall = 32, //out of board
		BP = 1, WP = BP + WhiteBit, //pawn
		BL = 2, WL = BL + WhiteBit, // lance
		BN = 3, WN = BN + WhiteBit, // knight
		BS = 4, WS = BS + WhiteBit, // silver
		BG = 5, WG = BG + WhiteBit, // gold
		BB = 6, WB = BB + WhiteBit, // bishop
		BR = 7, WR = BR + WhiteBit, // rook
		BK = 8, WK = BK + WhiteBit, // king
		BPP = BP + PromoteBit, WPP = BPP + WhiteBit, // propawn
		BPL = BL + PromoteBit, WPL = BPL + WhiteBit, // prolance
		BPN = BN + PromoteBit, WPN = BPN + WhiteBit, // proknight
		BPS = BS + PromoteBit, WPS = BPS + WhiteBit, // prosilver
		BHO = BB + PromoteBit, WHO = BHO + WhiteBit, // horce
		BDR = BR + PromoteBit, WDR = BDR + WhiteBit, // dragon
		
		size, 
		
		StandMax = 8, NB = 40,
	};

	const bool canPromote[32] =
	{  
		false,true,true,true,true,false,true,true,false,false,false,false,false,false,false,false,
		false,true,true,true,true,false,true,true,false,false,false,false,false,false,false,false,
	};

	const int incNum[32] = { 
		0x00, 0x01, 0x01, 0x02, 0x05, 0x06, 0x04, 0x04,	0x08, 0x06,	0x06, 0x06, 0x06, 0x00,	0x08, 0x08,
		0x00, 0x01, 0x01, 0x02, 0x05, 0x06, 0x04, 0x04,	0x08, 0x06,	0x06, 0x06, 0x06, 0x00,	0x08, 0x08
	};
	const int Inc[32][8] = {
		{ 0, 0, 0, 0, 0, 0, 0, 0 }, // NULL
		{-17,0, 0, 0, 0, 0, 0, 0 }, // Bpawn
		{-17,0, 0, 0, 0, 0, 0, 0 }, // Blance
		{-35,-33,0,0, 0, 0, 0, 0 }, // Bknight
		{-18,-17,-16,+16,+18,0,0,0},// Bsilver
		{-18,-17,-16,-1,+1,+17,0,0},// Bgold
		{-18,-16,+16,+18,0,0,0,0},  // Bbishop
		{-17,-1,+1,+17,  0,0,0,0},  // Brook
		{-18,-17,-16,-1,+1,+16,+17,+18}, // Bking
		{-18,-17,-16,-1,+1,+17,0,0},// Bpropawn
		{-18,-17,-16,-1,+1,+17,0,0},// Bprolance
		{-18,-17,-16,-1,+1,+17,0,0},// Bproknight
		{-18,-17,-16,-1,+1,+17,0,0},// Bprosilver
		{ 0, 0, 0, 0, 0, 0, 0, 0 }, // NULL
		{-18,-17,-16,-1,+1,+16,+17,+18}, // Bhorse
		{-18,-17,-16,-1,+1,+16,+17,+18}, // Bdragon
		{ 0, 0, 0, 0, 0, 0, 0, 0 }, // NULL
		{+17,0, 0, 0, 0, 0, 0, 0 }, // Wpawn
		{+17,0, 0, 0, 0, 0, 0, 0 }, // Wlance
		{+35,+33,0,0, 0, 0, 0, 0 }, // Wknight
		{+18,+17,+16,-16,-18,0,0,0},// Wsilver
		{+18,+17,+16,+1,-1,-17,0,0},// Wgold
		{+18,+16,-16,-18,0,0,0,0},  // Wbishop
		{+17,+1,-1,-17,  0,0,0,0},  // Wrook
		{+18,+17,+16,+1,-1,-16,-17,-18}, // Wking
		{+18,+17,+16,+1,-1,-17,0,0},// Wpropawn
		{+18,+17,+16,+1,-1,-17,0,0},// Wprolance
		{+18,+17,+16,+1,-1,-17,0,0},// Wproknight
		{+18,+17,+16,+1,-1,-17,0,0},// Wprosilver
		{ 0, 0, 0, 0, 0, 0, 0, 0 }, // NULL
		{+18,+17,+16,+1,-1,-16,-17,-18}, // Whorse
		{+18,+17,+16,+1,-1,-16,-17,-18}, // Wdragon
	};

	const int IncSliderNum[32] = { 
		0x00, 0x01, 0x01, 0x02, 0x05, 0x06, 0x04, 0x04,	0x08, 0x06,	0x06, 0x06, 0x06, 0x00,	0x08, 0x08,
		0x00, 0x01, 0x01, 0x02, 0x05, 0x06, 0x04, 0x04,	0x08, 0x06,	0x06, 0x06, 0x06, 0x00,	0x08, 0x08
	};
	const int IncSlider[32][8] = {
		{ 0, 0, 0, 0, 0, 0, 0, 0 }, // NULL
		{ 0, 0, 0, 0, 0, 0, 0, 0 }, // Bpawn
		{-17, 0, 0, 0, 0, 0, 0, 0 }, // Blance
		{ 0, 0, 0, 0, 0, 0, 0, 0 }, // Bknight
		{ 0, 0, 0, 0, 0, 0, 0, 0 },// Bsilver
		{ 0, 0, 0, 0, 0, 0, 0, 0 },// Bgold
		{-18,-16,+16,+18,0,0,0,0},  // Bbishop
		{-17,-1,+1,+17,  0,0,0,0},  // Brook
		{ 0, 0, 0, 0, 0, 0, 0, 0 }, // Bking
		{ 0, 0, 0, 0, 0, 0, 0, 0 },// Bpropawn
		{ 0, 0, 0, 0, 0, 0, 0, 0 },// Bprolance
		{ 0, 0, 0, 0, 0, 0, 0, 0 },// Bproknight
		{ 0, 0, 0, 0, 0, 0, 0, 0 },// Bprosilver
		{ 0, 0, 0, 0, 0, 0, 0, 0 }, // NULL
		{-18,-16,+16,+18,0,0,0,0},  // Bbishop
		{-17,-1,+1,+17,  0,0,0,0},
		{ 0, 0, 0, 0, 0, 0, 0, 0 }, // NULL
		{ 0, 0, 0, 0, 0, 0, 0, 0 }, // Wpawn
		{+17,0, 0, 0, 0, 0, 0, 0 }, // Wlance
		{ 0, 0, 0, 0, 0, 0, 0, 0 }, // Wknight
		{ 0, 0, 0, 0, 0, 0, 0, 0 },// Wsilver
		{ 0, 0, 0, 0, 0, 0, 0, 0 },// Wgold
		{+18,+16,-16,-18,0,0,0,0},  // Wbishop
		{+17,+1,-1,-17,  0,0,0,0},  // Wrook
		{ 0, 0, 0, 0, 0, 0, 0, 0 }, // Wking
		{ 0, 0, 0, 0, 0, 0, 0, 0 },// Wpropawn
		{ 0, 0, 0, 0, 0, 0, 0, 0 },// Wprolance
		{ 0, 0, 0, 0, 0, 0, 0, 0 },// Wproknight
		{ 0, 0, 0, 0, 0, 0, 0, 0 },// Wprosilver
		{ 0, 0, 0, 0, 0, 0, 0, 0 }, // NULL
		{+18,+16,-16,-18,0,0,0,0},  // Wbishop
		{+17,+1,-1,-17,  0,0,0,0}  // Wrook
	};

	const int IncNonSlider[32][8] = {
		{ 0, 0, 0, 0, 0, 0, 0, 0 }, // NULL
		{ 0, 0, 0, 0, 0, 0, 0, 0 }, // Bpawn
		{ 0, 0, 0, 0, 0, 0, 0, 0 }, // Blance
		{ 0, 0, 0, 0, 0, 0, 0, 0 }, // Bknight
		{ 0, 0, 0, 0, 0, 0, 0, 0 }, // Bsilver
		{ 0, 0, 0, 0, 0, 0, 0, 0 }, // Bgold
		{ 0, 0, 0, 0, 0, 0, 0, 0 }, // Bbishop
		{ 0, 0, 0, 0, 0, 0, 0, 0 }, // Brook
		{ 0, 0, 0, 0, 0, 0, 0, 0 }, // Bking
		{ 0, 0, 0, 0, 0, 0, 0, 0 }, // Bpropawn
		{ 0, 0, 0, 0, 0, 0, 0, 0 }, // Bprolance
		{ 0, 0, 0, 0, 0, 0, 0, 0 }, // Bproknight
		{ 0, 0, 0, 0, 0, 0, 0, 0 }, // Bprosilver
		{ 0, 0, 0, 0, 0, 0, 0, 0 }, // NULL
		{+17,+1,-1,-17,  0,0,0,0},  // Bbishop
		{+18,+16,-16,-18,0,0,0,0},
		{ 0, 0, 0, 0, 0, 0, 0, 0 }, // NULL
		{ 0, 0, 0, 0, 0, 0, 0, 0 }, // Wpawn
		{ 0, 0, 0, 0, 0, 0, 0, 0 }, // Wlance
		{ 0, 0, 0, 0, 0, 0, 0, 0 }, // Wknight
		{ 0, 0, 0, 0, 0, 0, 0, 0 }, // Wsilver
		{ 0, 0, 0, 0, 0, 0, 0, 0 }, // Wgold
		{ 0, 0, 0, 0, 0, 0, 0, 0 }, // Wbishop
		{ 0, 0, 0, 0, 0, 0, 0, 0 }, // Wrook
		{ 0, 0, 0, 0, 0, 0, 0, 0 }, // Wking
		{ 0, 0, 0, 0, 0, 0, 0, 0 }, // Wpropawn
		{ 0, 0, 0, 0, 0, 0, 0, 0 }, // Wprolance
		{ 0, 0, 0, 0, 0, 0, 0, 0 }, // Wproknight
		{ 0, 0, 0, 0, 0, 0, 0, 0 }, // Wprosilver
		{ 0, 0, 0, 0, 0, 0, 0, 0 }, // NULL
		{+17,+1,-1,-17,  0,0,0,0},  // Wbishop
		{+18,+16,-16,-18,0,0,0,0}   // Wrook
	};

	const int pieceValue[size] = { 
		0, +90, +315, +405, +495, +540, +855, +990, +15000, +540, +540, +540, +540, 0, +945, +1395,
        0, +90, +315, +405, +495, +540, +855, +990, +15000, +540, +540, +540, +540, 0, +945, +1395,
	};

	const int exchangeValue[size] = {
		0, 180, 630, 810, 990, 1080, 1710, 1980, 30000, 630, 855, 945, 1035, 0, 1800, 2385,
    	0, 180, 630, 810, 990, 1080, 1710, 1980, 30000, 630, 855, 945, 1035, 0, 1800, 2385,
	};

	int abs(int pc) { return pc & ~WhiteBit; }
	bool is_black(int pc) { return BP <= pc && pc <= BDR; }
	bool is_white(int pc) { return WP <= pc && pc <= WDR; }
	int color_is(int pc) { return is_black(pc) ? color::Black : color::White; }
	int KING(int cl) { return cl ? WK : BK; }
	int PAWN(int cl) { return cl ? WP : BP; }
	int value(int pc) { return pieceValue[pc]; }
	int exchange_value(int pc) { return exchangeValue[pc]; }
	bool can_promote(int pc) { return canPromote[pc]; }
	bool is_slider(int pc, int inc) {

		switch (pc)
		{
			case BL:
				if (inc == -17) return true; // lance 
				else return false;
			
			case WL:
				if (inc == +17) return true; // lance 
				else return false;
			
			case BB: case WB: case BHO: case WHO:
				if (inc == -18 || inc == -16 || inc == +16 || inc == +18) return true; // bishop && horse
				else return false;

			case BR: case WR: case BDR:	case WDR:
				if (inc == -17 || inc == +17 || inc == -1 || inc == +1) return true; // rook && dragon
				else return false;
			
			default: return false;
		};
	}
}

namespace square {

	enum { size = 221 };

	int coord_x(int sq) { return 13 - sq % 17; }
	int coord_y(int sq) { return sq / 17 - 1; }
	int notation(int sq) { return sq <= piece::WDR ? 0 : 10 * coord_x(sq) + coord_y(sq); }
	std::string to_string(int sq) {

        std::string s;
        s += '0' + coord_x(sq);
        s += 'a' + (coord_y(sq) - 1);

        return s;
    }
}

namespace config {

	enum {
		HeightMax = 1024   // = 1024
	};
}

namespace score {
	
	enum {
		NONE = -32767,
		DRAW = 0,
		INF  = 30000, // mate 0 ply
		EVALINF = INF - 1024, // handle mates upto 1023 plies
		ResignEval = -INF,
	};

	bool is_mate(int value) { return value < -EVALINF || +EVALINF < value; }
	int mate_value(int height) { return -INF + height; }
}

namespace delta {

	enum { Offset = 144, }; // = 144 ( 182 - 38 )

	int *ToIncMask = nullptr;

	int to_inc(int delta) { return ToIncMask[Offset + delta]; } // no line -> return 0

	void init() {

		ToIncMask = new int[Offset * 2 + 1];

		for (int i = 0; i < Offset * 2 + 1; i++) ToIncMask[i] = 0;
		
		ToIncMask[Offset - 35] = -35;
		ToIncMask[Offset - 33] = -33;
		ToIncMask[Offset + 33] = +33;
		ToIncMask[Offset + 35] = +35;
		for (int dist = 1; dist < 9; dist++)
		{
			ToIncMask[Offset + (-18) * dist] = -18;
			ToIncMask[Offset + (-17) * dist] = -17;
			ToIncMask[Offset + (-16) * dist] = -16;
			ToIncMask[Offset + (-01) * dist] = -01;
			ToIncMask[Offset + (+01) * dist] = +01;
			ToIncMask[Offset + (+16) * dist] = +16;
			ToIncMask[Offset + (+17) * dist] = +17;
			ToIncMask[Offset + (+18) * dist] = +18;
		}
	}
}

namespace hash {

	uint64_t zob_board[piece::size][square::size];
	uint64_t zob_hand[color::size][piece::StandMax];

	enum { NONE = 0, };

	// XOR疑似乱数
	unsigned long Xor128_()	{

		static unsigned long x=123456789,y=362436069,z=521288629,w=88675123;
		unsigned long t;
		t=(x^(x<<11));
		x=y;y=z;z=w; 
		return( w=(w^(w>>19))^(t^(t>>8)) );
	}
	// 64bit乱数の生成
	unsigned long long Xor128(){
		return Xor128_() + (Xor128_() << 31);
	}

	void init() {

		for (int i = 0; i < piece::size; i++)
			for (int j = 0; j < square::size; j++)
				zob_board[i][j] = (uint64_t)Xor128() << 1;
		
		for (int i = 0; i < color::size; i++)
			for (int j = 0; j < piece::StandMax; j++)
				zob_hand[i][j] = (uint64_t)Xor128() << 1;
	}
}

namespace move {

	std::vector<std::string> PieceToString = { "[]","FU","KY","KE","GI","KI","KA","HI","OU","TO","NK","NE","NG","XX","UM","RY" };
	std::vector<std::string> USIDropString = { "   ", "P*", "L*", "N*", "S*", "G*", "B*", "R*" };
    std::vector<std::string> RankString = { "  ", "a", "b", "c", "d", "e", "f", "g", "h", "i" };

	enum { NONE = 0, };

	const int BITS = 32;
	const uint64_t SIZE = 1ULL << BITS;
	const uint64_t MASK = SIZE - 1;

	int make(int from, int to, int cap, int tKind, int fKind) {
		return (from) | (to << 8) | (fKind << 16) | (tKind << 21) | (cap << 26);
	}

	int from(int mv) { return mv & 0xff; }
	int to(int mv) { return (mv >> 8) & 0xff; }
	int from_kind(int mv) { return (mv >> 16) & 0x1f; }
	int to_kind(int mv) { return (mv >> 21) & 0x1f; }
	int capture(int mv) { return (mv >> 26) & 0x1f; }
	bool is_drop(int mv) { return (mv & 0xff) <= piece::WR; }
	bool is_drop_pawn(int mv) {
		int fKind = from_kind(mv);
		return is_drop(mv) && (fKind == piece::BP || fKind == piece::WP);
	}
	bool is_promote(int mv) { return ((mv >> 16) & 0x1f) != ((mv >> 21) & 0x1f); }
	bool is_capture(int mv) { return ((mv >> 26) & 0x1f) != piece::Empty; }
	
	std::string to_can(int mv) {

        std::string s;

		if (mv == move::NONE) return "0000";

        if (is_drop(mv)) {
            s += USIDropString[piece::abs(from_kind(mv))];
        }
        else 
            s += square::to_string(from(mv));

        s += square::to_string(to(mv));

        if (to_kind(mv) != from_kind(mv)) s += "+";

        return s;
    }
}

namespace board {

	// reprisentation
	/*
    38,39,40,41,42,43,44,45,46,
    55,56,57,58,59,60,61,62,63
    **************************
    ****************
    ************
    *******
    ***
    157,*******************165,
    174,*******************182   */

	class Board
	{
	public:
		uint64_t stand_key() { return standKey; }
		uint64_t board_key() { return boardKey; }
		uint64_t full_key() { return turn == color::Black ? (boardKey ^ standKey) : (boardKey ^ standKey ^ 1); }
		uint64_t comp_board_hash();
		uint64_t comp_stand_hash();
		uint64_t attack_board(int cl, int sq) { return attackBoard[cl][sq]; }
		void comp_attack_all();
		int turn_is() const { return turn; }
		int stage_is() { return stage; }
		int square_is(int sq) const { return square[sq]; }
		int stand_is(int cl, int pc) const { return stand[cl][pc]; }
		int square_num(int n) { return squareNum[n]; }
		int last_move() { return moveStack[stage - 1]; }
		int piece_list(int n) const { return pieceList[n]; }
		int king_pos(int cl) { return kingPos[cl]; }
		bool is_attack(int cl, int sq) { return (attackBoard[color::opp(cl)][sq] == 0) ? false : true; }
		bool in_check() { return (attackBoard[color::opp(turn)][kingPos[turn]] == 0) ? false : true; }
		bool is_check_move(int mv);
		void clear();
		void set(const std::string& sfen);
		int in_check_all(int cl, int *attacker);
		int is_attack_all(int color, int* attacker, int sq);
		int is_attack_all(int color, int* attacker, int sq, int except); // for see
		int history_is(int kind, int sq) { return history[kind][sq]; }
		int killer_is(int stg, int n) { return killer[stg][n]; }
		void good_move(int goodMove, int searchDepth);
		bool recapture(int to);
		bool is_ok(int mv);
		void add_long_effect(int color, int pos);
		void remove_long_effect(int color, int pos);
		void flip_turn() { turn = color::opp(turn); }
		void move(int mv);
		void undo(int mv);
		void move_null();
		void undo_null();

	private:
		uint64_t boardKeyStack[config::HeightMax];
		uint64_t standKeyStack[config::HeightMax];
		uint64_t boardKey;
		uint64_t standKey;
		int square[square::size];
		int kingPos[color::size];
		int moveStack[config::HeightMax];
		int stand[color::size][piece::StandMax];
		int pieceList[piece::NB];
		int squareNum[square::size]; // [square] -> PieceNumber
		int pieceStack[piece::StandMax][piece::size];
		// 0~3bit attackNum 4~43bit whichPiece
		uint64_t attackBoard[color::size][square::size];
		int history[piece::size][square::size];
		int killer[config::HeightMax][2];
		int turn;
		int stage;
	};

	void Board::clear() {
		
		for ( int i = 0; i < square::size; i++ ) square[i] = piece::Wall;
		for ( int rank = 0; rank < 9; rank++ ) {

			int pos = 38 + 17 * rank;
			for ( int i = 0; i < 9; i++ ) square[pos++] = piece::Empty; 
		}

		for ( int pc = 0; pc < (piece::BR + 1); pc++ )
		{
			stand[color::Black][pc] = 0;
			stand[color::White][pc] = 0;
		}

		for ( int c = 0; c < 2; c++ )
			for ( int sq = 0; sq < square::size; sq++ ) attackBoard[c][sq] = 0;

		for ( int i = 0; i < square::size; i++ ) squareNum[i] = -1;

		for ( int i = 0; i < 8; i++ )
			for( int j = 0; j < 32; j++ ) pieceStack[i][j] = 0;
		
		for (int i = 0; i < 32; i++)
			for (int j = 0; j < square::size; j++) history[i][j] = 0;
		
		for (int i = 0; i < config::HeightMax; i++)
			for (int j = 0; j < 2; j++) killer[i][j] = 0;

		for (int i = 0; i < 2; i++)
			kingPos[i] = 0;
		
		for (int i = 0; i < config::HeightMax; i++) {

			moveStack[i] = move::NONE;
			boardKeyStack[i] = 0;
			standKeyStack[i] = 0;
		}

		stage = 0;
		turn  = color::Black;
		boardKey = 0;
		standKey = 0;
	}

	void Board::set(const std::string& sfen) {

		const std::string sfenPiece = "XPLNSGBRKXXXXXXXXplnsgbrk"; // P==1 p==17 

		clear();

		std::string token;
		std::istringstream command(sfen);
		
		command >> token; // board

		int file = 0, rank = 0, pro = 0;
		for (auto type : token) {

			if ('0' < type && type <= '9') file += type - '0';  // empty
			else if (type == '+') pro = 1;  
			else if (type == '/') { file = 0; rank++; }
			else if (type == ' ') break; // end of board
			else {

				int pos = 38 + 17 * rank + file;
				auto piece = sfenPiece.find(type);
				square[pos] = (int)piece;
				if (pro) square[pos] += piece::PromoteBit;
				pro = 0;
				file++;
				if (square[pos] == piece::BK) kingPos[color::Black] = pos; // Black King
				if (square[pos] == piece::WK) kingPos[color::White] = pos; // White King
			}
		}

		command >> token; // color
		turn = (token == "b") ? color::Black : color::White;

		command >> token; //hand
		int n = 0;
		for (auto type : token)
		{
			if (type == '-') break; // no hand
			else if ('0' <= type && type <= '9') n = n * 10 + (type - '0');
			else {

				auto pc = sfenPiece.find(type);
				if( pc <= piece::BDR ) stand[color::Black][pc] = (n == 0) ? 1 : n;
				else stand[color::White][ (pc & (~piece::WhiteBit)) ] = (n == 0) ? 1 : n;
				n = 0;
			}
		}

		// 手数
		command >> stage;

		int x;
		// 駒番号を設定
		for (x = 0; x < square::size; x++)
		{ 
			if (square[x] == piece::BK){ pieceList[0] = x; }
			if (square[x] == piece::WK){ pieceList[1] = x; }
		}
		// rook dragon 2~3
		n = 2;
		for (x = 0; x < square::size; x++)
		{ 
			if (square[x] == piece::BR || square[x] == piece::WR || square[x] == piece::BDR || square[x] == piece::WDR)
			{ pieceList[n] = x; n++; }
		}
		while (n < 4) { pieceList[n] = 0;  pieceStack[piece::BR][0]++; pieceStack[piece::BR][pieceStack[piece::BR][0]] = n; n++; }
		// bishop horse 4~5
		for (x = 0; x < square::size; x++)
		{
			if (square[x] == piece::BB || square[x] == piece::WB || square[x] == piece::BHO || square[x] == piece::WHO )
			{ pieceList[n] = x; n++; }
		}
		while (n < 6) { pieceList[n] = 0; pieceStack[piece::BB][0]++; pieceStack[piece::BB][pieceStack[piece::BB][0]] = n; n++; }
		
		for (x = 0;x < square::size; x++)
		{ 
			if (square[x] == piece::BG || square[x] == piece::WG)
			{ pieceList[n] = x; n++; }
		}
		while (n < 10){ pieceList[n] = 0; pieceStack[piece::BG][0]++; pieceStack[piece::BG][pieceStack[piece::BG][0]] = n; n++; }
		
		for (x = 0; x < square::size; x++)
		{ 
			if (square[x] == piece::BS || square[x] == piece::WS || square[x] == piece::BPS || square[x] == piece::WPS)
			{ pieceList[n] = x; n++; }
		}
		while (n < 14){ pieceList[n] = 0; pieceStack[piece::BS][0]++; pieceStack[piece::BS][pieceStack[piece::BS][0]] = n; n++; }
		
		for (x = 0; x < square::size; x++)
		{ 
			if (square[x] == piece::BN || square[x] == piece::WN || square[x] == piece::BPN || square[x] == piece::WPN)
			{ pieceList[n] = x; n++; }
		}
		while (n < 18){ pieceList[n] = 0; pieceStack[piece::BN][0]++; pieceStack[piece::BN][pieceStack[piece::BN][0]] = n; n++; }
		// lance 18 ~ 21
		for (x = 0; x < square::size; x++)
		{ 
			if (square[x] == piece::BL || square[x] == piece::WL || square[x] == piece::BPL || square[x] == piece::WPL)
			{ pieceList[n] = x; n++; }
		}
		while (n < 22){ pieceList[n] = 0; pieceStack[piece::BL][0]++; pieceStack[piece::BL][pieceStack[piece::BL][0]] = n; n++; }
		// Pawn 22 ~ 39
		for (x = 0; x < square::size; x++)
		{ 
			if (square[x] == piece::BP || square[x] == piece::WP || square[x] == piece::BPP || square[x] == piece::WPP)
			{ pieceList[n] = x; n++; }
		}
		while (n <= 39){ pieceList[n] = 0; pieceStack[piece::BP][0]++; pieceStack[piece::BP][pieceStack[piece::BP][0]] = n; n++; }
		
		for (x = 0; x < 40; x++)
		{
			if (pieceList[x])
			{
				squareNum[ pieceList[x] ] = x;
			}
		}

		comp_attack_all();
		boardKey = comp_board_hash();
		standKey = comp_stand_hash();
	}

	int Board::in_check_all(int cl, int* attacker) {
		
		int num, attacker_num = 0;
		int sq = kingPos[cl];
		uint64_t bit = attackBoard[color::opp(cl)][sq] >> 4;
		while (bit) {

			num = __builtin_ctzll(bit);
			attacker[attacker_num++] = num;
			bit ^= (1ULL << num);
		}

		return attacker_num;
	}

	uint64_t Board::comp_board_hash() {

		uint64_t result = 0;

		for (int sq = 38; sq <= 182; sq++) {
			if (square[sq] != piece::Wall)
				result ^= hash::zob_board[square[sq]][sq];
		}

		return result;
	}

	uint64_t Board::comp_stand_hash() {

		uint64_t result = 0;

		for (int i = piece::BP; i <= piece::BR; i++) {
			result += hash::zob_hand[color::Black][i] * stand[color::Black][i];
			result += hash::zob_hand[color::White][i] * stand[color::White][i];
		}

		return result;
	}

	void Board::comp_attack_all() {

		int i;   
		for (int x = 0; x < square::size; x++) {
			int sq = square[x];
			if ( sq == piece::Wall || sq == piece::Empty ) continue;
			int color = piece::color_is(sq); 

			for ( i = 0; i < 8; i++ )
			{
				int inc = piece::Inc[sq][i];
				if ( inc == 0 ) break;
				if ( piece::is_slider( sq , inc ) ) // slide move effect
				{
					int n = x;
					do {
						n += inc;
						attackBoard[color][n] += 1; // add effect num
						attackBoard[color][n] |= (1ULL << (squareNum[x] + 4)); // add effect piece num
					} while ( square[n] == piece::Empty );
				}
				else // no slide move effect
				{
					attackBoard[color][ x + inc ] += 1; // add effect num
					attackBoard[color][ x + inc ] |= (1ULL << (squareNum[x] + 4)); // add effect piece num
				}
			}
		}
	}

	int Board::is_attack_all(int color, int* attacker, int sq) {

		int num,attacker_num = 0;
		uint64_t bit = attackBoard[color][sq] >> 4;
		while (bit)
		{
			num = __builtin_ctzll(bit);
			attacker[attacker_num++] = num;
			bit ^= (1ULL << num);
		}
		return attacker_num;
	}

	int Board::is_attack_all(int color, int* attacker, int sq, int except) { // for see

		int num,attackerNum = 0;
		uint64_t bit = attackBoard[color][sq] >> 4;
		while (bit)
		{
			num = __builtin_ctzll(bit);
			if (num == except) 
			{ 
				bit ^= (1ULL << num); 
				continue; 
			}
			attacker[attackerNum++] = num;
			bit ^= (1ULL << num);
		}
		return attackerNum;
	}

	bool Board::recapture(int to) {

		if (stage == 0) return false;
		if (moveStack[stage - 1] == move::NONE) return false;

		return move::to(moveStack[stage - 1]) == to;
	}

	bool Board::is_check_move(int mv) {

		int to = move::to(mv);
		int tKind = move::to_kind(mv);
		int att = turn_is();
		int def = color::opp(att);
		int king = kingPos[def];

		int directInc, control;

		for (int i = 0; i < piece::incNum[tKind]; i++) {
			
			directInc = piece::Inc[tKind][i];

			if (piece::is_slider(tKind, directInc)) {

				for (int j = 0; ; j++) {

					control  = to + directInc * j;
					if (control == king) return true; // direct attacker
                	if (square_is(control) != piece::Empty) break;
				}
			}
			else if (to + directInc == king) return true;
		}

		if (move::is_drop(mv)) return false;
		int from = move::from(mv);
		int inc = delta::to_inc(king - from);

		if (inc == 0 || std::abs(inc) >= 33) return false; // no line or knight line

		int sq = from;
		do sq += inc; while (square_is(sq) == piece::Empty && sq != to);
		if (sq != king) return false; // blocker or me 

		sq = from; //search attacker
		do sq -= inc; while (square_is(sq) == piece::Empty && sq != to);
		int attacker = square_is(sq); // attacker? 
		return attacker != piece::Empty && attacker != piece::Wall
			&& piece::color_is(attacker) == att && piece::is_slider(attacker, inc);
	}

	void Board::good_move(int goodMove, int searchDepth) {

		if (in_check()) return; // tactical position return
		if (move::capture(goodMove) != move::NONE) return;
		if (move::is_promote(goodMove)) return;

		int tKind = move::to_kind(goodMove);
		int to = move::to(goodMove);

		history[tKind][to] += searchDepth * searchDepth; // HACK
		if (history[tKind][to] >= 16384) { // = 2 ^ 14
			for (int i = 0; i < piece::size; i++) 
				for (int j = 0; j < square::size; j++) history[i][j] /= 2; // clean
		}

		if (killer[stage][0] != goodMove) {
			killer[stage][1] = killer[stage][0];
			killer[stage][0] = goodMove;
		}
	}

	void to_string(Board &bd) {

		std::vector<std::string> PieceToString = 
		{ "[]","FU","KY","KE","GI","KI","KA","HI","OU","TO","NK","NE","NG","XX","UM","RY", 
		  "[]","fu","ky","ke","gi","ki","ka","hi","ou","to","nk","ne","ng","xx","um","ry" };

		for (int i = 0; i < square::size; i++)
		{
			if ( (i + 1) % 17 == 0 )
			{
				if (square::coord_y(i) == 1) std::cout << "\tturn\t=> " << (bd.turn_is() == color::Black ? "black" : "white");
				if (square::coord_y(i) == 2) std::cout << "\tstage\t=> " << bd.stage_is();
				if (square::coord_y(i) == 3) std::cout << "\tkey\t=> " << bd.full_key();
				if (square::coord_y(i) == 4)
				{
					std::cout << "\teval\t=> "; 
					//if (InCheck()) toString += "\tinCheck!";
					// else toString += "\teval\t=> " + Evaluator.Eval(this, Black);
				}
				if (square::coord_y(i) == 5) std::cout << "\tMaterial=> ";
				if (square::coord_y(i) == 6) { std::cout << "\tLastMove=> "; std::cout << move::to_can(bd.last_move()); }
				if (square::coord_y(i) == 8)
				{
					std::cout << "\tBstand\t=> ";
					for (int j = piece::BP; j <= piece::BR; j++)
					{ 
						if (bd.stand_is(color::Black, j) > 0) std::cout << PieceToString[j] << bd.stand_is(color::Black, j) << " ";
					}
				}
				if (square::coord_y(i) == 9)
				{
					std::cout << "\tWstand\t=> ";
					for (int j = piece::BP; j <= piece::BR; j++)
					{ 
						if (bd.stand_is(color::White, j) > 0) std::cout << PieceToString[j] << bd.stand_is(color::White, j) << " ";
					}
				}
				std::cout << std::endl;
			}
			int sq = bd.square_is(i);
			if (sq == piece::Wall) continue;
			std::cout << PieceToString[sq];
		}
		std::cout << std::endl;
	}

	#define EFFECT_LONG_MASK 0b11110000000000001111000000

	void Board::add_long_effect(int color, int pos) {

		uint64_t effect;

		int num, inc, sq;

		if ((effect = attackBoard[color][pos] & EFFECT_LONG_MASK))
		{
			effect >>= 4;
			while (effect)
			{
				num = __builtin_ctzll(effect);
				inc = delta::to_inc( pos - pieceList[num] );
				switch (num)
				{
					case 2:	case 3:

					if (std::abs(inc) == 16 || std::abs(inc) == 18) break;
					else
					{
						for (int i = 1; i < 9; i++)
						{
							sq =  pos + inc * i;
							if (square[sq] == piece::NONE)
							{
								attackBoard[color][sq]++;
								attackBoard[color][sq] ^= (1ULL << (num + 4));
							} 
							else
							{
								attackBoard[color][sq]++;
								attackBoard[color][sq] ^= (1ULL << (num + 4));
								break;
							}
						}
					}
					break;

					case 4:	case 5:

					if (std::abs(inc) == 17 || std::abs(inc) == 1) break;
					else
					{
						for (int i = 1; i < 9; i++)
						{
							sq =  pos + inc * i;
							if (square[sq] == piece::NONE)
							{
								attackBoard[color][sq]++;
								attackBoard[color][sq] ^= (1ULL << (num + 4));
							} 
							else
							{
								attackBoard[color][sq]++;
								attackBoard[color][sq] ^= (1ULL << (num + 4));
								break;
							}
						}
					}
					break;

					case 18: case 19: case 20: case 21:

					if (square[pieceList[num]] == piece::BL || square[pieceList[num]] == piece::WL)
					{
						for (int i = 1; i < 9; i++)
						{
							sq =  pos + inc * i;
							if (square[sq] == piece::NONE)
							{
								attackBoard[color][sq]++;
								attackBoard[color][sq] ^= (1ULL << (num + 4));
							} 
							else
							{
								attackBoard[color][sq]++;
								attackBoard[color][sq] ^= (1ULL << (num + 4));
								break;
							}
						}
					}
					break;
				}
				effect ^= (1ULL << num);
			}
		}
	}

	void Board::remove_long_effect(int color, int pos) {
		
		uint64_t effect;
		int num, inc, sq;

		if ((effect = attackBoard[color][pos] & EFFECT_LONG_MASK))
		{
			effect >>= 4;
			while (effect)
			{
				num = __builtin_ctzll(effect);
				inc = delta::to_inc( pos - pieceList[num] );
				switch (num)
				{
					case 2: case 3:

					if (std::abs(inc) == 16 || std::abs(inc) == 18) break;
					else
					{
						for (int i = 1; i < 9; i++)
						{
							sq =  pos + inc * i;
							if (square[sq] == piece::NONE)
							{
								attackBoard[color][sq]--;
								attackBoard[color][sq] ^= (1ULL << (num + 4));
							} 
							else
							{
								attackBoard[color][sq]--;
								attackBoard[color][sq] ^= (1ULL << (num + 4));
								break;
							}
						}
					}
					break;

					case 4: case 5:

					if (std::abs(inc) == 17 || std::abs(inc) == 1) break;
					else
					{
						for (int i = 1; i < 9; i++)
						{
							sq =  pos + inc * i;
							if (square[sq] == piece::NONE)
							{
								attackBoard[color][sq]--;
								attackBoard[color][sq] ^= (1ULL << (num + 4));
							} 
							else
							{
								attackBoard[color][sq]--;
								attackBoard[color][sq] ^= (1ULL << (num + 4));
								break;
							}
						}
					}
					break;

					case 18: case 19: case 20: case 21:

					if (square[pieceList[num]] == piece::BL || square[pieceList[num]] == piece::WL)
					{
						for (int i = 1; i < 9; i++)
						{
							sq =  pos + inc * i;
							if (square[sq] == piece::NONE)
							{
								attackBoard[color][sq]--;
								attackBoard[color][sq] ^= (1ULL << (num + 4));
							} 
							else
							{
								attackBoard[color][sq]--;
								attackBoard[color][sq] ^= (1ULL << (num + 4));
								break;
							}
						}
					}
					break;
				}
				effect ^= (1ULL << num);
			}
		}
	}

	void Board::move(int mv) {

		int me  = turn;
		int opp = color::opp(me);
		
		flip_turn(); // turn change
		
		int from = move::from(mv);
		int to = move::to(mv);
		int fKind = move::from_kind(mv);
		int tKind = move::to_kind(mv);
		int cap = move::capture(mv);

		if (fKind == piece::BK) kingPos[color::Black] = to;
		if (fKind == piece::WK) kingPos[color::White] = to;

		boardKeyStack[stage] = board_key();
		standKeyStack[stage] = stand_key();
		moveStack[stage] = mv;
		stage++;

		if (move::is_drop(mv)) {

			stand[me][piece::abs(fKind)]--;
			standKey -= hash::zob_hand[me][piece::abs(fKind)];

			remove_long_effect(color::Black, to);
			remove_long_effect(color::White, to);

			boardKey ^= hash::zob_board[piece::NONE][to];
			square[to] = tKind;
			boardKey ^= hash::zob_board[tKind][to];
			
			pieceList[ pieceStack[ piece::abs(fKind) ][ pieceStack[piece::abs(fKind)][0] ] ] = to;
			squareNum[to] = pieceStack[ piece::abs(fKind) ][ pieceStack[piece::abs(fKind)][0] ];
			pieceStack[piece::abs(fKind)][ pieceStack[piece::abs(fKind)][0] ] = 0;
			pieceStack[piece::abs(fKind)][0]--;
		
		} else {

			// remove old effects
			for (int i = 0; i < piece::incNum[fKind]; i++)
			{
				int inc = piece::Inc[fKind][i];
				assert(inc != 0);
				if (piece::is_slider( fKind , inc )) { // slide move effect
				
					int n = from;
					do {
						n += inc;
						attackBoard[me][n] -= 1; // remove effect num
						attackBoard[me][n] ^= (1ULL << (squareNum[from] + 4)); // remove effect piece num
					} while (square[n] == piece::NONE);
				}
				else  { // no slide move effect
				
					attackBoard[me][ from + inc ] -= 1; // remove effect num
					attackBoard[me][ from + inc ] ^= (1ULL << (squareNum[from] + 4)); // remove effect piece num
				}
			}
			boardKey ^= hash::zob_board[fKind][from];
			square[from] = piece::NONE;
			boardKey ^= hash::zob_board[piece::NONE][from];

			add_long_effect(color::Black, from);
			add_long_effect(color::White, from);

			if (cap != piece::NONE) {
				
				// remove old effects
				for (int i = 0; i < piece::incNum[cap]; i++)
				{
					int inc = piece::Inc[cap][i];
					assert(inc != 0);
					if (piece::is_slider(cap , inc)) // slide move effect
					{
						int n = to;
						do {
							n += inc;
							attackBoard[opp][n] -= 1; // remove effect num
							attackBoard[opp][n] ^= (1ULL << (squareNum[to] + 4)); // removeeffect piece num
						} while (square[n] == piece::NONE);
					}
					else // no slide move effect
					{
						attackBoard[opp][to + inc] -= 1; // remove effect num
						attackBoard[opp][to + inc] ^= (1ULL << (squareNum[to] + 4)); // remove effect piece num
					}
				}
				standKey += hash::zob_hand[me][piece::abs(cap) & ~(piece::PromoteBit)];
				stand[me][piece::abs(cap) & ~(piece::PromoteBit)]++;
				pieceStack[piece::abs(cap) & ~(piece::PromoteBit)][0]++; 
				pieceStack[piece::abs(cap) & ~(piece::PromoteBit)][pieceStack[piece::abs(cap) & ~(piece::PromoteBit)][0]] = squareNum[to];
				pieceList[ squareNum[to] ] = 0;
			}
			else {
				remove_long_effect(color::Black, to);
				remove_long_effect(color::White, to);
			}
			boardKey ^= hash::zob_board[cap][to];
			square[to] = tKind;
			boardKey ^= hash::zob_board[tKind][to];

			pieceList[ squareNum[from] ] = to;
			squareNum[ to ] = squareNum[from];
			squareNum[from] = 0;
		}
		// add new effects
		for (int i = 0; i < piece::incNum[tKind]; i++ ) {

			int inc = piece::Inc[tKind][i];
			assert( inc != 0 );
			if (piece::is_slider(tKind, inc)) { // slide move effect
			
				int n = to;
				do {
					n += inc;
					attackBoard[me][n] += 1; // add effect num
					attackBoard[me][n] ^= (1ULL << (squareNum[to] + 4)); // add effect piece num
				} while (square[n] == piece::NONE);
			}
			else { // no slide move effect
			
				attackBoard[me][to + inc] += 1; // add effect num
				attackBoard[me][to + inc] ^= (1ULL << (squareNum[to] + 4)); // add effect piece num
			}
		}
#if 0
		if (!is_ok(mv)) { std::cout << "move error" << std::endl; }
#endif
	}

	void Board::undo(int mv) {

		flip_turn(); // turn change

		int me = turn;
		int opp = color::opp(me);

		boardKeyStack[stage] = hash::NONE;
		standKeyStack[stage] = hash::NONE;
		moveStack[stage] = move::NONE;

		stage--;

		int from = move::from(mv);
		int to = move::to(mv);
		int fKind = move::from_kind(mv);
		int tKind = move::to_kind(mv);
		int cap = move::capture(mv);

		if (fKind == piece::BK) kingPos[color::Black] = from;
		if (fKind == piece::WK) kingPos[color::White] = from;

		// remove old effects
		for (int i = 0; i < piece::incNum[tKind]; i++)
		{
			int inc = piece::Inc[tKind][i];
			assert(inc != 0);
			if (piece::is_slider(tKind , inc)) // slide move effect
			{
				int n = to;
				do {
					n += inc;
					attackBoard[me][n] -= 1; // add effect num
					attackBoard[me][n] ^= (1ULL << (squareNum[to] + 4)); // add effect piece num
				} while (square[n] == piece::NONE);
			}
			else // no slide move effect
			{
				attackBoard[me][to + inc] -= 1; // add effect num
				attackBoard[me][to + inc] ^= (1ULL << (squareNum[to] + 4)); // add effect piece num
			}
		}

		if (move::is_drop(mv)) {

			stand[me][piece::abs(fKind)]++;
			standKey += hash::zob_hand[me][piece::abs(fKind)];
			boardKey ^= hash::zob_board[tKind][to];
			square[to] = piece::NONE;
			boardKey ^= hash::zob_board[piece::NONE][to];
			add_long_effect(color::Black, to);
			add_long_effect(color::White, to);
			pieceStack[piece::abs(fKind)][0]++;
			pieceStack[piece::abs(fKind)][ pieceStack[piece::abs(fKind)][0] ] = squareNum[to];
			squareNum[to] = 0;
			pieceList[ pieceStack[piece::abs(fKind)][ pieceStack[piece::abs(fKind)][0] ] ] = 0;
		}
		else {

			boardKey ^= hash::zob_board[tKind][to];
			square[to] = cap;
			boardKey ^= hash::zob_board[cap][to];
			squareNum[from] = squareNum[ to ];
			squareNum[ to ] = 0;
			pieceList[ squareNum[from] ] = from;
			
			if (cap != piece::NONE) {

				stand[me][piece::abs(cap) & ~(piece::PromoteBit)]--; // promtoe piece ? -> no promote piece
				standKey -= hash::zob_hand[me][piece::abs(cap) & ~(piece::PromoteBit)];
				squareNum[to] = pieceStack[piece::abs(cap)  & ~(piece::PromoteBit)][pieceStack[piece::abs(cap)  & ~(piece::PromoteBit)][0]];
				pieceList[ squareNum[to] ] = to;
				pieceStack[piece::abs(cap) & ~(piece::PromoteBit)][pieceStack[piece::abs(cap) & ~(piece::PromoteBit)][0]] = 0;
				pieceStack[piece::abs(cap) & ~(piece::PromoteBit)][0]--;

				// add new effects
				for (int i = 0; i < piece::incNum[cap]; i++ )
				{
					int inc = piece::Inc[cap][i];
					assert(inc != 0);
					if (piece::is_slider(cap , inc)) { // slide move effect
				
						int n = to;
						do {
							n += inc;
							attackBoard[opp][n] += 1; // add effect num
							attackBoard[opp][n] |= (1ULL << (squareNum[to] + 4)); // add effect piece num
						} while (square[n] == piece::NONE);
					}
					else { // no slide move effect
					
						attackBoard[opp][ to + inc ] += 1; // add effect num
						attackBoard[opp][ to + inc ] |= (1ULL << (squareNum[to] + 4)); // add effect piece num
					}
				}
			}
			else {

				add_long_effect(color::Black, to);
				add_long_effect(color::White, to);
			}
			boardKey ^= hash::zob_board[piece::NONE][from];
			square[from] = fKind;
			boardKey ^= hash::zob_board[fKind][from];
			remove_long_effect(color::Black, from);
			remove_long_effect(color::White, from);
			
			// add new effects
			for (int i = 0; i < piece::incNum[fKind]; i++) {

				int inc = piece::Inc[fKind][i];
				assert(inc != 0);
				if (piece::is_slider( fKind , inc )) // slide move effect
				{
					int n = from;
					do {
						n += inc;
						attackBoard[me][n] += 1; // add effect num
						attackBoard[me][n] |= (1ULL << (squareNum[from] + 4)); // add effect piece num
					} while (square[n] == piece::NONE);
				}
				else // no slide move effect
				{
					attackBoard[me][ from + inc ] += 1; // add effect num
					attackBoard[me][ from + inc ] |= (1ULL << (squareNum[from] + 4)); // add effect piece num
				}
			}
		}
	}

	void Board::move_null() {
		
		flip_turn(); // turn change

		boardKeyStack[stage] = board_key();
		standKeyStack[stage] = stand_key();
		moveStack[stage] = move::NONE;
		stage++;
	}

	void Board::undo_null() {

		flip_turn(); // turn change

		boardKeyStack[stage] = hash::NONE;
		standKeyStack[stage] = hash::NONE;
		moveStack[stage] = move::NONE;
		stage--;
	}

	bool Board::is_ok(int mv) {

		uint64_t attackBoard_[2][square::size] = {0};

		if (square_is(king_pos(color::Black)) != piece::BK || square_is(king_pos(color::White)) != piece::WK) 
		{ printf("kingPos_ERROR\n"); return false; }

		int i, j;   
		for (int x = 0; x < square::size; x++) {

			int sq = square_is(x);
			if ( sq == piece::Wall ) continue;
			int color = piece::color_is(sq); 

			for (i = 0; i < 8; i++)
			{
				int inc = piece::Inc[sq][i];
				if ( inc == 0 ) break;
				if ( piece::is_slider( sq , inc ) ) // slide move effect
				{
					int n = x;
					do {
						n += inc;
						attackBoard_[color][n] += 1; // add effect num
						attackBoard_[color][n] |= (1ULL << (square_num(x) + 4)); // add effect piece num
					} while ( square_is(n) == piece::Empty );
				}
				else // no slide move effect
				{
					attackBoard_[color][ x + inc ] += 1; // add effect num
					attackBoard_[color][ x + inc ] |= (1ULL << (square_num(x) + 4)); // add effect piece num
				}
			}
		}

		for (int i = 0; i < 2; i++) {
			for (int rank = 0; rank < 9; rank++) {
				int pos = 38 + 17 * rank;
				for (int file = 0; file < 9; file++)
				{
					j = pos + file;
					if (attack_board(i, j) != attackBoard_[i][j])
					{
						std::cout << "error color:" << i << " sq:" << square::notation(j) << std::endl;
						std::cout << "AB:" << attack_board(i, j) << " AB_:" << attackBoard_[i][j] << std::endl;

						return false;
					}
				}
			}
		}

		return true;
	}
}

namespace move_gen {

	class List {

	public:
		List() { clear(); }
		int size_is() { return size; } 
		void add(int mv, int sc);
		void sort();
		int move(int pos);
		bool contain(int mv);
		void clear() { size = 0; }
	private:
		int size;
		uint64_t moves[600]; // max legal moves is under 600
	};

	void List::add(int mv, int sc = 0) {
		// lower 32 bit is move , upper is score
		moves[size++] = (static_cast<uint64_t>(sc) << move::BITS) | mv; 
	}

	void List::sort() {
		std::sort(moves, moves + size, [](const uint64_t &x, const uint64_t &y){ return x > y; } );
	}

	int List::move(int pos) {
		return static_cast<int>(moves[pos] & move::MASK);
	}

	bool List::contain(int mv) {

		for (int pos = 0; pos < size_is(); pos++) {
			if (move(pos) == mv) {
				return true;
			}
		}

		return false;
	}

	bool piece_can_move(int cap, int opp) {
		return (cap == piece::Empty || (cap != piece::Wall && piece::color_is(cap) == opp));
	}

	bool no_promote(int from, int to, int fKind) { 

		switch (fKind) {
		
			case piece::BP: case piece::BB: case piece::BR:
				return !(square::coord_y(std::min(from, to)) <= 3);
			
			case piece::WP: case piece::WB: case piece::WR:
				return !(square::coord_y(std::max(from, to)) >= 7);
			
			case piece::BL: case piece::BN:
				return !(square::coord_y(std::min(from, to)) <= 2);
			
			case piece::WL: case piece::WN:
				return !(square::coord_y(std::max(from, to)) >= 8);
			
			default: 
				return true;
		}
	}

	bool promote(int from, int to, int fKind) {

		if (piece::color_is(fKind) == color::Black)
			return std::min(from, to) <= 80 && piece::can_promote(fKind); // min <= rank3
		else
			return std::max(from, to) >= 140 && piece::can_promote(fKind); // max >= rank7	
	}

	int mvv_lva(int mv) {

		int cap = move::capture(mv);
		int fKind = move::from_kind(mv);

		return piece::value(cap) * 1024 - piece::value(fKind); // Most Valuable Victim Least Valuable Attacker
	}

	bool escape_slider_line(board::Board &bd, int kingInc, int kingSq) {

		int attacker[16];
		int sq, piece;

		for (int i = 0; i < bd.in_check_all(bd.turn_is(), &attacker[0]); i++) {

			sq = bd.piece_list(attacker[i]);
			piece = bd.square_is(sq);
			if (!piece::is_slider(piece, kingInc)) continue; // no slider
			if (kingInc == delta::to_inc(kingSq - sq)) return true; // not avoid escape from attacker
		}
		return false;
	}

	bool is_pinned(board::Board &bd, int from) {

		int piece  = bd.square_is(from);
		int fri = piece::color_is(piece); // <- pinned ? piece color
		int ene  = color::opp(fri);
		int kingSq = bd.king_pos(fri);
		int inc    = delta::to_inc(kingSq - from);

		if (inc == 0 || std::abs(inc) >= 33) return false; // no line or knight line

		int sq = from; // search blocker
		do {sq += inc;} while (bd.square_is(sq) == piece::Empty);
		if (sq != kingSq) return false; // blocker 

		sq = from; // searcher attacker
		do {sq -= inc;} while (bd.square_is(sq) == piece::Empty);
		int attacker = bd.square_is(sq); // attacker?
		return (attacker != piece::Wall && piece::color_is(attacker) == ene && piece::is_slider(attacker, inc));
	}

	bool psuedo_move_is_legal(board::Board &bd, int mv) {

		if (move::is_drop(mv)) return true; // drop move is safety

		int fKind = move::from_kind(mv);
		int from  = move::from(mv);
		int to    = move::to(mv);
		int ME    = piece::color_is(fKind);

		if (fKind == piece::KING(ME)) return !bd.is_attack(ME, to);
		if (is_pinned(bd, from)) {

			int kingSq = bd.king_pos(ME);
			return delta::to_inc(kingSq - to) == delta::to_inc(kingSq - from); // same line ?
		}
		return true;
	}

	bool killer_is_ok(board::Board &bd, int killer) {

		if (killer == move::NONE) return false; // no killer 

		int to = move::to(killer);
		int fKind = move::from_kind(killer);

		if (bd.square_is(to) != piece::Empty) return false;
		if (move::is_drop(killer)) {

			if (bd.stand_is(bd.turn_is(), piece::abs(fKind)) == 0) return false; // no piece

			int file = 9 - square::coord_x(to);
			int color = piece::color_is(fKind);
			if (fKind == piece::PAWN(color)) {
				bool flag = false;
				for (int rank = 0; rank < 9; rank++) {

					if (bd.square_is(38 + 17 * rank + file) == fKind) {
						flag = true;
						break;
					}
				}
				if (flag) return false; // same file 2-pawns
			}
		}
		else { // no drop
			int from = move::from(killer);
			if (bd.square_is(from) != fKind) return false;
			int inc = delta::to_inc(to - from);
			int sq = from;
			do sq += inc; while (bd.square_is(sq) == piece::Empty && sq != to);
			if (sq != to) return false; // find blocker 
		}

		return true;
	}

	template<bool tactical> void add_move(List &ml, board::Board &bd, int from, int to, int cap, int kind); // 実体

	template<> void add_move<true> (List &ml, board::Board &bd, int from, int to, int cap, int kind) {

		if (!piece_can_move(cap, color::opp(bd.turn_is()))) return;

		int mv;

		if (promote(from, to, kind)) { // tactical => promote

			mv = move::make(from, to, cap, kind + piece::PromoteBit, kind);
			ml.add(mv, mvv_lva(mv)); // mvvlva
		}

		if (cap != piece::Empty && no_promote(from, to, kind)) { // tactical => cap

			mv = move::make(from, to, cap, kind, kind);
			ml.add(mv, mvv_lva(mv)); // mvvlva
		}
	}

	template<> void add_move<false>(List &ml, board::Board &bd, int from, int to, int cap, int kind) {
		
		if (!piece_can_move(cap, color::opp(bd.turn_is()))) return;

		if (cap == piece::Empty && no_promote(from, to, kind)) { // not tactical => no cap && no promote

			int mv = move::make(from, to, piece::Empty, kind, kind);
			ml.add(mv, bd.history_is(kind, to));  // history
		}
	}

	void drop(List &ml, board::Board &bd, int *space = nullptr, int spaceNum = 0) {

		int me = bd.turn_is();
		if (!(bd.stand_is(me, piece::BP) | bd.stand_is(me, piece::BL) | bd.stand_is(me, piece::BN) | bd.stand_is(me, piece::BS) 
			| bd.stand_is(me, piece::BG) | bd.stand_is(me, piece::BB) | bd.stand_is(me, piece::BR))) return;

		bool nf = false; //todo : fix
		if (!space) {
			nf = true;
			space = new int[square::size];
			for (int sq = 38; sq <= 182; sq++) {
				if (bd.square_is(sq) == piece::Empty) space[spaceNum++] = sq;
			}
		}

		if (bd.stand_is(me, piece::BP) > 0) {

			bool pawnFile[10] = { false, false, false, false, false, false, false, false, false, false };
			for (int i = 22; i < 40; i++) {

				if (bd.piece_list(i) != 0 && piece::color_is(bd.square_is(bd.piece_list(i))) == me) pawnFile[square::coord_x(bd.piece_list(i))] = true;
			}
			int banY = me == color::Black ? 1 : 9;
			int to, pawn, mv;
			for ( int i = 0; i < spaceNum; i++ ) {

				to = space[i];
				if (pawnFile[square::coord_x(to)]) continue;
				if (square::coord_y(to) == banY) continue;
				pawn = piece::PAWN(me);
				mv = move::make(pawn, to, piece::Empty, pawn, pawn);
				ml.add(mv, bd.history_is(pawn, to));  // history
			}
		}

		if (bd.stand_is(me, piece::BL) > 0) {

			int banY = me == color::Black ? 1 : 9;
			int kind = me == color::Black ? piece::BL : piece::BL + piece::WhiteBit;
			int to, mv;
			for (int i = 0; i < spaceNum; i++) {

				to = space[i];
				if (square::coord_y(to) == banY) continue;
				mv = move::make(kind, to, piece::Empty, kind, kind);
				ml.add(mv, bd.history_is(kind, to));  // history
			}
		}

		if (bd.stand_is(me, piece::BN) > 0) {

	#define banY(Y) (me == color::Black ? (Y <= 2) : (Y >= 8))
			int kind = me == color::Black ? piece::BN : piece::BN + piece::WhiteBit;
			int to, mv;
			for (int i = 0; i < spaceNum; i++) {

				to = space[i];
				if (banY(square::coord_y(to))) continue;
				mv = move::make(kind, to, piece::Empty, kind, kind);
				ml.add(mv, bd.history_is(kind, to));  // history
	#undef banY
			}
		}

		for (int absKind = piece::BS; absKind <= piece::BR; absKind++) {

			if (bd.stand_is(me, absKind) == 0) continue;
			int kind = me == color::Black ? absKind : absKind + piece::WhiteBit;
			int to, mv;
			for (int i = 0; i < spaceNum; i++) {

				to = space[i];
				mv = move::make(kind, to, piece::Empty, kind, kind);
				ml.add(mv, bd.history_is(kind, to));  // history
			}
		}

		if (nf) delete[] space;
	}
	
	template <bool tactical> void evasion(List &ml, board::Board &bd, int *attacker, int attackerCount) {

		int kingTo, kingCap;
		int me = bd.turn_is();
		int opp = color::opp(me);
		int kingSq = bd.king_pos(me);
		int king = piece::KING(me);

		for (int i = 0; i < 8; i++)	{

			kingTo = kingSq + piece::Inc[king][i];
			kingCap = bd.square_is(kingTo);
			if (!piece_can_move(kingCap, opp) || bd.attack_board(opp, kingTo) || escape_slider_line(bd, piece::Inc[king][i], kingSq)) continue; // fix
			add_move<tactical>(ml, bd, kingSq, kingTo, kingCap, king);
		}

		if ((bd.attack_board(opp, kingSq) & (uint64_t)0b1111) == 2) {

			// in this case, all drop moves are illegal
			ml.sort();
			return;
		}

		
		int inc = delta::to_inc((bd.piece_list(attacker[0]) - kingSq));
		int to, cap, from, piece;
		int space[32],spaceNum = 0;
		int attacker_[32];
		for (to = kingSq + inc; (cap = bd.square_is(to)) == piece::Empty; to += inc) { // slide ?
		
			space[spaceNum++] = to;
			for (int i = 0; i < bd.is_attack_all(me, &attacker_[0], to); i++) {

				from = bd.piece_list(attacker_[i]);
				if (from == kingSq) continue; // king move already done
				piece = bd.square_is(from);
				add_move<tactical>(ml, bd, from, to, cap, piece);
			}
		}

		for (int i = 0; i < bd.is_attack_all(me, &attacker_[0], to); i++) {

			from = bd.piece_list(attacker_[i]);
			if (from == kingSq) continue; // king move already done
			piece = bd.square_is(from);
			add_move<tactical>(ml, bd, from, to, cap, piece);
		}

		if (!tactical) drop(ml, bd, &space[0], spaceNum); // add drop blocker
		ml.sort();
	}

	template <bool tactical> void all_in_no_check(List &ml, board::Board &bd) {

		int kind, inc, from, to, cap;
		int opp = color::opp(bd.turn_is());

		for (int i = 0; i < 40; i++) {

			from = bd.piece_list(i);
			kind = bd.square_is(from);
			if (bd.piece_list(i) == 0 || piece::color_is(kind) == opp) continue;
			
			switch (kind) {

				case piece::BHO: case piece::WHO: case piece::BDR: case piece::WDR:

				for (int j = 0; j < 4; j++)
					add_move<tactical>(ml, bd, from, from + piece::IncNonSlider[kind][j], bd.square_is(from + piece::IncNonSlider[kind][j]), kind);
				
				case piece::BB:  case piece::WB: case piece::BR: case piece::WR:
				
				for ( int j = 0; j < 4; j++ ) {

					inc = piece::IncSlider[kind][j];
					for (to = from + inc; (cap = bd.square_is(to)) == piece::Empty; to += inc)
						add_move<tactical>(ml, bd, from, to, cap, kind);
					add_move<tactical>(ml, bd, from, to, cap, kind);
				}
				break;
				
				case piece::BL: case piece::WL:

				inc = piece::IncSlider[kind][0];
				for (to = from + inc; (cap = bd.square_is(to)) == piece::Empty; to += inc)
					add_move<tactical>(ml, bd, from, to, cap, kind);
				add_move<tactical>(ml, bd, from, to, cap, kind);

				break;

				default:

				for (int j = 0; j < piece::incNum[kind]; j++)
					add_move<tactical>(ml, bd, from, from + piece::Inc[kind][j], bd.square_is(from + piece::Inc[kind][j]), kind);
				break;
			}
		}
		if (!tactical) drop(ml, bd);
		ml.sort();
	}

	void all(List &ml, board::Board &bd) {

		 bool inCheck = bd.in_check(); 
		 
		 if (inCheck) {
			 int attacker[8], attackerCount = 0;
			 attackerCount = bd.in_check_all(bd.turn_is(), &attacker[0]);
			 evasion<true>(ml, bd, &attacker[0], attackerCount);
			 evasion<false>(ml, bd, &attacker[0], attackerCount);
		 } else {
			 all_in_no_check<true>(ml, bd);
			 all_in_no_check<false>(ml, bd);
		 }
	 }

	 void quiet(List &ml, board::Board &bd, int depth) {

		 int attacker[32], attackerCount = 0;
		 attackerCount = bd.in_check_all(bd.turn_is(), &attacker[0]);

		 if (attackerCount == 0) { // not in_check
		 	#define MustPromote(m) (!(move::from_kind(m) == move::to_kind(m) && promote(move::from(m), move::to(m), move::from_kind(m)))) // must promote
			#define Tanase(m) (!(depth <= -2 && move::capture(m) == piece::PAWN(color::opp(bd.turn_is())) && !bd.recapture(move::to(m) && !bd.is_check_move(mv))))

			List temp;
			all_in_no_check<true>(temp, bd);

			int mv;

			for (int i = 0; i < temp.size_is(); i++) {

				mv = temp.move(i);
				if (MustPromote(mv) && Tanase(mv)) ml.add(mv);
			}
			 
		 } else { // in check

		 	evasion<true>(ml, bd, &attacker[0], attackerCount);
			evasion<false>(ml, bd, &attacker[0], attackerCount);
		 }
	 }
}

namespace move {

	int from_sfen(board::Board &bd, const std::string& sfen) {

		const std::string sfenDrop = "XPLNSGBR",sfenRank = "abcdefghi";
		std::istringstream command(sfen);
		std::string token;
		command >> token;
		
		int fromR, fromF, from, toR, toF, to, fKind, tKind, cap;
		
		if ('0' < token[0] && token[0] <= '9') {

			fromF = 9 - (token[0]-'0'); // move
			fromR = (int)sfenRank.find(token[1]);
			from = 38 + 17 * fromR + fromF;
			fKind = bd.square_is(from);
		}
		else  {

			from = 0;
			fKind = (int)sfenDrop.find(token[0]);
			if (bd.turn_is() == color::White) fKind += piece::WhiteBit; 
		}

		toF = 9 - (token[2]-'0'); // move
		toR = (int)sfenRank.find(token[3]);
		to = 38 + 17 * toR + toF;
		cap = bd.square_is(to);

		if (token[4] == '+') tKind = fKind +  piece::PromoteBit;
		else tKind = fKind;

		return make(from, to, cap, tKind, fKind);
	}

	int find_hidden_attacker(board::Board &bd, int mv, int color, int *attacker) {

		int attackerNum = 0;
		if (move::is_drop(mv)) return attackerNum; // drop move can not add hidden attaker

		int from = move::from(mv);
		int to  = move::to(mv);
		int inc = delta::to_inc(to - from);

		if (inc != 0 && std::abs(inc) < 33)	{

			int sq = from;
			do sq -= inc; while (bd.square_is(sq) == piece::Empty);
			int att = bd.square_is(sq);

			if (att != piece::Wall && piece::color_is(att) == color && piece::is_slider(att, inc)) {
				
				attacker[attackerNum++] = bd.square_num(sq);
				return attackerNum;
			}
		}
		return attackerNum;
	}

	int see_rec(board::Board &bd, int to, int color, int pieceOnTo, int* attacker, int* defender, int attackerCount, int defenderCount) {

		if (attackerCount == 0) return 0; // no attacker
	
		int value = piece::exchange_value(pieceOnTo); // recaputure
		int i = attackerCount - 1; // min value piece index
		int from = bd.piece_list(attacker[i]);
		int fKind = bd.square_is(from);
		attackerCount--; // remove min value piece => KAMIKAZE attack

		int tKind = move_gen::promote(from, to, fKind) ? fKind + piece::PromoteBit : fKind;
		value += piece::value(tKind) - piece::value(fKind); // promote => value up

		return std::max(value - see_rec(bd, to, color::opp(color), tKind, defender, attacker, defenderCount, attackerCount), 0); // standpatOK
	}

	int see(board::Board &bd, int mv) {
		
		int from = move::from(mv);
		int to = move::to(mv);
		int fKind = move::from_kind(mv);
		int tKind = move::to_kind(mv);
		int cap = move::capture(mv);

		int att = piece::color_is(fKind);
		int def = color::opp(att);

		int value = (piece::value(tKind) - piece::value(fKind)) + piece::exchange_value(cap); // = (promote + capture) value

		int attacker[32], defender[32], attackerCount, defenderCount;
        defenderCount = bd.is_attack_all(def, &defender[0], to); // by-effect
        defenderCount += find_hidden_attacker(bd, mv, def, &defender[defenderCount]);

        if (defenderCount == 0) return value; // no defender -> stop SEE

        attackerCount = bd.is_attack_all(att, &attacker[0], to, bd.square_num(from)); // by-effect excepting moved piece
        attackerCount += find_hidden_attacker(bd, mv, att, &attacker[attackerCount]);

        std::sort(attacker, attacker + attackerCount, [&bd](const int& x, const int& y){ return piece::value(bd.square_is(bd.piece_list(x))) < piece::value(bd.square_is(bd.piece_list(y)));}); // sort by piece value
        std::sort(defender, defender + defenderCount, [&bd](const int& x, const int& y){ return piece::value(bd.square_is(bd.piece_list(x))) < piece::value(bd.square_is(bd.piece_list(y)));}); // sort by piece value

        return value - see_rec(bd, to, def, tKind, &defender[0], &attacker[0], defenderCount, attackerCount); // counter
	}
}

namespace move_picker {

	enum Inst {
		GEN_TACTICAL_EVASION,
		GEN_NON_TACTICAL_EVASION,
        GEN_TRANS,
        GEN_TACTICAL,
        GEN_KILLER,
        GEN_CHECK,
		GEN_NON_TACTICAL,
        GEN_QUIET,
        GEN_CAPTURE,
        GEN_BAD,
        GEN_END,
        POST_MOVE,
        POST_MOVE_SEE,
        POST_KILLER,
        POST_KILLER_SEE,
        POST_BAD,
        // debug
        GEN_ALL,
    };

	const Inst Prog_Main[]    = { GEN_TRANS, POST_KILLER, GEN_TACTICAL, POST_MOVE_SEE, GEN_KILLER, POST_KILLER, GEN_BAD, POST_BAD, GEN_NON_TACTICAL, POST_MOVE, GEN_END };
    const Inst Prog_QS[]      = { GEN_TRANS, POST_KILLER, GEN_QUIET, POST_MOVE_SEE, GEN_END };
    const Inst Prog_Evasion[] = { GEN_TACTICAL_EVASION, POST_MOVE, GEN_NON_TACTICAL_EVASION, POST_MOVE, GEN_END };
    // debug
    const Inst Prog_Debug[]   = { GEN_TRANS, POST_KILLER, GEN_ALL, POST_MOVE, GEN_END };

	class List {

	public:
		void init(board::Board & bd, const bool in_check, int trans_move, int depth);
        int next();
	private:
		board::Board *p_board;
		int p_trans_move;
		int p_pos;
		int p_depth;
		const Inst * p_ip;
        Inst p_gen;
        Inst p_post;
        move_gen::List p_todo;
        move_gen::List p_done;
        move_gen::List p_bad;

		bool gen();
		bool post(int mv);

	};

	bool List::gen() {

		p_todo.clear();
		p_pos = 0;

		switch (p_gen) { {

		} case GEN_TACTICAL_EVASION: {

			int attacker[8], attackerCount = 0;
			attackerCount = p_board->in_check_all(p_board->turn_is(), &attacker[0]);

			move_gen::evasion<true>(p_todo, *p_board, &attacker[0], attackerCount);

			break;

		} case GEN_NON_TACTICAL_EVASION: {

			int attacker[8], attackerCount = 0;
			attackerCount = p_board->in_check_all(p_board->turn_is(), &attacker[0]);

			move_gen::evasion<false>(p_todo, *p_board, &attacker[0], attackerCount);

			break;

		} case GEN_TRANS: {

			int mv = p_trans_move;

			if (mv != move::NONE && move_gen::killer_is_ok(*p_board, p_trans_move)) {
				p_todo.add(mv);
			}

			break;

		} case GEN_TACTICAL: {

			move_gen::all_in_no_check<true>(p_todo, *p_board);

			break;

		} case GEN_KILLER: {

			int k0 = p_board->killer_is(p_board->stage_is(), 0);

			if (k0 != move::NONE && move_gen::killer_is_ok(*p_board, k0)) {
				p_todo.add(k0);
			}

			int k1 = p_board->killer_is(p_board->stage_is(), 1);

			if (k0 != move::NONE && move_gen::killer_is_ok(*p_board, k1)) {
				p_todo.add(k1);
			}

			break;

		} case GEN_CHECK: {

			break;

		} case GEN_NON_TACTICAL: {

			move_gen::all_in_no_check<false>(p_todo, *p_board);

			break;

		} case GEN_QUIET: {

			move_gen::quiet(p_todo, *p_board, p_depth);

			break;

		} case GEN_CAPTURE:{
			
			break;
		
		} case GEN_BAD: {

			p_todo = p_bad;

			break;

		} case GEN_ALL: {

			move_gen::all_in_no_check<true>(p_todo, *p_board);
			move_gen::all_in_no_check<false>(p_todo, *p_board);

			break;

		} case GEN_END: {

			return false;

		} default: {

			assert(false);

		} }

		return true;
	}

	bool List::post(int mv) {

		switch (p_post) { {

		} case POST_MOVE: {

			if (p_done.contain(mv)) {
				return false;
			}

			if (!move_gen::psuedo_move_is_legal(*p_board, mv)) {
				return false;
			}

			break;

		} case POST_MOVE_SEE: {

			if (p_done.contain(mv)) {
				return false;
			}

			if (!move_gen::psuedo_move_is_legal(*p_board, mv)) {
				return false;
			}

			if (move::see(*p_board, mv) < 0) {
				p_bad.add(mv);
				return false;
			}

			break;

		} case POST_KILLER: {

			if (p_done.contain(mv)) {
				return false;
			}

			if (!move_gen::psuedo_move_is_legal(*p_board, mv)) {
				return false;
			}

			p_done.add(mv);

			break;

		} case POST_KILLER_SEE: {

			if (p_done.contain(mv)) {
				return false;
			}

			if (!move_gen::psuedo_move_is_legal(*p_board, mv)) {
				return false;
			}

			p_done.add(mv);

			if (move::see(*p_board, mv) < 0) {
				p_bad.add(mv);
				return false;
			}

			break;

		} case POST_BAD: {

			break;

		} default: {

			assert(false);

		} }

		return true;
	}

	void List::init(board::Board & bd, const bool in_check, int trans_move, int depth) {

		p_board = &bd;
		p_trans_move = trans_move;
		p_depth = depth;

		if (false) {
		} else if (in_check) { // in check
			
			p_ip = Prog_Evasion;
		
		} else if (depth <= 0) {
			
			p_ip = Prog_QS;
		
		} else {
#if 1
			p_ip = Prog_Main;
#else 
			p_ip = Prog_Debug;
#endif
		}

		p_todo.clear();
		p_done.clear();
		p_bad.clear();

		p_pos = 0;
	}

	int List::next() {

		while (true) {

			while (p_pos >= p_todo.size_is()) {

				p_gen  = *p_ip++;
				p_post = *p_ip++;

				if (!gen()) return move::NONE;
			}

			int mv = p_todo.move(p_pos++);
			if (post(mv)) return mv;
		}
	}
}

namespace engine {

	struct Engine {

		int hash;
		bool ponder;
		int threads;
	};

	Engine engine; // global
	
	void init() {

		engine.hash = 512;
		engine.ponder = false;
		engine.threads = 8;
	}
}

namespace trans {

	enum Bound {
		BOUND_NONE,
		BOUND_UPPER,
		BOUND_LOWER,
		BOUND_EXACT = BOUND_UPPER | BOUND_LOWER
	};

	#pragma pack(1) // block compiler alignment
	struct TTEntry
	{
	private:
		friend class PositionTable;
		uint32_t key32;
		uint32_t move32;
		int16_t  value16;
		int16_t  eval16;
		uint8_t  genBound8;
		int8_t   depth8;
	public:
		uint32_t move() const { return move32;  }
		int16_t value() const { return value16; }
		int16_t eval()  const { return eval16;  }
		int8_t  depth() const { return depth8;  }
		uint8_t bound() const { return genBound8; }

		void save(uint64_t k, int16_t v, uint8_t b, int8_t d, uint32_t m, int16_t ev, uint8_t g)
		{
			// preserve any existing move for the same position 
			if (m || (k >> 32) != key32) move32 = (uint32_t)m;

			// Don't overwrite more valuable entries
			if ((k >> 32) != key32 || d > depth8 - 2 || b == BOUND_EXACT) {
				key32     = (uint32_t)(k >> 32);
				value16   = (int16_t)v;
				eval16    = (int16_t)ev;
				genBound8 = (uint8_t)(g | b);
				depth8    = (int8_t)d;  
			}
		}
	};
	#pragma pack()

	class PositionTable 
	{
		static const int CacheLineSize = 64;
		static const int ClusterSize = 4;

		struct Cluster
		{
			TTEntry entry[ClusterSize];
			char padding[8]; // align to a divisor of CacheLineSize
		};

	private:
		size_t clusterCount;
		Cluster* table;
		void* mem;
		uint8_t generation8; // Size must be not bigger than TTEntry::genBound8

	public:
		~PositionTable() { free(mem); }
		void new_search() { generation8 += 4; } // lower 2 bits are used by Bound
		uint8_t generation() const { return generation8; }
		TTEntry* probe(const uint64_t key, bool& found) const;
		// int hashfull() const;
		void resize(size_t mbSize);
		void clear();

		// The lowest order bits of the key are used to get the index of the cluster
		TTEntry* first_entry(const uint64_t key) const {
			return &table[(size_t)key & (clusterCount - 1)].entry[0];
		}
	};

	int msb64(const uint64_t v) { return 63 - __builtin_clzll(v); }

	void PositionTable::resize(size_t mbSize)
	{
		size_t newClusterCount = size_t(1) << msb64((uint64_t)((mbSize * 1024 * 1024) / sizeof(Cluster)));
		
		if (newClusterCount == clusterCount) return;

		clusterCount = newClusterCount;

		free(mem);

		mem = calloc(clusterCount * sizeof(Cluster) + CacheLineSize - 1, 1);

		if (!mem)
		{
			std::cerr << "Failed to allocate " << mbSize << "MB for TPTable" << std::endl;
			exit(1);
		}

		table = (Cluster*)((uintptr_t(mem) + CacheLineSize - 1) & ~(CacheLineSize - 1));
	}

	void PositionTable::clear()
	{
		std::memset(table, 0, clusterCount * sizeof(Cluster));
	}

	TTEntry* PositionTable::probe(const uint64_t key, bool& found) const
	{
		TTEntry* const tte = first_entry(key);
		const uint32_t key32 = key >> 32; // convert to high 32bit key

		for (int i = 0; i < ClusterSize; ++i)
		{
			if (!tte[i].key32 || tte[i].key32 == key32)
			{
				if ((tte[i].genBound8 & 0xFC) != generation8 && tte[i].key32) tte[i].genBound8 = uint8_t(generation8 | tte[i].bound()); // refresh

				return found = (bool)tte[i].key32, &tte[i]; // found
			}
		}

		// not found entry and place
		TTEntry* replace = tte;
		for (int i = 1; i < ClusterSize; ++i)
		{
			if (replace->depth8 - ((259 + generation8 - replace->genBound8) & 0xFC) * 2
				> tte[i].depth8 - ((259 + generation8 -   tte[i].genBound8) & 0xFC) * 2) replace = &tte[i];
		}

		return found = false, replace;
	}
}

namespace eval {

	const int PieceVal[] = 
    {
        0, +90, +315, +405, +495, +540, +855, +990, +15000, +540, +540, +540, +540, 0, +945, +1395,
        0, -90, -315, -405, -495, -540, -855, -990, -15000, -540, -540, -540, -540, 0, -945, -1395
    };

    const int StandPieceValue[] = { 0, 90, 315, 405, 495, 540, 855, 990 };

    enum { nsquare = 81 };
    enum {
        f_hand_pawn   = 0, // 0
		e_hand_pawn   = f_hand_pawn   + 19,
		f_hand_lance  = e_hand_pawn   + 19,
		e_hand_lance  = f_hand_lance  +  5,
		f_hand_knight = e_hand_lance  +  5,
		e_hand_knight = f_hand_knight +  5,
		f_hand_silver = e_hand_knight +  5,
		e_hand_silver = f_hand_silver +  5,
		f_hand_gold   = e_hand_silver +  5,
		e_hand_gold   = f_hand_gold   +  5,
		f_hand_bishop = e_hand_gold   +  5,
		e_hand_bishop = f_hand_bishop +  3,
		f_hand_rook   = e_hand_bishop +  3,
		e_hand_rook   = f_hand_rook   +  3,
		fe_hand_end   = e_hand_rook   +  3,

        f_pawn        = fe_hand_end,
		e_pawn        = f_pawn        + 81,
		f_lance       = e_pawn        + 81,
		e_lance       = f_lance       + 81,
		f_knight      = e_lance       + 81,
		e_knight      = f_knight      + 81,
		f_silver      = e_knight      + 81,
		e_silver      = f_silver      + 81,
		f_gold        = e_silver      + 81,
		e_gold        = f_gold        + 81,
		f_bishop      = e_gold        + 81,
		e_bishop      = f_bishop      + 81,
		f_horse       = e_bishop      + 81,
		e_horse       = f_horse       + 81,
		f_rook        = e_horse       + 81,
		e_rook        = f_rook        + 81,
		f_dragon      = e_rook        + 81,
		e_dragon      = f_dragon      + 81,
		fe_end        = e_dragon      + 81,


        f_king = fe_end,
        e_king = f_king + nsquare,
        fe_end2 = e_king + nsquare // 玉も含めた末尾の番号。  
    };

    int32_t kk[nsquare][nsquare][2];
    int16_t kpp[nsquare][fe_end][fe_end][2];
    int32_t kkp[nsquare][nsquare][fe_end][2];

    static const struct {
        
        int f_pt, e_pt;
        
        } base_tbl[] = {
            {-1      , -1      },	//  0:---
            {f_pawn  , e_pawn  },	//  1:Bpawn
            {f_lance , e_lance },	//  2:Blance
            {f_knight, e_knight},	//  3:Bknight
            {f_silver, e_silver},	//  4:Bsilver
            {f_gold  , e_gold  },	//  5:Bgold
            {f_bishop, e_bishop},	//  6:Bbishop
            {f_rook  , e_rook  },	//  7:Brook
            {-1      , -1      },	//  8:Bking
            {f_gold  , e_gold  },	//  9:Bpropawn
            {f_gold  , e_gold  },	// 10:Bprolance
            {f_gold  , e_gold  },	// 11:Bproknight
            {f_gold  , e_gold  },	// 12:Bprosilver
            {-1      , -1      },	// 13:--
            {f_horse , e_horse },	// 14:Bhorse
            {f_dragon, e_dragon},	// 15:Bdragon
            {-1      , -1      },   // 16:--
            {e_pawn  , f_pawn  },	// 17:Wpawn
            {e_lance , f_lance },	// 18:Wlance
            {e_knight, f_knight},	// 19:Wknight
            {e_silver, f_silver},	// 20:Wsilver
            {e_gold  , f_gold  },	// 21:Wgold
            {e_bishop, f_bishop},	// 22:Wbishop
            {e_rook  , f_rook  },	// 23:Wrook
            {-1      , -1      },	// 24:--
            {e_gold  , f_gold  },	// 25:Wpropawn
            {e_gold  , f_gold  },	// 26:Wprolance
            {e_gold  , f_gold  },	// 27:Wproknight
            {e_gold  , f_gold  },	// 28:Wprosilver
            {-1      , -1      },	// 29:---
            {e_horse , f_horse },	// 30:Whorse
            {e_dragon, f_dragon}	// 31:Wdragon
    };

    const short z2sq[] = //変換用テーブル 
    {
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            72, 63, 54, 45, 36, 27, 18,  9,  0, -1, -1, -1, -1, -1, -1, -1, -1,
            73, 64, 55, 46, 37, 28, 19, 10,  1, -1, -1, -1, -1, -1, -1, -1, -1,
            74, 65, 56, 47, 38, 29, 20, 11,  2, -1, -1, -1, -1, -1, -1, -1, -1,
            75, 66, 57, 48, 39, 30, 21, 12,  3, -1, -1, -1, -1, -1, -1, -1, -1,
            76, 67, 58, 49, 40, 31, 22, 13,  4, -1, -1, -1, -1, -1, -1, -1, -1,
            77, 68, 59, 50, 41, 32, 23, 14,  5, -1, -1, -1, -1, -1, -1, -1, -1,
            78, 69, 60, 51, 42, 33, 24, 15,  6, -1, -1, -1, -1, -1, -1, -1, -1,
            79, 70, 61, 52, 43, 34, 25, 16,  7, -1, -1, -1, -1, -1, -1, -1, -1,
            80, 71, 62, 53, 44, 35, 26, 17,  8, -1, -1, -1, -1, -1, -1, -1, -1,
    };

    #define I2HandPawn(hand)    ( hand == color::Black ? bd.stand_is(color::Black, piece::BP) : bd.stand_is(color::White, piece::BP))
    #define I2HandLance(hand)   ( hand == color::Black ? bd.stand_is(color::Black, piece::BL) : bd.stand_is(color::White, piece::BL))
    #define I2HandKnight(hand)  ( hand == color::Black ? bd.stand_is(color::Black, piece::BN) : bd.stand_is(color::White, piece::BN))
    #define I2HandSilver(hand)  ( hand == color::Black ? bd.stand_is(color::Black, piece::BS) : bd.stand_is(color::White, piece::BS))
    #define I2HandGold(hand)    ( hand == color::Black ? bd.stand_is(color::Black, piece::BG) : bd.stand_is(color::White, piece::BG))
    #define I2HandBishop(hand)  ( hand == color::Black ? bd.stand_is(color::Black, piece::BB) : bd.stand_is(color::White, piece::BB))
    #define I2HandRook(hand)    ( hand == color::Black ? bd.stand_is(color::Black, piece::BR) : bd.stand_is(color::White, piece::BR))

    #define Inv(sq) (80 - (sq))
    #define FV_SCALE 32

#if 0
	int evaluate(board::Board &bd) {

		int sq, sum = 0;

		for ( int rank = 0; rank < 9; rank++ ) {
			for ( int file = 0; file < 9; file++ ) {
				sq = 38 + 17 * rank + file;
				sum += PieceVal[bd.square_is(sq)]; 
			}
		}
		
		for (int i = piece::BP; i <= piece::BR; i++) {

			sum += StandPieceValue[i] * bd.stand_is(color::Black, i);
			sum -= StandPieceValue[i] * bd.stand_is(color::White, i);
		}

		return bd.turn_is() == color::Black ? sum : -sum;
	}
#else
	int evaluate(const board::Board &bd) {

        int scoreBoard;
        int scoreTurn;
        int sum[3][2];
        int material = 0;
        int sq_bk,sq_wk;
        int16_t pieceListFb[38],pieceListFw[38];
        int nlist = 0;
        int sq;
        int i,j;
	
        sq_bk = z2sq[bd.piece_list(0)];
        material += PieceVal[bd.square_is(bd.piece_list(0))];
        sq_wk = z2sq[bd.piece_list(1)];
        material += PieceVal[bd.square_is(bd.piece_list(1))];
        
        for (int i = piece::BP; i <= piece::BR; i++)
        {
            material += StandPieceValue[i] * bd.stand_is(color::Black,i);
            material -= StandPieceValue[i] * bd.stand_is(color::White,i);
        }

#define FOO(hand, Piece, list0_index, list1_index)    \
        for ( i = I2Hand##Piece(hand); i >= 1; --i) {     \
            pieceListFb[nlist] = list0_index + i;         \
            pieceListFw[nlist] = list1_index + i;         \
            ++nlist; \
        }

        FOO( color::Black, Pawn  , f_hand_pawn  , e_hand_pawn  )
        FOO( color::White, Pawn  , e_hand_pawn  , f_hand_pawn  )
        FOO( color::Black, Lance , f_hand_lance , e_hand_lance )
        FOO( color::White, Lance , e_hand_lance , f_hand_lance )
        FOO( color::Black, Knight, f_hand_knight, e_hand_knight)
        FOO( color::White, Knight, e_hand_knight, f_hand_knight)
        FOO( color::Black, Silver, f_hand_silver, e_hand_silver)
        FOO( color::White, Silver, e_hand_silver, f_hand_silver)
        FOO( color::Black, Gold  , f_hand_gold  , e_hand_gold  )
        FOO( color::White, Gold  , e_hand_gold  , f_hand_gold  )
        FOO( color::Black, Bishop, f_hand_bishop, e_hand_bishop)
        FOO( color::White, Bishop, e_hand_bishop, f_hand_bishop)
        FOO( color::Black, Rook  , f_hand_rook  , e_hand_rook  )
        FOO( color::White, Rook  , e_hand_rook  , f_hand_rook  )
#undef FOO
        
        for (i = 2; i < 40; i++) {

            const int z = bd.piece_list(i);
            if (z == 0) continue; //持ち駒を除く
            int pc = bd.square_is(z);
            material += PieceVal[pc];
            sq = z2sq[z];
            pieceListFb[nlist] = base_tbl[pc].f_pt + sq;
            pieceListFw[nlist] = base_tbl[pc].e_pt + Inv(sq);
            nlist++;
        }

        sum[0][0] = sum[0][1] = sum[1][0] = sum[1][1] = 0;

        //KK
        sum[2][0] = kk[sq_bk][sq_wk][0];
        sum[2][1] = kk[sq_bk][sq_wk][1];

        int16_t k0,k1,l0,l1;
        
        sum[2][0] += kkp[sq_bk][sq_wk][pieceListFb[0]][0];
        sum[2][1] += kkp[sq_bk][sq_wk][pieceListFb[0]][1];

        for (i = 1; i < 38; ++i)
        {
            k0 = pieceListFb[i];
            k1 = pieceListFw[i];

            for (j = 0; j < i; ++j)
            {
                l0 = pieceListFb[j];
                l1 = pieceListFw[j];
                sum[0][0] += kpp[sq_bk][k0][l0][0];
                sum[0][1] += kpp[sq_bk][k0][l0][1];
                sum[1][0] += kpp[Inv(sq_wk)][k1][l1][0];
                sum[1][1] += kpp[Inv(sq_wk)][k1][l1][1];
            }
            sum[2][0] += kkp[sq_bk][sq_wk][k0][0];
            sum[2][1] += kkp[sq_bk][sq_wk][k0][1];
        }

        sum[2][0] += material * FV_SCALE;
        
        scoreBoard = sum[0][0] - sum[1][0] + sum[2][0];
        scoreTurn  = sum[0][1] + sum[1][1] + sum[2][1];

        return bd.turn_is() == color::Black ? (+scoreBoard + scoreTurn) / FV_SCALE
										   :  (-scoreBoard + scoreTurn) / FV_SCALE;
        // return material;
    }
#endif

    void init() {

        FILE *fp;
        size_t size;

        //============================
        //             KK
        //============================
        fp = fopen( "KK_synthesized.bin" , "rb" );
        if ( fp == NULL ) { std::cout << "KK file is unknown" << std::endl; exit(1); }
        
        size = nsquare * nsquare * 2 * sizeof(int32_t) ;
        if( fread( kk, sizeof(char), size, fp ) != size )
        {
            fclose( fp );
            std::cout << "KK file size is incorrect" << std::endl;
            exit(1);
        }
        
        fclose( fp );

        //============================
        //             KPP
        //============================
        fp = fopen( "KPP_synthesized.bin" , "rb" );
        if ( fp == NULL ) { std::cout << "KPP file is unknown" << std::endl; exit(1); }
        
        size = nsquare * fe_end * fe_end * 2 * sizeof(int16_t) ;
        if ( fread( kpp, sizeof(char), size, fp ) != size )
        {
            fclose( fp );
            std::cout << "KPP file size is incorrect" << std::endl;
            exit(1);
        }
        fclose( fp );
        
        //============================
        //             KKP
        //============================
        fp = fopen( "KKP_synthesized.bin" , "rb" );
        if ( fp == NULL ) { std::cout << "KKP file is unknown" << std::endl; exit(1); }
        
        size = nsquare *nsquare * fe_end * 2 * sizeof(int32_t) ;
        if ( fread( kkp, sizeof(char), size, fp ) != size )
        {
            fclose( fp );
            std::cout << "KKP file size is incorrect" << std::endl;
            exit(1);
        }
        
        fclose( fp );
    }
}

namespace bench {
	
	void gen_move(board::Board &bd) {

		move_picker::List ml;
		ml.init(bd, bd.in_check(), move::NONE, 1) ;

		for (int move = ml.next(); move != move::NONE; move = ml.next()) {

			std::cout << move::to_can(move) << " ";
		}

		std::cout << std::endl;
	}
}

namespace BookDB {

	#define BOOKFILE "BOOK"

	const int DepthMax = 15;
	const int countMax = 41108;
	const int countMin = 5;
	std::unordered_multimap<uint64_t, int> dic;
	
	void write_dic(std::vector<int> &moves) {

		board::Board bd; 
		bd.set("lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1");
		for (int i = 0; i < DepthMax && i < moves.size(); i++) {

			bd.move(moves[i]);
			uint64_t key = bd.full_key();
			dic.insert(std::pair<uint64_t, int>(key, moves[i]));
		}
	}

	void init(){
		
		FILE *fp;
		size_t size;

		fp = fopen( BOOKFILE , "rb" );
		if (fp == NULL) {printf("BOOKFILE is not found\n"); return;}
		
		int *temp = new int[4801236];

		size = 4801236;
		size_t fileSize = fread(temp, sizeof(int), size, fp);

		if (fileSize != size){

			fclose( fp );
			std::cout << "BookFileSize=> " << fileSize << std::endl;
			std::cout << "Size=> " << size << std::endl;
			return;
		}
		
		int count = 0, index = 0;
		std::vector<int> moves;
		while (count < countMax) {

			int code = temp[index];
			if (code == 0){

				write_dic(moves);
				moves.clear();
				count++;
			}
			else {

				moves.push_back(code);
			}
			index++;
		}

		std::cout << "BookDB Init" << std::endl;
		delete[] temp;
		fclose( fp );
	}

	int out(board::Board &bd) {

		if (bd.stage_is() >= DepthMax) return move::NONE;
		
		move_picker::List ml;
		ml.init(bd, bd.in_check(), move::NONE, 1) ;

		std::vector<std::tuple<int, int>> roulette;
		for (int move = ml.next(); move != move::NONE; move = ml.next()) {

			bd.move(move);
			uint64_t key = bd.full_key();
			bd.undo(move);
			//ルーレットとか
			int tcount = dic.count(key);
			if (tcount > 0) { // contains key
			
				std::tuple<int, int> t = std::make_tuple(move, tcount);
				roulette.push_back(t);
			}
		}
		int sumAll = 0;
		for (auto t : roulette){ int &i = std::get<1>(t); sumAll += i; };
		if (sumAll < countMin) return move::NONE; // no trust 
		
		std::cout << "info string "; // usi
		for (auto t : roulette){
			int &tm = std::get<0>(t); int &tp = std::get<1>(t); std::cout << move::to_can(tm) << " " << (float)(tp * 100 / sumAll) << "% ";
		};
		std::cout << std::endl;

		int best;
		int sum = 0;
		std::random_device rd;
		std::mt19937 mt(rd());
		
		for (auto t : roulette){
			int &tm = std::get<0>(t); int &tp = std::get<1>(t);
			std::uniform_int_distribution<int> next(0,sum);
			sum += tp ;
			best = next(mt) > tp ? best : tm; 
		};

		return best;
	}
}

namespace search {

	struct SearchGlobal {

		board::Board bd;
		uint64_t nodes;
		trans::PositionTable tt;
		int threadCount;
		bool abort;
	};

	struct SearchLocal {

		board::Board bd;
		std::thread *thread = nullptr;
		uint64_t nodes;
		int max_depth;
		int best_move;
		int best_value;
		int id;
		std::vector<int> pv;

		void init(board::Board &cb) {
			bd = cb;
			nodes = 0;
			max_depth = 0;
			best_move  = move::NONE;
			best_value = score::NONE;
			pv.clear();
		}
	};

	SearchGlobal search_g;
	SearchLocal  *SearchCurrent = nullptr;

	enum { NodeAll = -1, NodePV = 0, NodeCut = +1, };

	#define ABS(x) ((x) < 0 ? -(x) : (x))
	#define MIN(X, Y)  ((X) < (Y) ? (X) : (Y))
	
	// constants and variables

	// main search

	static const bool UseDistancePruning = true;

	// transposition table

	static const bool UseTrans = true;
	static const int TransDepth = 1;
	static const bool UseExact = true;

	static const bool UseMateValues = true; // use mate values from shallower searches?

	// null move

	static /* const */ bool UseNull = true;
	static /* const */ bool UseNullEval = true; // true
	static const int NullDepth = 2; // was 2
	static /* const */ int NullReduction = 3;

	static /* const */ bool UseVer = true;
	static /* const */ bool UseVerEndgame = true; // true
	static /* const */ int VerReduction = 5; // was 3

	// move ordering

	static const bool UseIID = true;
	static const int IIDDepth = 3;
	static const int IIDReduction = 2;

	// extensions

	static const bool ExtendSingleReply = true; // true

	// razoring

	static const int RazorDepth = 1;
	static const int RazorMargin = 300; 

	// history pruning

	static /* const */ bool UseHistory = true;
	static const int HistoryDepth = 3; // was 3
	static const int HistoryPVDepth = 3; // was 3
	static const int HistoryMoveNb = 12; // was 3
	static /* const */ int HistoryValue = 9830; // 60%
	//static /* const */ int HistoryBound = 6400; // * 16384 + 50) / 100 10%=1638 15%=2458 20%=3277
	//static /* const */ bool UseExtendedHistory = true;
	//static const bool HistoryReSearch = true;

	// Late Move Reductions (Stockfish/Protector)
	static int quietPvMoveReduction[64][64];       /* [depth][played move count] */
	static int quietMoveReduction[64][64]; /* [depth][played move count] */

	// futility pruning

	static /* const */ bool UseFutility = true; // false
	static const int FutilityMargin = 100;
	static /* const */ int FutilityMargin1 = 200; // was 100
	static /* const */ int FutilityMargin2 = 300; // was 200
	static /* const */ int FutilityMargin3 = 950;

	// quiescence search

	static /* const */ bool UseDelta = true; // false
	static /* const */ int DeltaMargin = 50;

	void init() {
		
		search_g.bd.clear();
		search_g.nodes = 0;
		search_g.threadCount = 0;
		search_g.tt.resize(512);
		search_g.tt.clear();

		for (int i = 0; i < 64; i++) {    
			for (int j = 0; j < 64; j++) {
				if (i == 0 || j == 0){
					quietPvMoveReduction[i][j] = quietMoveReduction[i][j] = 0;
				}
				else{
					double pvReduction = log((double) (i)) * log((double) (j)) / 3.5;
					double nonPvReduction = 0.35 + log((double) (i)) * log((double) (j)) / 2.25;
					quietPvMoveReduction[i][j] = (int) (pvReduction >= 1.0 ? floor(pvReduction) : 0);
					quietMoveReduction[i][j] = (int) (nonPvReduction >= 1.0 ? floor(nonPvReduction) : 0);
				}
			}
		}
	}

	void pv_cat(std::vector<int> &dist, std::vector<int> &src, int move) {

		dist.clear();
		dist.push_back(move);
		dist.insert(dist.end(), src.begin(), src.end());
	}

	#define NODE_OPP(type) (-(type))
	#define DEPTH_MATCH(d1,d2) ((d1)>=(d2))

	int full_new_depth(int depth, bool isCheckMove, bool inCheck, int ordring, int nodeType);
	int full_search(board::Board &bd, int alpha, int beta, int depth, int height, std::vector<int> &pv, int node_type, bool extended, int ThreadId);
	int full_quiescence(board::Board &bd, int alpha, int beta, int depth, int height, std::vector<int> &pv, int ThreadId);
	int full_no_null(board::Board &bd, int alpha, int beta, int depth, int height, std::vector<int> &pv, int node_type, int trans_move, int * best_move, bool extended, int ThreadId);
	
	int full_new_depth(int depth, bool isCheckMove, bool inCheck, int ordring, int nodeType) {

		if (nodeType == NodePV && ordring == 1) { 

			if (inCheck) return depth; // one reply
		}

		if (isCheckMove) return depth;
		else return depth - 1;
	}

	int full_quiescence(board::Board &bd, int alpha, int beta, int depth, int height, std::vector<int> &pv, int ThreadId) {

		bool in_check;
		int old_alpha;
		int value, best_value;
		int best_move;
		int move;
		int opt_value;
		std::vector<int> new_pv;
		int probe_score, probe_depth;  
		int trans_move, trans_depth, trans_flags, trans_value;
		move_picker::List ml;
		// entry_t * found_entry;

		// init

   		SearchCurrent[ThreadId].nodes++;

		pv.clear();

		alpha = std::max(score::mate_value(height), alpha);
		beta = std::min(-1 * score::mate_value(height + 1), beta);
		if (alpha >= beta) return alpha; // distance pruning

		if (height > SearchCurrent[ThreadId].max_depth) SearchCurrent[ThreadId].max_depth = height;

		trans_move = move::NONE;

		in_check = bd.in_check();

		if (in_check) {
			depth++;
		} 

		 // more init

		old_alpha = alpha;
		best_value = score::NONE;
		best_move = move::NONE;

		opt_value = +score::EVALINF;

		if (!in_check) {

			// stand pat

			value = eval::evaluate(bd);

			best_value = value;
			if (value > alpha) {
				alpha = value;
				if (value >= beta) goto cut;
			}

			opt_value = value + DeltaMargin;

		}

		ml.init(bd, in_check, trans_move, depth);

		for (int move = ml.next(); move != move::NONE; move = ml.next()) {

			bd.move(move);
			value = -full_quiescence(bd, -beta, -alpha, depth - 1, height + 1, new_pv, ThreadId);
			bd.undo(move);

			if (search_g.abort) {
				pv_cat(pv,new_pv,move);
				return best_value;
			}

			if (value > best_value) {
				best_value = value;
				pv_cat(pv,new_pv,move);
				if (value > alpha) {
					alpha = value;
					best_move = move;
					if (value >= beta) goto cut;
				}
			}
		}

		// all_node

		if (best_value == score::NONE) {

			if (move::is_drop_pawn(bd.last_move())) return -1 * score::mate_value(height); // drop pawn mate
			else return score::mate_value(height);
		}

		cut:

			return best_value;
	}

	int full_no_null(board::Board &bd, int alpha, int beta, int depth, int height, std::vector<int> &pv, int node_type, int trans_move, int * best_move, bool extended, int ThreadId) {
	
		int value, best_value;
		int move;
		int new_depth;
		std::vector<int> new_pv;
		std::vector<int> searched;
		bool cap_extended;
		move_picker::List ml;

		// init

   		SearchCurrent[ThreadId].nodes++;

		pv.clear();

		if (height > SearchCurrent[ThreadId].max_depth) SearchCurrent[ThreadId].max_depth = height;

		*best_move = move::NONE;
   		best_value = score::NONE;

		ml.init(bd, bd.in_check(), trans_move, depth);

		for (int move = ml.next(); move != move::NONE; move = ml.next()) {

			searched.push_back(move);

			new_depth = full_new_depth(depth, bd.is_check_move(move), false, searched.size(), node_type);

			bd.move(move);
			value = -full_search(bd, -beta, -alpha, new_depth, height + 1, new_pv, NODE_OPP(node_type), false, ThreadId);
			bd.undo(move);

			if (value > best_value) {
				best_value = value;
				pv_cat(pv,new_pv,move);
				if (value > alpha) {
					alpha = value;
					*best_move = move;
					if (value >= beta) goto cut;
				}
			}
		}

		// all_node

		if (best_value == score::NONE) {

			if (move::is_drop_pawn(bd.last_move())) return -1 * score::mate_value(height); // drop pawn mate
			else return score::mate_value(height);
		}

		cut:

			return best_value;
	}

	int full_search(board::Board &bd, int alpha, int beta, int depth, int height, std::vector<int> &pv, int node_type, bool extended, int ThreadId) {

		bool in_check;
		bool single_reply;
		bool good_cap;
		int trans_move, trans_depth, trans_flags, trans_value;
		int old_alpha;
		int value, best_value;
		int move, best_move;
		int new_depth;
		int played_nb;
		int opt_value;
		bool reduced, cap_extended;
		std::vector<int> new_pv;
		std::vector<int> played;
		int FutilityMargin;
		int probe_score, probe_depth;
		int newHistoryValue;
		int threshold;
		int reduction;
		int standpat;
		move_picker::List ml;
		bool ttHit;
		bool dangerous;
		trans::TTEntry* found_entry;

		// horizon?

		if (depth <= 0) {
			return full_quiescence(bd, alpha, beta, 0, height, pv, ThreadId);
		}

		SearchCurrent[ThreadId].nodes++;
		pv.clear();

		alpha = std::max(score::mate_value(height), alpha);
		beta  = std::min(-1 * score::mate_value(height + 1), beta);
		if (alpha >= beta) return alpha; // distance pruning

		if (height > SearchCurrent[ThreadId].max_depth) SearchCurrent[ThreadId].max_depth = height;

		// transposition table
		trans_move = move::NONE;
		trans_value = score::NONE;
		ttHit = false;
		#define value_from_tt(v) (v == score::NONE ? score::NONE : v >= score::EVALINF ? v - height : v <= -score::EVALINF ? v + height : v)

		if (depth >= TransDepth) {
			found_entry = search_g.tt.probe(bd.full_key(), ttHit);
			if (ttHit) {
				trans_value = value_from_tt(found_entry->value());
				trans_move  = found_entry->move();
			}

			if (node_type != NodePV && ttHit && found_entry->depth() >= (uint8_t)depth && trans_value != score::NONE
				&& (trans_value >= beta ? (found_entry->bound() & trans::BOUND_LOWER) : (found_entry->bound() & trans::BOUND_UPPER))) {

					return trans_value;
			} 
		}

		// more init

		old_alpha = alpha;
		best_value = score::NONE;
		best_move = move::NONE;

		in_check = bd.in_check();

		standpat = eval::evaluate(bd);

		// beta pruning
#if 1
		if (node_type != NodePV && depth <= 3 && !score::is_mate(beta) && !in_check) {

			value = standpat - depth * 60;
			if (value >= beta) return value;
		}
#endif
		// null-move pruning

		if (depth >= NullDepth && node_type != NodePV) {

			if (!in_check
				&& !score::is_mate(beta)
				&& (depth <= NullReduction + 1 || standpat >= beta)) {
					
				// null-move-search

				new_depth = depth - NullReduction - 1;
				bd.move_null();
				value = -full_search(bd, -beta, -beta + 1, new_depth, height + 1, new_pv, NODE_OPP(node_type), false, ThreadId);
				bd.undo_null();

				if (depth > VerReduction) {

					if (value >= beta) {

						new_depth = depth - VerReduction;

						value = full_no_null(bd, alpha, beta, new_depth, height, new_pv, NodeCut, trans_move, &move, false, ThreadId);

						if (value >= beta) {
							played.push_back(move);
							best_move = move;
							best_value = value;
							std::copy(new_pv.begin(), new_pv.end(), std::back_inserter(pv));
							goto cut;
						}
					}
				}

				// pruning

				if (value >= beta) {

					if (value > +score::EVALINF) value = +score::EVALINF; // do not return unproven mates
					best_move = move::NONE;
					best_value = value;
					goto cut;
				}
			}
		}

		// razoring

		if (node_type != NodePV && !in_check && trans_move == move::NONE && depth <= 3){ 

			threshold = beta - RazorMargin - (depth-1)*39; // Values from Protector
			if (standpat < threshold) {
				value = full_quiescence(bd, threshold - 1, threshold, 0, height, pv, ThreadId); 
			if (value < threshold) // corrected - was < beta which is too risky at depth > 1
				return value;
			}
		}

		// Internal Iterative Deepening
#if 1
		if (depth >= IIDDepth && node_type == NodePV && trans_move == move::NONE) {

			new_depth = std::min(depth - IIDReduction, depth / 2);

			value = full_search(bd, alpha, beta, new_depth, height, new_pv, node_type, false, ThreadId);
			if (value <= alpha) value = full_search(bd, -score::INF, beta, new_depth, height, new_pv, node_type, false, ThreadId);

			if (!new_pv.empty()) trans_move = new_pv[0];
		}
#endif
		// move generation

		ml.init(bd, in_check, trans_move, depth);

		single_reply = false;
   		// if (in_check && ml.size_is() == 1) single_reply = true; // HACK

		// move loop

		opt_value = +score::INF;
		good_cap = true;

		for (int move = ml.next(); move != move::NONE; move = ml.next()) {

			dangerous = move::capture(move) != move::NONE || move::is_promote(move) || bd.is_check_move(move);

			// extensions
			new_depth = full_new_depth(depth, bd.is_check_move(move), false, played.size(), node_type);

			// history pruning
#if 0
			value = bd.history_is(move::to_kind(move), move::to(move)); // history score
			if (!in_check && depth <= 6 && node_type != NodePV 
				&& new_depth < depth && value < 2 * HistoryValue / (depth + depth % 2) /*2*/ 
				&& played.size() >= 1 + depth && !dangerous){ 
					continue;
			}
#endif
#if 1
			if (node_type != NodePV && depth <= 5) {
				if (!in_check && new_depth < depth && !dangerous) {

					// optimistic evaluation

					if (opt_value == +score::INF) {
						if (depth >= 2) {
							FutilityMargin = FutilityMargin2 + (depth % 2) * 100;
						}
						else {
							FutilityMargin = FutilityMargin1;
						}
						opt_value = standpat + FutilityMargin;
					}

					value = opt_value;

					// pruning

					if (value <= alpha) {

						if (value > best_value) {
							best_value = value;
							pv.clear();
						}

						continue;
					}
				}
			} 
#endif
			// Late Move Reductions

			// init
			reduced = false;
			reduction = 0;
#if 1
			// lookup reduction  
			if (!in_check && new_depth < depth && played.size() >= HistoryMoveNb
				&& depth >= HistoryDepth && !dangerous) {
					/*
					played_nb = played.size();

					reduction = (node_type == NodePV ? quietPvMoveReduction[depth<64 ? depth: 63][played_nb<64? played_nb: 63]:
                                     quietMoveReduction[depth<64 ? depth: 63][played_nb<64? played_nb: 63]);
					*/
					reduction = 1;
					if (reduction > 0)  reduced = true;
			}
#endif
			// recursive search

			bd.move(move);

			if (node_type != NodePV || best_value == score::NONE) { // first move or non-pv

				value = -full_search(bd, -beta, -alpha, new_depth - reduction, height + 1, new_pv, NODE_OPP(node_type), false, ThreadId);
				
				// The move was reduced and fails high; so we research with full depth
         		if (reduced && value > alpha) {
		        	value = -full_search(bd, -beta, -alpha, new_depth, height + 1, new_pv, NODE_OPP(node_type), false, ThreadId);
         		}

			} else { // other moves (all PV children)

				value = -full_search(bd, -alpha - 1, -alpha, new_depth - reduction, height + 1, new_pv, NodeCut, false, ThreadId);

				// In case of fail high:
               
         		// If reduced then we try a research with node_type = NodePV
				if (value > alpha && reduced) { // research

					value = -full_search(bd, -beta, -alpha, new_depth - reduction, height + 1, new_pv, NodePV, false, ThreadId);

					// Still fails high! We research to full depth
            		if (value > alpha){
						value = -full_search(bd, -beta, -alpha, new_depth, height + 1, new_pv, NodePV, false, ThreadId);
					}
				
				// If not reduced we research as a PV node   
				} else if (value > alpha) {
					value = -full_search(bd, -beta, -alpha, new_depth, height + 1, new_pv, NodePV, false, ThreadId);
				}
			}
			
			bd.undo(move);

			if (search_g.abort) {
				pv_cat(pv,new_pv,move);
				return best_value;
			}

			played.push_back(move);

			if (value > best_value) {
				best_value = value;
				pv_cat(pv,new_pv,move);
				if (value > alpha) {
					alpha = value;
					best_move = move;
					if (value >= beta) goto cut;
				}
			}

			if (node_type == NodeCut) node_type = NodeAll;
		}

		// all_node

		if (best_value == score::NONE) {

			if (move::is_drop_pawn(bd.last_move())) return -1 * score::mate_value(height); // drop pawn mate
			else return score::mate_value(height);
		}

	cut:

		// move ordering

		if (best_move != move::NONE) {

			bd.good_move(best_move, depth);
		} 

		// transposition table
		#define value_to_tt(v) (v >= score::EVALINF ? v + height : v <= -score::EVALINF ? v - height : v)
#if 1
		if (depth >= TransDepth && pv.size() > 0) {
			found_entry->save(bd.full_key(), value_to_tt(best_value), 
					best_value >= beta ? trans::BOUND_LOWER : node_type == NodePV ? trans::BOUND_EXACT : trans::BOUND_UPPER,
					(uint8_t)depth, pv[0], standpat, search_g.tt.generation());
		}
#endif
		return best_value;
	}

	int comp_nps(double elapsed) {
		return (int)(search_g.nodes * 1000 / (elapsed + 1));
	}

	void iteration(int timeLimit, TIME start,int ThreadId) {

		if (ThreadId == 0) {
			search_g.tt.new_search();
			search_g.nodes = 0;
		}

		for (int dep = 1; ; dep++) {

			int depth = ThreadId == 0 ? dep : SearchCurrent[0].max_depth + (int)(3 * log(1 + ThreadId));
			SearchCurrent[ThreadId].pv.clear();
			SearchCurrent[ThreadId].best_value = full_search(SearchCurrent[ThreadId].bd, -score::INF, +score::INF, depth, 0, SearchCurrent[ThreadId].pv, NodePV, false, ThreadId);
			SearchCurrent[ThreadId].max_depth = depth;
			auto end = TimerNow();
        	double elapsed = TimerElapsed(start, end);
			search_g.nodes += SearchCurrent[ThreadId].nodes;
			if (!SearchCurrent[ThreadId].pv.empty() && !search_g.abort) {
				SearchCurrent[ThreadId].best_move = SearchCurrent[ThreadId].pv[0];
			}
			if (SearchCurrent[ThreadId].best_value != score::NONE && !search_g.abort && ThreadId == 0) // search PV-node
			{
				std::cout << "info depth " << depth << " seldepth " << SearchCurrent[ThreadId].pv.size();
				std::cout << " time " << elapsed;
				std::cout << " nodes " << SearchCurrent[ThreadId].nodes;
				std::cout << " score cp " << SearchCurrent[ThreadId].best_value;
				std::cout << " pv";
				std::for_each(SearchCurrent[ThreadId].pv.begin(), SearchCurrent[ThreadId].pv.end(), [](int m){ std::cout << " " << move::to_can(m); });
				std::cout << " nps " << comp_nps(elapsed);
				std::cout << std::endl;
			}

			if (score::is_mate(SearchCurrent[ThreadId].best_value)) break; // mate
			if (search_g.abort) break;
			if (ThreadId == 0 && timeLimit / 3 < elapsed) break; // timeLimit
		}

		search_g.abort = true;

		const int count = (search_g.threadCount ^= 1 << ThreadId);

		//以下の処理は最後のスレッドのみ
		if (count != 0) return;

		SearchLocal *bestThread = &SearchCurrent[0];
		for (int i = 1; i < engine::engine.threads; ++i) {

			if (SearchCurrent[i].best_move == move::NONE) continue;
			if ((SearchCurrent[i].best_value > bestThread->best_value && SearchCurrent[i].max_depth > bestThread->max_depth) || bestThread->best_move == move::NONE) 
				bestThread = &SearchCurrent[i];
		}

		int bestMove = bestThread->best_move;
		int bestVal = bestThread->best_value;

		if (bestMove == move::NONE) std::cout << "bestmove resign" << std::endl;
		else if (bestVal <= score::ResignEval && bestVal != score::NONE) std::cout << "bestmove resign" << std::endl;
		else {
			std::cout << "bestmove " << move::to_can(bestMove) << std::endl;
			if (bestVal == +score::INF  - 1) std::cout << "info string ∩･ω･)∩わーい" << std::endl;
		} 
	}

	void thread_search(SearchLocal *SL, int &timeLimit, TIME &start) {
		
		while (1) {

			iteration(timeLimit, start, SL->id);

			while (!(search_g.threadCount & (1 << SL->id))) 
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	}

	int TimeTable( board::Board& bd, std::istringstream& command ) {

		const int ClockPerSec = 1000;
		const int ClockPerMin = 60 * 1000;
		int margin = 500; // byoyomi margin

		int bTime, wTime, byoyomi, binc, winc; 
		byoyomi = binc = winc = 0;

		for ( std::string token; command >> token; ) 
		{
			if ( token == "infinite") return 20 * ClockPerSec; // infinite time = 20sec
			if ( token == "btime" )   command >> bTime; 
			if ( token == "wtime" )   command >> wTime; 
			if ( token == "byoyomi" ) command >> byoyomi; 
			if ( token == "binc" )    command >> binc;
			if ( token == "winc" )    command >> winc; 
		}

		int myTime = bd.turn_is() == color::Black ? bTime : wTime;
		int myInc  = bd.turn_is() == color::Black ? binc : winc;

		if (byoyomi > 0) return byoyomi;
		else if (myInc > 0) // fisher
		{
			if (myTime > 15 * ClockPerMin) return 45 * ClockPerSec;
			else if (myTime > 12 * ClockPerMin) return 40 * ClockPerSec;
			else if (myTime > 10 * ClockPerMin) return 35 * ClockPerSec;
			else if (myTime > 05 * ClockPerMin) return 30 * ClockPerSec;
			else if (myTime > 02 * ClockPerMin) return 25 * ClockPerSec;
			else if (myTime > 01 * ClockPerMin) return 20 * ClockPerSec;
			else /*if (myTime == 0)*/ return myInc * ClockPerSec - margin;
			//else return myInc * ClockPerSec;
		}
		else { // kiremake
		
			if (myTime > 15 * ClockPerMin) return 32 * ClockPerSec;
			else if (myTime > 12 * ClockPerMin) return 27 * ClockPerSec;
			else if (myTime > 10 * ClockPerMin) return 22 * ClockPerSec;
			else if (myTime > 05 * ClockPerMin) return 20 * ClockPerSec;
			else if (myTime > 02 * ClockPerMin) return 15 * ClockPerSec;
			else if (myTime > 01 * ClockPerMin) return 05 * ClockPerSec;
			else return 2 * ClockPerSec;
		}
	}

	void think(board::Board &bd, std::istringstream &command) {

		// look in BookDB
		if (bd.stage_is() < BookDB::DepthMax) {
			int expertMove = BookDB::out(bd);
			if (!expertMove == move::NONE) {
				std::cout << "bestmove " << move::to_can(expertMove) << std::endl;
				return;
			}
		}

		int timeLimit = TimeTable(bd, command);
		TIME start = TimerNow();

		if (SearchCurrent) {

			search_g.abort = false;
			for (int i = 0; i < engine::engine.threads; i++) {
				SearchCurrent[i].id = i;
    			SearchCurrent[i].init(bd);
            	search_g.threadCount ^= (1 << i);
        	}
		} 
		else {

			SearchCurrent = new SearchLocal[engine::engine.threads];
			for (int i = 0; i < engine::engine.threads; i++) {
				SearchCurrent[i].id = i;
    			SearchCurrent[i].init(bd);
				search_g.threadCount ^= (1 << i);
        	}
			search_g.abort = false;
			for (int i = 0; i < engine::engine.threads; i++) {
				SearchCurrent[i].thread = new std::thread{&thread_search, &SearchCurrent[i], std::ref(timeLimit), std::ref(start)};
			}
		}

		std::thread* timerThread = nullptr;
		timerThread = new std::thread([&]{
			while (TimerElapsed(start, TimerNow()) < timeLimit && !search_g.abort)
				sleep(1);
			search_g.abort = true;
		});
		if (timerThread != nullptr) {

			timerThread->join();
			delete timerThread;
		}
	}
}

namespace usi {

	void set_position(board::Board& bd, std::istringstream& command) {

		std::string token, sfen;
		command >> token; // "startpos" or "sfen"

		if (token == "startpos") {

			sfen = "lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1";
			command >> token; // "moves"
		}
		else if (token == "sfen") {

			while (command >> token && token != "moves") { sfen += token + " "; }
		}
		bd.set(sfen);

		for (std::string sfenMove; command >> sfenMove; ) {

			bd.move(move::from_sfen(bd, sfenMove));
		}

		board::to_string(bd);
	}

	void loop() {

		std::string cmd, token;

		while (std::getline(std::cin, cmd)) {

			std::istringstream command(cmd);
			command >> token;

			if (token == "usi") {
				std::cout << "id name 20170502" << std::endl;
				std::cout << "id author kumasan"  << std::endl;
				std::cout << "usiok" << std::endl;
			}
			else if (token == "isready") {

				std::cout << "readyok" << std::endl;
			}
			else if (token == "position") set_position(search::search_g.bd, command);
			else if (token == "go") {
				search::think(search::search_g.bd, command);
			}
			else if (token == "usinewgame") {
				search::search_g.tt.clear();
			}
			else if (token == "setoption") {

			}
			else if (token == "bench") {
				
				bench::gen_move(search::search_g.bd);
			}
			else if (token == "eval") {
				std::cout << "eval : " << eval::evaluate(search::search_g.bd) << std::endl;
			}
			else if (token == "stop") {

			}
			else if (token == "quit") break;
		}

		exit(1);
	}
}

int main(int argv, char *args[]) {

    delta::init();
    hash::init();
    eval::init();
    engine::init();
    BookDB::init();
    search::init();

    usi::loop();

    return 0;
}