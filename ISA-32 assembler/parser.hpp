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
		using ID = u64;
		using Element = u64;

		class Item {
		public:
			u64 prodId;
			u64 dotPos;
			u64 setId;
		};

		class Rule {
		public:
			Element lhs;
			std::vector<Element> rhs;
		};

		template <typename NonTerminal, typename Terminal>
		using Grammer = std::unordered_map < NonTerminal, std::vector<Element>> ;

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

		typedef struct _SetLessRule {
			bool operator() (const std::vector<Item>& left, const std::vector<Item>& right) const {
				if (left.size() < right.size())
					return true;
				if (left.size() > right.size())
					return false;

				auto li = left.cbegin(), ri = right.cbegin();
				while (true) {
					if (li->prodId != ri->prodId) {
						if (li->prodId < ri->prodId)
							return true;
						return false;
					}
					if (li->dotPos < ri->dotPos)
						return true;

					++li; ++ri;
				}
				return false;
			}
		} SetLessRule;

		void createC0(
			std::vector<std::vector<Item>>& closureVec,
			std::unordered_multimap<ID, std::pair<ID, Element>>& transitionMap,
			std::vector<std::pair<Element, std::vector<Element>>>& grammerVec,
			std::unordered_multimap<Element, std::pair<std::vector<Element>, u64>>& grammerMap,
			Item startItem) {

			typedef struct _QData {
				std::vector<Item> closure;
				ID id;
			} QData;

			ID closureId = 0;
			ID startId = closureId++;
			std::set<std::vector<Item>, SetLessRule> visited;

			std::queue<QData> setQ;
			std::vector<Item> startClosure;
			std::vector<Item> startItemVec = { startItem };
			getClosure(startClosure, grammerVec, grammerMap, startItemVec);
			setQ.push({ startClosure, startId });
			
			while (!setQ.empty()) {
				QData cur = setQ.front(); setQ.pop();

				visited.insert(cur.closure);
				closureVec.push_back(cur.closure);

				std::unordered_map<Element, std::vector<Item>> gotoMap;
				getGotoItem(gotoMap, grammerVec, cur.closure);
				for (auto it = gotoMap.begin(); it != gotoMap.end(); ++it) {
					std::vector<Item> closure;
					getClosure(closure, grammerVec, grammerMap, it->second);

					if (visited.find(cur.closure) != visited.end())
						continue;

					ID id = closureId++;
					transitionMap.insert({ cur.id, { id, it->first} });

					setQ.push({ closure, id });
				}
			}
		}

		void getFirstTable(
			std::unordered_map<Element, std::vector<u64>>& table,
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

		// 
		//	LA(p, [A->x.y]) {
		//		set = {}
		//		
		//		auto range = revTransitionMap[x].equal_range(p);
		//		for (auto qIt = range.first; qIt != range.second; ++qIt) {
		//			q = grammerVec[*qIt];
		//			for (auto it = q.begin(); it != q.end(); ++it) {
		//				set.insert(firstTable[it->b]);
		//				if (firstTable[it->b].epslon)
		//					set.insert(LA(q, [B->a.Ab]));
		//			}	
		//		}
		//	}
		//	
		//	q = PRED(p, x);
		//	[B->aAb] << q;
		//	LA(p, [A->x.y]) = first(b) + LA(q, [B->aAb]);
		// 

		void getLookahead(
			std::vector<u64>& lookahead,
			std::vector<u64>& nextLookahead,
			std::unordered_map<Element, std::unordered_multimap<ID, ID>>& revTransitionMap,
			std::vector<std::vector<Item>>& closureVec,
			std::vector<std::pair<Element, std::vector<Element>>>& grammerVec,
			std::unordered_map<Element, std::vector<u64>>& firstTable,
			ID closureID, Item item) {

			std::pair<Element, std::vector<Element>>& core = grammerVec[item.prodId];
			Element mark = core.second[item.dotPos];
			auto range = revTransitionMap[mark].equal_range(closureID);
			for (auto it = range.first; it != range.second; ++it) {
				std::vector<Item>& q = closureVec[it->second];
				for (auto si = q.begin(); si != q.end(); ++si) {
					std::vector<Element>& rhs = grammerVec[si->prodId].second;

					if (rhs.size() == si->dotPos + 1)
						goto first_epsilon;

					{
						std::vector<u64>& first = firstTable[rhs[si->dotPos + 1]];
						if (!first.size())
							goto first_epsilon;
						if (first[0] & 0b1)
							goto first_epsilon;

						lookahead.resize(std::max(lookahead.size(), first.size()), 0);
						for (size_t i = 0; i < first.size(); i++)
							lookahead[i] |= first[i];
					}

					continue;

				first_epsilon:
					lookahead.resize(std::max(lookahead.size(), nextLookahead.size()), 0);
					for (size_t i = 0; i < nextLookahead.size(); i++)
						lookahead[i] |= nextLookahead[i];
				}
			}
		}

		void setLookahead(
			std::vector<std::unordered_set<Element>>& lookaheadSet,
			std::vector<std::vector<Item>>& c0graph,
			std::unordered_map<Element, std::unordered_multimap<ID, ID>>& revTransitionMap,
			std::vector<std::pair<Element, std::vector<Element>>>& grammerVec,
			std::unordered_multimap<Element, std::pair<std::vector<Element>, u64>>& grammerMap,
			std::unordered_map<Element, std::vector<u64>>& firstTable) {
		}

		void createTable(std::vector<std::pair<Element, std::vector<Element>>>& grammerVec) {

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