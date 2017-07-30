#include "LogParser.h"

#include <iostream>

int main(int argc, char** argv) {
	std::cout << "#######################################\n";
	std::cout << "#                                     #\n";
	std::cout << "#   GMLogParser [int3/Smertig] 2017   #\n";
	std::cout << "#                                     #\n";
	std::cout << "#######################################\n";
	std::cout << "\n\n";

	if (argc != 2) {
		std::cerr << "Usage: " << argv[0] << " %log_file%";
		return 1;
	}

	try {
		LogParser::Parser parser;

		const std::string path = argv[1];
		parser.Parse(path);
		parser.Convert(path + ".txt");
		std::cout << "\nSuccessfully parsed and converted" << std::endl;
	}
	catch (std::exception& e) {
		std::cerr << "\nexception: " << e.what() << std::endl;
	}	
	return 0;
}