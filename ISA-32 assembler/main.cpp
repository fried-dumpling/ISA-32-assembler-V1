#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <stack>
#include <queue>
#include <map>
#include <set>
#include <functional>

#include "lexer.hpp"

using namespace std;

namespace assembler {
	namespace tools {
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

			unknown = -1
		};

		using LexerFactoy = lexer_generator::LexerFactory<TokenType>;
		using LFCD = LexerFactoy::CreateData;
		using Lexer = lexer_generator::Lexer<TokenType>;
		using Token = Lexer::Token;

		LFCD CreateData = {
			{
				{ "add", TokenType::add },
				{ "addc", TokenType::addc },
				{ "addi", TokenType::addi },
				{ "sub", TokenType::sub },
				{ "subc", TokenType::subc },
				{ "subi", TokenType::subi },
				{ "bxr", TokenType::bxr },
				{ "bxri", TokenType::bxri },
				{ "bor", TokenType::bor },
				{ "bori", TokenType::bori },
				{ "bnd", TokenType::bnd },
				{ "bndi", TokenType::bndi },
				{ "shiftl", TokenType::shiftl },
				{ "shiftr", TokenType::shiftr },
				{ "shiftli", TokenType::shiftli },
				{ "shiftri", TokenType::shiftri },
				{ "cmp", TokenType::cmp },
				{ "mov", TokenType::mov },
				{ "set", TokenType::set },
				{ "push", TokenType::push },
				{ "pop", TokenType::pop },
				{ "ld", TokenType::ld },
				{ "st", TokenType::st },
				{ "jmp", TokenType::jmp },
				{ "call", TokenType::call },
				{ "ret", TokenType::ret },
				{ "nop", TokenType::nop },
				{ "halt", TokenType::halt },

				{ "reg",TokenType::reg },
				{ "gen", TokenType::gen },
				{ "sbp", TokenType::sbp },
				{ "zero", TokenType::zero },
				{ "one", TokenType::one },
				{ "full", TokenType::full },
				{ "pc", TokenType::pc },
				{ "stack", TokenType::stack },
				{ "flag", TokenType::flag },

				{ "neg", TokenType::neg },
				{ "pos", TokenType::pos },
				{ "carry", TokenType::carry },
				{ "carry4", TokenType::carry4 },
				{ "overflow", TokenType::overflow },

				{ "32[bB]", TokenType::_32B_ },
				{ "16[lL]", TokenType::_16L_ },
				{ "8[lL]", TokenType::_8L_ },
				{ "8[hH]", TokenType::_8H_ },
				{ "16[hH]", TokenType::_16H_ },
				{ "[sS]16[lL]", TokenType::_S16L_ },
				{ "[sS]8[lL]", TokenType::_S8L_ },
				{ "[sS]8[hH]", TokenType::_S8H_ },

				{ "\\.text", TokenType::text },
				{ "\\.data", TokenType::data },
				{ "\\.bss", TokenType::bss },

				{ "\\.", TokenType::dot },
				{ "\\(", TokenType::openparen },
				{ "\\)", TokenType::closeparen },
				{ "\\[", TokenType::openbrack },
				{ "\\]", TokenType::closebrack },

				{ ":", TokenType::colon },
				{ ";", TokenType::semicolon },

				{ "\\+", TokenType::plus },
				{ "-", TokenType::minus },
				{ "\\*\\*", TokenType::dstar },
				{ "\\*", TokenType::star },
				{ "/", TokenType::slash },
				{ "%", TokenType::percent },
				{ "~", TokenType::tilde },
				{ "&", TokenType::ampersend },
				{ "\\|", TokenType::verticalbar },
				{ "^", TokenType::caret }
			},
			{
				{ "0x[0-9a-fA-F]+", TokenType::hexnum },
				{ "[0-9]+", TokenType::decnum },
				{ "0o[0-7]+", TokenType::octnum },
				{ "0b[01]+", TokenType::binnum },

				{ "[a-zA-Z_][a-zA-Z0-9_]*", TokenType::identifier },

				{ "(#.*)|(#\\*.*\\*\\#)", TokenType::comment },
				{ "[ \t]+", TokenType::whitespace },
				{ "\n", TokenType::newline }
			}
		};

		std::unordered_map<TokenType, std::string> tokenStr{
			{ TokenType::add, "add" },
			{ TokenType::addc, "addc" },
			{ TokenType::addi, "addi" },
			{ TokenType::sub, "sub" },
			{ TokenType::subc, "subc" },
			{ TokenType::subi, "subi" },
			{ TokenType::bxr, "bxr" },
			{ TokenType::bxri, "bxri" },
			{ TokenType::bor, "bor" },
			{ TokenType::bori, "bori" },
			{ TokenType::bnd, "bnd" },
			{ TokenType::bndi, "bndi" },
			{ TokenType::shiftl, "shiftl" },
			{ TokenType::shiftr, "shiftr" },
			{ TokenType::shiftli, "shiftli" },
			{ TokenType::shiftri, "shiftri" },
			{ TokenType::cmp, "cmp" },
			{ TokenType::mov, "mov" },
			{ TokenType::set, "set" },
			{ TokenType::push, "push" },
			{ TokenType::pop, "pop" },
			{ TokenType::ld, "ld" },
			{ TokenType::st, "st" },
			{ TokenType::jmp, "jmp" },
			{ TokenType::call, "call" },
			{ TokenType::ret, "ret" },
			{ TokenType::nop, "nop" },
			{ TokenType::halt, "halt" },

			{ TokenType::reg, "reg" },
			{ TokenType::gen, "gen" },
			{ TokenType::sbp, "sbp" },
			{ TokenType::zero, "zero" },
			{ TokenType::one, "one" },
			{ TokenType::full, "full" },
			{ TokenType::pc, "pc" },
			{ TokenType::stack, "stack" },
			{ TokenType::flag, "flag" },

			{ TokenType::neg, "neg" },
			{ TokenType::pos, "pos" },
			{ TokenType::carry, "carry" },
			{ TokenType::carry4, "carry4" },
			{ TokenType::overflow, "overflow" },

			{ TokenType::_32B_, "32B" },
			{ TokenType::_16L_, "16L" },
			{ TokenType::_8L_, "8L" },
			{ TokenType::_8H_, "8H" },
			{ TokenType::_16H_, "16H" },
			{ TokenType::_S16L_, "S16L" },
			{ TokenType::_S8L_, "S8L" },
			{ TokenType::_S8H_, "S8H" },

			{ TokenType::text, ".text" },
			{ TokenType::data, ".data" },
			{ TokenType::bss, ".bss" },

			{ TokenType::dot, "dot" },
			{ TokenType::openparen, "openparen" },
			{ TokenType::closeparen, "closeparen" },
			{ TokenType::openbrack, "openbrack" },
			{ TokenType::closebrack, "closebrack" },

			{ TokenType::colon, "colon" },
			{ TokenType::semicolon, "semicolon" },
			{ TokenType::plus, "plus" },
			{ TokenType::minus, "minus" },
			{ TokenType::dstar, "dstar" },
			{ TokenType::star, "star" },
			{ TokenType::slash, "slash" },
			{ TokenType::percent, "percent" },
			{ TokenType::tilde, "tilde" },
			{ TokenType::ampersend, "ampersend" },
			{ TokenType::verticalbar, "verticalbar" },
			{ TokenType::caret, "caret" },

			{ TokenType::hexnum, "hexnum" },
			{ TokenType::decnum, "decnum" },
			{ TokenType::octnum, "octnum" },
			{ TokenType::binnum, "binnum" },
			{ TokenType::comment, "comment" },
			{ TokenType::whitespace, "whitespace" },

			{ TokenType::newline, "newline" },
			{ TokenType::identifier, "identifier" },
			{ TokenType::unknown, "unknown" }
		};

		LexerFactoy lexerFactory;
		Lexer lexer;

		void createLexer(void) {
			lexerFactory.setRules(CreateData);
			lexerFactory.update();
			lexer = lexerFactory.create();
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

			/*
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

			{ S::run, S::run, {false, true, T::__expressionL7__, {T::__operationL6__, T::__expressionL6__}, [](TT t)->bool {return true; }}},
			{ S::run, S::run, {false, true, T::__operationL6__, {T::__operationL6__, T::__operatorL6__, T::__expressionL6__}, [](TT t)->bool {return true; }}},
			{ S::run, S::run, {false, true, T::__operationL6__, {}, [](TT t)->bool {return true; }}},

			{ S::run, S::run, {false, true, T::__expressionL6__, {T::__operationL5__, T::__expressionL5__}, [](TT t)->bool {return true; }}},
			{ S::run, S::run, {false, true, T::__operationL5__, {T::__operationL5__, T::__operatorL5__, T::__expressionL5__}, [](TT t)->bool {return true; }}},
			{ S::run, S::run, {false, true, T::__operationL5__, {}, [](TT t)->bool {return true; }}},

			{ S::run, S::run, {false, true, T::__expressionL5__, {T::__operationL4__, T::__expressionL4__}, [](TT t)->bool {return true; }}},
			{ S::run, S::run, {false, true, T::__operationL4__, {T::__operationL4__, T::__operatorL4__, T::__expressionL4__}, [](TT t)->bool {return true; }}},
			{ S::run, S::run, {false, true, T::__operationL4__, {}, [](TT t)->bool {return true; }}},

			{ S::run, S::run, {false, true, T::__expressionL4__, {T::__operationL3__, T::__expressionL3__}, [](TT t)->bool {return true; }}},
			{ S::run, S::run, {false, true, T::__operationL3__, {T::__operationL3__, T::__operatorL3__, T::__expressionL3__}, [](TT t)->bool {return true; }}},
			{ S::run, S::run, {false, true, T::__operationL3__, {}, [](TT t)->bool {return true; }}},

			{ S::run, S::run, {false, true, T::__expressionL3__, {T::__operationL2__, T::__expressionL2__}, [](TT t)->bool {return true; }}},
			{ S::run, S::run, {false, true, T::__operationL2__, {T::__operationL2__, T::__expressionL2__, T::__operatorL2__}, [](TT t)->bool {return true; }}},
			{ S::run, S::run, {false, true, T::__operationL2__, {}, [](TT t)->bool {return true; }}},

			{ S::run, S::run, {false, true, T::__expressionL2__, {T::__operationL1__, T::__expressionL1__}, [](TT t)->bool {return true; }}},
			{ S::run, S::run, {false, true, T::__operationL1__, {T::__operationL1__, T::__expressionL1__, T::__operatorL1__}, [](TT t)->bool {return true; }}},
			{ S::run, S::run, {false, true, T::__operationL1__, {}, [](TT t)->bool {return true; }}},

			{ S::run, S::run, {true, true, T::__expressionL1__, {}, [](TT t)->bool {return true && ((TT::hexnum <= t && t <= TT::binnum)); }}},
			{ S::run, S::run, {false, true, T::__expressionL1__, {T::__closeparen__, T::__expression__, T::__openparen__}, [](TT t)->bool {return true; }}},

			{ S::run, S::run, {true, true, T::__operatorL7__, {}, [](TT t)->bool {return (t == TT::verticalbar); }}},
			{ S::run, S::run, {true, true, T::__operatorL6__, {}, [](TT t)->bool {return (t == TT::caret); }}},
			{ S::run, S::run, {true, true, T::__operatorL5__, {}, [](TT t)->bool {return (t == TT::ampersend); }}},
			{ S::run, S::run, {true, true, T::__operatorL4__, {}, [](TT t)->bool {return (t == TT::minus || t == TT::plus); }}},
			{ S::run, S::run, {true, true, T::__operatorL3__, {}, [](TT t)->bool {return (t == TT::star || t == TT::slash); }}},
			{ S::run, S::run, {true, true, T::__operatorL2__, {}, [](TT t)->bool {return (t == TT::dstar); }}},
			{ S::run, S::run, {true, true, T::__operatorL1__, {}, [](TT t)->bool {return (t == TT::tilde); }}},
			}*/
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
			int mov = 0;
			int count = 0;
			while (!stateQ.empty()) {
				PPDA::QueueData qData = stateQ.front();

				curToken = { " ", lexer::TokenType::unknown };
				if (qData.id < len)
					curToken = tokens[qData.id];

				PRET ret = parserPDA.run(stateQ, curToken.type);

				mov++;
				if (ret == PRET::NoTransition) {
					count++;
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
			std::cout << "mov: " << mov << std::endl;
			std::cout << "end: " << count << std::endl;
			
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
	lexer::createLexer();

	ifstream file;
	file.open("input.txt");

	string buff;
	char c;
	while (file.get(c)) {
		buff.push_back(c);
	}
	file.close();

	if (buff.back() != '\n')
		buff.push_back('\n');

	cout << "file->" << endl;
	cout << "------------------------------------------------------------" << endl;
	cout << buff << endl;
	cout << "------------------------------------------------------------" << endl;

	std::vector<Token> tokens;
	lexer::lexer.lex(tokens, buff);
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
		cout << tmp << "; " << ((it->type == TokenType::unknown) ? "error" : lexer::tokenStr[it->type]) << endl;
	}
	cout << "------------------------------------------------------------" << endl;

	/*
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
	cout << "------------------------------------------------------------" << endl;*/

	return 0;
}