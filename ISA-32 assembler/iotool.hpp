#ifndef DPICP_IOTOOL_HPP
#define DPICP_IOTOOL_HPP

#include <iostream>
#include <fstream>

#include <string>
#include <vector>
#include <stack>
#include <queue>
#include <map>
#include <set>

namespace iotools {
	using u8 = unsigned __int8;
	using u32 = unsigned __int32;

	bool readTextFile(const std::string& filename, std::string& outText) {
		std::fstream fin;
		fin.open(filename, std::ios::in);
		if (!fin.is_open() || !fin.good())
			return false;

		char c;
		while (fin.get(c)) {
			outText.push_back(c);
		}
		fin.close();
		return true;
	}

	bool writeBinaryFile(const std::string& filename, const std::vector<u8>& data, int rep) {
		std::fstream fout;
		fout.open(filename, std::ios::out | std::ios::binary);
		if (!fout.is_open() || !fout.good())
			return false;

		for (auto it = data.cbegin(); it != data.cend(); ++it) {
			char tmp = *it;
			fout.write(&tmp, sizeof(char));
		}

		fout.close();
		return true;
	}
}

#endif