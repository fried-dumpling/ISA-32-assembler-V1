#ifndef DPICP_EVALUATOR
#define DPICP_EVALUATOR

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

#include "lexer.hpp"
#include "parser.hpp"

namespace evaluator {
	namespace convert {

	}

	template<typename>
	class EvaluatorFactory;

	template<typename>
	class Evaluator {
	private:
		EvaluatorFactory<>

	public:
		void evaluate(AST) {

		}
	};

	template<typename>
	class EvaluatorFactory {
	public:

		using CreateData = ;

	private:


	public:
		EvaluatorFactory() {

		}

		~EvaluatorFactory() {

		}

		void setRules(const CreateData& createData) {

		}

		void update(void) {

		}

		Evaluator<> create(void) {

		}
	};
}

#endif