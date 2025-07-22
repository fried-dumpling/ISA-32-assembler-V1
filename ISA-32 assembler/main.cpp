#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <stack>
#include <queue>
#include <map>
#include <set>
#include <functional>

using namespace std;

namespace assembler {
	namespace tools {
		template <class ST, typename IN>
		class FSM {
		public:
			class Rule {
			public:
				bool increment;
				std::function<bool(IN)>function;
			};

			typedef struct _TransitionData {
				ST end;
				Rule rule;
			}TransitionData;

			typedef struct _QueueData {
				ST cur;
				size_t id;
				typename std::multimap<ST, TransitionData>::iterator transition;
			} QueueData;

			enum ReturnData {
				Success,
				Empty,
				NoTransition
			};

			typedef struct _CreateSD {
				ST state;
				bool end;
			} CreateSD;

			typedef struct _CreateTD {
				ST start;
				ST end;
				Rule rule;
			} CreateTD;

			typedef struct _CreateData {
				std::vector<CreateSD> states;
				std::vector<CreateTD> transitions;
			} CreateData;

		private:
			std::map<ST, bool> states;
			std::multimap<ST, TransitionData> transitions;

			void addState(ST state, bool end) {
				if (this->states.find(state) != this->states.end())
					return;
				this->states.insert({ state, end });
			}

			void addTransition(ST start, ST end, Rule rule) {
				this->transitions.insert({ start, {end, rule} });
			}

		public:
			FSM(CreateData& cd) {
				for (auto it = cd.states.begin(); it != cd.states.end(); it++) {
					this->addState(it->state, it->end);
				}
				for (auto it = cd.transitions.begin(); it != cd.transitions.end(); it++) {
					this->addTransition(it->start, it->end, it->rule);
				}
			}

			bool isEnd(ST state) {
				auto it = this->states.find(state);
				if (it == this->states.end())
					return false;
				return it->second;
			}

			ReturnData run(std::queue<QueueData>& inputQ, IN data) {
				if (inputQ.empty())
					return ReturnData::Empty;

				QueueData QD = inputQ.front();
				inputQ.pop();

				auto range = this->transitions.equal_range(QD.cur);
				bool moved = false;

				for (auto it = range.first; it != range.second; it++) {
					if (it->second.rule.function(data)) {
						inputQ.push({ it->second.end, QD.id + it->second.rule.increment, it});
						moved = true;
					}
				}

				if (!moved) {
					return ReturnData::NoTransition;
				}
				
				return ReturnData::Success;
			}
		};

		template <class ST, class SK, typename IN>
		class PDA {
		public:
			class Rule {
			public:
				bool increment;
				bool pop;
				std::vector<SK> pushList;
				std::function<bool(IN, SK)> function;
			};

			typedef struct _TransitionData {
				ST end;
				Rule rule;
			}TransitionData;

			typedef struct _QueueData {
				using TransitionIT = typename std::multimap<ST, TransitionData>::iterator;
				typedef struct _QueueDataRD {
					ST cur;
					size_t id;
					TransitionIT transition;
				} QueueDataRD;

				ST cur;
				std::stack<SK> stack;
				size_t id;
				TransitionIT transition;
				std::vector<QueueDataRD> tracker;
			} QueueData;

			enum ReturnData {
				Success,
				Empty,
				NoTransition
			};

			typedef struct _CreateSD {
				ST state;
				bool end;
			} CreateSD;

			typedef struct _CreateTD {
				ST start;
				ST end;
				Rule rule;
			} CreateTD;

			typedef struct _CreateData {
				std::vector<CreateSD> states;
				std::vector<CreateTD> transitions;
			} CreateData;

		private:
			std::map<ST, bool> states;
			std::multimap<ST, TransitionData> transitions;

			void addState(ST state, bool end) {
				if (this->states.find(state) != this->states.end())
					return;
				this->states.insert({ state, end });
			}

			void addTransition(ST start, ST end, Rule rule) {
				this->transitions.insert({ start, {end, rule} });
			}

		public:
			PDA(CreateData& cd) {
				for (auto it = cd.states.begin(); it != cd.states.end(); it++) {
					this->addState(it->state, it->end);
				}
				for (auto it = cd.transitions.begin(); it != cd.transitions.end(); it++) {
					this->addTransition(it->start, it->end, it->rule);
				}
			}

			typename std::multimap<ST, TransitionData>::iterator defTransition(void) {
				return this->transitions.end();
			}

			bool isEnd(ST state) {
				auto it = this->states.find(state);
				if (it == this->states.end())
					return false;
				return it->second;
			}

			ReturnData run(std::queue<QueueData>& inputQ, IN data) {
				if (inputQ.empty())
					return ReturnData::Empty;

				QueueData QD = inputQ.front();
				inputQ.pop();

				auto range = this->transitions.equal_range(QD.cur);
				bool moved = false;

				for (auto it = range.first; it != range.second; it++) {
					if (it->second.rule.function(data, ((QD.stack.empty()) ? (SK)-1 : QD.stack.top()))) {
						QueueData tmp = {};
						tmp.cur = it->second.end;
						tmp.stack = QD.stack;
						tmp.id = QD.id + it->second.rule.increment;
						tmp.transition = it;
						tmp.tracker = QD.tracker;
						tmp.tracker.push_back({ QD.cur, QD.id, tmp.transition });
						if (it->second.rule.pop && !tmp.stack.empty())
							tmp.stack.pop();
						if (!it->second.rule.pushList.empty()) {
							for (auto si = it->second.rule.pushList.begin(); si != it->second.rule.pushList.end(); si++)
								tmp.stack.push(*si);
						}
						inputQ.push(tmp);
						moved = true;
					}
				}

				if (!moved) {
					return ReturnData::NoTransition;
				}

				return ReturnData::Success;
			}
		};
	}

	namespace lexer {
		enum class S {
			__start__,

			a,
			ad, add, addc, addi,

			b,
			bn, bnd, bndi,
			bo, bor, bori,
			bx, bxr, bxri,

			c,
			ca, cal, call,
			cm, cmp,

			h,
			ha, hal, halt,

			j,
			jm, jmp,

			l,
			ld,

			m,
			mo, mov,

			n,
			no, nop,

			p,
			po, pop,
			pu, pus, push,

			r,
			re, ret,

			s,
			se, set,
			sh, shi, shif, shift,
			shiftl, shiftli, shiftr, shiftri,
			st,
			su, sub, subc, subi,


			//===================================================================================


			_percent_,

			_percent_f,
			_percent_fl, _percent_fla, _percent_flag,
			_percent_fu, _percent_ful, _percent_full,

			_percent_g,
			_percent_ge, _percent_gen,

			_percent_gen_openbrack_,
			_percent_gen_openbrack_0, _percent_gen_openbrack_0_closebrack_,
			_percent_gen_openbrack_1, _percent_gen_openbrack_1_closebrack_,
			_percent_gen_openbrack_2, _percent_gen_openbrack_2_closebrack_,
			_percent_gen_openbrack_3, _percent_gen_openbrack_3_closebrack_,
			_percent_gen_openbrack_4, _percent_gen_openbrack_4_closebrack_,
			_percent_gen_openbrack_5, _percent_gen_openbrack_5_closebrack_,
			_percent_gen_openbrack_6, _percent_gen_openbrack_6_closebrack_,
			_percent_gen_openbrack_7, _percent_gen_openbrack_7_closebrack_,
			_percent_gen_openbrack_8, _percent_gen_openbrack_8_closebrack_,
			_percent_gen_openbrack_9, _percent_gen_openbrack_9_closebrack_,
			_percent_gen_openbrack_10, _percent_gen_openbrack_10_closebrack_,
			_percent_gen_openbrack_11, _percent_gen_openbrack_11_closebrack_,
			_percent_gen_openbrack_12, _percent_gen_openbrack_12_closebrack_,
			_percent_gen_openbrack_13, _percent_gen_openbrack_13_closebrack_,
			_percent_gen_openbrack_14, _percent_gen_openbrack_14_closebrack_,
			_percent_gen_openbrack_15, _percent_gen_openbrack_15_closebrack_,
			_percent_gen_openbrack_16, _percent_gen_openbrack_16_closebrack_,
			_percent_gen_openbrack_17, _percent_gen_openbrack_17_closebrack_,
			_percent_gen_openbrack_18, _percent_gen_openbrack_18_closebrack_,
			_percent_gen_openbrack_19, _percent_gen_openbrack_19_closebrack_,
			_percent_gen_openbrack_20, _percent_gen_openbrack_20_closebrack_,
			_percent_gen_openbrack_21, _percent_gen_openbrack_21_closebrack_,
			_percent_gen_openbrack_22, _percent_gen_openbrack_22_closebrack_,
			_percent_gen_openbrack_23, _percent_gen_openbrack_23_closebrack_,
			_percent_gen_openbrack_24, _percent_gen_openbrack_24_closebrack_,

			_percent_o,
			_percent_on, _percent_one,

			_percent_p,
			_percent_pc,

			_percent_s,
			_percent_sb, _percent_sbp,
			_percent_st, _percent_sta, _percent_stac, _percent_stack,

			_percent_z,
			_percent_ze, _percent_zer, _percent_zero,


			//===================================================================================


			_dot_,

			_dot_1,
			_dot_16, _dot_16H, _dot_16L,

			_dot_3,
			_dot_32,

			_dot_8,
			_dot_8H, _dot_8L,

			_dot_b,
			_dot_bs, _dot_bss,

			_dot_c,
			_dot_ca, _dot_car, _dot_carr, _dot_carry, _dot_carry4,

			_dot_d,
			_dot_da, _dot_dat, _dot_data,

			_dot_f,
			_dot_fu, _dot_ful, _dot_full,
			_dot_fl, _dot_fla, _dot_flag,

			_dot_g,
			_dot_ge, _dot_gen,

			_dot_gen_openbrack_,
			_dot_gen_openbrack_0, _dot_gen_openbrack_0_closebrack_,
			_dot_gen_openbrack_1, _dot_gen_openbrack_1_closebrack_,
			_dot_gen_openbrack_2, _dot_gen_openbrack_2_closebrack_,
			_dot_gen_openbrack_3, _dot_gen_openbrack_3_closebrack_,
			_dot_gen_openbrack_4, _dot_gen_openbrack_4_closebrack_,
			_dot_gen_openbrack_5, _dot_gen_openbrack_5_closebrack_,
			_dot_gen_openbrack_6, _dot_gen_openbrack_6_closebrack_,
			_dot_gen_openbrack_7, _dot_gen_openbrack_7_closebrack_,
			_dot_gen_openbrack_8, _dot_gen_openbrack_8_closebrack_,
			_dot_gen_openbrack_9, _dot_gen_openbrack_9_closebrack_,
			_dot_gen_openbrack_10, _dot_gen_openbrack_10_closebrack_,
			_dot_gen_openbrack_11, _dot_gen_openbrack_11_closebrack_,
			_dot_gen_openbrack_12, _dot_gen_openbrack_12_closebrack_,
			_dot_gen_openbrack_13, _dot_gen_openbrack_13_closebrack_,
			_dot_gen_openbrack_14, _dot_gen_openbrack_14_closebrack_,
			_dot_gen_openbrack_15, _dot_gen_openbrack_15_closebrack_,
			_dot_gen_openbrack_16, _dot_gen_openbrack_16_closebrack_,
			_dot_gen_openbrack_17, _dot_gen_openbrack_17_closebrack_,
			_dot_gen_openbrack_18, _dot_gen_openbrack_18_closebrack_,
			_dot_gen_openbrack_19, _dot_gen_openbrack_19_closebrack_,
			_dot_gen_openbrack_20, _dot_gen_openbrack_20_closebrack_,
			_dot_gen_openbrack_21, _dot_gen_openbrack_21_closebrack_,
			_dot_gen_openbrack_22, _dot_gen_openbrack_22_closebrack_,
			_dot_gen_openbrack_23, _dot_gen_openbrack_23_closebrack_,
			_dot_gen_openbrack_24, _dot_gen_openbrack_24_closebrack_,

			_dot_n,
			_dot_ne, _dot_neg,

			_dot_o,
			_dot_on, _dot_one,
			_dot_ov, _dot_ove, _dot_over, _dot_overf, _dot_overfl, _dot_overflo, _dot_overflow,

			_dot_p,
			_dot_pc,
			_dot_po, _dot_pos,

			_dot_s,
			_dot_sb, _dot_sbp,
			_dot_st, _dot_sta, _dot_stac, _dot_stack,
			_dot_S,
			_dot_S1, _dot_S16, _dot_S16L,
			_dot_S8, _dot_S8H, _dot_S8L,

			_dot_t, _dot_te, _dot_tex, _dot_text,

			_dot_z,
			_dot_ze, _dot_zer, _dot_zero,


			//===================================================================================


			_openparen_, _closeparen_,

			_colon_, _semicolon_,

			_plus_, _minus_,
			_star_, _star_star_, _slash_,
			_tilde_, _ampersend_, _verticalbar_, _caret_,

			_whitespace_,
			_newline_,

			//===================================================================================

			__firstnum__,
			__hexnum__, __decnum__, __octnum__, __binnum__,

			__InterTreeEpsilonID__,
			__InterTreeEpsilonUK__,
			__KeyTreeEpsilonID__,
			__KeyTreeEpsilonUK__,

			__identifier__,

			__unknown__
		};
		
		using LFSM = tools::FSM<S, char>;
		using LRET = LFSM::ReturnData;

		LFSM::CreateData lexerTable = {
			{
			{S::__start__, false},

			{S::a, false},
			{S::ad, false}, {S::add, false}, {S::addc, false}, {S::addi, false},

			{S::b, false},
			{S::bn, false}, {S::bnd, false}, {S::bndi, false},
			{S::bo, false}, {S::bor, false}, {S::bori, false},
			{S::bx, false}, {S::bxr, false}, {S::bxri, false},

			{S::c, false},
			{S::ca, false}, {S::cal, false}, {S::call, false},
			{S::cm, false}, {S::cmp, false},

			{S::h, false},
			{S::ha, false}, {S::hal, false}, {S::halt, false},
				
			{S::j, false}, 
			{S::jm, false}, {S::jmp, false},

			{S::l, false},
			{S::ld, false},

			{S::m, false},
			{S::mo, false}, {S::mov, false},

			{S::n, false},
			{S::no, false}, {S::nop, false},

			{S::p, false},
			{S::po, false}, {S::pop, false},
			{S::pu, false}, {S::pus, false}, {S::push, false},

			{S::r, false}, 
			{S::re, false}, {S::ret, false},
			
			{S::s, false},
			{S::se, false}, {S::set, false},
			{S::sh, false}, {S::shi, false}, {S::shif, false}, {S::shift, false},
			{S::shiftl, false}, {S::shiftli, false}, {S::shiftr, false}, {S::shiftri, false},
			{S::st, false},
			{S::su, false}, {S::sub, false}, {S::subc, false}, {S::subi, false},

			{S::_dot_, false},

			{S::_dot_1, false},
			{S::_dot_16, false}, {S::_dot_16H, false}, {S::_dot_16L, false},

			{S::_dot_3, false},
			{S::_dot_32, false},

			{S::_dot_8, false}, {S::_dot_8H, false}, {S::_dot_8L, false},

			{S::_dot_b, false}, {S::_dot_bs, false}, {S::_dot_bss, false},
			
			{S::_dot_c, false},
			{S::_dot_ca, false}, {S::_dot_car, false}, {S::_dot_carr, false}, {S::_dot_carry, false}, {S::_dot_carry4, false},

			{S::_dot_d, false},
			{S::_dot_da, false}, {S::_dot_dat, false}, {S::_dot_data, false},

			{S::_dot_f, false},	
			{S::_dot_fu, false}, {S::_dot_ful, false}, {S::_dot_full, false},
			{S::_dot_fl, false}, {S::_dot_fla, false}, {S::_dot_flag, false},

			{S::_dot_g, false},
			{S::_dot_ge, false}, {S::_dot_gen, false}, {S::_dot_gen_openbrack_, false},
			{S::_dot_gen_openbrack_0, false}, {S::_dot_gen_openbrack_0_closebrack_, false},
			{S::_dot_gen_openbrack_1, false}, {S::_dot_gen_openbrack_1_closebrack_, false},
			{S::_dot_gen_openbrack_2, false}, {S::_dot_gen_openbrack_2_closebrack_, false},
			{S::_dot_gen_openbrack_3, false}, {S::_dot_gen_openbrack_3_closebrack_, false},
			{S::_dot_gen_openbrack_4, false}, {S::_dot_gen_openbrack_4_closebrack_, false},
			{S::_dot_gen_openbrack_5, false}, {S::_dot_gen_openbrack_5_closebrack_, false},
			{S::_dot_gen_openbrack_6, false}, {S::_dot_gen_openbrack_6_closebrack_, false},
			{S::_dot_gen_openbrack_7, false}, {S::_dot_gen_openbrack_7_closebrack_, false},
			{S::_dot_gen_openbrack_8, false}, {S::_dot_gen_openbrack_8_closebrack_, false},
			{S::_dot_gen_openbrack_9, false}, {S::_dot_gen_openbrack_9_closebrack_, false},
			{S::_dot_gen_openbrack_10, false}, {S::_dot_gen_openbrack_10_closebrack_, false},
			{S::_dot_gen_openbrack_11, false}, {S::_dot_gen_openbrack_11_closebrack_, false},
			{S::_dot_gen_openbrack_12, false}, {S::_dot_gen_openbrack_12_closebrack_, false},
			{S::_dot_gen_openbrack_13, false}, {S::_dot_gen_openbrack_13_closebrack_, false},
			{S::_dot_gen_openbrack_14, false}, {S::_dot_gen_openbrack_14_closebrack_, false},
			{S::_dot_gen_openbrack_15, false}, {S::_dot_gen_openbrack_15_closebrack_, false},
			{S::_dot_gen_openbrack_16, false}, {S::_dot_gen_openbrack_16_closebrack_, false},
			{S::_dot_gen_openbrack_17, false}, {S::_dot_gen_openbrack_17_closebrack_, false},
			{S::_dot_gen_openbrack_18, false}, {S::_dot_gen_openbrack_18_closebrack_, false},
			{S::_dot_gen_openbrack_19, false}, {S::_dot_gen_openbrack_19_closebrack_, false},
			{S::_dot_gen_openbrack_20, false}, {S::_dot_gen_openbrack_20_closebrack_, false},
			{S::_dot_gen_openbrack_21, false}, {S::_dot_gen_openbrack_21_closebrack_, false},
			{S::_dot_gen_openbrack_22, false}, {S::_dot_gen_openbrack_22_closebrack_, false},
			{S::_dot_gen_openbrack_23, false}, {S::_dot_gen_openbrack_23_closebrack_, false},
			{S::_dot_gen_openbrack_24, false}, {S::_dot_gen_openbrack_24_closebrack_, false},

			{S::_dot_n, false},
			{S::_dot_ne, false}, {S::_dot_neg, false},

			{S::_dot_o, false},
			{S::_dot_on, false}, {S::_dot_one, false},
			{S::_dot_ov, false}, {S::_dot_ove, false}, {S::_dot_over, false}, {S::_dot_overf, false}, 
			{S::_dot_overfl, false}, {S::_dot_overflo, false}, {S::_dot_overflow, false},

			{S::_dot_p, false},
			{S::_dot_pc, false},
			{S::_dot_po, false}, {S::_dot_pos, false},

			{S::_dot_s, false},
			{S::_dot_sb, false}, {S::_dot_sbp, false},
			{S::_dot_st, false}, {S::_dot_sta, false}, {S::_dot_stac, false}, {S::_dot_stack, false},
			{S::_dot_S1, false}, {S::_dot_S16, false}, {S::_dot_S16L, false},
			{S::_dot_S8, false}, {S::_dot_S8H, false}, {S::_dot_S8L, false},

			{S::_dot_t, false},
			{S::_dot_te, false}, {S::_dot_tex, false}, {S::_dot_text, false},

			{S::_dot_z, false}, 
			{S::_dot_ze, false}, {S::_dot_zer, false}, {S::_dot_zero, false},

			{ S::_percent_, false },

			{ S::_percent_f, false },
			{ S::_percent_fl, false }, { S::_percent_fla, false }, { S::_percent_flag, false },
			{ S::_percent_fu, false }, { S::_percent_ful, false }, { S::_percent_full, false },

			{ S::_percent_g, false },
			{ S::_percent_ge, false }, { S::_percent_gen, false },

			{ S::_percent_gen_openbrack_, false },
			{ S::_percent_gen_openbrack_0, false }, { S::_percent_gen_openbrack_0_closebrack_, false },
			{ S::_percent_gen_openbrack_1, false }, { S::_percent_gen_openbrack_1_closebrack_, false },
			{ S::_percent_gen_openbrack_2, false }, { S::_percent_gen_openbrack_2_closebrack_, false },
			{ S::_percent_gen_openbrack_3, false }, { S::_percent_gen_openbrack_3_closebrack_, false },
			{ S::_percent_gen_openbrack_4, false }, { S::_percent_gen_openbrack_4_closebrack_, false },
			{ S::_percent_gen_openbrack_5, false }, { S::_percent_gen_openbrack_5_closebrack_, false },
			{ S::_percent_gen_openbrack_6, false }, { S::_percent_gen_openbrack_6_closebrack_, false },
			{ S::_percent_gen_openbrack_7, false }, { S::_percent_gen_openbrack_7_closebrack_, false },
			{ S::_percent_gen_openbrack_8, false }, { S::_percent_gen_openbrack_8_closebrack_, false },
			{ S::_percent_gen_openbrack_9, false }, { S::_percent_gen_openbrack_9_closebrack_, false },
			{ S::_percent_gen_openbrack_10, false }, { S::_percent_gen_openbrack_10_closebrack_, false },
			{ S::_percent_gen_openbrack_11, false }, { S::_percent_gen_openbrack_11_closebrack_, false },
			{ S::_percent_gen_openbrack_12, false }, { S::_percent_gen_openbrack_12_closebrack_, false },
			{ S::_percent_gen_openbrack_13, false }, { S::_percent_gen_openbrack_13_closebrack_, false },
			{ S::_percent_gen_openbrack_14, false }, { S::_percent_gen_openbrack_14_closebrack_, false },
			{ S::_percent_gen_openbrack_15, false }, { S::_percent_gen_openbrack_15_closebrack_, false },
			{ S::_percent_gen_openbrack_16, false }, { S::_percent_gen_openbrack_16_closebrack_, false },
			{ S::_percent_gen_openbrack_17, false }, { S::_percent_gen_openbrack_17_closebrack_, false },
			{ S::_percent_gen_openbrack_18, false }, { S::_percent_gen_openbrack_18_closebrack_, false },
			{ S::_percent_gen_openbrack_19, false }, { S::_percent_gen_openbrack_19_closebrack_, false },
			{ S::_percent_gen_openbrack_20, false }, { S::_percent_gen_openbrack_20_closebrack_, false },
			{ S::_percent_gen_openbrack_21, false }, { S::_percent_gen_openbrack_21_closebrack_, false },
			{ S::_percent_gen_openbrack_22, false }, { S::_percent_gen_openbrack_22_closebrack_, false },
			{ S::_percent_gen_openbrack_23, false }, { S::_percent_gen_openbrack_23_closebrack_, false },
			{ S::_percent_gen_openbrack_24, false }, { S::_percent_gen_openbrack_24_closebrack_, false },

			{ S::_percent_o, false },
			{ S::_percent_on, false }, { S::_percent_one, false },

			{ S::_percent_p, false },
			{ S::_percent_pc, false },

			{ S::_percent_s, false },
			{ S::_percent_sb, false }, { S::_percent_sbp, false },
			{ S::_percent_st, false }, { S::_percent_sta, false }, { S::_percent_stac, false }, { S::_percent_stack, false },

			{ S::_percent_z, false },
			{ S::_percent_ze, false }, { S::_percent_zer, false }, { S::_percent_zero, false },

			{S::_openparen_, false}, {S::_closeparen_, false},

			{S::_colon_, false}, {S::_semicolon_, false},

			{S::_plus_, false}, {S::_minus_, false},
			{S::_star_, false}, {S::_star_star_, false}, {S::_slash_, false},
			{S::_tilde_, false}, {S::_ampersend_, false}, {S::_verticalbar_, false}, {S::_caret_, false},

			{S::__firstnum__, false},
			{S::__hexnum__, false}, {S::__decnum__, false}, {S::__octnum__, false}, {S::__binnum__, false},
			
			{ S::_whitespace_, false },
			{ S::_newline_, false },

			{S::__InterTreeEpsilonID__, false},
			{S::__InterTreeEpsilonUK__, false},
			{S::__KeyTreeEpsilonID__, false},
			{S::__KeyTreeEpsilonUK__, false},

			{S::__identifier__, false},

			{S::__unknown__, false}
			},

			{
			{S::__start__, S::a, {true, [](char c)->bool {return c == 'a'; }}},
			{S::__start__, S::b, {true, [](char c)->bool {return c == 'b'; }}},
			{S::__start__, S::c, {true, [](char c)->bool {return c == 'c'; }}},
			{S::__start__, S::h, {true, [](char c)->bool {return c == 'h'; }}},
			{S::__start__, S::j, {true, [](char c)->bool {return c == 'j'; }}},
			{S::__start__, S::l, {true, [](char c)->bool {return c == 'l'; }}},
			{S::__start__, S::m, {true, [](char c)->bool {return c == 'm'; }}},
			{S::__start__, S::n, {true, [](char c)->bool {return c == 'n'; }}},
			{S::__start__, S::p, {true, [](char c)->bool {return c == 'p'; }}},
			{S::__start__, S::r, {true, [](char c)->bool {return c == 'r'; }}},
			{S::__start__, S::s, {true, [](char c)->bool {return c == 's'; }}},

			{S::a, S::ad, {true, [](char c)->bool {return c == 'd'; }}},
			{S::b, S::bn, {true, [](char c)->bool {return c == 'n'; }}},
			{S::b, S::bo, {true, [](char c)->bool {return c == 'o'; }}},
			{S::b, S::bx, {true, [](char c)->bool {return c == 'x'; }}},
			{S::c, S::ca, {true, [](char c)->bool {return c == 'a'; }}},
			{S::c, S::cm, {true, [](char c)->bool {return c == 'm'; }}},
			{S::h, S::ha, {true, [](char c)->bool {return c == 'a'; }}},
			{S::j, S::jm, {true, [](char c)->bool {return c == 'm'; }}},
			{S::l, S::ld, {true, [](char c)->bool {return c == 'd'; }}},
			{S::m, S::mo, {true, [](char c)->bool {return c == 'o'; }}},
			{S::n, S::no, {true, [](char c)->bool {return c == 'o'; }}},
			{S::p, S::po, {true, [](char c)->bool {return c == 'o'; }}},
			{S::p, S::pu, {true, [](char c)->bool {return c == 'u'; }}},
			{S::r, S::re, {true, [](char c)->bool {return c == 'e'; }}},
			{S::s, S::se, {true, [](char c)->bool {return c == 'e'; }}},
			{S::s, S::sh, {true, [](char c)->bool {return c == 'h'; }}},
			{S::s, S::st, {true, [](char c)->bool {return c == 't'; }}},
			{S::s, S::su, {true, [](char c)->bool {return c == 'u'; }}},

			{S::ad, S::add, {true, [](char c)->bool {return c == 'd'; }}},
			{S::bn, S::bnd, {true, [](char c)->bool {return c == 'd'; }}},
			{S::bo, S::bor, {true, [](char c)->bool {return c == 'r'; }}},
			{S::bx, S::bxr, {true, [](char c)->bool {return c == 'r'; }}},
			{S::ca, S::cal, {true, [](char c)->bool {return c == 'l'; }}},
			{S::cm, S::cmp, {true, [](char c)->bool {return c == 'p'; }}},
			{S::ha, S::hal, {true, [](char c)->bool {return c == 'l'; }}},
			{S::jm, S::jmp, {true, [](char c)->bool {return c == 'p'; }}},
			{S::mo, S::mov, {true, [](char c)->bool {return c == 'v'; }}},
			{S::no, S::nop, {true, [](char c)->bool {return c == 'p'; }}},
			{S::po, S::pop, {true, [](char c)->bool {return c == 'p'; }}},
			{S::pu, S::pus, {true, [](char c)->bool {return c == 's'; }}},
			{S::re, S::ret, {true, [](char c)->bool {return c == 't'; }}},
			{S::se, S::set, {true, [](char c)->bool {return c == 't'; }}},
			{S::sh, S::shi, {true, [](char c)->bool {return c == 'i'; }}},
			{S::su, S::sub, {true, [](char c)->bool {return c == 'b'; }}},

			{S::add, S::addc, {true, [](char c)->bool {return c == 'c'; }}},
			{S::add, S::addi, {true, [](char c)->bool {return c == 'i'; }}},
			{S::bnd, S::bndi, {true, [](char c)->bool {return c == 'i'; }}},
			{S::bor, S::bori, {true, [](char c)->bool {return c == 'i'; }}},
			{S::bxr, S::bxri, {true, [](char c)->bool {return c == 'i'; }}},
			{S::cal, S::call, {true, [](char c)->bool {return c == 'i'; }}},
			{S::hal, S::halt, {true, [](char c)->bool {return c == 't'; }}},
			{S::pus, S::push, {true, [](char c)->bool {return c == 'h'; }}},
			{S::shi, S::shif, {true, [](char c)->bool {return c == 'f'; }}},
			{S::sub, S::subc, {true, [](char c)->bool {return c == 'c'; }}},
			{S::sub, S::subi, {true, [](char c)->bool {return c == 'i'; }}},

			{S::shif, S::shift, {true, [](char c)->bool {return c == 't'; }}},

			{S::shift, S::shiftl, {true, [](char c)->bool {return c == 'l'; }}},
			{S::shift, S::shiftr, {true, [](char c)->bool {return c == 'r'; }}},

			{S::shiftl, S::shiftli, {true, [](char c)->bool {return c == 'i'; }}},
			{S::shiftr, S::shiftri, {true, [](char c)->bool {return c == 'i'; }}},

			{S::__start__, S::_dot_, {true , [](char c)->bool {return c == '.'; }}},
			
			{S::_dot_, S::_dot_1, {true , [](char c)->bool {return c == '1'; }}},
			{S::_dot_, S::_dot_3, {true , [](char c)->bool {return c == '3'; }}},
			{S::_dot_, S::_dot_8, {true , [](char c)->bool {return c == '8'; }}},
			{S::_dot_, S::_dot_b, {true , [](char c)->bool {return c == 'b'; }}},
			{S::_dot_, S::_dot_c, {true , [](char c)->bool {return c == 'c'; }}},
			{S::_dot_, S::_dot_d, {true , [](char c)->bool {return c == 'd'; }}},
			{S::_dot_, S::_dot_f, {true , [](char c)->bool {return c == 'f'; }}},
			{S::_dot_, S::_dot_g, {true , [](char c)->bool {return c == 'g'; }}},
			{S::_dot_, S::_dot_n, {true , [](char c)->bool {return c == 'n'; }}},
			{S::_dot_, S::_dot_o, {true , [](char c)->bool {return c == 'o'; }}},
			{S::_dot_, S::_dot_p, {true , [](char c)->bool {return c == 'p'; }}},
			{S::_dot_, S::_dot_s, {true , [](char c)->bool {return c == 's'; }}},
			{S::_dot_, S::_dot_S, {true , [](char c)->bool {return c == 'S'; }}},
			{S::_dot_, S::_dot_t, {true , [](char c)->bool {return c == 't'; }}},
			{S::_dot_, S::_dot_z, {true , [](char c)->bool {return c == 'z'; }}},

			{ S::_dot_1, S::_dot_16, {true , [](char c)->bool {return c == '6'; }} },
			{ S::_dot_3, S::_dot_32, {true , [](char c)->bool {return c == '2'; }} },
			{ S::_dot_8, S::_dot_8H, {true , [](char c)->bool {return c == 'h' || c == 'H'; }} },
			{ S::_dot_8, S::_dot_8L, {true , [](char c)->bool {return c == 'l' || c == 'L'; }} },
			{ S::_dot_d, S::_dot_da, {true , [](char c)->bool {return c == 'a'; }} },
			{ S::_dot_f, S::_dot_fl, {true , [](char c)->bool {return c == 'l'; }} },
			{ S::_dot_f, S::_dot_fu, {true , [](char c)->bool {return c == 'u'; }} },
			{ S::_dot_g, S::_dot_ge, {true , [](char c)->bool {return c == 'e'; }} },
			{ S::_dot_n, S::_dot_ne, {true , [](char c)->bool {return c == 'e'; }} },
			{ S::_dot_o, S::_dot_on, {true , [](char c)->bool {return c == 'n'; }} },
			{ S::_dot_o, S::_dot_ov, {true , [](char c)->bool {return c == 'v'; }} },
			{ S::_dot_p, S::_dot_pc, {true , [](char c)->bool {return c == 'c'; }} },
			{ S::_dot_p, S::_dot_po, {true , [](char c)->bool {return c == 'o'; }} },
			{ S::_dot_s, S::_dot_sb, {true , [](char c)->bool {return c == 'b'; }} },
			{ S::_dot_s, S::_dot_st, {true , [](char c)->bool {return c == 't'; }} },
			{ S::_dot_s, S::_dot_S1, {true , [](char c)->bool {return c == '1'; }} },
			{ S::_dot_S, S::_dot_S1, {true , [](char c)->bool {return c == '1'; }} },
			{ S::_dot_s, S::_dot_S8, {true , [](char c)->bool {return c == '8'; }} },
			{ S::_dot_S, S::_dot_S8, {true , [](char c)->bool {return c == '8'; }} },
			{ S::_dot_t, S::_dot_te, {true , [](char c)->bool {return c == 'e'; }} },
			{ S::_dot_z, S::_dot_ze, {true , [](char c)->bool {return c == 'e'; }} },

			{ S::_dot_16, S::_dot_16L, {true , [](char c)->bool {return c == 'l' || c == 'L'; }} },
			{ S::_dot_16, S::_dot_16H, {true , [](char c)->bool {return c == 'h' || c == 'H'; }} },
			{ S::_dot_da, S::_dot_dat, {true , [](char c)->bool {return c == 't'; }} },
			{ S::_dot_fl, S::_dot_fla, {true , [](char c)->bool {return c == 'a'; }} },
			{ S::_dot_fu, S::_dot_ful, {true , [](char c)->bool {return c == 'l'; }} },
			{ S::_dot_ge, S::_dot_gen, {true , [](char c)->bool {return c == 'n'; }} },
			{ S::_dot_ne, S::_dot_neg, {true , [](char c)->bool {return c == 'g'; }} },
			{ S::_dot_on, S::_dot_one, {true , [](char c)->bool {return c == 'e'; }} },
			{ S::_dot_ov, S::_dot_ove, {true , [](char c)->bool {return c == 'e'; }} },
			{ S::_dot_po, S::_dot_pos, {true , [](char c)->bool {return c == 's'; }} },
			{ S::_dot_sb, S::_dot_sbp, {true , [](char c)->bool {return c == 'p'; }} },
			{ S::_dot_st, S::_dot_sta, {true , [](char c)->bool {return c == 'a'; }} },
			{ S::_dot_S1, S::_dot_S16, {true , [](char c)->bool {return c == '6'; }} },
			{ S::_dot_te, S::_dot_tex, {true , [](char c)->bool {return c == 'x'; }} },
			{ S::_dot_ze, S::_dot_zer, {true , [](char c)->bool {return c == 'r'; }} },

			{ S::_dot_dat, S::_dot_data, {true , [](char c)->bool {return c == 'a'; }} },
			{ S::_dot_fla, S::_dot_flag, {true , [](char c)->bool {return c == 'g'; }} },
			{ S::_dot_ful, S::_dot_full, {true , [](char c)->bool {return c == 'l'; }} },
			{ S::_dot_gen, S::_dot_gen_openbrack_, {true , [](char c)->bool {return c == '['; }} },
			{ S::_dot_ove, S::_dot_over, {true , [](char c)->bool {return c == 'r'; }} },
			{ S::_dot_sta, S::_dot_stac, {true , [](char c)->bool {return c == 'c'; }} },
			{ S::_dot_S16, S::_dot_S16L, {true , [](char c)->bool {return c == 'l' || c == 'L'; }} },
			{ S::_dot_tex, S::_dot_text, {true , [](char c)->bool {return c == 't'; }} },
			{ S::_dot_zer, S::_dot_zero, {true , [](char c)->bool {return c == 'o'; }} },

			{ S::_dot_over, S::_dot_overf, {true , [](char c)->bool {return c == 'f'; }} },
			{ S::_dot_overf, S::_dot_overfl, {true , [](char c)->bool {return c == 'l'; }} },
			{ S::_dot_overfl, S::_dot_overflo, {true , [](char c)->bool {return c == 'o'; }} },
			{ S::_dot_overflo, S::_dot_overflow, {true , [](char c)->bool {return c == 'w'; }} },
			{ S::_dot_stac, S::_dot_stack, {true , [](char c)->bool {return c == 'k'; }} },

			{ S::_dot_gen_openbrack_, S::_dot_gen_openbrack_0, {true , [](char c)->bool {return c == '0'; }} },
			{ S::_dot_gen_openbrack_, S::_dot_gen_openbrack_1, {true , [](char c)->bool {return c == '1'; }} },
			{ S::_dot_gen_openbrack_, S::_dot_gen_openbrack_2, {true , [](char c)->bool {return c == '2'; }} },
			{ S::_dot_gen_openbrack_, S::_dot_gen_openbrack_3, {true , [](char c)->bool {return c == '3'; }} },
			{ S::_dot_gen_openbrack_, S::_dot_gen_openbrack_4, {true , [](char c)->bool {return c == '4'; }} },
			{ S::_dot_gen_openbrack_, S::_dot_gen_openbrack_5, {true , [](char c)->bool {return c == '5'; }} },
			{ S::_dot_gen_openbrack_, S::_dot_gen_openbrack_6, {true , [](char c)->bool {return c == '6'; }} },
			{ S::_dot_gen_openbrack_, S::_dot_gen_openbrack_7, {true , [](char c)->bool {return c == '7'; }} },
			{ S::_dot_gen_openbrack_, S::_dot_gen_openbrack_8, {true , [](char c)->bool {return c == '8'; }} },
			{ S::_dot_gen_openbrack_, S::_dot_gen_openbrack_9, {true , [](char c)->bool {return c == '9'; }} },
			{ S::_dot_gen_openbrack_1, S::_dot_gen_openbrack_10, {true , [](char c)->bool {return c == '0'; }} },
			{ S::_dot_gen_openbrack_1, S::_dot_gen_openbrack_11, {true , [](char c)->bool {return c == '1'; }} },
			{ S::_dot_gen_openbrack_1, S::_dot_gen_openbrack_12, {true , [](char c)->bool {return c == '2'; }} },
			{ S::_dot_gen_openbrack_1, S::_dot_gen_openbrack_13, {true , [](char c)->bool {return c == '3'; }} },
			{ S::_dot_gen_openbrack_1, S::_dot_gen_openbrack_14, {true , [](char c)->bool {return c == '4'; }} },
			{ S::_dot_gen_openbrack_1, S::_dot_gen_openbrack_15, {true , [](char c)->bool {return c == '5'; }} },
			{ S::_dot_gen_openbrack_1, S::_dot_gen_openbrack_16, {true , [](char c)->bool {return c == '6'; }} },
			{ S::_dot_gen_openbrack_1, S::_dot_gen_openbrack_17, {true , [](char c)->bool {return c == '7'; }} },
			{ S::_dot_gen_openbrack_1, S::_dot_gen_openbrack_18, {true , [](char c)->bool {return c == '8'; }} },
			{ S::_dot_gen_openbrack_1, S::_dot_gen_openbrack_19, {true , [](char c)->bool {return c == '9'; }} },
			{ S::_dot_gen_openbrack_2, S::_dot_gen_openbrack_20, {true , [](char c)->bool {return c == '0'; }} },
			{ S::_dot_gen_openbrack_2, S::_dot_gen_openbrack_21, {true , [](char c)->bool {return c == '1'; }} },
			{ S::_dot_gen_openbrack_2, S::_dot_gen_openbrack_22, {true , [](char c)->bool {return c == '2'; }} },
			{ S::_dot_gen_openbrack_2, S::_dot_gen_openbrack_23, {true , [](char c)->bool {return c == '3'; }} },
			{ S::_dot_gen_openbrack_2, S::_dot_gen_openbrack_24, {true , [](char c)->bool {return c == '4'; }} },

			{ S::_dot_gen_openbrack_0, S::_dot_gen_openbrack_0_closebrack_, {true , [](char c)->bool {return c == ']'; }} },
			{ S::_dot_gen_openbrack_1, S::_dot_gen_openbrack_1_closebrack_, {true , [](char c)->bool {return c == ']'; }} },
			{ S::_dot_gen_openbrack_2, S::_dot_gen_openbrack_2_closebrack_, {true , [](char c)->bool {return c == ']'; }} },
			{ S::_dot_gen_openbrack_3, S::_dot_gen_openbrack_3_closebrack_, {true , [](char c)->bool {return c == ']'; }} },
			{ S::_dot_gen_openbrack_4, S::_dot_gen_openbrack_4_closebrack_, {true , [](char c)->bool {return c == ']'; }} },
			{ S::_dot_gen_openbrack_5, S::_dot_gen_openbrack_5_closebrack_, {true , [](char c)->bool {return c == ']'; }} },
			{ S::_dot_gen_openbrack_6, S::_dot_gen_openbrack_6_closebrack_, {true , [](char c)->bool {return c == ']'; }} },
			{ S::_dot_gen_openbrack_7, S::_dot_gen_openbrack_7_closebrack_, {true , [](char c)->bool {return c == ']'; }} },
			{ S::_dot_gen_openbrack_8, S::_dot_gen_openbrack_8_closebrack_, {true , [](char c)->bool {return c == ']'; }} },
			{ S::_dot_gen_openbrack_9, S::_dot_gen_openbrack_9_closebrack_, {true , [](char c)->bool {return c == ']'; }} },
			{ S::_dot_gen_openbrack_10, S::_dot_gen_openbrack_10_closebrack_, {true , [](char c)->bool {return c == ']'; }} },
			{ S::_dot_gen_openbrack_11, S::_dot_gen_openbrack_11_closebrack_, {true , [](char c)->bool {return c == ']'; }} },
			{ S::_dot_gen_openbrack_12, S::_dot_gen_openbrack_12_closebrack_, {true , [](char c)->bool {return c == ']'; }} },
			{ S::_dot_gen_openbrack_13, S::_dot_gen_openbrack_13_closebrack_, {true , [](char c)->bool {return c == ']'; }} },
			{ S::_dot_gen_openbrack_14, S::_dot_gen_openbrack_14_closebrack_, {true , [](char c)->bool {return c == ']'; }} },
			{ S::_dot_gen_openbrack_15, S::_dot_gen_openbrack_15_closebrack_, {true , [](char c)->bool {return c == ']'; }} },
			{ S::_dot_gen_openbrack_16, S::_dot_gen_openbrack_16_closebrack_, {true , [](char c)->bool {return c == ']'; }} },
			{ S::_dot_gen_openbrack_17, S::_dot_gen_openbrack_17_closebrack_, {true , [](char c)->bool {return c == ']'; }} },
			{ S::_dot_gen_openbrack_18, S::_dot_gen_openbrack_18_closebrack_, {true , [](char c)->bool {return c == ']'; }} },
			{ S::_dot_gen_openbrack_19, S::_dot_gen_openbrack_19_closebrack_, {true , [](char c)->bool {return c == ']'; }} },
			{ S::_dot_gen_openbrack_20, S::_dot_gen_openbrack_20_closebrack_, {true , [](char c)->bool {return c == ']'; }} },
			{ S::_dot_gen_openbrack_21, S::_dot_gen_openbrack_21_closebrack_, {true , [](char c)->bool {return c == ']'; }} },
			{ S::_dot_gen_openbrack_22, S::_dot_gen_openbrack_22_closebrack_, {true , [](char c)->bool {return c == ']'; }} },
			{ S::_dot_gen_openbrack_23, S::_dot_gen_openbrack_23_closebrack_, {true , [](char c)->bool {return c == ']'; }} },
			{ S::_dot_gen_openbrack_24, S::_dot_gen_openbrack_24_closebrack_, {true , [](char c)->bool {return c == ']'; }} },

			{ S::__start__, S::_percent_, {true , [](char c)->bool {return c == '%'; }} },

			{ S::_percent_, S::_percent_f, {true , [](char c)->bool {return c == 'f'; }} },
			{ S::_percent_, S::_percent_g, {true , [](char c)->bool {return c == 'g'; }} },
			{ S::_percent_, S::_percent_o, {true , [](char c)->bool {return c == 'o'; }} },
			{ S::_percent_, S::_percent_p, {true , [](char c)->bool {return c == 'p'; }} },
			{ S::_percent_, S::_percent_s, {true , [](char c)->bool {return c == 's'; }} },
			{ S::_percent_, S::_percent_z, {true , [](char c)->bool {return c == 'z'; }} },

			{ S::_percent_f, S::_percent_fl, {true , [](char c)->bool {return c == 'l'; }} },
			{ S::_percent_f, S::_percent_fu, {true , [](char c)->bool {return c == 'u'; }} },
			{ S::_percent_g, S::_percent_ge, {true , [](char c)->bool {return c == 'e'; }} },
			{ S::_percent_o, S::_percent_on, {true , [](char c)->bool {return c == 'n'; }} },
			{ S::_percent_p, S::_percent_pc, {true , [](char c)->bool {return c == 'c'; }} },
			{ S::_percent_s, S::_percent_sb, {true , [](char c)->bool {return c == 'b'; }} },
			{ S::_percent_s, S::_percent_st, {true , [](char c)->bool {return c == 't'; }} },
			{ S::_percent_z, S::_percent_ze, {true , [](char c)->bool {return c == 'e'; }} },

			{ S::_percent_fl, S::_percent_fla, {true , [](char c)->bool {return c == 'a'; }} },
			{ S::_percent_fu, S::_percent_ful, {true , [](char c)->bool {return c == 'l'; }} },
			{ S::_percent_ge, S::_percent_gen, {true , [](char c)->bool {return c == 'n'; }} },
			{ S::_percent_on, S::_percent_one, {true , [](char c)->bool {return c == 'e'; }} },
			{ S::_percent_st, S::_percent_sta, {true , [](char c)->bool {return c == 'a'; }} },
			{ S::_percent_sb, S::_percent_sbp, {true , [](char c)->bool {return c == 'p'; }} },
			{ S::_percent_ze, S::_percent_zer, {true , [](char c)->bool {return c == 'r'; }} },

			{ S::_percent_fla, S::_percent_flag, {true , [](char c)->bool {return c == 'g'; }} },
			{ S::_percent_ful, S::_percent_full, {true , [](char c)->bool {return c == 'l'; }} },
			{ S::_percent_gen, S::_percent_gen_openbrack_, {true , [](char c)->bool {return c == '['; }} },
			{ S::_percent_sta, S::_percent_stac, {true , [](char c)->bool {return c == 'c'; }} },
			{ S::_percent_zer, S::_percent_zero, {true , [](char c)->bool {return c == 'o'; }} },

			{ S::_percent_stac, S::_percent_stack, {true , [](char c)->bool {return c == 'k'; }} },

			{ S::_percent_gen_openbrack_, S::_percent_gen_openbrack_0, {true , [](char c)->bool {return c == '0'; }} },
			{ S::_percent_gen_openbrack_, S::_percent_gen_openbrack_1, {true , [](char c)->bool {return c == '1'; }} },
			{ S::_percent_gen_openbrack_, S::_percent_gen_openbrack_2, {true , [](char c)->bool {return c == '2'; }} },
			{ S::_percent_gen_openbrack_, S::_percent_gen_openbrack_3, {true , [](char c)->bool {return c == '3'; }} },
			{ S::_percent_gen_openbrack_, S::_percent_gen_openbrack_4, {true , [](char c)->bool {return c == '4'; }} },
			{ S::_percent_gen_openbrack_, S::_percent_gen_openbrack_5, {true , [](char c)->bool {return c == '5'; }} },
			{ S::_percent_gen_openbrack_, S::_percent_gen_openbrack_6, {true , [](char c)->bool {return c == '6'; }} },
			{ S::_percent_gen_openbrack_, S::_percent_gen_openbrack_7, {true , [](char c)->bool {return c == '7'; }} },
			{ S::_percent_gen_openbrack_, S::_percent_gen_openbrack_8, {true , [](char c)->bool {return c == '8'; }} },
			{ S::_percent_gen_openbrack_, S::_percent_gen_openbrack_9, {true , [](char c)->bool {return c == '9'; }} },
			{ S::_percent_gen_openbrack_1, S::_percent_gen_openbrack_10, {true , [](char c)->bool {return c == '0'; }} },
			{ S::_percent_gen_openbrack_1, S::_percent_gen_openbrack_11, {true , [](char c)->bool {return c == '1'; }} },
			{ S::_percent_gen_openbrack_1, S::_percent_gen_openbrack_12, {true , [](char c)->bool {return c == '2'; }} },
			{ S::_percent_gen_openbrack_1, S::_percent_gen_openbrack_13, {true , [](char c)->bool {return c == '3'; }} },
			{ S::_percent_gen_openbrack_1, S::_percent_gen_openbrack_14, {true , [](char c)->bool {return c == '4'; }} },
			{ S::_percent_gen_openbrack_1, S::_percent_gen_openbrack_15, {true , [](char c)->bool {return c == '5'; }} },
			{ S::_percent_gen_openbrack_1, S::_percent_gen_openbrack_16, {true , [](char c)->bool {return c == '6'; }} },
			{ S::_percent_gen_openbrack_1, S::_percent_gen_openbrack_17, {true , [](char c)->bool {return c == '7'; }} },
			{ S::_percent_gen_openbrack_1, S::_percent_gen_openbrack_18, {true , [](char c)->bool {return c == '8'; }} },
			{ S::_percent_gen_openbrack_1, S::_percent_gen_openbrack_19, {true , [](char c)->bool {return c == '9'; }} },
			{ S::_percent_gen_openbrack_2, S::_percent_gen_openbrack_20, {true , [](char c)->bool {return c == '0'; }} },
			{ S::_percent_gen_openbrack_2, S::_percent_gen_openbrack_21, {true , [](char c)->bool {return c == '1'; }} },
			{ S::_percent_gen_openbrack_2, S::_percent_gen_openbrack_22, {true , [](char c)->bool {return c == '2'; }} },
			{ S::_percent_gen_openbrack_2, S::_percent_gen_openbrack_23, {true , [](char c)->bool {return c == '3'; }} },
			{ S::_percent_gen_openbrack_2, S::_percent_gen_openbrack_24, {true , [](char c)->bool {return c == '4'; }} },

			{ S::_percent_gen_openbrack_0, S::_percent_gen_openbrack_0_closebrack_, {true , [](char c)->bool {return c == ']'; }} },
			{ S::_percent_gen_openbrack_1, S::_percent_gen_openbrack_1_closebrack_, {true , [](char c)->bool {return c == ']'; }} },
			{ S::_percent_gen_openbrack_2, S::_percent_gen_openbrack_2_closebrack_, {true , [](char c)->bool {return c == ']'; }} },
			{ S::_percent_gen_openbrack_3, S::_percent_gen_openbrack_3_closebrack_, {true , [](char c)->bool {return c == ']'; }} },
			{ S::_percent_gen_openbrack_4, S::_percent_gen_openbrack_4_closebrack_, {true , [](char c)->bool {return c == ']'; }} },
			{ S::_percent_gen_openbrack_5, S::_percent_gen_openbrack_5_closebrack_, {true , [](char c)->bool {return c == ']'; }} },
			{ S::_percent_gen_openbrack_6, S::_percent_gen_openbrack_6_closebrack_, {true , [](char c)->bool {return c == ']'; }} },
			{ S::_percent_gen_openbrack_7, S::_percent_gen_openbrack_7_closebrack_, {true , [](char c)->bool {return c == ']'; }} },
			{ S::_percent_gen_openbrack_8, S::_percent_gen_openbrack_8_closebrack_, {true , [](char c)->bool {return c == ']'; }} },
			{ S::_percent_gen_openbrack_9, S::_percent_gen_openbrack_9_closebrack_, {true , [](char c)->bool {return c == ']'; }} },
			{ S::_percent_gen_openbrack_10, S::_percent_gen_openbrack_10_closebrack_, {true , [](char c)->bool {return c == ']'; }} },
			{ S::_percent_gen_openbrack_11, S::_percent_gen_openbrack_11_closebrack_, {true , [](char c)->bool {return c == ']'; }} },
			{ S::_percent_gen_openbrack_12, S::_percent_gen_openbrack_12_closebrack_, {true , [](char c)->bool {return c == ']'; }} },
			{ S::_percent_gen_openbrack_13, S::_percent_gen_openbrack_13_closebrack_, {true , [](char c)->bool {return c == ']'; }} },
			{ S::_percent_gen_openbrack_14, S::_percent_gen_openbrack_14_closebrack_, {true , [](char c)->bool {return c == ']'; }} },
			{ S::_percent_gen_openbrack_15, S::_percent_gen_openbrack_15_closebrack_, {true , [](char c)->bool {return c == ']'; }} },
			{ S::_percent_gen_openbrack_16, S::_percent_gen_openbrack_16_closebrack_, {true , [](char c)->bool {return c == ']'; }} },
			{ S::_percent_gen_openbrack_17, S::_percent_gen_openbrack_17_closebrack_, {true , [](char c)->bool {return c == ']'; }} },
			{ S::_percent_gen_openbrack_18, S::_percent_gen_openbrack_18_closebrack_, {true , [](char c)->bool {return c == ']'; }} },
			{ S::_percent_gen_openbrack_19, S::_percent_gen_openbrack_19_closebrack_, {true , [](char c)->bool {return c == ']'; }} },
			{ S::_percent_gen_openbrack_20, S::_percent_gen_openbrack_20_closebrack_, {true , [](char c)->bool {return c == ']'; }} },
			{ S::_percent_gen_openbrack_21, S::_percent_gen_openbrack_21_closebrack_, {true , [](char c)->bool {return c == ']'; }} },
			{ S::_percent_gen_openbrack_22, S::_percent_gen_openbrack_22_closebrack_, {true , [](char c)->bool {return c == ']'; }} },
			{ S::_percent_gen_openbrack_23, S::_percent_gen_openbrack_23_closebrack_, {true , [](char c)->bool {return c == ']'; }} },
			{ S::_percent_gen_openbrack_24, S::_percent_gen_openbrack_24_closebrack_, {true , [](char c)->bool {return c == ']'; }} },

			{ S::__start__, S::__firstnum__, {true , [](char c)->bool {return '0' <= c && c <= '9'; }}},
			{ S::__firstnum__, S::__hexnum__, {true , [](char c)->bool {return c == 'x' || c == 'X'; }}},
			{ S::__firstnum__, S::__decnum__, {false , [](char c)->bool {return true; }} },
			{ S::__firstnum__, S::__octnum__, {true , [](char c)->bool {return c == 'o' || c == 'O'; }}},
			{ S::__firstnum__, S::__binnum__, {true , [](char c)->bool {return c == 'b' || c == 'B'; }}},

			{ S::__hexnum__, S::__hexnum__, {true , [](char c)->bool {return ('0' <= c && c <= '9') || ('a' <= c && c <= 'f') || ('A' <= c && c <= 'F'); }}},
			{ S::__decnum__, S::__decnum__, {true , [](char c)->bool {return '0' <= c && c <= '9'; }} },
			{ S::__octnum__, S::__octnum__, {true , [](char c)->bool {return '0' <= c && c <= '7'; }} },
			{ S::__binnum__, S::__binnum__, {true , [](char c)->bool {return '0' <= c && c <= '1'; }} },

			{ S::__start__, S::_openparen_, {true , [](char c)->bool {return c == '('; }} },
			{ S::__start__, S::_closeparen_, {true , [](char c)->bool {return c == ')'; }} },
			{ S::__start__, S::_colon_, {true , [](char c)->bool {return c == ':'; }} },
			{ S::__start__, S::_semicolon_, {true , [](char c)->bool {return c == ';'; }} },
			{ S::__start__, S::_plus_, {true , [](char c)->bool {return c == '+'; }} },
			{ S::__start__, S::_minus_, {true , [](char c)->bool {return c == '-'; }} },
			{ S::__start__, S::_star_, {true , [](char c)->bool {return c == '*'; }} },
			{ S::_star_, S::_star_star_, {true , [](char c)->bool {return c == '*'; }} },
			{ S::__start__, S::_slash_, {true , [](char c)->bool {return c == '/'; }} },
			{ S::__start__, S::_ampersend_, {true , [](char c)->bool {return c == '&'; }} },
			{ S::__start__, S::_verticalbar_, {true , [](char c)->bool {return c == '|'; }} },
			{ S::__start__, S::_caret_, {true , [](char c)->bool {return c == '^'; }} },

			{ S::__start__, S::_whitespace_, {true , [](char c)->bool {return c == ' '; }} },
			{ S::__start__, S::_newline_, {true , [](char c)->bool {return c == '\n'; }} },

			{ S::_whitespace_, S::_whitespace_, {true , [](char c)->bool {return c == ' ' && c != '\n'; }}},

			{ S::__start__, S::__InterTreeEpsilonID__, {false, [](char c)->bool {return ('9' < c || c < '0'); }}},

			{ S::a, S::__InterTreeEpsilonID__, {false, [](char c)->bool {return true;}}},
			{ S::ad, S::__InterTreeEpsilonID__, {false, [](char c)->bool {return true;}}}, { S::add, S::__KeyTreeEpsilonID__, {false, [](char c)->bool {return false;  }}}, {S::addc, S::__KeyTreeEpsilonID__, {false, [](char c)->bool {return false;  }}}, {S::addi, S::__KeyTreeEpsilonID__, {false, [](char c)->bool {return false;  }}},

			{ S::b, S::__InterTreeEpsilonID__, {false, [](char c)->bool {return true;}}},
			{ S::bn, S::__InterTreeEpsilonID__, {false, [](char c)->bool {return true;}}}, { S::bnd, S::__KeyTreeEpsilonID__, {false, [](char c)->bool {return false; }}}, { S::bndi, S::__KeyTreeEpsilonID__, {false, [](char c)->bool {return false; }}},
			{ S::bo, S::__InterTreeEpsilonID__, {false, [](char c)->bool {return true;}}}, { S::bor, S::__KeyTreeEpsilonID__, {false, [](char c)->bool {return false; }}}, { S::bori, S::__KeyTreeEpsilonID__, {false, [](char c)->bool {return false; }}},
			{ S::bx, S::__InterTreeEpsilonID__, {false, [](char c)->bool {return true;}}}, { S::bxr, S::__KeyTreeEpsilonID__, {false, [](char c)->bool {return false; }}}, { S::bxri, S::__KeyTreeEpsilonID__, {false, [](char c)->bool {return false; }}},

			{ S::c, S::__InterTreeEpsilonID__, {false, [](char c)->bool {return true;}}},
			{ S::ca, S::__InterTreeEpsilonID__, {false, [](char c)->bool {return true;}}}, { S::cal, S::__InterTreeEpsilonID__, {false, [](char c)->bool {return true;}}}, { S::call, S::__KeyTreeEpsilonID__, {false, [](char c)->bool {return false; }}},
			{ S::cm, S::__InterTreeEpsilonID__, {false, [](char c)->bool {return true;}}}, { S::cmp, S::__InterTreeEpsilonID__, {false, [](char c)->bool {return true;}}},

			{ S::h, S::__InterTreeEpsilonID__, {false, [](char c)->bool {return true;}}},
			{ S::ha, S::__InterTreeEpsilonID__, {false, [](char c)->bool {return true;}}}, { S::hal, S::__InterTreeEpsilonID__, {false, [](char c)->bool {return true;}}}, { S::halt, S::__KeyTreeEpsilonID__, {false, [](char c)->bool {return false; }}},

			{ S::j, S::__InterTreeEpsilonID__, {false, [](char c)->bool {return true;}}},
			{ S::jm, S::__InterTreeEpsilonID__, {false, [](char c)->bool {return true;}}}, { S::jmp, S::__KeyTreeEpsilonID__, {false, [](char c)->bool {return false; }}},

			{ S::l, S::__InterTreeEpsilonID__, {false, [](char c)->bool {return true;}}},
			{ S::ld, S::__KeyTreeEpsilonID__, {false, [](char c)->bool {return false; }}},

			{ S::m, S::__InterTreeEpsilonID__, {false, [](char c)->bool {return true;}}},
			{ S::mo, S::__InterTreeEpsilonID__, {false, [](char c)->bool {return true;}}}, { S::mov, S::__KeyTreeEpsilonID__, {false, [](char c)->bool {return false; }}},

			{ S::n, S::__InterTreeEpsilonID__, {false, [](char c)->bool {return true;}}},
			{ S::no, S::__InterTreeEpsilonID__, {false, [](char c)->bool {return true;}}}, { S::nop, S::__KeyTreeEpsilonID__, {false, [](char c)->bool {return false; }}},

			{ S::p, S::__InterTreeEpsilonID__, {false, [](char c)->bool {return true;}}},
			{ S::po, S::__InterTreeEpsilonID__, {false, [](char c)->bool {return true;}}}, { S::pop, S::__KeyTreeEpsilonID__, {false, [](char c)->bool {return false; }}},
			{ S::pu, S::__InterTreeEpsilonID__, {false, [](char c)->bool {return true;}}}, { S::pus, S::__InterTreeEpsilonID__, {false, [](char c)->bool {return true;}}}, { S::push, S::__KeyTreeEpsilonID__, {false, [](char c)->bool {return false; }}},

			{ S::r, S::__InterTreeEpsilonID__, {false, [](char c)->bool {return true;}}},
			{ S::re, S::__InterTreeEpsilonID__, {false, [](char c)->bool {return true;}}}, { S::ret, S::__KeyTreeEpsilonID__, {false, [](char c)->bool {return false; }}},

			{ S::s, S::__InterTreeEpsilonID__, {false, [](char c)->bool {return true;}}},
			{ S::se, S::__InterTreeEpsilonID__, {false, [](char c)->bool {return true;}}}, { S::set, S::__KeyTreeEpsilonID__, {false, [](char c)->bool {return false; }}},
			{ S::sh, S::__InterTreeEpsilonID__, {false, [](char c)->bool {return true;}}}, { S::shi, S::__InterTreeEpsilonID__, {false, [](char c)->bool {return true;}}}, { S::shif, S::__InterTreeEpsilonID__, {false, [](char c)->bool {return true;}}}, { S::shift, S::__InterTreeEpsilonID__, {false, [](char c)->bool {return true;}}},
			{ S::shiftl, S::__KeyTreeEpsilonID__, {false, [](char c)->bool {return false; }}}, { S::shiftli, S::__KeyTreeEpsilonID__, {false, [](char c)->bool {return false; }}}, { S::shiftr, S::__KeyTreeEpsilonID__, {false, [](char c)->bool {return false; }}}, { S::shiftri, S::__KeyTreeEpsilonID__, {false, [](char c)->bool {return false; }}},
			{ S::st, S::__KeyTreeEpsilonID__, {false, [](char c)->bool {return false; }}},
			{ S::su, S::__InterTreeEpsilonID__, {false, [](char c)->bool {return true;}}}, { S::sub, S::__KeyTreeEpsilonID__, {false, [](char c)->bool {return false; }}}, { S::subc, S::__KeyTreeEpsilonID__, {false, [](char c)->bool {return false; }}}, { S::subi, S::__KeyTreeEpsilonID__, {false, [](char c)->bool {return false; }}},

			{ S::_dot_, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true;}}},

			{ S::_dot_1, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true;}}},
			{ S::_dot_16, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true;}}}, { S::_dot_16H, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false; }}}, { S::_dot_16L, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false; }}},

			{ S::_dot_3, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true;}}},
			{ S::_dot_32, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false; }}},

			{ S::_dot_8, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true;}}}, { S::_dot_8H, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false; }}}, { S::_dot_8L, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false; }}},

			{ S::_dot_b, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true;}}}, { S::_dot_bs, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true;}}}, { S::_dot_bss, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false; }}},

			{ S::_dot_c, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true;}}},
			{ S::_dot_ca, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true;}}}, { S::_dot_car, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true;}}}, { S::_dot_carr, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true;}}}, { S::_dot_carry, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false; }}}, { S::_dot_carry4, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false; }}},

			{ S::_dot_d, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true;}}},
			{ S::_dot_da, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true;}}}, { S::_dot_dat, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true;}}}, { S::_dot_data, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false; }}},

			{ S::_dot_f, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true;}}},
			{ S::_dot_fu, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true;}}}, { S::_dot_ful, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true;}}}, { S::_dot_full, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false; }}},
			{ S::_dot_fl, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true;}}}, { S::_dot_fla, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true;}}}, { S::_dot_flag, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false; }}},

			{ S::_dot_g, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true;}}},
			{ S::_dot_ge, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true;}}}, { S::_dot_gen, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return false;  }}}, {S::_dot_gen_openbrack_, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true; }}},
			{ S::_dot_gen_openbrack_0, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true;}}}, { S::_dot_gen_openbrack_0_closebrack_, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false; }}},
			{ S::_dot_gen_openbrack_1, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true;}}}, { S::_dot_gen_openbrack_1_closebrack_, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false; }}},
			{ S::_dot_gen_openbrack_2, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true;}}}, { S::_dot_gen_openbrack_2_closebrack_, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false; }}},
			{ S::_dot_gen_openbrack_3, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true;}}}, { S::_dot_gen_openbrack_3_closebrack_, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false; }}},
			{ S::_dot_gen_openbrack_4, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true;}}}, { S::_dot_gen_openbrack_4_closebrack_, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false; }}},
			{ S::_dot_gen_openbrack_5, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true;}}}, { S::_dot_gen_openbrack_5_closebrack_, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false; }}},
			{ S::_dot_gen_openbrack_6, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true;}}}, { S::_dot_gen_openbrack_6_closebrack_, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false; }}},
			{ S::_dot_gen_openbrack_7, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true;}}}, { S::_dot_gen_openbrack_7_closebrack_, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false; }}},
			{ S::_dot_gen_openbrack_8, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true;}}}, { S::_dot_gen_openbrack_8_closebrack_, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false; }}},
			{ S::_dot_gen_openbrack_9, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true;}}}, { S::_dot_gen_openbrack_9_closebrack_, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false; }}},
			{ S::_dot_gen_openbrack_10, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true;}}}, { S::_dot_gen_openbrack_10_closebrack_, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false; }}},
			{ S::_dot_gen_openbrack_11, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true;}}}, { S::_dot_gen_openbrack_11_closebrack_, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false; }}},
			{ S::_dot_gen_openbrack_12, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true;}}}, { S::_dot_gen_openbrack_12_closebrack_, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false; }}},
			{ S::_dot_gen_openbrack_13, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true;}}}, { S::_dot_gen_openbrack_13_closebrack_, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false; }}},
			{ S::_dot_gen_openbrack_14, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true;}}}, { S::_dot_gen_openbrack_14_closebrack_, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false; }}},
			{ S::_dot_gen_openbrack_15, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true;}}}, { S::_dot_gen_openbrack_15_closebrack_, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false; }}},
			{ S::_dot_gen_openbrack_16, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true;}}}, { S::_dot_gen_openbrack_16_closebrack_, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false; }}},
			{ S::_dot_gen_openbrack_17, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true;}}}, { S::_dot_gen_openbrack_17_closebrack_, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false; }}},
			{ S::_dot_gen_openbrack_18, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true;}}}, { S::_dot_gen_openbrack_18_closebrack_, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false; }}},
			{ S::_dot_gen_openbrack_19, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true;}}}, { S::_dot_gen_openbrack_19_closebrack_, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false; }}},
			{ S::_dot_gen_openbrack_20, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true;}}}, { S::_dot_gen_openbrack_20_closebrack_, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false; }}},
			{ S::_dot_gen_openbrack_21, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true;}}}, { S::_dot_gen_openbrack_21_closebrack_, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false; }}},
			{ S::_dot_gen_openbrack_22, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true;}}}, { S::_dot_gen_openbrack_22_closebrack_, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false; }}},
			{ S::_dot_gen_openbrack_23, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true;}}}, { S::_dot_gen_openbrack_23_closebrack_, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false; }}},
			{ S::_dot_gen_openbrack_24, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true;}}}, { S::_dot_gen_openbrack_24_closebrack_, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false; }}},

			{ S::_dot_n, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true;}}},
			{ S::_dot_ne, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true;}}}, { S::_dot_neg, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false; }}},

			{ S::_dot_o, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true;}}},
			{ S::_dot_on, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true;}}}, { S::_dot_one, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false; }}},
			{ S::_dot_ov, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true;}}}, { S::_dot_ove, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true;}}}, { S::_dot_over, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true;}}}, { S::_dot_overf, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true;}}},
			{ S::_dot_overfl, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true;}}}, { S::_dot_overflo, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true;}}}, { S::_dot_overflow, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false; }}},

			{ S::_dot_p, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true;}}},
			{ S::_dot_pc, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false; }}},
			{ S::_dot_po, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true;}}}, { S::_dot_pos, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false; }}},

			{ S::_dot_s, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true;}}},
			{ S::_dot_sb, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true;}}}, { S::_dot_sbp, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false; }}},
			{ S::_dot_st, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true;}}}, { S::_dot_sta, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true;}}}, { S::_dot_stac, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true;}}}, { S::_dot_stack, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false; }}},
			{ S::_dot_S1, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true;}}}, { S::_dot_S16, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true;}}}, { S::_dot_S16L, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false; }}},
			{ S::_dot_S8, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true;}}}, { S::_dot_S8H, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false; }}}, { S::_dot_S8L, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false; }}},

			{ S::_dot_t, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true;}}},
			{ S::_dot_te, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true;}}}, { S::_dot_tex, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true;}}}, { S::_dot_text, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false; }}},

			{ S::_dot_z, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true;}}},
			{ S::_dot_ze, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true;}}}, { S::_dot_zer, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true;}}}, { S::_dot_zero, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false; }}},

			{ S::_percent_, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false;  }}},

			{ S::_percent_f, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true; }} },
			{ S::_percent_fl, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true; }} }, { S::_percent_fla, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true; }} }, { S::_percent_flag, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false;  }} },
			{ S::_percent_fu, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true; }} }, { S::_percent_ful, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true; }} }, { S::_percent_full, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false;  }} },

			{ S::_percent_g, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true; }} },
			{ S::_percent_ge, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true; }} }, { S::_percent_gen, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true; }} },

			{ S::_percent_gen_openbrack_, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true; }} },
			{ S::_percent_gen_openbrack_0, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true; }} }, { S::_percent_gen_openbrack_0_closebrack_, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false;  }} },
			{ S::_percent_gen_openbrack_1, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true; }} }, { S::_percent_gen_openbrack_1_closebrack_, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false;  }} },
			{ S::_percent_gen_openbrack_2, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true; }} }, { S::_percent_gen_openbrack_2_closebrack_, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false;  }} },
			{ S::_percent_gen_openbrack_3, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true; }} }, { S::_percent_gen_openbrack_3_closebrack_, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false;  }} },
			{ S::_percent_gen_openbrack_4, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true; }} }, { S::_percent_gen_openbrack_4_closebrack_, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false;  }} },
			{ S::_percent_gen_openbrack_5, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true; }} }, { S::_percent_gen_openbrack_5_closebrack_, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false;  }} },
			{ S::_percent_gen_openbrack_6, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true; }} }, { S::_percent_gen_openbrack_6_closebrack_, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false;  }} },
			{ S::_percent_gen_openbrack_7, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true; }} }, { S::_percent_gen_openbrack_7_closebrack_, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false;  }} },
			{ S::_percent_gen_openbrack_8, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true; }} }, { S::_percent_gen_openbrack_8_closebrack_, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false;  }} },
			{ S::_percent_gen_openbrack_9, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true; }} }, { S::_percent_gen_openbrack_9_closebrack_, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false;  }} },
			{ S::_percent_gen_openbrack_10, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true; }} }, { S::_percent_gen_openbrack_10_closebrack_, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false;  }} },
			{ S::_percent_gen_openbrack_11, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true; }} }, { S::_percent_gen_openbrack_11_closebrack_, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false;  }} },
			{ S::_percent_gen_openbrack_12, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true; }} }, { S::_percent_gen_openbrack_12_closebrack_, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false;  }} },
			{ S::_percent_gen_openbrack_13, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true; }} }, { S::_percent_gen_openbrack_13_closebrack_, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false;  }} },
			{ S::_percent_gen_openbrack_14, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true; }} }, { S::_percent_gen_openbrack_14_closebrack_, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false;  }} },
			{ S::_percent_gen_openbrack_15, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true; }} }, { S::_percent_gen_openbrack_15_closebrack_, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false;  }} },
			{ S::_percent_gen_openbrack_16, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true; }} }, { S::_percent_gen_openbrack_16_closebrack_, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false;  }} },
			{ S::_percent_gen_openbrack_17, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true; }} }, { S::_percent_gen_openbrack_17_closebrack_, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false;  }} },
			{ S::_percent_gen_openbrack_18, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true; }} }, { S::_percent_gen_openbrack_18_closebrack_, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false;  }} },
			{ S::_percent_gen_openbrack_19, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true; }} }, { S::_percent_gen_openbrack_19_closebrack_, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false;  }} },
			{ S::_percent_gen_openbrack_20, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true; }} }, { S::_percent_gen_openbrack_20_closebrack_, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false;  }} },
			{ S::_percent_gen_openbrack_21, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true; }} }, { S::_percent_gen_openbrack_21_closebrack_, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false;  }} },
			{ S::_percent_gen_openbrack_22, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true; }} }, { S::_percent_gen_openbrack_22_closebrack_, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false;  }} },
			{ S::_percent_gen_openbrack_23, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true; }} }, { S::_percent_gen_openbrack_23_closebrack_, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false;  }} },
			{ S::_percent_gen_openbrack_24, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true; }} }, { S::_percent_gen_openbrack_24_closebrack_, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false;  }} },

			{ S::_percent_o, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true; }} },
			{ S::_percent_on, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true; }} }, { S::_percent_one, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false;  }} },

			{ S::_percent_p, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true; }} },
			{ S::_percent_pc, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false;  }} },

			{ S::_percent_s, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true; }} },
			{ S::_percent_sb, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true; }} }, { S::_percent_sbp, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false;  }} },
			{ S::_percent_st, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true; }} }, { S::_percent_sta, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true; }} }, { S::_percent_stac, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true; }} }, { S::_percent_stack, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false;  }} },

			{ S::_percent_z, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true; }} },
			{ S::_percent_ze, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true; }} }, { S::_percent_zer, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true; }} }, { S::_percent_zero, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false;  }} },

			{ S::_openparen_, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false; }}}, { S::_closeparen_, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false; }}},

			{ S::_colon_, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false; }}}, { S::_semicolon_, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false; }}},

			{ S::_plus_, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false; }}}, { S::_minus_, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false; }}},
			{ S::_star_, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false; }}}, { S::_star_star_, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false; }}}, { S::_slash_, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false; }}},
			{ S::_tilde_, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false; }}}, { S::_ampersend_, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false; }}}, { S::_verticalbar_, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false; }}}, { S::_caret_, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false; }}},

			{ S::__firstnum__, S::__InterTreeEpsilonUK__, {false, [](char c)->bool {return true;}}},
			{ S::__hexnum__, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false; }}}, { S::__decnum__, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false; }}}, { S::__octnum__, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false; }}}, { S::__binnum__, S::__KeyTreeEpsilonUK__, {false, [](char c)->bool {return false; }}},

			{ S::__InterTreeEpsilonID__, S::__identifier__, {true, [](char c)->bool {return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || ('0' <= c && c <= '9') || (c == '_'); }}},
			{ S::__KeyTreeEpsilonID__, S::__identifier__, {true, [](char c)->bool {return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || ('0' <= c && c <= '9') || (c == '_'); }} },
			{ S::__identifier__, S::__identifier__, {true, [](char c)->bool {return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || ('0' <= c && c <= '9') || (c == '_'); }} },
				
			{ S::__identifier__, S::__unknown__, {true, [](char c)->bool {return !(('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || ('0' <= c && c <= '9') || (c == '_')) && c != ' ' && c != '\n'; }}},
			{ S::__InterTreeEpsilonUK__, S::__unknown__, {true, [](char c)->bool {return c != ' ' && c != '\n'; }}},
			{ S::__KeyTreeEpsilonUK__, S::__unknown__, {true, [](char c)->bool {return c != ' ' && c != '\n'; }} },
			{ S::__unknown__, S::__unknown__, { true, [](char c)->bool {return c != ' ' && c != '\n'; }}}
			}
		};

		LFSM lexerFSM(lexerTable);

		enum class TokenType {
			add, addc, addi,
			sub, subc, subi,
			bxr, bxri,
			bor, bori,
			bnd, bndi,
			shiftl, shiftr, shiftli, shiftri,
			cmp,

			mov, set,
			push, pop,
			ld, st,
			jmp,
			call, ret,
			nop, halt,

			dot_gen0, dot_gen1, dot_gen2, dot_gen3, dot_gen4, dot_gen5, dot_gen6, dot_gen7, dot_gen8,
			dot_gen9, dot_gen10, dot_gen11, dot_gen12, dot_gen13, dot_gen14, dot_gen15, dot_gen16,
			dot_gen17, dot_gen18, dot_gen19, dot_gen20, dot_gen21, dot_gen22, dot_gen23, dot_gen24,
			dot_sbp, dot_zero, dot_one, dot_full, dot_pc, dot_stack, dot_flag,

			dot_neg, dot_pos, dot_carry, dot_carry4, dot_overflow, dot_gen,

			dot_32, dot_16L, dot_8L, dot_8H, dot_16H, dot_S16L, dot_S8L, dot_S8H,

			percent_gen0, percent_gen1, percent_gen2, percent_gen3, percent_gen4, percent_gen5, percent_gen6, percent_gen7, percent_gen8,
			percent_gen9, percent_gen10, percent_gen11, percent_gen12, percent_gen13, percent_gen14, percent_gen15, percent_gen16,
			percent_gen17, percent_gen18, percent_gen19, percent_gen20, percent_gen21, percent_gen22, percent_gen23, percent_gen24,
			percent_sbp, percent_zero, percent_one, percent_full, percent_pc, percent_stack, percent_flag,

			text, data, bss,

			openparen, closeparen,

			hexnum, decnum, octnum, binnum,

			colon, semicolon,

			plus, minus,
			star, dstar, slash, percent,
			tilde, ampersend, verticalbar, caret,

			whitespace,
			newline,

			identifier,

			unknown
		};

		std::map<S, TokenType> tokenTypeMap = {
			{S::add, TokenType::add}, {S::addc, TokenType::addc}, {S::addi, TokenType::addi},
			{S::sub, TokenType::sub}, {S::subc, TokenType::subc}, {S::subi, TokenType::subi},
			{S::bxr, TokenType::bxr}, {S::bxri, TokenType::bxri},
			{S::bor, TokenType::bor}, {S::bori, TokenType::bori},
			{S::bnd, TokenType::bnd}, {S::bndi, TokenType::bndi},
			{S::shiftl, TokenType::shiftl }, {S::shiftr, TokenType::shiftr}, {S::shiftli, TokenType::shiftli}, {S::shiftri, TokenType::shiftri},
			{S::cmp, TokenType::cmp},

			{S::mov, TokenType::mov}, {S::set, TokenType::set},
			{S::push, TokenType::push}, {S::pop, TokenType::pop},
			{S::ld, TokenType::ld}, {S::st, TokenType::st},
			{S::jmp, TokenType::jmp},
			{S::call, TokenType::call}, {S::ret, TokenType::ret},
			{S::nop, TokenType::nop}, {S::halt, TokenType::halt},

			{S::_dot_gen_openbrack_0_closebrack_, TokenType::dot_gen0},
			{S::_dot_gen_openbrack_1_closebrack_, TokenType::dot_gen1},
			{S::_dot_gen_openbrack_2_closebrack_, TokenType::dot_gen2},
			{S::_dot_gen_openbrack_3_closebrack_, TokenType::dot_gen3},
			{S::_dot_gen_openbrack_4_closebrack_, TokenType::dot_gen4},
			{S::_dot_gen_openbrack_5_closebrack_, TokenType::dot_gen5},
			{S::_dot_gen_openbrack_6_closebrack_, TokenType::dot_gen6},
			{S::_dot_gen_openbrack_7_closebrack_, TokenType::dot_gen7},
			{S::_dot_gen_openbrack_8_closebrack_, TokenType::dot_gen8},
			{S::_dot_gen_openbrack_9_closebrack_, TokenType::dot_gen9},
			{S::_dot_gen_openbrack_10_closebrack_, TokenType::dot_gen10},
			{S::_dot_gen_openbrack_11_closebrack_, TokenType::dot_gen11},
			{S::_dot_gen_openbrack_12_closebrack_, TokenType::dot_gen12},
			{S::_dot_gen_openbrack_13_closebrack_, TokenType::dot_gen13},
			{S::_dot_gen_openbrack_14_closebrack_, TokenType::dot_gen14},
			{S::_dot_gen_openbrack_15_closebrack_, TokenType::dot_gen15},
			{S::_dot_gen_openbrack_16_closebrack_, TokenType::dot_gen16},
			{S::_dot_gen_openbrack_17_closebrack_, TokenType::dot_gen17},
			{S::_dot_gen_openbrack_18_closebrack_, TokenType::dot_gen18},
			{S::_dot_gen_openbrack_19_closebrack_, TokenType::dot_gen19},
			{S::_dot_gen_openbrack_20_closebrack_, TokenType::dot_gen20},
			{S::_dot_gen_openbrack_21_closebrack_, TokenType::dot_gen21},
			{S::_dot_gen_openbrack_22_closebrack_, TokenType::dot_gen22},
			{S::_dot_gen_openbrack_23_closebrack_, TokenType::dot_gen23},
			{S::_dot_gen_openbrack_24_closebrack_, TokenType::dot_gen24},
			{S::_dot_sbp, TokenType::dot_sbp},
			{S::_dot_stack, TokenType::dot_zero},
			{S::_dot_one, TokenType::dot_one},
			{S::_dot_full, TokenType::dot_full},
			{S::_dot_pc, TokenType::dot_pc},
			{S::_dot_stack, TokenType::dot_stack},
			{S::_dot_flag, TokenType::dot_flag},

			{S::_percent_gen_openbrack_0_closebrack_, TokenType::percent_gen0},
			{S::_percent_gen_openbrack_1_closebrack_, TokenType::percent_gen1},
			{S::_percent_gen_openbrack_2_closebrack_, TokenType::percent_gen2},
			{S::_percent_gen_openbrack_3_closebrack_, TokenType::percent_gen3},
			{S::_percent_gen_openbrack_4_closebrack_, TokenType::percent_gen4},
			{S::_percent_gen_openbrack_5_closebrack_, TokenType::percent_gen5},
			{S::_percent_gen_openbrack_6_closebrack_, TokenType::percent_gen6},
			{S::_percent_gen_openbrack_7_closebrack_, TokenType::percent_gen7},
			{S::_percent_gen_openbrack_8_closebrack_, TokenType::percent_gen8},
			{S::_percent_gen_openbrack_9_closebrack_, TokenType::percent_gen9},
			{S::_percent_gen_openbrack_10_closebrack_, TokenType::percent_gen10},
			{S::_percent_gen_openbrack_11_closebrack_, TokenType::percent_gen11},
			{S::_percent_gen_openbrack_12_closebrack_, TokenType::percent_gen12},
			{S::_percent_gen_openbrack_13_closebrack_, TokenType::percent_gen13},
			{S::_percent_gen_openbrack_14_closebrack_, TokenType::percent_gen14},
			{S::_percent_gen_openbrack_15_closebrack_, TokenType::percent_gen15},
			{S::_percent_gen_openbrack_16_closebrack_, TokenType::percent_gen16},
			{S::_percent_gen_openbrack_17_closebrack_, TokenType::percent_gen17},
			{S::_percent_gen_openbrack_18_closebrack_, TokenType::percent_gen18},
			{S::_percent_gen_openbrack_19_closebrack_, TokenType::percent_gen19},
			{S::_percent_gen_openbrack_20_closebrack_, TokenType::percent_gen20},
			{S::_percent_gen_openbrack_21_closebrack_, TokenType::percent_gen21},
			{S::_percent_gen_openbrack_22_closebrack_, TokenType::percent_gen22},
			{S::_percent_gen_openbrack_23_closebrack_, TokenType::percent_gen23},
			{S::_percent_gen_openbrack_24_closebrack_, TokenType::percent_gen24},
			{S::_percent_sbp, TokenType::percent_sbp},
			{S::_percent_stack, TokenType::percent_zero},
			{S::_percent_one, TokenType::percent_one},
			{S::_percent_full, TokenType::percent_full},
			{S::_percent_pc, TokenType::percent_pc},
			{S::_percent_stack, TokenType::percent_stack},
			{S::_percent_flag, TokenType::percent_flag},

			{S::_dot_neg, TokenType::dot_neg}, {S::_dot_pos, TokenType::dot_pos}, {S::_dot_carry, TokenType::dot_carry}, {S::_dot_carry4, TokenType::dot_carry4}, {S::_dot_overflow, TokenType::dot_overflow}, {S::_dot_gen, TokenType::dot_gen},

			{S::_dot_32, TokenType::dot_32}, {S::_dot_16L, TokenType::dot_16L}, {S::_dot_8L, TokenType::dot_8L}, {S::_dot_8H, TokenType::dot_8H}, {S::_dot_16H, TokenType::dot_16H}, {S::_dot_S16L, TokenType::dot_S16L}, {S::_dot_S8L, TokenType::dot_S8L}, {S::_dot_S8H, TokenType::dot_S8H},

			{S::_dot_text, TokenType::text}, {S::_dot_data, TokenType::data}, {S::_dot_bss, TokenType::bss},

			{S::_openparen_, TokenType::openparen}, {S::_closeparen_, TokenType::closeparen},

			{S::__hexnum__, TokenType::hexnum}, {S::__decnum__, TokenType::decnum}, {S::__octnum__, TokenType::octnum}, {S::__binnum__, TokenType::binnum},

			{S::_colon_, TokenType::colon}, {S::_semicolon_, TokenType::semicolon},

			{S::_plus_, TokenType::plus}, {S::_minus_, TokenType::minus},
			{S::_star_, TokenType::star}, {S::_star_star_, TokenType::dstar}, {S::_slash_, TokenType::slash}, {S::_percent_, TokenType::percent},
			{S::_tilde_, TokenType::tilde}, {S::_ampersend_, TokenType::ampersend}, {S::_verticalbar_, TokenType::verticalbar}, {S::_caret_, TokenType::caret},

			{S::_whitespace_, TokenType::whitespace},
			{S::_newline_, TokenType::newline},

			{S::__identifier__, TokenType::identifier},

			{S::__unknown__, TokenType::unknown}
		};

		typedef struct _Token {
			std::string text;
			TokenType type;
		} Token;

		void Lexer(std::string input, std::vector<Token>& tokens) {
			//input.push_back(' ');
			// ----<<tmp>>----
			std::string removedTab;
			for (auto it = input.begin(); it != input.end(); it++) {
				if (*it == '\t') {
					for (int i = 0; i < 4; i++)
						removedTab.push_back(' ');
				}
				else
					removedTab.push_back(*it);
			}
			input = removedTab;
			// ----<<end>>----

			size_t len = input.size();
			size_t baseID = 0;
			size_t maxID = 0;
			S curState = S::__unknown__;
			std::string curString; 
			
			auto baseIt = input.begin();
			
			std::queue<LFSM::QueueData> stateQ;
			while (baseID < len) {
				stateQ.push({ S::__start__, 0,  });
				char c = -1;
				while (!stateQ.empty()) {
					LFSM::QueueData qData = stateQ.front();
					c = -1;
					if (qData.id + baseID < len)
						c = input.at(qData.id + baseID);

					LRET ret = lexerFSM.run(stateQ, c);

					if (ret == LRET::NoTransition) {
						if (maxID <= qData.id && (int)curState >= (int)qData.cur) {
							maxID = qData.id;
							curState = qData.cur;

							curString.clear();
							int ct = 0;
							for (auto it = baseIt; it != input.end() && ct < maxID; it++) { 
								curString.push_back(*it);

								ct++; 
							}
						}
					}
				}

				if (!maxID) {
					std::string tmp;
					tmp.push_back(c);
					tokens.push_back({tmp , TokenType::unknown});
					maxID = 1;
				}
				else {
					if (curState == S::__unknown__)
						tokens.push_back({ curString, TokenType::unknown });
					else {
						auto it = tokenTypeMap.find(curState);
						if (it == tokenTypeMap.end())
							tokens.push_back({ curString, TokenType::unknown });
						else
							tokens.push_back({ curString, it->second});
					}
				}

				int ct = 0;
				for (baseIt; baseIt != input.end() && ct < maxID; baseIt++) { ct++; }

				baseID = baseID + maxID;
				maxID = 0;
				curState = S::__unknown__;
				curString.clear();
			}
		}
	}

	namespace parser {
		enum class S {
			start,
			run,
			success,
		};

		enum class T {
			__unknown__ = -1,

			__start__,
			__success__,

			__section__,

			__text__,
			__data__,
			__bss__,

			__instruction__,

			__regvarient__,
			__flagvarient__,

			__register__,
			__regname__,
			__regmode__,

			__immidate__,

			__label__,
			__labelid__,
			__labelsign__,

			__identifier__,

			__WSPAL__,
			__WSPDC__,
			__NEWL__,
		};

		std::map<T, std::string> treeMap = {
			{ T::__unknown__, "unknown" },
			
			{ T::__start__, "start" },
			{ T::__success__, "success" },

			{ T::__section__, "section" },

			{ T::__text__, "text" },
			{ T::__data__, "data" },
			{ T::__bss__, "bss" },

			{ T::__instruction__, "instruction" },

			{ T::__regvarient__, "regvarient" },
			{ T::__flagvarient__, "flagvarient" },

			{ T::__register__, "register" },
			{ T::__regname__, "regname" },
			{ T::__regmode__, "regmode" },

			{ T::__immidate__, "immidate" },

			{ T::__label__, "label" },
			{ T::__labelid__, "labelid" },
			{ T::__labelsign__, "labelsign" },

			{ T::__identifier__, "identifier" },

			{ T::__WSPAL__, "WSPAL" },
			{ T::__WSPDC__, "WSPDC" },
			{ T::__NEWL__, "NEWL" }
		};

		typedef struct _Tree {
			lexer::Token token;
			T type;
			bool useToken;
			struct _Tree* prev; 
			std::vector<struct _Tree*>::iterator it;
			std::vector<struct _Tree*> child;
		} Tree;

		using TT = lexer::TokenType;
		using PPDA = tools::PDA<S, T, TT>;
		using PRET = PPDA::ReturnData;

		PPDA::CreateData parserTable = {
			{
			{S::start, false},
			{S::run, false},
			{S::success, false}
			},

			{
			{S::start, S::run, {false, false, {T::__success__, T::__start__}, [](TT t, T s)->bool {return true; }}},
			{S::run, S::success, {false, true, {}, [](TT t, T s)->bool {return (s == T::__success__); }}},

			{S::run, S::run, {true, true, {}, [](TT t, T s)->bool {return (s == T::__WSPAL__) && (t == TT::whitespace); }}},
			{S::run, S::run, {true, true, {}, [](TT t, T s)->bool {return (s == T::__WSPDC__) && (t == TT::whitespace); }}},
			{S::run, S::run, {false, true, {}, [](TT t, T s)->bool {return (s == T::__WSPDC__) && (t != TT::whitespace); }}},
			{S::run, S::run, {true, true, {}, [](TT t, T s)->bool {return (s == T::__NEWL__) && (t == TT::newline); }}},

			{S::run, S::run, {false, true, {T::__section__}, [](TT t, T s)->bool {return (s == T::__start__); }}},

			{S::run, S::run, {true, true, {T::__text__,	T::__NEWL__, T::__WSPDC__}, [](TT t, T s)->bool {return (s == T::__section__) && (t == TT::text); }}},
			{S::run, S::run, {true, true, {T::__data__, T::__NEWL__, T::__WSPDC__}, [](TT t, T s)->bool {return (s == T::__section__) && (t == TT::data); }}},
			{S::run, S::run, {true, true, {T::__bss__, T::__NEWL__, T::__WSPDC__}, [](TT t, T s)->bool {return (s == T::__section__) && (t == TT::bss); }}},
			{S::run, S::run, {true, true, {T::__section__, T::__text__, T::__NEWL__, T::__WSPDC__}, [](TT t, T s)->bool {return (s == T::__section__) && (t == TT::text); }}},
			{S::run, S::run, {true, true, {T::__section__, T::__data__, T::__NEWL__, T::__WSPDC__}, [](TT t, T s)->bool {return (s == T::__section__) && (t == TT::data); }}},
			{S::run, S::run, {true, true, {T::__section__, T::__bss__, T::__NEWL__, T::__WSPDC__}, [](TT t, T s)->bool {return (s == T::__section__) && (t == TT::bss); }}},

			{S::run, S::run, {false, true, {T::__NEWL__, T::__WSPDC__, T::__instruction__}, [](TT t, T s)->bool {return (s == T::__text__) && ((TT::add <= t && t <= TT::halt)); }}},
			{S::run, S::run, {false, true, {T::__text__, T::__NEWL__, T::__WSPDC__, T::__instruction__}, [](TT t, T s)->bool {return (s == T::__text__) && ((TT::add <= t && t <= TT::halt)); }}},

			{S::run, S::run, {true, true, {}, [](TT t, T s)->bool {return (s == T::__instruction__) && (t == TT::ret || t == TT::nop || t == TT::halt); }}}, //N
			{S::run, S::run, {true, true,  {T::__register__, T::__WSPAL__}, [](TT t, T s)->bool {return (s == T::__instruction__) && (t == TT::pop || t == TT::push); }}}, //R
			{S::run, S::run, {true, true, {T::__register__, T::__WSPAL__, T::__register__, T::__WSPAL__}, [](TT t, T s)->bool {return (s == T::__instruction__) && (t == TT::add || t == TT::sub || t == TT::addc || t == TT::subc || t == TT::bxr || t == TT::bor || t == TT::bnd || t == TT::shiftl || t == TT::shiftr || t == TT::cmp || t == TT::mov); }}}, //R-R
			{S::run, S::run, {true, true, {T::__immidate__, T::__WSPAL__, T::__register__, T::__WSPAL__}, [](TT t, T s)->bool {return (s == T::__instruction__) && (t == TT::addi || t == TT::subi || t == TT::bxri || t == TT::bori || t == TT::bndi || t == TT::shiftli || t == TT::shiftri || t == TT::set); }}}, //R-DI
			{S::run, S::run, {true, true, {T::__immidate__, T::__WSPAL__, T::__register__, T::__WSPAL__}, [](TT t, T s)->bool {return (s == T::__instruction__) && (t == TT::call); }}}, //R-SDI
			{S::run, S::run, {true, true, {T::__immidate__, T::__WSPAL__, T::__register__, T::__WSPAL__, T::__regvarient__}, [](TT t, T s)->bool {return (s == T::__instruction__) && (t == TT::ld || t == TT::st); }}}, //RV-R-SDI
			{S::run, S::run, {true, true, {T::__immidate__, T::__WSPAL__, T::__register__, T::__WSPAL__, T::__flagvarient__}, [](TT t, T s)->bool {return (s == T::__instruction__) && (t == TT::jmp); }}}, //FV-R-SDI

			{S::run, S::run, {true, true, {}, [](TT t, T s)->bool {return (s == T::__regvarient__) && ((TT::dot_gen0 <= t && t <= TT::dot_flag)); }}},
			{S::run, S::run, {true, true, {}, [](TT t, T s)->bool {return (s == T::__flagvarient__) && ((TT::dot_neg <= t && t <= TT::dot_gen) || t == TT::dot_zero || t == TT::dot_one); }}},

			{S::run, S::run, {false, true, {T::__regmode__, T::__regname__}, [](TT t, T s)->bool {return (s == T::__register__); }}},
			{S::run, S::run, {true, true, {}, [](TT t, T s)->bool {return (s == T::__regname__) && ((TT::percent_gen0 <= t && t <= TT::percent_flag)); }}},
			{S::run, S::run, {true, true, {}, [](TT t, T s)->bool {return (s == T::__regmode__) && ((TT::dot_32 <= t && t <= TT::dot_S8H)); }}},

			{S::run, S::run, {true, true, {}, [](TT t, T s)->bool {return (s == T::__immidate__) && (TT::hexnum <= t && t <= TT::binnum); }}},

			{S::run, S::run, {false, true, {T::__label__}, [](TT t, T s)->bool {return (s == T::__text__) && (t == TT::identifier); }}},
			{S::run, S::run, {false, true, {T::__text__, T::__label__}, [](TT t, T s)->bool {return (s == T::__text__) && (t == TT::identifier); }}},

			{S::run, S::run, {false, true, {T::__NEWL__, T::__WSPDC__, T::__labelsign__, T::__WSPDC__, T::__labelid__}, [](TT t, T s)->bool {return (s == T::__label__); }}},
			{S::run, S::run, {true, true, {}, [](TT t, T s)->bool {return (s == T::__labelid__) && (t == TT::identifier); }}},
			{S::run, S::run, {true, true, {}, [](TT t, T s)->bool {return (s == T::__labelsign__) && (t == TT::colon); }}}
			}
		};

		PPDA parserPDA(parserTable);

		bool Parser(std::vector<lexer::Token>& tokens, Tree*& outTree) {

			size_t len = tokens.size();
			size_t maxID = 0;
			Tree* tree = NULL;
			std::vector<PPDA::QueueData::QueueDataRD> tracker;
			S curState = S::success;
			bool success = false;

			std::queue<PPDA::QueueData> stateQ;
			stateQ.push({ S::start, {}, 0, {} });
			PPDA::QueueData tmp = {};
			lexer::Token curToken = {};
			while (!stateQ.empty()) {
				PPDA::QueueData qData = stateQ.front();

				curToken = { " ", lexer::TokenType::unknown };
				if (qData.id < len)
					curToken = tokens[qData.id];

				std::queue<PPDA::QueueData> outQ;
				PRET ret = parserPDA.run(stateQ, curToken.type);

				if (ret == PRET::NoTransition) {
					if (qData.id == len && qData.cur == S::success) {
						curState = qData.cur;
						tracker = qData.tracker;

						success = true;
					}
					if (!success && maxID < qData.id) {
						maxID = qData.id;
						curState = qData.cur;
						tracker = qData.tracker;
					}
				}
			}
			
			Tree* cur = new Tree;
			cur->prev = NULL;
			cur->type = T::__start__;
			cur->useToken = false;
			tree = cur;
			for (auto it = tracker.begin(); it != tracker.end(); it++) {
				auto track = *it;
				cur->child = {};
				cur->token = (0 <= track.id && track.id < len) ? tokens[track.id] : lexer::Token({ "u.k", TT::unknown });
				cur->it = cur->child.end();
				if (track.transition->second.rule.pushList.empty() && track.transition->second.rule.pop) {
					if (!cur)
						break;
					while (cur->prev) {
						cur->prev->it++;
						if (cur->prev->it == cur->prev->child.end())
							cur = cur->prev;
						else
							break;
					}
					if (!cur->prev)
						break;
					cur = *cur->prev->it;
				}
				else {
					for (auto si = track.transition->second.rule.pushList.rbegin(); si != track.transition->second.rule.pushList.rend(); si++) {
						//cout << treeMap[*si] << ", " << track.transition->second.rule.increment << ", " << track.transition->second.rule.pop << "|";
						Tree* nTree = new Tree;
						nTree->type = *si;
						nTree->prev = cur;
						nTree->useToken = track.transition->second.rule.increment;
						cur->child.push_back(nTree);
					}
					//cout << endl;
					cur->it = cur->child.begin();

					cur = *cur->it;
				}
			}

			outTree = tree;

			return success;
		}
	}

	namespace evaluater {

	}

	namespace iosystem {

	}
}

using namespace assembler;
using namespace assembler::tools;
using namespace assembler::lexer;
using namespace assembler::parser;


void printTree(Tree* pTree, std::vector<vector<vector<Tree*>>>& out, std::vector<int>& tmp, int level, int div) {
	if (!pTree)
		return;

	if (out.size() <= level)
		out.resize(level + 1, {});
	if (out[level].size() <= div)
		out[level].resize(div + 1, {});
	if (tmp.size() <= level)
		tmp.resize(level + 1, 0);

	out[level][div].push_back(pTree);


	for (auto it = pTree->child.begin(); it != pTree->child.end(); it++) {
		printTree(*it, out, tmp, level + 1, tmp[level]);
	}
	tmp[level]++;
}

int main() {

	char buff[1024];
	cin.getline(buff, 1024, '\\');
	string str = buff;

	std::vector<Token> tokens;
	Lexer(str, tokens);

	cout << "lex->" << endl;
	cout << "------------------------------------------------------------" << endl;
	for (auto it = tokens.begin(); it != tokens.end(); it++) {
		cout << ((it->text.front() == '\n') ? "newl" : it->text) << "; " << ((it->type == TokenType::unknown) ? "error" : "fine") << endl;
	}
	cout << "------------------------------------------------------------" << endl;

	
	Tree* tree;
	bool valid = Parser(tokens, tree); 
	cout << "parse->" << endl;
	cout << "------------------------------------------------------------" << endl;
	cout << valid << endl;
	cout << "------------------------------------------------------------" << endl;
	std::vector<int> tmp;
	std::vector<std::vector<std::vector<Tree*>>> out;
	printTree(tree, out, tmp, 0, 0);
	for (auto it = out.begin(); it != out.end(); it++) {
		cout << "|";
		for (auto si = it->begin(); si != it->end(); si++) {
			for (auto ti = si->begin(); ti != si->end(); ti++) {
				if ((*ti)->token.text.empty())
					cout << (*ti)->token.text << ":" << treeMap[(*ti)->type];
				else
					cout << (((*ti)->token.text.front() == '\n') ? "\\n" : (*ti)->token.text) << ":" << treeMap[(*ti)->type];
				if (ti + 1 != si->end())
					cout << ", ";
			}
			cout << "|";
		}
		cout << endl;
	}
	cout << "------------------------------------------------------------" << endl;

	return 0;
}