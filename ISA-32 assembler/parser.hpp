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
			u64 lookaheadId;
		};

		class Rule {
		public:
			Element lhs;
			std::vector<Element> rhs;
		};

		template <typename NonTerminal, typename Terminal>
		using Grammer = std::unordered_map < NonTerminal, std::vector<Element>> ;

		inline void getClosure(
			std::vector<u64>& reduceItems,
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
			int counter = 0;
			for (auto it = items.begin(); it != items.end(); ++it) {
				if (grammerVec[it->prodId].second.size() == it->dotPos) {
					it->lookaheadId = 0;
					reduceItems.push_back(counter++);
					continue;
				}

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
					tmp.lookaheadId = (u64)-1;

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

		inline void getGotoItem(
			std::unordered_map<Element, std::vector<Item>>& gotoMap,
			std::vector<std::pair<Element, std::vector<Element>>>& grammerVec,
			std::vector<Item>& closure) {
			
			for (auto it = closure.begin(); it != closure.end(); ++it) {
				if (it->dotPos + 1 > grammerVec[it->prodId].second.size())
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

		inline void createC0(
			std::unordered_map<u64, std::vector<u64>>& reduceItem,
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

			u64 counter = 0;

			std::queue<QData> setQ;
			std::vector<Item> startClosure;
			std::vector<Item> startItemVec = { startItem };
			getClosure(reduceItem[counter++], startClosure, grammerVec, grammerMap, startItemVec);
			setQ.push({ startClosure, startId });
			
			while (!setQ.empty()) {
				QData cur = setQ.front(); setQ.pop();

				visited.insert(cur.closure);
				closureVec.push_back(cur.closure);

				std::unordered_map<Element, std::vector<Item>> gotoMap;
				getGotoItem(gotoMap, grammerVec, cur.closure);
				for (auto it = gotoMap.begin(); it != gotoMap.end(); ++it) {
					std::vector<Item> closure;
					getClosure(reduceItem[counter++], closure, grammerVec, grammerMap, it->second);

					if (visited.find(cur.closure) != visited.end())
						continue;

					ID id = closureId++;
					transitionMap.insert({ cur.id, { id, it->first} });

					setQ.push({ closure, id });
				}
			}
		}

		inline void getFirstTable(
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

		inline void lookaheadHelper(
			std::vector<u64>& lookahead,
			std::vector<std::pair<u64, u64>> ref,
			std::vector<std::vector<Item>>& closureVec,
			std::unordered_map<Element, std::unordered_multimap<ID, ID>>& revTransitionMap,
			std::vector<std::pair<Element, std::vector<Element>>>& grammerVec,
			std::unordered_map<Element, std::vector<u64>>& firstTable,
			ID closureID, Item item) {

			std::pair<Element, std::vector<Element>>& core = grammerVec[item.prodId];
			Element mark = core.second[item.dotPos];
			auto range = revTransitionMap[mark].equal_range(closureID);
			for (auto it = range.first; it != range.second; ++it) {
				std::vector<Item>& q = closureVec[it->second];
				u64 counter = 0;
				for (auto si = q.begin(); si != q.end(); ++si) {
					u64 itemId = counter++;
					if (grammerVec[si->prodId].second[si->dotPos] != core.first)
						continue;

					std::vector<Element>& rhs = grammerVec[si->prodId].second;
					if (rhs.size() > si->dotPos + 1) {
						Element follow = rhs[si->dotPos + 1];
						std::vector<u64>& first = firstTable[follow];
						if (first.size()) {
							lookahead.resize(std::max(lookahead.size(), (rhs[follow] >> 6) + 1), 0);
							lookahead[rhs[follow] >> 6] |= (u64)1 << (rhs[follow] & 0b111111);
							if (first[0] & 0b1)
								ref.push_back({ it->second, itemId });
							continue;
						}
					}
					if (!lookahead.size())
						lookahead.resize(1, 0);
					lookahead[0] |= 1;
					ref.push_back({ it->second, itemId });
				}
			}
		}

		inline void getLookahead(
			std::vector<std::vector<u64>>& lookaheadSets,
			std::vector<std::vector<Item>>& closureVec,
			std::unordered_map<u64, std::vector<u64>>& reduceItem,
			std::unordered_map<Element, std::unordered_multimap<ID, ID>>& revTransitionMap,
			std::vector<std::pair<Element, std::vector<Element>>>& grammerVec,
			std::unordered_multimap<Element, std::pair<std::vector<Element>, u64>>& grammerMap,
			std::unordered_map<Element, std::vector<u64>>& firstTable) {

			typedef struct _TableData {
				std::vector<std::pair<u64, u64>> ref;
				std::vector<u64> lookaheadSet;
			} TableData;

			std::unordered_map<u64, std::unordered_map<u64, TableData>> lookaheadTable;
			std::unordered_map<u64, std::unordered_map<u64, bool>> visited;
			std::unordered_map<u64, std::unordered_map<u64, u64>> orderMap;

			typedef struct _QData {
				u64 closureId, itemId;
			} QData;

			std::queue<QData> itemQ;

			for (auto it = reduceItem.begin(); it != reduceItem.end(); ++it) {
				for (auto si = it->second.begin(); si != it->second.end(); ++si)
					itemQ.push({ it->first, *si });
			}

			u64 count = 0;
			while (!itemQ.empty()) {
				QData cur = itemQ.front(); itemQ.pop();
				orderMap[cur.closureId][cur.itemId] = count++;

				if (visited[cur.closureId][cur.itemId])
					continue;
				visited[cur.closureId][cur.itemId] = true;
				
				std::vector<Item>& closure = closureVec[cur.closureId];
				lookaheadHelper(lookaheadTable[cur.closureId][cur.itemId].lookaheadSet, lookaheadTable[cur.closureId][cur.itemId].ref, closureVec, revTransitionMap, grammerVec, firstTable, cur.closureId, closure[cur.itemId]);

				std::vector<std::pair<u64, u64>>& ref = lookaheadTable[cur.closureId][cur.itemId].ref;
				for (auto it = ref.begin(); it != ref.end(); ++it)
					itemQ.push({ it->first, it->second });
			}

			std::map<u64, QData, std::greater<u64>> orderedMap;

			for (auto it = orderedMap.begin(); it != orderedMap.end(); ++it) {
				std::vector<u64>& curSet = lookaheadTable[it->second.closureId][it->second.itemId].lookaheadSet;

				std::vector<std::pair<u64, u64>>& ref = lookaheadTable[it->second.closureId][it->second.itemId].ref;
				for (auto si = ref.begin(); si != ref.end(); ++it) {
					std::vector<u64>& lookaheadSet = lookaheadTable[si->first][si->second].lookaheadSet;
					lookaheadSet.resize(std::max(lookaheadSet.size(), curSet.size()), 0);
					for (u64 i = 0; i < curSet.size(); i++)
						lookaheadSet[i] |= curSet[i];
				}
			}

			count = 0;
			for (auto it = lookaheadTable.begin(); it != lookaheadTable.end(); ++it) {
				for (auto si = it->second.begin(); si != it->second.end(); ++si) {
					lookaheadSets.push_back(si->second.lookaheadSet);
					closureVec[it->first][si->first].lookaheadId = count++;
				}
			}
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