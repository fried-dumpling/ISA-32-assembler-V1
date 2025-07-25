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
				SK popData;
				std::vector<SK> pushList;
				std::function<bool(IN)> function;
			};

			typedef struct _TransitionData {
				ST end;
				Rule rule;
			}TransitionData;

			typedef struct _QueueData {
				using TransitionIT = typename std::multimap<SK, TransitionData>::iterator;
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
			std::map<ST, std::multimap<SK, TransitionData>> transitions;

			void addState(ST state, bool end) {
				if (this->states.find(state) != this->states.end())
					return;
				this->states.insert({ state, end });
			}

			void addTransition(ST start, ST end, Rule rule) {
				auto it = this->transitions.find(start);
				if (it == this->transitions.end()) {
					this->transitions.insert({ start, {} });
				}
				it = this->transitions.find(start);

				it->second.insert({ rule.popData, {end, rule} });
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
					auto rangeSec = it->second.equal_range(((QD.stack.empty()) ? (SK)-1 : QD.stack.top()));
					for (auto si = rangeSec.first; si != rangeSec.second; si++) {
						if (si->second.rule.function(data)) {
							QueueData tmp = {};
							tmp.cur = si->second.end;
							tmp.stack = QD.stack;
							tmp.id = QD.id + si->second.rule.increment;
							tmp.transition = si;
							tmp.tracker = QD.tracker;
							tmp.tracker.push_back({ QD.cur, QD.id, tmp.transition });
							if (si->second.rule.pop && !tmp.stack.empty())
								tmp.stack.pop();
							if (!si->second.rule.pushList.empty()) {
								for (auto ti = si->second.rule.pushList.begin(); ti != si->second.rule.pushList.end(); ti++)
									tmp.stack.push(*ti);
							}
							inputQ.push(tmp);
							moved = true;
						}
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
			ca, 
			cal, call,
			car, carr, carry, carry4,
			cm, cmp,

			f,
			fu, ful, full,
			fl, fla, flag,

			g,
			ge, gen,

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
			ne, neg,

			o,
			on, one,
			ov, ove, over, overf, overfl, overflo, overflow,

			p,
			pc,
			po, 
			pop, 
			pos,
			pu, pus, push,

			r,
			re, 
			ret,
			reg,

			s,
			sb, sbp,
			se, set,
			sh, shi, shif, shift,
			shiftl, shiftli, shiftr, shiftri,
			st,
			sta, stac, stack,
			su, sub, subc, subi,

			S,
			S1, S16, S16L,
			S8, S8H, S8L,

			z,
			ze, zer, zero,

			//===================================================================================

			_percent_,

			_dot_,

			_dot_b,
			_dot_bs, _dot_bss,

			_dot_d,
			_dot_da, _dot_dat, _dot_data,

			_dot_t, _dot_te, _dot_tex, _dot_text,

			_openparen_, _closeparen_,
			_openbrack_, _closebrack_,

			_colon_,

			_plus_, _minus_,
			_star_, _star_star_, _slash_,
			_tilde_, _ampersend_, _verticalbar_, _caret_,

			_whitespace_,
			_newline_,

			//===================================================================================

			__firstnum__,
			__hexnum__, __decnum__, __octnum__, __binnum__,

			_1_,
			_16_, _16H_, _16L_,

			_3_,
			_32_, _32B_,

			_8_,
			_8H_, _8L_,

			__commentline__, 
			__commentrange_comment__, __commentrange_end1__, __commentrange_end2__,

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
			{S::ca, false},
			{S::cal, false}, {S::call, false},
			{S::car, false}, {S::carr, false}, {S::carry, false}, {S::carry4, false},
			{S::cm, false}, {S::cmp, false},

			{S::f, false},
			{S::fu, false}, {S::ful, false}, {S::full, false},
			{S::fl, false}, {S::fla, false}, {S::flag, false},

			{S::g, false},
			{S::ge, false}, {S::gen, false},

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
			{S::ne, false}, {S::neg, false},

			{S::o, false},
			{S::on, false}, {S::one, false},
			{S::ov, false}, {S::ove, false}, {S::over, false}, {S::overf, false}, {S::overfl, false}, {S::overflo, false}, {S::overflow, false},

			{S::p, false},
			{S::pc, false},
			{S::po, false},
			{S::pop, false},
			{S::pos, false},
			{S::pu, false}, {S::pus, false}, {S::push, false},

			{S::r, false},
			{S::re, false},
			{S::ret, false},
			{S::reg, false},

			{S::s, false},
			{S::sb, false}, {S::sbp, false},
			{S::se, false}, {S::set, false},
			{S::sh, false}, {S::shi, false}, {S::shif, false}, {S::shift, false},
			{S::shiftl, false}, {S::shiftli, false}, {S::shiftr, false}, {S::shiftri, false},
			{S::st, false},
			{S::sta, false}, {S::stac, false}, {S::stack, false},
			{S::su, false}, {S::sub, false}, {S::subc, false}, {S::subi, false},

			{S::S, false},
			{S::S1, false}, {S::S16, false}, {S::S16L, false},
			{S::S8, false}, {S::S8H, false}, {S::S8L, false},

			{S::z, false},
			{S::ze, false}, {S::zer, false}, {S::zero, false},

			//===================================================================================

			{S::_percent_, false},

			{S::_dot_, false},

			{S::_dot_b, false},
			{S::_dot_bs, false}, {S::_dot_bss, false},

			{S::_dot_d, false},
			{S::_dot_da, false}, {S::_dot_dat, false}, {S::_dot_data, false},

			{S::_dot_t, false}, {S::_dot_te, false}, {S::_dot_tex, false}, {S::_dot_text, false},

			{S::_openparen_, false}, {S::_closeparen_, false},
			{S::_openbrack_, false}, {S::_closebrack_, false},

			{S::_colon_, false},

			{S::_plus_, false}, {S::_minus_, false},
			{S::_star_, false}, {S::_star_star_, false}, {S::_slash_, false},
			{S::_tilde_, false}, {S::_ampersend_, false}, {S::_verticalbar_, false}, {S::_caret_, false},

			{S::_whitespace_, false},
			{S::_newline_, false},

			//===================================================================================

			{S::__firstnum__, false},
			{S::__hexnum__, false}, {S::__decnum__, false}, {S::__octnum__, false}, {S::__binnum__, false},

			{ S::_1_, false },
			{ S::_16_, false }, { S::_16H_, false }, { S::_16L_, false },

			{ S::_3_, false },
			{ S::_32_, false }, { S::_32B_, false },

			{ S::_8_, false },
			{ S::_8H_, false }, { S::_8L_, false },

			{S::__commentline__, false},
			{S::__commentrange_comment__, false}, {S::__commentrange_end1__, false}, {S::__commentrange_end2__, false},

			{S::__identifier__, false},

			{S::__unknown__, false}
			},

			{
			{S::__start__, S::_1_, {true, [](char c)->bool {return c == '1'; }}},
			{S::__start__, S::_3_, {true, [](char c)->bool {return c == '3'; }}},
			{S::__start__, S::_8_, {true, [](char c)->bool {return c == '8'; }}},

			{S::__start__, S::a, {true, [](char c)->bool {return c == 'a'; }}},
			{S::__start__, S::b, {true, [](char c)->bool {return c == 'b'; }}},
			{S::__start__, S::c, {true, [](char c)->bool {return c == 'c'; }}},
			{S::__start__, S::f, {true, [](char c)->bool {return c == 'f'; }}},
			{S::__start__, S::g, {true, [](char c)->bool {return c == 'g'; }}},
			{S::__start__, S::h, {true, [](char c)->bool {return c == 'h'; }}},
			{S::__start__, S::j, {true, [](char c)->bool {return c == 'j'; }}},
			{S::__start__, S::l, {true, [](char c)->bool {return c == 'l'; }}},
			{S::__start__, S::m, {true, [](char c)->bool {return c == 'm'; }}},
			{S::__start__, S::n, {true, [](char c)->bool {return c == 'n'; }}},
			{S::__start__, S::o, {true, [](char c)->bool {return c == 'o'; }}},
			{S::__start__, S::p, {true, [](char c)->bool {return c == 'p'; }}},
			{S::__start__, S::r, {true, [](char c)->bool {return c == 'r'; }}},
			{S::__start__, S::s, {true, [](char c)->bool {return c == 's'; }}},
			{S::__start__, S::S, {true, [](char c)->bool {return c == 'S'; }}},
			{S::__start__, S::z, {true, [](char c)->bool {return c == 'z'; }}},

			{S::_1_, S::_16_, {true, [](char c)->bool {return c == '6'; }}},
			{S::_3_, S::_32_, {true, [](char c)->bool {return c == '2'; }}},
			{S::_8_, S::_8H_, {true, [](char c)->bool {return c == 'h' || c == 'H'; }}},
			{S::_8_, S::_8L_, {true, [](char c)->bool {return c == 'l' || c == 'L'; }}},

			{S::a, S::ad, {true, [](char c)->bool {return c == 'd'; }}},
			{S::b, S::bn, {true, [](char c)->bool {return c == 'n'; }}},
			{S::b, S::bo, {true, [](char c)->bool {return c == 'o'; }}},
			{S::b, S::bx, {true, [](char c)->bool {return c == 'x'; }}},
			{S::c, S::ca, {true, [](char c)->bool {return c == 'a'; }}},
			{S::c, S::cm, {true, [](char c)->bool {return c == 'm'; }}},
			{S::f, S::fu, {true, [](char c)->bool {return c == 'u'; }}},
			{S::f, S::fl, {true, [](char c)->bool {return c == 'l'; }}},
			{S::g, S::ge, {true, [](char c)->bool {return c == 'e'; }}},
			{S::h, S::ha, {true, [](char c)->bool {return c == 'a'; }}},
			{S::j, S::jm, {true, [](char c)->bool {return c == 'm'; }}},
			{S::l, S::ld, {true, [](char c)->bool {return c == 'd'; }}},
			{S::m, S::mo, {true, [](char c)->bool {return c == 'o'; }}},
			{S::n, S::no, {true, [](char c)->bool {return c == 'o'; }}},
			{S::n, S::ne, {true, [](char c)->bool {return c == 'e'; }}},
			{S::o, S::on, {true, [](char c)->bool {return c == 'n'; }}},
			{S::o, S::ov, {true, [](char c)->bool {return c == 'v'; }}},
			{S::p, S::pc, {true, [](char c)->bool {return c == 'c'; }}},
			{S::p, S::po, {true, [](char c)->bool {return c == 'o'; }}},
			{S::p, S::pu, {true, [](char c)->bool {return c == 'u'; }}},
			{S::r, S::re, {true, [](char c)->bool {return c == 'e'; }}},
			{S::s, S::sb, {true, [](char c)->bool {return c == 'b'; }}},
			{S::s, S::se, {true, [](char c)->bool {return c == 'e'; }}},
			{S::s, S::sh, {true, [](char c)->bool {return c == 'h'; }}},
			{S::s, S::st, {true, [](char c)->bool {return c == 't'; }}},
			{S::s, S::su, {true, [](char c)->bool {return c == 'u'; }}},
			{S::s, S::S1, {true, [](char c)->bool {return c == '1'; }}},
			{S::s, S::S8, {true, [](char c)->bool {return c == '8'; }}},
			{S::S, S::S1, {true, [](char c)->bool {return c == '1'; }}},
			{S::S, S::S8, {true, [](char c)->bool {return c == '8'; }}},
			{S::z, S::ze, {true, [](char c)->bool {return c == 'e'; }}},

			{S::_32_, S::_32B_, {true, [](char c)->bool {return c == 'b' || c == 'B'; }}},
			{S::_16_, S::_16H_, {true, [](char c)->bool {return c == 'h' || c == 'H'; }}},
			{S::_16_, S::_16L_, {true, [](char c)->bool {return c == 'l' || c == 'L'; }}},

			{S::ad, S::add, {true, [](char c)->bool {return c == 'd'; }}},
			{S::bn, S::bnd, {true, [](char c)->bool {return c == 'd'; }}},
			{S::bo, S::bor, {true, [](char c)->bool {return c == 'r'; }}},
			{S::bx, S::bxr, {true, [](char c)->bool {return c == 'r'; }}},
			{S::ca, S::cal, {true, [](char c)->bool {return c == 'l'; }}},
			{S::ca, S::car, {true, [](char c)->bool {return c == 'r'; }}},
			{S::cm, S::cmp, {true, [](char c)->bool {return c == 'p'; }}},
			{S::fu, S::ful, {true, [](char c)->bool {return c == 'l'; }}},
			{S::fl, S::fla, {true, [](char c)->bool {return c == 'a'; }}},
			{S::ge, S::gen, {true, [](char c)->bool {return c == 'n'; }}},
			{S::ha, S::hal, {true, [](char c)->bool {return c == 'l'; }}},
			{S::jm, S::jmp, {true, [](char c)->bool {return c == 'p'; }}},
			{S::mo, S::mov, {true, [](char c)->bool {return c == 'v'; }}},
			{S::no, S::nop, {true, [](char c)->bool {return c == 'p'; }}},
			{S::ne, S::neg, {true, [](char c)->bool {return c == 'g'; }}},
			{S::on, S::one, {true, [](char c)->bool {return c == 'e'; }}},
			{S::ov, S::ove, {true, [](char c)->bool {return c == 'e'; }}},
			{S::po, S::pop, {true, [](char c)->bool {return c == 'p'; }}},
			{S::po, S::pos, {true, [](char c)->bool {return c == 's'; }}},
			{S::pu, S::pus, {true, [](char c)->bool {return c == 's'; }}},
			{S::re, S::ret, {true, [](char c)->bool {return c == 't'; }}},
			{S::re, S::reg, {true, [](char c)->bool {return c == 'g'; }}},
			{S::sb, S::sbp, {true, [](char c)->bool {return c == 'p'; }}},
			{S::se, S::set, {true, [](char c)->bool {return c == 't'; }}},
			{S::sh, S::shi, {true, [](char c)->bool {return c == 'i'; }}},
			{S::st, S::sta, {true, [](char c)->bool {return c == 'a'; }}},
			{S::su, S::sub, {true, [](char c)->bool {return c == 'b'; }}},
			{S::S1, S::S16, {true, [](char c)->bool {return c == '6'; }}},
			{S::S8, S::S8H, {true, [](char c)->bool {return c == 'h' || c == 'H'; }}},
			{S::S8, S::S8L, {true, [](char c)->bool {return c == 'l' || c == 'L'; }}},
			{S::ze, S::zer, {true, [](char c)->bool {return c == 'r'; }}},

			{S::add, S::addc, {true, [](char c)->bool {return c == 'c'; }}},
			{S::add, S::addi, {true, [](char c)->bool {return c == 'i'; }}},
			{S::bnd, S::bndi, {true, [](char c)->bool {return c == 'i'; }}},
			{S::bor, S::bori, {true, [](char c)->bool {return c == 'i'; }}},
			{S::bxr, S::bxri, {true, [](char c)->bool {return c == 'i'; }}},
			{S::cal, S::call, {true, [](char c)->bool {return c == 'l'; }}},
			{S::car, S::carr, {true, [](char c)->bool {return c == 'r'; }}},
			{S::hal, S::halt, {true, [](char c)->bool {return c == 't'; }}},
			{S::ove, S::over, {true, [](char c)->bool {return c == 'r'; }}},
			{S::pus, S::push, {true, [](char c)->bool {return c == 'h'; }}},
			{S::shi, S::shif, {true, [](char c)->bool {return c == 'f'; }}},
			{S::sta, S::stac, {true, [](char c)->bool {return c == 'c'; }}},
			{S::sub, S::subc, {true, [](char c)->bool {return c == 'c'; }}},
			{S::sub, S::subi, {true, [](char c)->bool {return c == 'i'; }}},
			{S::zer, S::zero, {true, [](char c)->bool {return c == 'o'; }}},
			
			{S::over, S::overf, {true, [](char c)->bool {return c == 'f'; }}},
			{S::shif, S::shift, {true, [](char c)->bool {return c == 't'; }}},
			{S::stac, S::stack, {true, [](char c)->bool {return c == 'k'; }}},
			
			{S::overf, S::overfl, {true, [](char c)->bool {return c == 'l'; }}},
			{S::shift, S::shiftl, {true, [](char c)->bool {return c == 'l'; }}},
			{S::shift, S::shiftr, {true, [](char c)->bool {return c == 'r'; }}},
			
			{S::overfl, S::overflo, {true, [](char c)->bool {return c == 'o'; }}},
			{S::shiftl, S::shiftli, {true, [](char c)->bool {return c == 'i'; }}},
			{S::shiftr, S::shiftri, {true, [](char c)->bool {return c == 'i'; }}},
				
			{S::overflo, S::overflow, {true, [](char c)->bool {return c == 'w'; }}},

			{S::__start__, S::_dot_, {true , [](char c)->bool {return c == '.'; }}},
			
			
			{S::_dot_, S::_dot_b, {true , [](char c)->bool {return c == 'b'; }}},
			{S::_dot_, S::_dot_d, {true , [](char c)->bool {return c == 'd'; }}},
			{S::_dot_, S::_dot_t, {true , [](char c)->bool {return c == 't'; }}},

			{ S::_dot_b, S::_dot_bs, {true , [](char c)->bool {return c == 's'; }} },
			{ S::_dot_d, S::_dot_da, {true , [](char c)->bool {return c == 'a'; }} },
			{ S::_dot_t, S::_dot_te, {true , [](char c)->bool {return c == 'e'; }} },

			{ S::_dot_b, S::_dot_bss, {true , [](char c)->bool {return c == 's'; }} },
			{ S::_dot_da, S::_dot_dat, {true , [](char c)->bool {return c == 't'; }} },
			{ S::_dot_te, S::_dot_tex, {true , [](char c)->bool {return c == 'x'; }} },

			{ S::_dot_dat, S::_dot_data, {true , [](char c)->bool {return c == 'a'; }} },
			{ S::_dot_tex, S::_dot_text, {true , [](char c)->bool {return c == 't'; }} },

			{ S::__start__, S::_percent_, {true , [](char c)->bool {return c == '%'; }} },

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
			{ S::__start__, S::_openbrack_, {true , [](char c)->bool {return c == '['; }} },
			{ S::__start__, S::_closebrack_, {true , [](char c)->bool {return c == ']'; }} },

			{ S::__start__, S::_colon_, {true , [](char c)->bool {return c == ':'; }} },
			{ S::__start__, S::_plus_, {true , [](char c)->bool {return c == '+'; }} },
			{ S::__start__, S::_minus_, {true , [](char c)->bool {return c == '-'; }} },
			{ S::__start__, S::_star_, {true , [](char c)->bool {return c == '*'; }} },
			{ S::_star_, S::_star_star_, {true , [](char c)->bool {return c == '*'; }} },
			{ S::__start__, S::_slash_, {true , [](char c)->bool {return c == '/'; }} },
			{ S::__start__, S::_ampersend_, {true , [](char c)->bool {return c == '&'; }} },
			{ S::__start__, S::_verticalbar_, {true , [](char c)->bool {return c == '|'; }} },
			{ S::__start__, S::_caret_, {true , [](char c)->bool {return c == '^'; }} },

			{ S::__start__, S::__commentline__, {true , [](char c)->bool {return c == ';'; }} },
			{ S::__commentline__, S::__commentline__, {true , [](char c)->bool {return c != '\n'; }} },

			{ S::_slash_, S::__commentrange_comment__, {true , [](char c)->bool {return c == '*'; }} },
			{ S::__commentrange_comment__, S::__commentrange_comment__, {true , [](char c)->bool {return c != '['; }} },
			{ S::__commentrange_comment__, S::__commentrange_end1__, {true , [](char c)->bool {return c == ']'; }} },
			{ S::__commentrange_end1__, S::__commentrange_comment__, {true , [](char c)->bool {return c != ';'; }} },
			{ S::__commentrange_end1__, S::__commentrange_end2__, {true , [](char c)->bool {return c == ';'; }} },

			{ S::__start__, S::_whitespace_, {true , [](char c)->bool {return c == ' '; }} },
			{ S::__start__, S::_newline_, {true , [](char c)->bool {return c == '\n'; }} },

			{ S::_whitespace_, S::_whitespace_, {true , [](char c)->bool {return c == ' ' && c != '\n'; }}},

			{ S::__start__, S::__identifier__, {true, [](char c)->bool {return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || (c == '_'); }} },
			{ S::__identifier__, S::__identifier__, {true, [](char c)->bool {return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || ('0' <= c && c <= '9') || (c == '_'); }} },
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

			reg,
			gen,
			sbp, zero, one, full, pc, stack, flag,

			neg, pos, carry, carry4, overflow,

			_32B_, _16L_, _8L_, _8H_, _16H_, _S16L_, _S8L_, _S8H_,

			text, data, bss,

			dot,

			openparen, closeparen,
			openbrack, closebrack,

			hexnum, decnum, octnum, binnum,

			colon, semicolon,

			plus, minus,
			star, dstar, slash, percent,
			tilde, ampersend, verticalbar, caret,

			comment,

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

			{S::reg, TokenType::reg},
			{S::gen, TokenType::gen},
			{S::sbp, TokenType::sbp}, {S::zero, TokenType::zero}, {S::one, TokenType::one}, {S::full, TokenType::full}, {S::pc, TokenType::pc}, {S::stack, TokenType::stack}, {S::flag, TokenType::flag},

			{S::neg, TokenType::neg}, {S::pos, TokenType::pos}, {S::carry, TokenType::carry}, {S::carry4, TokenType::carry4}, {S::overflow, TokenType::overflow},

			{S::_32B_, TokenType::_32B_}, {S::_16L_, TokenType::_16L_}, {S::_8L_, TokenType::_8L_}, {S::_8H_, TokenType::_8H_}, {S::_16H_, TokenType::_16H_}, {S::S16L, TokenType::_S16L_}, {S::S8L, TokenType::_S8L_}, {S::S8H, TokenType::_S8H_},

			{S::mov, TokenType::mov}, {S::set, TokenType::set},
			{S::push, TokenType::push}, {S::pop, TokenType::pop},
			{S::ld, TokenType::ld}, {S::st, TokenType::st},
			{S::jmp, TokenType::jmp},
			{S::call, TokenType::call}, {S::ret, TokenType::ret},
			{S::nop, TokenType::nop}, {S::halt, TokenType::halt},

			{S::_dot_text, TokenType::text}, {S::_dot_data, TokenType::data}, {S::_dot_bss, TokenType::bss},

			{S::_dot_, TokenType::dot},

			{S::_openparen_, TokenType::openparen}, {S::_closeparen_, TokenType::closeparen},
			{S::_openbrack_, TokenType::openbrack}, {S::_closebrack_, TokenType::closebrack},

			{S::__hexnum__, TokenType::hexnum}, {S::__decnum__, TokenType::decnum}, {S::__octnum__, TokenType::octnum}, {S::__binnum__, TokenType::binnum},

			{S::_colon_, TokenType::colon},

			{S::_plus_, TokenType::plus}, {S::_minus_, TokenType::minus},
			{S::_star_, TokenType::star}, {S::_star_star_, TokenType::dstar}, {S::_slash_, TokenType::slash}, {S::_percent_, TokenType::percent},
			{S::_tilde_, TokenType::tilde}, {S::_ampersend_, TokenType::ampersend}, {S::_verticalbar_, TokenType::verticalbar}, {S::_caret_, TokenType::caret},

			{S::__commentline__, TokenType::comment},
			{S::__commentrange_comment__, TokenType::comment},
			{S::__commentrange_end1__, TokenType::comment},
			{S::__commentrange_end2__, TokenType::comment},

			{S::_whitespace_, TokenType::whitespace},
			{S::_newline_, TokenType::newline},

			{S::__identifier__, TokenType::identifier},

			{S::__unknown__, TokenType::unknown},
		};

		typedef struct _Token {
			std::string text;
			TokenType type;
		} Token;

		void Lexer(std::string input, std::vector<Token>& tokens) {
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

			size_t len = input.size();
			size_t baseID = 0;
			size_t maxID = 0;
			S curState;
			std::string curString; 
			
			auto baseIt = input.begin();
			
			std::queue<LFSM::QueueData> stateQ;
			while (baseID < len) {
				stateQ.push({ S::__start__, 0,  });

				char c = -1;
				while (!stateQ.empty()) {
					LFSM::QueueData qData = stateQ.front();
					c = '\n';
					if (qData.id + baseID < len)
						c = input.at(qData.id + baseID);

					LRET ret = lexerFSM.run(stateQ, c);

					if (ret == LRET::NoTransition || qData.id + baseID >= len) {
						if (maxID < qData.id || (maxID == qData.id && (qData.cur < curState))) {
							maxID = qData.id;

							curState = qData.cur;
							curString.clear();
							int ct = 0;
							for (auto it = baseIt; it != input.end() && ct < qData.id; it++) { 
								curString.push_back(*it); ct++; 
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
					if (!tokenTypeMap.count(curState))
						tokens.push_back({ curString, TokenType::unknown });
					else
						tokens.push_back({ curString, tokenTypeMap[curState] });
				}

				int ct = 0;
				for (baseIt; baseIt != input.end() && ct < maxID; baseIt++) { ct++; }

				baseID = baseID + maxID;
				maxID = 0;
				curString.clear();
			}
		}

		void PreProcesser(std::vector<lexer::Token>& tokens) {
		}
	}

	namespace parser {
		enum class S {
			start,
			run,
			success,
		};

		enum class T {
			__unknown__ = -2,
			__notusing__ = -1,

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

			__expression__,
			__expressionL0__,
			__expressionL1__,
			__expressionL2__,
			__expressionL3__,
			__expressionL4__,
			__expressionL5__,
			__expressionL6__,
			__expressionL7__,

			__operatorL1__,
			__operatorL2__,
			__operatorL3__,
			__operatorL4__,
			__operatorL5__,
			__operatorL6__,
			__operatorL7__,

			__operationL1__,
			__operationL2__,
			__operationL3__,
			__operationL4__,
			__operationL5__,
			__operationL6__,
			__operationL7__,

			__openparen__,
			__closeparen__,

			__index__,
			__openbrack__,
			__closebrack__,

			__dot__,

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

			{ T::__expression__, "expression"},

			{ T::__expressionL0__, "expressionL0"},
			{ T::__expressionL1__, "expressionL1"},
			{ T::__expressionL2__, "expressionL2"},
			{ T::__expressionL3__, "expressionL3"},
			{ T::__expressionL4__, "expressionL4"},
			{ T::__expressionL5__, "expressionL5"},
			{ T::__expressionL6__, "expressionL6"},
			{ T::__expressionL7__, "expressionL7"},

			{ T::__operatorL1__, "operatorL1"},
			{ T::__operatorL2__, "operatorL2"},
			{ T::__operatorL3__, "operatorL3"},
			{ T::__operatorL4__, "operatorL4"},
			{ T::__operatorL5__, "operatorL5"},
			{ T::__operatorL6__, "operatorL6"},
			{ T::__operatorL7__, "operatorL7"},

			{ T::__operationL1__, "operationL1"},
			{ T::__operationL2__, "operationL2"},
			{ T::__operationL3__, "operationL3"},
			{ T::__operationL4__, "operationL4"},
			{ T::__operationL5__, "operationL5"},
			{ T::__operationL6__, "operationL6"},
			{ T::__operationL7__, "operationL7"},

			{T::__openparen__, "openparen"},
			{T::__closeparen__, "closeparen"},

			{T::__index__, "index"},
			{T::__openbrack__, "openbrack"},
			{T::__closebrack__, "closebrack"},

			{T::__dot__, "dot"},

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
			{S::start, S::run, {false, false, T::__notusing__, {T::__success__, T::__start__}, [](TT t)->bool {return true; }}},
			{S::run, S::success, {false, true, T::__success__, {}, [](TT t)->bool {return true; }}},

			{S::run, S::run, {true, true, T::__WSPAL__, {}, [](TT t)->bool {return (t == TT::whitespace); }}},
			{S::run, S::run, {true, true, T::__WSPDC__, {}, [](TT t)->bool {return (t == TT::whitespace); }}},
			{S::run, S::run, {false, true, T::__WSPDC__, {}, [](TT t)->bool {return (t != TT::whitespace); }}},
			{S::run, S::run, {true, true, T::__NEWL__, {T::__WSPDC__}, [](TT t)->bool {return (t == TT::newline); }}},
			{S::run, S::run, {true, true, T::__NEWL__, {T::__NEWL__}, [](TT t)->bool {return (t == TT::newline); }}},
			{S::run, S::run, {false, true, T::__NEWL__, {T::__NEWL__, T::__WSPAL__}, [](TT t)->bool {return true; }}},
			{S::run, S::run, {true, false, T::__notusing__, {}, [](TT t)->bool {return (t == TT::comment); }}},

			{S::run, S::run, {false, true, T::__start__, {T::__section__}, [](TT t)->bool {return true; }}},
			{S::run, S::run, {false, true, T::__start__, {T::__section__, T::__WSPDC__}, [](TT t)->bool {return true; }}},
			{S::run, S::run, {false, true, T::__start__, {T::__section__, T::__NEWL__}, [](TT t)->bool {return true; }}},
			{S::run, S::run, {false, true, T::__start__, {}, [](TT t)->bool {return true; }}},

			{S::run, S::run, {true, true, T::__section__, {T::__text__, T::__NEWL__}, [](TT t)->bool {return (t == TT::text); }}},
			{S::run, S::run, {true, true, T::__section__, {T::__data__, T::__NEWL__}, [](TT t)->bool {return (t == TT::data); }}},
			{S::run, S::run, {true, true, T::__section__, {T::__bss__, T::__NEWL__}, [](TT t)->bool {return (t == TT::bss); }}},
			{S::run, S::run, {true, true, T::__section__, {T::__section__, T::__text__, T::__NEWL__}, [](TT t)->bool {return (t == TT::text); }}},
			{S::run, S::run, {true, true, T::__section__, {T::__section__, T::__data__, T::__NEWL__}, [](TT t)->bool {return (t == TT::data); }}},
			{S::run, S::run, {true, true, T::__section__, {T::__section__, T::__bss__, T::__NEWL__}, [](TT t)->bool {return (t == TT::bss); }}},

			{S::run, S::run, {false, true, T::__text__, {T::__NEWL__, T::__instruction__}, [](TT t)->bool {return ((TT::add <= t && t <= TT::halt)); }}},
			{S::run, S::run, {false, true, T::__text__,  {T::__text__, T::__NEWL__, T::__instruction__}, [](TT t)->bool {return ((TT::add <= t && t <= TT::halt)); }}},

			{S::run, S::run, {true, true, T::__instruction__, {}, [](TT t)->bool {return (t == TT::ret || t == TT::nop || t == TT::halt); }}}, //N
			{S::run, S::run, {true, true, T::__instruction__, {T::__register__, T::__WSPAL__}, [](TT t)->bool {return (t == TT::pop || t == TT::push); }}}, //R
			{S::run, S::run, {true, true, T::__instruction__, {T::__register__, T::__WSPAL__, T::__register__, T::__WSPAL__}, [](TT t)->bool {return (t == TT::add || t == TT::sub || t == TT::addc || t == TT::subc || t == TT::bxr || t == TT::bor || t == TT::bnd || t == TT::shiftl || t == TT::shiftr || t == TT::cmp || t == TT::mov); }}}, //R-R
			{S::run, S::run, {true, true, T::__instruction__, {T::__immidate__, T::__WSPAL__, T::__register__, T::__WSPAL__}, [](TT t)->bool {return (t == TT::addi || t == TT::subi || t == TT::bxri || t == TT::bori || t == TT::bndi || t == TT::shiftli || t == TT::shiftri || t == TT::set); }}}, //R-DI
			{S::run, S::run, {true, true, T::__instruction__, {T::__immidate__, T::__WSPAL__, T::__register__, T::__WSPAL__}, [](TT t)->bool {return (t == TT::call); }}}, //R-SDI
			{S::run, S::run, {true, true, T::__instruction__, {T::__immidate__, T::__WSPAL__, T::__register__, T::__WSPAL__, T::__regvarient__, T::__dot__}, [](TT t)->bool {return (t == TT::ld || t == TT::st); }}}, //RV-R-SDI
			{S::run, S::run, {true, true, T::__instruction__, {T::__immidate__, T::__WSPAL__, T::__register__, T::__WSPAL__, T::__flagvarient__, T::__dot__}, [](TT t)->bool {return (t == TT::jmp); }}}, //FV-R-SDI

			{S::run, S::run, {true, true, T::__regvarient__, {}, [](TT t)->bool {return ((TT::sbp <= t && t <= TT::flag)); }}},
			{S::run, S::run, {true, true, T::__regvarient__, {T::__index__}, [](TT t)->bool {return (t == TT::gen || t == TT::reg); }}},

			{S::run, S::run, {true, true, T::__flagvarient__, {}, [](TT t)->bool {return ((TT::neg <= t && t <= TT::overflow) || t == TT::zero || t == TT::one || t == TT::gen); }}},

			{S::run, S::run, {true, true, T::__register__, {T::__regmode__, T::__dot__, T::__regname__}, [](TT t)->bool {return (t == TT::percent); }}},
			{S::run, S::run, {true, true, T::__regname__, {T::__index__}, [](TT t)->bool {return (t == TT::gen || t == TT::reg); }}},
			{S::run, S::run, {true, true, T::__regname__, {}, [](TT t)->bool {return ((TT::sbp <= t && t <= TT::flag)); }}},
			{S::run, S::run, {true, true, T::__regmode__, {}, [](TT t)->bool {return ((TT::_32B_ <= t && t <= TT::_S8H_)); }}},

			{S::run, S::run, {false, true, T::__index__, {T::__closebrack__, T::__expression__, T::__openbrack__}, [](TT t)->bool {return true; }}},
			{S::run, S::run, {true, true, T::__openbrack__, {}, [](TT t)->bool {return (t == TT::openbrack); }}},
			{S::run, S::run, {true, true, T::__closebrack__, {}, [](TT t)->bool {return (t == TT::closebrack); }}},

			{S::run, S::run, {true, true, T::__dot__, {}, [](TT t)->bool {return (t == TT::dot); }}},

			{S::run, S::run, {false, true, T::__immidate__, {T::__expression__}, [](TT t)->bool {return true; }}},

			{S::run, S::run, {false, true, T::__text__, {T::__NEWL__, T::__label__}, [](TT t)->bool {return (t == TT::identifier); }}},
			{S::run, S::run, {false, true, T::__text__, {T::__text__, T::__NEWL__, T::__label__}, [](TT t)->bool {return (t == TT::identifier); }}},

			{S::run, S::run, {false, true, T::__label__, {T::__labelsign__, T::__WSPDC__, T::__labelid__}, [](TT t)->bool {return true; }}},
			{S::run, S::run, {true, true, T::__labelid__, {}, [](TT t)->bool {return (t == TT::identifier); }}},
			{S::run, S::run, {true, true,  T::__labelsign__, {}, [](TT t)->bool {return (t == TT::colon); }}},

			{ S::run, S::run, {false, true, T::__expression__, {T::__operationL7__, T::__expressionL7__}, [](TT t)->bool {return true; }}},
			{ S::run, S::run, {false, true, T::__operationL7__, {T::__operationL7__, T::__operatorL7__, T::__expressionL7__}, [](TT t)->bool {return true; }}},
			{ S::run, S::run, {false, true, T::__operationL7__, {}, [](TT t)->bool {return true; }}},

			{ S::run, S::run, {false, true, T::__operationL7__, {T::__operationL6__, T::__expressionL6__}, [](TT t)->bool {return true; }}},
			{ S::run, S::run, {false, true, T::__operationL6__, {T::__operationL6__, T::__operatorL6__, T::__expressionL6__}, [](TT t)->bool {return true; }}},
			{ S::run, S::run, {false, true, T::__operationL6__, {}, [](TT t)->bool {return true; }}},

			{ S::run, S::run, {false, true, T::__operationL6__, {T::__operationL5__, T::__expressionL5__}, [](TT t)->bool {return true; }}},
			{ S::run, S::run, {false, true, T::__operationL5__, {T::__operationL5__, T::__operatorL5__, T::__expressionL5__}, [](TT t)->bool {return true; }}},
			{ S::run, S::run, {false, true, T::__operationL5__, {}, [](TT t)->bool {return true; }}},

			{ S::run, S::run, {false, true, T::__operationL5__, {T::__operationL4__, T::__expressionL4__}, [](TT t)->bool {return true; }}},
			{ S::run, S::run, {false, true, T::__operationL4__, {T::__operationL4__, T::__operatorL4__, T::__expressionL4__}, [](TT t)->bool {return true; }}},
			{ S::run, S::run, {false, true, T::__operationL4__, {}, [](TT t)->bool {return true; }}},

			{ S::run, S::run, {false, true, T::__operationL4__, {T::__operationL3__, T::__expressionL3__}, [](TT t)->bool {return true; }}},
			{ S::run, S::run, {false, true, T::__operationL3__, {T::__operationL3__, T::__operatorL3__, T::__expressionL3__}, [](TT t)->bool {return true; }}},
			{ S::run, S::run, {false, true, T::__operationL3__, {}, [](TT t)->bool {return true; }}},

			{ S::run, S::run, {false, true, T::__operationL3__, {T::__operationL2__, T::__expressionL2__}, [](TT t)->bool {return true; }}},
			{ S::run, S::run, {false, true, T::__operationL2__, {T::__operationL2__, T::__expressionL2__, T::__operatorL2__}, [](TT t)->bool {return true; }}},
			{ S::run, S::run, {false, true, T::__operationL2__, {}, [](TT t)->bool {return true; }}},

			{ S::run, S::run, {false, true, T::__operationL2__, {T::__operationL1__, T::__expressionL1__}, [](TT t)->bool {return true; }}},
			{ S::run, S::run, {false, true, T::__operationL1__, {T::__operationL1__, T::__expressionL1__, T::__operatorL1__}, [](TT t)->bool {return true; }}},
			{ S::run, S::run, {false, true, T::__operationL1__, {}, [](TT t)->bool {return true; }}},

			{ S::run, S::run, {true, true, T::__operationL1__, {}, [](TT t)->bool {return true && ((TT::hexnum <= t && t <= TT::binnum)); }}},
			{ S::run, S::run, {false, true, T::__operationL1__, {T::__closeparen__, T::__expression__, T::__openparen__}, [](TT t)->bool {return true; }}},

			{ S::run, S::run, {true, true, T::__operatorL7__, {}, [](TT t)->bool {return (t == TT::verticalbar); }}},
			{ S::run, S::run, {true, true, T::__operatorL6__, {}, [](TT t)->bool {return (t == TT::caret); }}},
			{ S::run, S::run, {true, true, T::__operatorL5__, {}, [](TT t)->bool {return (t == TT::ampersend); }}},
			{ S::run, S::run, {true, true, T::__operatorL4__, {}, [](TT t)->bool {return (t == TT::minus || t == TT::plus); }}},
			{ S::run, S::run, {true, true, T::__operatorL3__, {}, [](TT t)->bool {return (t == TT::star || t == TT::slash); }}},
			{ S::run, S::run, {true, true, T::__operatorL2__, {}, [](TT t)->bool {return (t == TT::dstar); }}},
			{ S::run, S::run, {true, true, T::__operatorL1__, {}, [](TT t)->bool {return (t == TT::tilde); }}},
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
				cur->token = (0 <= track.id && track.id < len) ? tokens[track.id] : lexer::Token({ "u.k", {TT::unknown } });
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
						Tree* nTree = new Tree;
						nTree->type = *si;
						nTree->prev = cur;
						nTree->useToken = track.transition->second.rule.increment;
						cur->child.push_back(nTree);
					}
					cur->it = cur->child.begin();

					cur = *cur->it;
				}
			}

			outTree = tree;

			return success;
		}
	}

	namespace evaluator {
		void Evaluator(parser::Tree& tree) {
			std::map<std::string, unsigned __int32> varTable;


		}
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
	ifstream file;
	file.open("input.txt");

	string buff;
	char c;
	while (file.get(c)) {
		buff.push_back(c);
	}
	if (buff.back() != '\n')
		buff.push_back('\n');

	cout << "file->" << endl;
	cout << "------------------------------------------------------------" << endl;
	cout << buff << endl;
	cout << "------------------------------------------------------------" << endl;

	std::vector<Token> tokens;
	Lexer(buff, tokens);
	PreProcesser(tokens);
	cout << "lex->" << endl;
	cout << "------------------------------------------------------------" << endl;
	for (auto it = tokens.begin(); it != tokens.end(); it++) {
		string tmp = it->text;
		for (auto si = tmp.begin(); si != tmp.end(); ) {
			if (*si == '\n') {
				si = tmp.erase(si);
				si = tmp.insert(si, '\\');
				si++;
				si = tmp.insert(si, 'n');
			}
			else
				si++;
		}
		tmp.push_back('\t');
		cout << tmp << "; " << ((it->type == TokenType::unknown) ? "error" : "fine") << endl;
	}
	cout << "------------------------------------------------------------" << endl;

	Tree* tree;
	bool validParse = Parser(tokens, tree); 
	cout << "parse->" << endl;
	cout << "------------------------------------------------------------" << endl;
	cout << (validParse ? "success" : "failed") << endl;
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