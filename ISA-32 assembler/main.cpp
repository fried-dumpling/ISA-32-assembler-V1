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

#include "assembler.hpp"
#include "iotool.hpp"

using namespace std;

using namespace assembler;
using namespace assembler::lexer;
using namespace assembler::parser;
using namespace assembler::evaluator;

int main(int argc, char* argv[]) {
	assembler::createAssembler();

		if (argc < 3) {
		std::cout << "usage: assembler <input file> <output file> <-file, -token, -preproc, -eval, -bin, -double>..." << std::endl;
		return -1;
	}

	const char* inputFile = argv[1];
	const char* outputFile = argv[2];

	bool dumpFile = false, dumpToken = false, dumpPreproc = false, dumpAST = false, dumpEvaluate = false, dumpBinary = false, doubleOutput = false;
	for (int i = 3; i < argc; i++) {
		std::string flag = argv[i];
		if (flag == "-file")
			dumpFile = true;
		else if (flag == "-token")
			dumpToken = true;
		else if (flag == "-preproc")
			dumpPreproc = true;
		else if (flag == "-eval")
			dumpEvaluate = true;
		else if (flag == "-bin")
			dumpBinary = true;
		else if (flag == "-double")
			doubleOutput = true;
		else {
			cout << "unknown flag \'" << flag << "\'" << endl;
			return -1;
		}
	}

	std::string buff;
	if (!iotools::readTextFile(inputFile, buff)) {
		cout << "failed to read input file \'" << inputFile << "\'" << endl;
		return -1;
	}

	if (dumpFile) {
		cout << "file->" << endl;
		cout << "------------------------------------------------------------" << endl;
		cout << buff << endl;
		cout << "------------------------------------------------------------" << endl;
	}

	assembler::AssemblerDump dump;
	if (dumpToken)
		dump.flags |= assembler::AssemblerDump::getToken;
	if (dumpPreproc)
		dump.flags |= assembler::AssemblerDump::getPreproc;
	if (dumpAST)
		dump.flags |= assembler::AssemblerDump::getAST;
	if (dumpEvaluate)
		dump.flags |= assembler::AssemblerDump::getEval;
	if (dumpBinary)
		dump.flags |= assembler::AssemblerDump::getBin;

	std::vector<u8> binary;
	assembler::assemble(buff, binary, dump);

	if (dumpToken) {
		cout << "lex->" << endl;
		cout << "------------------------------------------------------------" << endl;
		for (auto it = dump.tokens.begin(); it != dump.tokens.end(); it++) {
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
	}

	if (dumpPreproc) {
		cout << "preproc->" << endl;
		cout << "------------------------------------------------------------" << endl;
		for (auto it = dump.preprocTokens.begin(); it != dump.preprocTokens.end(); it++) {
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
	}

	if (dumpEvaluate || dumpBinary) {
		cout << "parse->" << endl;
		cout << (dump.parseSuccess ? "success" : "failed") << endl;
		if (!dump.parseSuccess)
			cout << dump.errorMessage << " <- here" << endl;
		cout << "------------------------------------------------------------" << endl;
		cout << "evaluate->" << endl;
		cout << (dump.evalSuccess ? "success" : "failed") << endl;
		cout << "------------------------------------------------------------" << endl;
	}

	if (dumpBinary) {
		cout << "binary->" << endl;
		cout << "------------------------------------------------------------" << endl;
		cout << "text section" << endl;
		for (auto it = dump.textSection.begin(); it != dump.textSection.end(); ) {
			for (int i = 0; it != dump.textSection.end() && i++ < 4; ++it)
				printf("%02X ", *it);
			cout << endl;
		}
		cout << "data section" << endl;
		for (auto it = dump.dataSection.begin(); it != dump.dataSection.end(); ) {
			for (int i = 0; it != dump.dataSection.end() && i++ < 4; ++it)
				printf("%02X ", *it);
			cout << endl;
		}
		cout << "------------------------------------------------------------" << endl;
		cout << "final binary" << endl;
		for (auto it = binary.begin(); it != binary.end(); ) {
			for (int i = 0; it != binary.end() && i++ < 4; ++it)
				printf("%02X ", *it);
			cout << endl;
		}
		cout << "------------------------------------------------------------" << endl;
	}
	
	if (!iotools::writeBinaryFile(outputFile, binary, (doubleOutput) ? 2 : 1)) {
		cout << "failed to create output file \'" << inputFile << "\'" << endl;
		return -1;
	}

	return 0;
}

/*

int main(int argc, char* argv[]) {
	lexer::createLexer();
	parser::createParser();

	if (argc < 3) {
		std::cout << "usage: assembler <input file> <output file>" << std::endl;
		return -1;
	}

	const char* inputFile = argv[1];
	const char* outputFile = argv[2];

	std::string buff;
	iotools::readTextFile(inputFile, buff);

	bool dumpFile = false;
	if (dumpFile) {
		cout << "file->" << endl;
		cout << "------------------------------------------------------------" << endl;
		cout << buff << endl;
		cout << "------------------------------------------------------------" << endl;
	}

	std::vector<Token> tokens;
	lexer::lexer.lex(tokens, buff);

	bool dumpLex = false;
	if (dumpLex) {
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
	}

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
	bool validParse = parser::parser.parse(pAST, tokens);
	bool dumpParse = false;
	if (dumpParse) {
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
	}

	bool dumpAST = false;
	if (dumpAST) {
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
	}
	
	evaluator::functions::EvalData data;
	evaluator::setTree(pAST);
	bool validEvaluate = evaluator::evaluate(data);

	bool dumpEvaluate = false;
	if (dumpEvaluate) {
		cout << "evaluate->" << endl;
		cout << "------------------------------------------------------------" << endl;
		cout << (validEvaluate ? "success" : "failed") << endl;
		cout << "------------------------------------------------------------" << endl;
	}

	parser::parser.delAST(pAST);

	cout << "text section" << endl;
	for (auto it = data.textSection.begin(); it != data.textSection.end(); ++it) {
		printf("0x%08X\n", *it);
	}
	cout << "data section" << endl;
	for (auto it = data.dataSection.begin(); it != data.dataSection.end(); ++it) {
		printf("0x%08X\n", *it);
	}

	iotools::writeBinaryFile(outputFile, data.rawData, true);

	return 0;
}
*/