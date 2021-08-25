#include "letters.h"

class ssGenerator {
  std::vector<letter> glyphs;

  void populate() { // get the glyphs into the glyphs array
    std::ifstream i("resources/hoard-of-bitfonts/optimized.json");
    json j; i >> j;
		for (auto& element : j) { // per character
			letter temp; int y = 0;
			temp.data.resize(element.size());
			for(auto& row : element) {
				temp.buildrow(y, row);
				y++;
			}
			if(!temp.nfg) letters.push_back(temp);
		}
  }

};
