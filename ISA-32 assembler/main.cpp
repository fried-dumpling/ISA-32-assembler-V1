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
#include "parser.hpp"

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
		TOKENSTART(TokenType)
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
		TOKENEND();

		using LexerFactoy = lexer_generator::LexerFactory<TokenType>;
		using LFCD = LexerFactoy::CreateData;
		using Lexer = lexer_generator::Lexer<TokenType>;
		using Token = Lexer::Token;

		const LFCD CreateData = {
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

				{ "(;[^\n]*)|(##.*##)", TokenType::comment },
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

			{ TokenType::__epsilon, "epsilon" },
			{ TokenType::__unknown, "unknown" }
		};

		LexerFactoy lexerFactory;
		Lexer lexer;

		inline void createLexer(void) {
			lexerFactory.setRules(CreateData);
			lexerFactory.update();
			lexer = lexerFactory.create();
		}
	}

	namespace parser {
		using TokenType = lexer::TokenType;
		int a = (int)TokenType::__end;

		NTSTART(NonterminalType)
			program,
			section,
			text_section,
			data_section,
			bss_section,

			intruction,

			reg,
			regid,
			regmode,
			reg_gen,
			reg_reg,

			label,

			expression,
			number,
		NTEND();

		using ParserFactoy = parser_generator::ParserFactory<TokenType, NonterminalType>;
		using PFCD = ParserFactoy::CreateData;
		using Parser = parser_generator::Parser<TokenType, NonterminalType>;
		using NT = NonterminalType;
		using TT = TokenType;

		GRAMMERSTART(const PFCD, CreateData, NonterminalType, NonterminalType::program)
			{ TT::__epsilon, { TT::whitespace } },
			{ TT::__epsilon, { TT::newline } },

			{ NT::program, { NT::section } },

			{ NT::section, { TT::text, NT::text_section, NT::section } },
			{ NT::section, { TT::data, NT::data_section, NT::section } },
			{ NT::section, { TT::bss, NT::bss_section, NT::section } },
			{ NT::section, { TT::__epsilon } },

			{ NT::text_section, { NT::intruction, NT::text_section} },
			{ NT::text_section, { NT::label, NT::text_section} },
			{ NT::text_section, { TT::__epsilon } },

			{ NT::intruction, { TT::add, NT::reg, NT::reg} },
			{ NT::intruction, { TT::addc, NT::reg, NT::reg} },
			{ NT::intruction, { TT::addi, NT::reg, NT::expression } },
			{ NT::intruction, { TT::sub, NT::reg, NT::reg} },
			{ NT::intruction, { TT::subc, NT::reg, NT::reg} },
			{ NT::intruction, { TT::subi, NT::reg, NT::expression } },
			{ NT::intruction, { TT::bxr, NT::reg, NT::reg} },
			{ NT::intruction, { TT::bxri, NT::reg, NT::expression } },
			{ NT::intruction, { TT::bor, NT::reg, NT::reg} },
			{ NT::intruction, { TT::bori, NT::reg, NT::expression } },
			{ NT::intruction, { TT::bnd, NT::reg, NT::reg} },
			{ NT::intruction, { TT::bndi, NT::reg, NT::expression } },
			{ NT::intruction, { TT::shiftl, NT::reg, NT::reg} },
			{ NT::intruction, { TT::shiftli, NT::reg, NT::expression } },
			{ NT::intruction, { TT::shiftr, NT::reg, NT::reg} },
			{ NT::intruction, { TT::shiftri, NT::reg, NT::expression } },
			{ NT::intruction, { TT::cmp, NT::reg, NT::reg} },

			{ NT::intruction, { TT::mov, NT::reg, NT::reg} },
			{ NT::intruction, { TT::set,  NT::reg, NT::expression } },

			{ NT::intruction, { TT::push, NT::reg } },
			{ NT::intruction, { TT::pop, NT::reg } },

			{ NT::intruction, { TT::ld, TT::dot, NT::regid, NT::reg, NT::expression} },
			{ NT::intruction, { TT::st, TT::dot, NT::regid, NT::reg, NT::expression} },

			{ NT::intruction, { TT::jmp, NT::expression, TT::dot, NT::regmode } },
			{ NT::intruction, { TT::call, NT::expression } },
			{ NT::intruction, { TT::ret } },
			{ NT::intruction, { TT::nop } },
			{ NT::intruction, { TT::halt } },

			{ NT::reg, { TT::percent, NT::regid, TT::dot, NT::regmode } },

			{ NT::regid, { NT::reg_gen } },
			{ NT::regid, { NT::reg_reg } },
			{ NT::regid, { TT::sbp } },
			{ NT::regid, { TT::zero } },
			{ NT::regid, { TT::one } },
			{ NT::regid, { TT::full } },
			{ NT::regid, { TT::pc } },
			{ NT::regid, { TT::stack } },
			{ NT::regid, { TT::flag } },

			{ NT::reg_gen, { TT::gen, TT::openbrack, NT::expression, TT::closebrack } },
			{ NT::reg_reg, { TT::reg, TT::openbrack, NT::expression, TT::closebrack } },

			{ NT::regmode, { TT::_32B_} },
			{ NT::regmode, { TT::_16L_} },
			{ NT::regmode, { TT::_16H_} },
			{ NT::regmode, { TT::_8L_} },
			{ NT::regmode, { TT::_8H_} },
			{ NT::regmode, { TT::_S16L_} },
			{ NT::regmode, { TT::_S8L_} },
			{ NT::regmode, { TT::_S8H_} },

			{ NT::label, { TT::identifier, TT::colon } },

			{ NT::expression, { NT::number } },

			{ NT::number, { TT::hexnum } },
			{ NT::number, { TT::decnum } },
			{ NT::number, { TT::octnum } },
			{ NT::number, { TT::binnum } },

		GRAMMEREND();

		ParserFactoy parserFactory;
		Parser parser;

		inline void createParser(void) {
			parserFactory.setRules(CreateData);
			parserFactory.update();
			parser = parserFactory.create();
		}

		typedef struct _Tree {
			lexer::Token token;
			NonterminalType type;
			bool useToken;
			struct _Tree* prev;
			std::vector<struct _Tree*>::iterator it;
			std::vector<struct _Tree*> child;
		} Tree;
	}

	namespace evaluator {
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

int main(int argc, char* argv[]) {
	lexer::createLexer();
	parser::createParser();

	ifstream file;
	file.open(argv[1]);

	std::cout << argc << std::endl;
	std::cout << argv[0] << std::endl;
	std::cout << argv[1] << std::endl;

	if (!file.is_open() || !file.good())
		return -1;

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
			switch (*si) {
			case '\n':
				si = tmp.erase(si);	
				si = tmp.insert(si, '\\');
				si++;
				si = tmp.insert(si, 'n');
				break;
			case '\t':
				si = tmp.erase(si);
				si = tmp.insert(si, '\\');
				si++;
				si = tmp.insert(si, 't');
				break;
			case ' ':
				si = tmp.erase(si);
				si = tmp.insert(si, '\'');
				si++;
				si = tmp.insert(si, ' ');
				si++;
				si = tmp.insert(si, '\'');
				break;
			default:
				si++;
				break;
			}
		}
		tmp.push_back('\t');
		cout << tmp << "; " << ((it->type == TokenType::__unknown) ? "error" : lexer::tokenStr[it->type]) << endl;
	}
	cout << "------------------------------------------------------------" << endl;


	std::vector<TokenType> ttv;
	ttv.reserve(tokens.size());
	for (auto it = tokens.begin(); it != tokens.end(); ++it)
		ttv.push_back(it->type);

	std::vector<unsigned long long> parseVec;
	bool validParse = parser::parser.parse(parseVec, ttv);
	cout << "parse->" << endl;
	cout << "------------------------------------------------------------" << endl;
	cout << (validParse ? "success" : "failed") << endl;
	cout << "------------------------------------------------------------" << endl;
	for (auto it = parseVec.begin(); it != parseVec.end(); it++) {
		cout << (int)(*it) << endl;
	}
	cout << "------------------------------------------------------------" << endl;

	return 0;
}