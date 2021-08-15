#include <vector>
#include <random>
#include <fstream>
#include <iostream>
#include "json.h"

using json = nlohmann::json;

class letter {
public:
	std::vector<std::vector<int>> data;
};

class letter_selector {
public:
	void write(int x, int y, int z, float color[4]; std::vector<unsigned char> &data, int dim) {
		// this function has to do bounds checking
		if(x < 0 || x > DIM || y < 0 || y > DIM || z < 0 || z > DIM) return;
		int index = x + y*dim + z*dim*dim;
		data[index+0]=color[0]*255;
		data[index+1]=color[1]*255;
		data[index+2]=color[2]*255;
		data[index+3]=color[3]*255;
	}
	void populate(std::vector<unsigned char> &data, int dim, int count, float color[4]){
		// load the model into the list of letters
		std::vector<letter> letters;
		std::ifstream i("monolithic_model.json");
		json j; i >> j;

		for (auto& element : j) { // per font
		  std::cout << element["label"] << '\n';
		}



		// begin to pick letters

		// write(x,y,z,color,data,dim);





	}
};
