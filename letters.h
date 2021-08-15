#include <vector>
#include <random>
#include "json.h"

using json = nlohmann::json;

class letter {
public:
	std::vector<std::vector<int>> data;
};

class letter_selector {
public:
	void populate(std::vector<unsigned char> &data, int count, float color[4]){
		// load the model into the list of letters
		json j;
		std::vector<letter> letters;

		// begin to pick letters





	}
};
