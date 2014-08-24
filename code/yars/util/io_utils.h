#ifndef IO_UTILS_H
#define IO_UTILS_H

#include <ios>      
#include <iostream>
#include <istream>
#include <limits>   

class IoUtils {
public:
	static void blockUntilNewline() {
		std::cout << "Press Enter to continue...\n";
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		std::cin.clear();		
	}
};

#endif
