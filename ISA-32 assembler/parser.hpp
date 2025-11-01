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

#define NTSTART(name) enum class name { __accept, 
#define NTEND() __end }

namespace parser_generator {
	namespace convert {
		using u64 = unsigned __int64;
		using ID = u64;
		using Element = u64;

		typedef struct _Item {
			u64 prodId;
			u64 dotPos;
			u64 lookaheadId;
		} Item;

		inline void getClosure(
			std::vector<ID>& reduceItems,
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
				if (grammerVec[it->prodId].second.size() == it->dotPos) {
					it->lookaheadId = 0;
					continue;
				}

				itemQ.push({ *it, *it });
			}

			while (!itemQ.empty()) {
				QData cur = itemQ.front(); itemQ.pop();

				Element mark = grammerVec[cur.item.prodId].second[cur.item.dotPos];
				Element lhs = grammerVec[cur.item.prodId].first;
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

			u64 counter = 0;
			for (auto it = items.begin(); it != items.end(); ++it) {
				if (visited.find(grammerVec[it->prodId].first) == visited.end() || it->dotPos) {
					closure.push_back(*it);
					if (!it->lookaheadId)
						reduceItems.push_back(counter);
				}
				counter++;
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
				while (li != left.cend() && ri != right.cend()) {
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
			std::unordered_map<ID, std::vector<ID>>& reduceItem,
			std::vector<std::vector<Item>>& closureVec,
			std::unordered_multimap<ID, std::pair<ID, Element>>& transitionMap,
			std::vector<std::pair<Element, std::vector<Element>>>& grammerVec,
			std::unordered_multimap<Element, std::pair<std::vector<Element>, u64>>& grammerMap,
			std::vector<Item> startItemVec) {

			typedef struct _QData {
				std::vector<Item> closure;
				ID id;
			} QData;

			ID closureId = 0;
			std::map<std::vector<Item>, ID, SetLessRule> visited;

			std::queue<QData> setQ;
			std::vector<Item> startClosure;
			getClosure(reduceItem[closureId], startClosure, grammerVec, grammerMap, startItemVec);
			setQ.push({ startClosure, closureId });
			closureVec.push_back(startClosure);
			visited[startClosure] = closureId++;
			
			std::map<u64, u64> dupeCount;

			while (!setQ.empty()) {
				QData cur = setQ.front(); setQ.pop();

				std::unordered_map<Element, std::vector<Item>> gotoMap;
				getGotoItem(gotoMap, grammerVec, cur.closure);
				for (auto it = gotoMap.begin(); it != gotoMap.end(); ++it) {
					std::vector<Item> closure;
					getClosure(reduceItem[closureId], closure, grammerVec, grammerMap, it->second);

					if (visited.find(closure) != visited.end()) {
						ID id = visited[closure];

						dupeCount[id]++;

						transitionMap.insert({ cur.id, { id, it->first} });
						continue;
					}
					ID id = closureId++;
					visited[closure] = id;
					closureVec.push_back(closure);
					transitionMap.insert({ cur.id, { id, it->first} });

					setQ.push({ closure, id });
				}
			}

			std::cout << "Duplicate States:" << std::endl;
			for (auto it = dupeCount.begin(); it != dupeCount.end(); ++it)
				std::cout << "  " << it->first << ": " << it->second << std::endl;
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
			if (item.dotPos <= 0)
				return;
			Element mark = core.second[item.dotPos - 1];
			auto range = revTransitionMap[mark].equal_range(closureID);
			for (auto it = range.first; it != range.second; ++it) {
				std::vector<Item>& q = closureVec[it->second];
				u64 counter = 0;
				for (auto si = q.begin(); si != q.end(); ++si) {
					u64 itemId = counter++;
					if (grammerVec[si->prodId].second.size() <= si->dotPos || grammerVec[si->prodId].second[si->dotPos] != core.first)
						continue;

					std::vector<Element>& rhs = grammerVec[si->prodId].second;
					if (rhs.size() > si->dotPos + 1) {
						Element follow = rhs[si->dotPos + 1];
						std::vector<u64>& first = firstTable[follow];
						if (first.size()) {
							lookahead.resize(std::max(lookahead.size(), (follow >> 6) + 1), 0);
							lookahead[follow >> 6] |= (u64)1 << (follow & 0b111111);
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
			std::unordered_map<ID, std::vector<ID>>& reduceItem,
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

		enum class ActionType {
			Shift,
			Reduce,
			Accept,
			Error
		};

		typedef struct _Action {
			ID args;
			ActionType type;
		} Action;

		typedef struct _Goto {
			ID state;
			bool error;
		} Goto;

		typedef struct _ParserTable {
			Action** actionTable;
			Goto** gotoTable;
			size_t size;
			size_t actionSize;
			size_t gotoSize;
		} ParserTable;

		void printTable(ParserTable&, int);

		void createTable(ParserTable& table, std::vector<std::pair<Element, std::vector<Element>>>& grammerVec, Element terminalEnd, Element nonterminalEnd, Element acceptNonterminal) {
			acceptNonterminal += terminalEnd;

			std::cout << "grammerVec:" << std::endl;
			for (auto it = grammerVec.begin(); it != grammerVec.end(); ++it) {
				static size_t k = 0;
				std::cout << "  " << k++ << ": " << it->first << " -> ";
				for (auto si = it->second.begin(); si != it->second.end(); ++si)
					std::cout << *si << ", ";
				std::cout << std::endl;
			}

			std::vector<Item> startItemVec;
			std::unordered_multimap<Element, Element> revFirstGrammmer;
			std::unordered_multimap<Element, std::pair<std::vector<Element>, u64>> grammerMap;
			u64 count = 0;
			for (auto it = grammerVec.begin(); it != grammerVec.end(); ++it) {
				grammerMap.insert({ it->first, { it->second, count } });
				if (it->second.size())
					revFirstGrammmer.insert({ it->second[0], it->first });
				if (it->first == acceptNonterminal)
					startItemVec.push_back({ count, 0, (u64)-1 });
				count++;
			}

			std::cout << "startItem:" << std::endl;
			for (auto it = startItemVec.begin(); it != startItemVec.end(); ++it)
				std::cout << "  prod: " << it->prodId << ", dot: " << it->dotPos << ", look: " << (int)it->lookaheadId << std::endl;
			std::cout << "revFirst:" << std::endl;
			for (auto it = revFirstGrammmer.begin(); it != revFirstGrammmer.end(); ++it)
				std::cout << "  " << it->first << " <- " << it->second << std::endl;
			std::cout << "grammerMap:" << std::endl;
			for (auto it = grammerMap.begin(); it != grammerMap.end(); ++it) {
				std::cout << "  " << it->first << " -> ";
				for (auto si = it->second.first.begin(); si != it->second.first.end(); ++si)
					std::cout << *si << ", ";
				std::cout << "(id: " << it->second.second << ")" << std::endl;
			}

			std::unordered_map<ID, std::vector<ID>> reduceItem;
			std::vector<std::vector<Item>> closureVec;
			std::unordered_multimap<ID, std::pair<ID, Element>> transitionMap;
			createC0(reduceItem, closureVec, transitionMap, grammerVec, grammerMap, startItemVec);

			std::cout << "reduceItem:" << std::endl;
			for (auto it = reduceItem.begin(); it != reduceItem.end(); ++it) {
				for (auto si = it->second.begin(); si != it->second.end(); ++si)
					std::cout << "  " << it->first << ", " << *si << std::endl;
			}
			std::cout << "closureVec:" << std::endl;
			for (size_t i = 0; i < closureVec.size(); i++) {
				std::cout << "  state " << i << ":" << std::endl;

				for (size_t j = 0; j < closureVec[i].size(); j++) {
					Item& item = closureVec[i][j];
					std::cout << "    prod: " << item.prodId << ", dot: " << item.dotPos << ", look: " << (int)item.lookaheadId << std::endl;
				}
			}
			std::cout << "transitionMap:" << std::endl;
			for (auto it = transitionMap.begin(); it != transitionMap.end(); ++it)
				std::cout << "  from: " << it->first << ", to: " << it->second.first << ", via: " << it->second.second << std::endl;

			std::unordered_map<Element, std::unordered_multimap<ID, ID>> revTransitionMap;
			for (auto it = transitionMap.begin(); it != transitionMap.end(); ++it)
				revTransitionMap[it->second.second].insert({ it->second.first, it->first });

			std::cout << "revTransitionMap:" << std::endl;
			for (auto it = revTransitionMap.begin(); it != revTransitionMap.end(); ++it) {
				std::cout << "  via: " << it->first << std::endl;
				for (auto si = it->second.begin(); si != it->second.end(); ++si)
					std::cout << "    to: " << si->first << ", from: " << si->second << std::endl;
			}

			std::unordered_map<Element, std::vector<u64>> firstTable;
			getFirstTable(firstTable, revFirstGrammmer, terminalEnd);

			std::cout << "firstTable:" << std::endl;
			for (auto it = firstTable.begin(); it != firstTable.end(); ++it) {
				std::cout << "  " << it->first << " : ";
				for (u64 i = 0; i < it->second.size(); i++) {
					u64 bit = it->second[i];
					for (u64 j = 0; j < 64 && bit; j++) {
						if (bit & 0b1)
							std::cout << (i * 64 + j) << ", ";
						bit >>= 1;
					}
				}
				std::cout << std::endl;
			}

			std::vector<std::vector<u64>> lookaheadSets;
			getLookahead(lookaheadSets, closureVec, reduceItem, revTransitionMap, grammerVec, grammerMap, firstTable);

			std::cout << "closureVec:" << std::endl;
			for (size_t i = 0; i < closureVec.size(); i++) {
				std::cout << "  state " << i << ":" << std::endl;

				for (size_t j = 0; j < closureVec[i].size(); j++) {
					Item& item = closureVec[i][j];
					std::cout << "    prod: " << item.prodId << ", dot: " << item.dotPos << ", look: " << (int)item.lookaheadId << std::endl;
				}
			}

			std::cout << "lookaheadSets:" << std::endl;
			for (auto it = lookaheadSets.begin(); it != lookaheadSets.end(); ++it) {
				static size_t k = 0;
				std::cout << "  set" << k++ << ": ";
				for (u64 i = 0; i < it->size(); i++) {
					u64 bit = (*it)[i];
					for (u64 j = 0; j < 64 && bit; j++) {
						if (bit & 0b1)
							std::cout << (i * 64 + j) << ", ";
						bit >>= 1;
					}
				}
				std::cout << std::endl;
			}

			size_t stateSize = closureVec.size(), actionSize = terminalEnd, gotoSize = nonterminalEnd;
			table.actionTable = new Action*[stateSize];
			table.gotoTable = new Goto*[stateSize];
			table.size = stateSize;
			table.actionSize = actionSize;
			table.gotoSize = gotoSize;

			for (size_t i = 0; i < stateSize; i++) {
				table.actionTable[i] = new Action[actionSize];
				for (size_t j = 0; j < actionSize; j++)
					table.actionTable[i][j] = { 0, ActionType::Error};

				table.gotoTable[i] = new Goto[gotoSize];
				for (size_t j = 0; j < gotoSize; j++)
					table.gotoTable[i][j] = { 0, true };
			}

			for (auto it = transitionMap.begin(); it != transitionMap.end(); ++it) {
				if (it->second.second < terminalEnd)
					table.actionTable[it->first][it->second.second] = { it->second.first, ActionType::Shift };
				else
					table.gotoTable[it->first][it->second.second - terminalEnd] = { it->second.first, false };
			}

			for (auto it = reduceItem.begin(); it != reduceItem.end(); ++it) {
				for (auto si = it->second.begin(); si != it->second.end(); ++si) {
					Item& item = closureVec[it->first][*si];
					ActionType type = ActionType::Reduce;
					if (grammerVec[item.prodId].first == acceptNonterminal)
						type = ActionType::Accept;

					std::vector<u64> lookahead = lookaheadSets[item.lookaheadId];
					for (u64 i = 0; i < lookahead.size(); i++) {
						u64 bit = lookahead[i];
						for (u64 j = 0; j < 64 && bit; j++) {
							if (bit & 0b1)
								table.actionTable[it->first][i * 64 + j] = { item.prodId, type };
							bit >>= 1;
						}
					}
				}
			}

			printTable(table, 3);
		}

		std::string int2str(int num, int len) {
			std::string out;
			std::vector<char> buff;
			for (int i = 0; i < len; i++) {
				buff.push_back((num % 10) + '0');
				num /= 10;
			}
			for (auto it = buff.rbegin(); it != buff.rend(); ++it)
				out.push_back(*it);

			return out;
		}

		std::string space(int len) {
			std::string out;
			for (int i = 0; i < len; i++)
				out.push_back(' ');
			return out;
		}

		void printTable(ParserTable& table, int len) {
			std::cout << space(len + 4);
			for (size_t i = 0; i < table.actionSize; i++) {
				std::cout << "A" << int2str(i, len) << " ";
			}
			std::cout << "| ";
			for (size_t i = 0; i < table.gotoSize; i++) {
				std::cout << "G" << int2str(i, len) << " ";
			}
			std::cout << std::endl;

			for (size_t i = 0; i < table.size; i++) {
				std::cout << "ST" << int2str(i, len) << ": ";

				for (size_t j = 0; j < table.actionSize; j++) {
					Action& action = table.actionTable[i][j];
					switch (action.type) {
					case ActionType::Shift:
						std::cout << "S" << int2str(action.args, len) << " ";
						break;
					case ActionType::Reduce:
						std::cout << "R" << int2str(action.args, len) << " ";
						break;
					case ActionType::Accept:
						std::cout << "A" << int2str(action.args, len) << " ";
						break;
					case ActionType::Error:
						std::cout << "E" << int2str(action.args, len) << " ";
						break;
					}
				}

				std::cout << "| ";

				for (size_t j = 0; j < table.gotoSize; j++) {
					Goto& go = table.gotoTable[i][j];
					if (go.error)
						std::cout << "E" << int2str(go.state, len) << " ";
					else
						std::cout << "G" << int2str(go.state, len) << " ";
				}

				std::cout << std::endl;
			}
		}
	}

	template <typename Terminal, typename NonTerminal>
	class ParserFactory;

	template <typename Terminal, typename NonTerminal>
	class Parser {
	private:
		friend ParserFactory<Terminal, NonTerminal>;

		using u64 = unsigned __int64;
		using ID = convert::ID;
		using Element = convert::Element;
		using Table = convert::ParserTable;

		std::vector<std::pair<u64, std::vector<u64>>> grammerVec;
		Table table;

	public:
		bool parse(std::vector<ID>& parseList, std::vector<Terminal>& inputs) {
			inputs.push_back(Terminal::__eot);

			typedef struct _StackData {
				Element symbol;
				ID state;
			} StackData;

			std::stack<StackData> stack;
			stack.push({ (Element)0 , 0 });

			for (auto it = inputs.begin(); it != inputs.end(); ++it) {
				ID state = stack.top().state;

				using AT = convert::ActionType;

				auto action = this->table.actionTable[state][(Element)(*it)];
				switch (action.type) {
					case AT::Shift:
						stack.push({ (Element)(*it), action.args });
						break;
					case AT::Reduce: {
						parseList.push_back(action.args);
						auto grammer = this->grammerVec[action.args];
						for (size_t i = 0; i < grammer.second.size(); i++)
							stack.pop();
						auto goData = this->table.gotoTable[grammer.first];
						if (goData->error)
							return false;
						stack.push({ (Element)grammer.first, goData->state });
					}
						break;
					case AT::Accept:
						return true;
					case AT::Error:
						return false;
				}
			}

			return true;
		}
	};

	template <typename Terminal, typename NonTerminal>
	class ParserFactory {
	public:
		class ElementWrapper {
		private:
			bool isTerminal;
			union {
				Terminal terminal;
				NonTerminal nonTerminal;
			};

		public:
			ElementWrapper(void) : isTerminal(true), terminal(Terminal(0)) {}
			ElementWrapper(Terminal t) : isTerminal(true), terminal(t) {}
			ElementWrapper(NonTerminal nt) : isTerminal(false), nonTerminal(nt) {}

			const ElementWrapper& operator = (const ElementWrapper& other) {
				this->isTerminal = other.isTerminal;
				if (this->isTerminal)
					this->terminal = other.terminal;
				else
					this->nonTerminal = other.nonTerminal;
				return *this;
			}

			convert::Element get(void) const {
				if (this->isTerminal)
					return (convert::Element)this->terminal;
				return (convert::Element)this->nonTerminal + (convert::Element)Terminal::__end;
			}
		};

		using CreateData = std::vector<std::pair<ElementWrapper, std::vector<ElementWrapper>>>;

	private:
		using u64 = unsigned __int64;
		using ID = convert::ID;
		using Element = convert::Element;
		using Table = convert::ParserTable;

		std::vector<std::pair<Element, std::vector<Element>>> grammerVec;
		Table table;

		void clearTable(void) {
			if (this->table.actionTable != NULL) {
				for (size_t i = 0; i < this->table.size; i++)
					delete[] this->table.actionTable[i];
				delete[] this->table.actionTable;
			}
			if (this->table.gotoTable!= NULL) {
				for (size_t i = 0; i < this->table.size; i++)
					delete[] this->table.gotoTable[i];
				delete[] this->table.gotoTable;
			}
			this->table.size = 0;
			this->table.actionTable= NULL;
			this->table.gotoTable = NULL;
		}

	public:
		ParserFactory(void) {
			this->grammerVec.clear();
			this->table.actionTable = NULL;
			this->table.gotoTable = NULL;
			this->table.size = 0;
		}

		~ParserFactory() {
			this->grammerVec.clear();
			this->clearTable();
		}

		void setRules(const CreateData& data) {
			for (auto it = data.cbegin(); it != data.cend(); ++it) {
				std::vector<u64> rhs;
				for (auto si = it->second.cbegin(); si != it->second.cend(); ++si)
					rhs.push_back(si->get());
				grammerVec.push_back({ it->first.get(), rhs });
			}
		}

		void update(void) {
			convert::createTable(this->table, this->grammerVec, (Element)Terminal::__end, (Element)NonTerminal::__end, (Element)NonTerminal::__accept);
		}

		Parser<Terminal, NonTerminal> create(void) {
			Parser<Terminal, NonTerminal> parser;
			parser.table = this->table;
			parser.grammerVec = this->grammerVec;
			return parser;
		}
	};
}

#endif