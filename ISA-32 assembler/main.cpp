#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <stack>
#include <queue>
#include <map>
#include <set>
#include <functional>
#include <chrono>

#include "lexer.hpp"
#include "parser.hpp"

using namespace std;

namespace assembler {
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

			flagid,

			label,

			index,

			expression,
			expressionL0,
			expressionL1,
			expressionL2,
			expressionL3,
			expressionL4,
			expressionL5,
			expressionL6,
			expressionL7,

			operatorL1,
			operatorL2,
			operatorL3,
			operatorL4,
			operatorL5,
			operatorL6,
			operatorL7,

			operationL1,
			operationL2,
			operationL3,
			operationL4,
			operationL5,
			operationL6,
			operationL7,

			define,
			macro,
			scope,

			constant,

			allocate,

			number,
		NTEND();

		enum class ASTNodeType {
			__epsilon = -1,
			__temp = 0,
			__accept = 1,

			program,

			text_section,
			data_section,
			bss_section,

			instruction_N,
			instruction_R,
			instruction_RR,
			instruction_RI,
			instruction_VRI,

			reg,
			regid,
			regmode,
			reg_index,

			flagid,

			label,

			index,

			expression,
			operator_,
			operation,

			hex, 
			dec, 
			oct, 
			bin
		};

		std::unordered_map<NonterminalType, std::string> nonterminalStr{
			{ NonterminalType::__accept, "accept" },

			{ NonterminalType::program, "program" },
			{ NonterminalType::section, "section" },
			{ NonterminalType::text_section, "text_section" },
			{ NonterminalType::data_section, "data_section" },
			{ NonterminalType::bss_section, "bss_section" },


			{ NonterminalType::intruction, "intruction" },

			{ NonterminalType::reg, "reg" },
			{ NonterminalType::regid, "regid" },
			{ NonterminalType::regmode, "regmode" },
			{ NonterminalType::reg_gen, "reg_gen" },
			{ NonterminalType::reg_reg, "reg_reg" },

			{ NonterminalType::flagid, "flagid" },

			{ NonterminalType::label, "label" },

			{ NonterminalType::index, "index" },

			{ NonterminalType::expression, "expression" },

			{ NonterminalType::expressionL0, "expressionL0" },
			{ NonterminalType::expressionL1, "expressionL1" },
			{ NonterminalType::expressionL2, "expressionL2" },
			{ NonterminalType::expressionL3, "expressionL3" },
			{ NonterminalType::expressionL4, "expressionL4" },
			{ NonterminalType::expressionL5, "expressionL5" },
			{ NonterminalType::expressionL6, "expressionL6" },
			{ NonterminalType::expressionL7, "expressionL7" },

			{ NonterminalType::operatorL1, "operatorL1" },
			{ NonterminalType::operatorL2, "operatorL2" },
			{ NonterminalType::operatorL3, "operatorL3" },
			{ NonterminalType::operatorL4, "operatorL4" },
			{ NonterminalType::operatorL5, "operatorL5" },
			{ NonterminalType::operatorL6, "operatorL6" },
			{ NonterminalType::operatorL7, "operatorL7" },

			{ NonterminalType::operationL1, "operationL1" },
			{ NonterminalType::operationL2, "operationL2" },
			{ NonterminalType::operationL3, "operationL3" },
			{ NonterminalType::operationL4, "operationL4" },
			{ NonterminalType::operationL5, "operationL5" },
			{ NonterminalType::operationL6, "operationL6" },
			{ NonterminalType::operationL7, "operationL7" },

			{ NonterminalType::number, "number" }
		};

		std::unordered_map<ASTNodeType, std::string> asttypeStr = {
			{ ASTNodeType::__epsilon, "epsilon" },
			{ ASTNodeType::__temp, "temp" },
			{ ASTNodeType::__accept, "accept" },

			{ ASTNodeType::program, "program" },

			{ ASTNodeType::text_section, "text_section" },
			{ ASTNodeType::data_section, "data_section" },
			{ ASTNodeType::bss_section, "bss_section" },

			{ ASTNodeType::instruction_N, "instruction_N" },
			{ ASTNodeType::instruction_R, "instruction_R" },
			{ ASTNodeType::instruction_RR, "instruction_RR" },
			{ ASTNodeType::instruction_RI, "instruction_RI" },
			{ ASTNodeType::instruction_VRI, "instruction_VRI" },

			{ ASTNodeType::reg, "reg" },
			{ ASTNodeType::regid, "regid" },
			{ ASTNodeType::regmode, "regmode" },
			{ ASTNodeType::reg_index, "reg_index" },

			{ ASTNodeType::flagid, "flagid" },

			{ ASTNodeType::label, "label" },

			{ ASTNodeType::index, "index" },

			{ ASTNodeType::expression, "expression" },
			{ ASTNodeType::operator_, "operator_" },
			{ ASTNodeType::operation, "operation" },

			{ ASTNodeType::hex, "hex" },
			{ ASTNodeType::dec, "dec" },
			{ ASTNodeType::oct, "oct" },
			{ ASTNodeType::bin, "bin" }
		};

		using ParserFactoy = parser_generator::ParserFactory<TokenType, NonterminalType, ASTNodeType>;
		using PFCD = ParserFactoy::CreateData;
		using Parser = parser_generator::Parser<TokenType, NonterminalType, ASTNodeType>;
		using Tree = parser_generator::PTNode;
		using NT = NonterminalType;
		using TT = TokenType;
		using AT = ASTNodeType;

		const PFCD CreateData = {
			{ { NT::__accept, AT::__accept, -1 }, { { NT::program, true, false } } },

			{ { NT::program, AT::program, -1}, { { NT::section, true, true } } },

			{ { NT::section, AT::text_section, -1 }, { { TT::text, false , false }, {NT::text_section, true, false }, {NT::section, true, true } } },
			{ { NT::section, AT::data_section, -1 }, { { TT::data, false, false }, {NT::data_section, true, false }, {NT::section, true, true } } },
			{ { NT::section, AT::bss_section, -1 }, { { TT::bss, false, false }, {NT::bss_section, true, false }, {NT::section, true, true } } },

			{ { NT::section, AT::text_section, -1 }, { { TT::text, false, false }, {NT::text_section, true, false } } },
			{ { NT::section, AT::data_section, -1 }, { { TT::data, false, false }, {NT::data_section, true, false} } },
			{ { NT::section, AT::bss_section, -1 }, { { TT::bss, false, false }, {NT::bss_section, true, false } } },

			{ { NT::text_section, AT::text_section, -1 }, { { NT::intruction, true, false }, {NT::text_section, true, true } } },
			{ { NT::text_section, AT::text_section, -1 }, { { NT::label, true, false }, { NT::text_section, true, true } } },

			{ { NT::text_section, AT::text_section, -1 }, { { NT::intruction, true, false } } },
			{ { NT::text_section, AT::text_section, -1 }, { { NT::label, true, false } } },

			{ { NT::intruction, AT::instruction_RR, 0 }, { { TT::add, false, false }, {NT::reg, true, false }, { NT::reg, true, false } } },
			{ { NT::intruction, AT::instruction_RR, 0 }, { { TT::addc, false, false }, {NT::reg, true, false }, { NT::reg, true, false } } },
			{ { NT::intruction, AT::instruction_RI, 0 }, { { TT::addi, false, false }, {NT::reg, true, false }, {NT::expression, true, false } } },
			{ { NT::intruction, AT::instruction_RR, 0 }, { { TT::sub, false, false }, {NT::reg, true, false }, { NT::reg, true, false } } },
			{ { NT::intruction, AT::instruction_RR, 0 }, { { TT::subc, false, false }, {NT::reg, true, false }, { NT::reg, true, false } } },
			{ { NT::intruction, AT::instruction_RI, 0 }, { { TT::subi, false, false }, {NT::reg, true, false }, {NT::expression, true, false } } },
			{ { NT::intruction, AT::instruction_RR, 0 }, { { TT::bxr, false, false }, {NT::reg, true, false }, { NT::reg, true, false } } },
			{ { NT::intruction, AT::instruction_RI, 0 }, { { TT::bxri, false, false }, {NT::reg, true, false }, {NT::expression, true, false } } },
			{ { NT::intruction, AT::instruction_RR, 0 }, { { TT::bor, false, false }, {NT::reg, true, false }, { NT::reg, true, false } } },
			{ { NT::intruction, AT::instruction_RI, 0 }, { { TT::bori, false, false }, {NT::reg, true, false }, {NT::expression, true, false } } },
			{ { NT::intruction, AT::instruction_RR, 0 }, { { TT::bnd, false, false }, {NT::reg, true, false }, { NT::reg, true, false } } },
			{ { NT::intruction, AT::instruction_RI, 0 }, { { TT::bndi, false, false }, {NT::reg, true, false }, {NT::expression, true, false } } },
			{ { NT::intruction, AT::instruction_RR, 0 }, { { TT::shiftl, false, false }, {NT::reg, true, false }, { NT::reg, true, false } } },
			{ { NT::intruction, AT::instruction_RI, 0 }, { { TT::shiftli, false, false }, {NT::reg, true, false }, {NT::expression, true, false } } },
			{ { NT::intruction, AT::instruction_RR, 0 }, { { TT::shiftr, false, false }, {NT::reg, true, false }, { NT::reg, true, false } } },
			{ { NT::intruction, AT::instruction_RI, 0 }, { { TT::shiftri, false, false }, {NT::reg, true, false }, {NT::expression, true, false } } },
			{ { NT::intruction, AT::instruction_RR, 0 }, { { TT::cmp, false, false }, {NT::reg, true, false }, { NT::reg, true, false } } },

			{ { NT::intruction, AT::instruction_RR, 0 }, { { TT::mov, false, false }, { NT::reg, true, false }, {NT::reg, true, false } } },
			{ { NT::intruction, AT::instruction_RI, 0 }, { { TT::set, false, false }, { NT::reg, true, false }, {NT::expression, true, false } } },

			{ { NT::intruction, AT::instruction_R, 0 }, { { TT::push, false, false }, {NT::reg, true, false } } },
			{ { NT::intruction, AT::instruction_R, 0 }, { { TT::pop, false, false }, {NT::reg, true, false } } },

			{ { NT::intruction, AT::instruction_VRI, 0 }, { { TT::ld, false, false }, {TT::dot, false, false }, {NT::regid, true, false }, {NT::reg, true, false }, {NT::expression, true, false } } },
			{ { NT::intruction, AT::instruction_VRI, 0 }, { { TT::st, false, false }, {TT::dot, false, false }, {NT::regid, true, false }, {NT::reg, true, false }, {NT::expression, true, false } } },

			{ { NT::intruction, AT::instruction_VRI, 0 }, { { TT::jmp, false, false }, {TT::dot, false, false }, {NT::flagid, true, true }, {NT::reg, true, false }, {NT::expression, true, false } } },
			{ { NT::intruction, AT::instruction_N, 0 }, { { TT::call, false, false } } },
			{ { NT::intruction, AT::instruction_N, 0 }, { { TT::ret, false, false } } },
			{ { NT::intruction, AT::instruction_N, 0 }, { { TT::nop, false, false } } },
			{ { NT::intruction, AT::instruction_N, 0 }, { { TT::halt, false, false } } },

			{ { NT::reg, AT::reg, 1 }, { { TT::percent, false, false }, {NT::regid, true, false }, {TT::dot, false, false }, {NT::regmode, true, true } } },

			{ { NT::regid, AT::__epsilon, 0 }, { { NT::reg_gen, true, true } } },
			{ { NT::regid, AT::__epsilon, 0 }, { { NT::reg_reg, true, true } } },
			{ { NT::regid, AT::regid, -1 }, { { TT::sbp, true, false } } },
			{ { NT::regid, AT::regid, -1 }, { { TT::zero, true, false } } },
			{ { NT::regid, AT::regid, -1 }, { { TT::one, true, false } } },
			{ { NT::regid, AT::regid, -1 }, { { TT::full, true, false } } },
			{ { NT::regid, AT::regid, -1 }, { { TT::pc, true, false } } },
			{ { NT::regid, AT::regid, -1 }, { { TT::stack, true, false } } },
			{ { NT::regid, AT::regid, -1 }, { { TT::flag, true, false } } },

			{ { NT::reg_gen, AT::reg_index, -1 }, { { TT::gen, true, false }, { NT::index, true, false } } },
			{ { NT::reg_reg, AT::reg_index, -1 }, { { TT::reg, true, false }, { NT::index, true, false } } },

			{ { NT::regmode, AT::regmode, -1 }, { { TT::_32B_, true, false } } },
			{ { NT::regmode, AT::regmode, -1 }, { { TT::_16L_, true, false } } },
			{ { NT::regmode, AT::regmode, -1 }, { { TT::_16H_, true, false } } },
			{ { NT::regmode, AT::regmode, -1 }, { { TT::_8L_, true, false } } },
			{ { NT::regmode, AT::regmode, -1 }, { { TT::_8H_, true, false } } },
			{ { NT::regmode, AT::regmode, -1 }, { { TT::_S16L_, true, false } } },
			{ { NT::regmode, AT::regmode, -1 }, { { TT::_S8L_, true, false } } },
			{ { NT::regmode, AT::regmode, -1 }, { { TT::_S8H_, true, false } } },

			{ { NT::flagid, AT::flagid, -1 }, { { TT::zero, true, false } } },
			{ { NT::flagid, AT::flagid, -1 }, { { TT::neg, true, false } } },
			{ { NT::flagid, AT::flagid, -1 }, { { TT::pos, true, false } } },
			{ { NT::flagid, AT::flagid, -1 }, { { TT::carry, true, false } } },
			{ { NT::flagid, AT::flagid, -1 }, { { TT::carry4, true, false } } },
			{ { NT::flagid, AT::flagid, -1 }, { { TT::overflow, true, false } } },
			{ { NT::flagid, AT::flagid, -1 }, { { TT::one, true, false } } },
			{ { NT::flagid, AT::flagid, -1 }, { { TT::gen, true, false } } },

			{ { NT::label, AT::label, 0 }, { { TT::identifier, false, false }, {TT::colon, false, false } } },

			{ { NT::index, AT::index, -1 }, { { TT::openbrack, false, false }, { NT::expression, true, false }, { TT::closebrack, false, false } } },

			{ { NT::expression, AT::__epsilon, 0 }, { { NT::expressionL7, true, true } } },

			{ { NT::expressionL7, AT::__epsilon, 1 }, { { NT::expressionL7, true, false }, {TT::verticalbar, false, false }, { NT::expressionL6, true, false } } },
			{ { NT::expressionL7, AT::__epsilon, 0 }, { { NT::expressionL6, true, true } } },

			{ { NT::expressionL6, AT::__epsilon, 1 }, { { NT::expressionL6, true, false }, {TT::caret, false, false }, { NT::expressionL5, true, false } } },
			{ { NT::expressionL6, AT::__epsilon, 0 }, { { NT::expressionL5, true, true } } },

			{ { NT::expressionL5, AT::__epsilon, 1 }, { { NT::expressionL5, true, false }, {TT::ampersend, false, false }, { NT::expressionL4, true, false } } },
			{ { NT::expressionL5, AT::__epsilon, 0 }, { { NT::expressionL4, true, true } } },

			{ { NT::expressionL4, AT::__epsilon, 1 }, { { NT::expressionL4, true, false }, {TT::plus, false, false }, { NT::expressionL3, true, false } } },
			{ { NT::expressionL4, AT::__epsilon, 1 }, { { NT::expressionL4, true, false }, {TT::minus, false, false }, { NT::expressionL3, true, false } } },
			{ { NT::expressionL4, AT::__epsilon, 0 }, { { NT::expressionL3, true, true } } },

			{ { NT::expressionL3, AT::__epsilon, 1 }, { { NT::expressionL3, true, false }, {TT::star, false, false }, { NT::expressionL2, true, false } } },
			{ { NT::expressionL3, AT::__epsilon, 1 }, { { NT::expressionL3, true, false }, {TT::slash, false, false }, { NT::expressionL2, true, false } } },
			{ { NT::expressionL3, AT::__epsilon, 0 }, { { NT::expressionL2, true, true } } },

			{ { NT::expressionL2, AT::__epsilon, 1 }, { { NT::expressionL1, true, false }, {TT::dstar, false, false }, { NT::expressionL2, true, false } } },
			{ { NT::expressionL2, AT::__epsilon, 0 }, { { NT::expressionL1, true, true } } },

			{ { NT::expressionL1, AT::__epsilon, 0 }, { { TT::tilde, false, false }, { NT::expressionL0, true, false } } },
			{ { NT::expressionL1, AT::__epsilon, 0 }, { { NT::expressionL0, true, true } } },

			{ { NT::expressionL0, AT::__epsilon, 1 }, { { TT::openparen, false, false }, {NT::expressionL7, true, false }, { TT::closeparen, false, false } } },
			{ { NT::expressionL0, AT::__epsilon, 0 }, { { NT::number, false , false } } },

			{ { NT::number, AT::hex, 0 }, { { TT::hexnum, false, false } } },
			{ { NT::number, AT::dec, 0 }, { { TT::decnum, false, false } } },
			{ { NT::number, AT::oct, 0 }, { { TT::octnum, false, false } } },
			{ { NT::number, AT::bin, 0 }, { { TT::binnum, false, false } } }
		};

		ParserFactoy parserFactory;
		Parser parser;

		inline void createParser(void) {
			parserFactory.setRules(CreateData);

			std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
			parserFactory.update();
			parser = parserFactory.create();
		}
	}

	namespace evaluator {
	}

	namespace iosystem {

	}
}

using namespace assembler;
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

	for (auto it = tokens.begin(); it != tokens.end();) {
		if (it->type == TokenType::whitespace || it->type == TokenType::newline || it->type == TokenType::comment)
			it = tokens.erase(it);
		else
			it++;
	}


	using ElementWrapper = typename assembler::parser::ParserFactoy::ElementWrapper;
	using AST = Parser::ASTNode;

	AST* pAST;
	std::vector<unsigned long long> parseVec;
	bool validParse = parser::parser.parse(pAST, parseVec, tokens);
	cout << "parse->" << endl;
	cout << "------------------------------------------------------------" << endl;
	cout << (validParse ? "success" : "failed") << endl;
	cout << "------------------------------------------------------------" << endl;
	for (auto it = parseVec.begin(); it != parseVec.end(); it++) {
		cout << (int)(*it) << ": ";
		auto tmp = parser::CreateData[*it];
		cout << ((tmp.first.symbol.getRaw().first == ElementWrapper::Type::TERMINAL) ? lexer::tokenStr[(TokenType)tmp.first.symbol.getRaw().second] : parser::nonterminalStr[(NonterminalType)tmp.first.symbol.getRaw().second]) << " -> ";
		for (auto si = tmp.second.begin(); si != tmp.second.end(); si++)
			cout << ((si->symbol.getRaw().first == ElementWrapper::Type::TERMINAL) ? lexer::tokenStr[(TokenType)si->symbol.getRaw().second] : parser::nonterminalStr[(NonterminalType)si->symbol.getRaw().second]) << ", ";
		cout << endl;
	}
	cout << "------------------------------------------------------------" << endl;

	std::map<int, std::vector<AST*>> levelMap;
	std::queue<std::pair<AST*, int>> visitQ;
	visitQ.push({ pAST, 0 });
	while (!visitQ.empty()) {
		std::pair<AST*, int> cur = visitQ.front(); visitQ.pop();
		levelMap[cur.second].push_back(cur.first);

		for (auto it = cur.first->child.begin(); it != cur.first->child.end(); ++it)
			visitQ.push({ *it, cur.second + 1 });
	}
	int prevLevel = -1;
	for (auto it = levelMap.begin(); it != levelMap.end(); ++it) {
		for (auto si = it->second.begin(); si != it->second.end(); ++si) {
			cout << " | " << (((*si)->type < (size_t)TokenType::__end) ? lexer::tokenStr[(TokenType)(*si)->type] : parser::asttypeStr[(ASTNodeType)((unsigned __int64)(*si)->type - (unsigned __int64)(TokenType::__end))]) << ", " << (*si)->text;
		}
		if (it->first != prevLevel)
			cout << endl;
		prevLevel = it->first;
	}
	cout << endl;

	parser::parser.delAST(pAST);

	return 0;
}