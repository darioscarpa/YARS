#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <string>
#include <algorithm>

class StringUtils {
public:
	static std::string strReplace(std::string subject, const std::string& search, const std::string& replace) {
		size_t pos = 0;
		while ((pos = subject.find(search, pos)) != std::string::npos) {
			 subject.replace(pos, search.length(), replace);
			 pos += replace.length();
		}
		return subject;
	}
};

#endif