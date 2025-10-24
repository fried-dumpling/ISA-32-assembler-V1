#ifndef DPICP_PARSER
#define DPICP_PARSER

#include <iostream>

#include <string>
#include <vector>

#include <map>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <bitset>

#include <queue>
#include <stack>

#include <utility>

//
// 
// 
// typedef struct {
//		lhs,
//		rhs,
//		dotPos
// } core;
// 
// typedef struct {
//		core,
//		lookahead
// } Item;
// 
// terminator
// nonTerminator
// 
// terminator 0 -> epsilon
// 
// First(x)
// RingSum(x)
// 
// CLOSURE()
// Goto()
// 
// GenC1()
// GenTable()
// 
// typedef struct {
//		union {
//			NonTerminal nonTerminal
//			Terminal terminal
//		}
//		bool isTerminal
// } Element
// 
// grammer:
// { NonTerminal, std::vector<Element>{...} } <--- input
// 
// typedef struct _ParseTree {
//		Token token
//		std::vector<struct _ParseTree*> child;
// } ParseTree
// 
// template < enum NT, enum T>
// class Parser {
// private:
// public:
//		void Parse (std::vector<Token = Terminal> input, ParseTree output) {
//			-->> algorithem <<--
//			std::vector<GrammerID> parseResult
//			getParse(parseResult);
//			formTree(parseResult);
//		}
// }
// 
// 
// 
//

namespace parser_generator {
	namespace convert {
		using u64 = unsigned __int64;
		using Element = u64;

		class Item {
		public:
			u64 prodId;
			u64 dotPos;
		};

		class Rule {
		public:
			Element lhs;
			std::vector<Element> rhs;
		};

		template <typename NonTerminal, typename Terminal>
		using Grammer = std::unordered_map < NonTerminal, std::vector<Element>> ;

		void getFirstTable(
			std::unordered_map<Element, std::vector<u64>> &table, 
			std::unordered_multimap<Element, Element>& revFirstGrammmer, 
			Element terminalEnd) {

			table.clear();
			size_t termCount = (terminalEnd >> 6) + 1;

			typedef struct _QData {
				Element cur;
				Element prev;
			} QData;

			std::queue<QData> elementQ;

			for (Element i = 0; i < terminalEnd; i++) {
				std::vector<u64> tmp(termCount, 0);
				tmp[i >> 6] |= (u64)1 << (i & 0b111111);
				table[i] = tmp;

				elementQ.push({ i, i });
			}
	
			while (!elementQ.empty()) {
				QData cur = elementQ.front(); elementQ.pop();

				std::vector<u64>& curV = table[cur.cur], prevV = table[cur.prev];
				for (size_t i = 0; i < curV.size(); i++)
					curV[i] |= prevV[i];

				auto range = revFirstGrammmer.equal_range(cur.cur);
				for (auto it = range.first; it != range.second; ++it)
					elementQ.push({ it->second, cur.cur });
			}
		}

		void getLookahead(
			std::vector<u64>& lookahead,
			std::unordered_map<Element, std::vector<u64>>& firstTable,
			std::vector<u64>& prevLookahead,
			Element next) {

			lookahead = firstTable[next];

			if (firstTable[next][0] & 0b1) {
				size_t size = std::max(lookahead.size(), prevLookahead.size());
				lookahead.resize(size, 0);

				for (size_t i = 0; i < size; i++)
					lookahead[i] |= prevLookahead[i];
			}
		}

		typedef struct _ItemLessRule {
			bool operator() (const Item& left, const Item& right) const {
				if (left.prodId != right.prodId) {
					if (left.prodId < right.prodId)
						return true;
					return false;
				}
				if (left.dotPos < right.dotPos)
					return true;
				return false;
			}
		} ItemLessRule;

		void getClosure(
			std::vector<Item>& closure,
			std::vector<std::pair<Element, std::vector<Element>>> &grammerVec,
			std::unordered_multimap<Element, std::pair<std::vector<Element>, u64>>& grammerMap,
			std::vector<Item>& items) {

			typedef struct _QData {
				Item item;
				Item prev;
			} QData;

			std::unordered_set<Element> visited;

			std::queue<QData> itemQ;
			for (auto it = items.begin(); it != items.end(); ++it) {
				itemQ.push({ *it, *it });
			}

			while (!itemQ.empty()) {
				QData cur = itemQ.front(); itemQ.pop();

				Element mark = grammerVec[cur.item.prodId].second[cur.item.dotPos];
				if (visited.find(mark) != visited.end())
					continue;

				visited.insert(mark);

				auto range = grammerMap.equal_range(mark);
				for (auto it = range.first; it != range.second; ++it) {
					Item tmp;
					tmp.prodId = it->second.second;
					tmp.dotPos = 0;

					itemQ.push({ tmp, cur.item });
					closure.push_back(tmp);
				}
			}

			for (auto it = items.begin(); it != items.end(); ++it) {
				if (visited.find(grammerVec[it->prodId].first) == visited.end())
					closure.push_back(*it);
				else if (it->dotPos) 
					closure.push_back(*it);
			}
		}

		void getGotoItem(
			std::unordered_map<Element, std::vector<Item>>& gotoMap,
			std::vector<std::pair<Element, std::vector<Element>>>& grammerVec,
			std::vector<Item>& closure) {
			
			for (auto it = closure.begin(); it != closure.end(); ++it) {
				if (it->dotPos + 1 >= grammerVec[it->prodId].second.size())
					continue;
				gotoMap[grammerVec[it->prodId].second[it->dotPos]].push_back({ it->prodId, it->dotPos + 1 });
			}
		}

		void createC0(
			std::vector<std::pair<Element, std::vector<Element>>>& grammerVec,
			Item startItem) {
		}

		void createTable() {

		}
	}

	template <typename NonTerminal, typename Terminal>
	class ParserFactory;

	template <typename NonTerminal, typename Terminal>
	class Parser {
	private:
		friend ParserFactory<NonTerminal, Terminal>;

	public:

	};

	template <typename NonTerminal, typename Terminal>
	class ParserFactory {
	private:

	public:

	};
}

#endif