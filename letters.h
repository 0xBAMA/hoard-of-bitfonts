#ifndef LETTER
#define LETTER

#include <vector>
#include <random>
#include <fstream>
#include <iostream>

#include "json.h"
using json = nlohmann::json;


class letter {
public:
	bool flaggedForRemoval = false;
	std::vector<std::vector<unsigned char>> data;
	void buildrow(int r, std::string s) {
		std::vector<unsigned char> ints;
		for(unsigned int i = 0; i < s.size(); i++) {
			if(s[i] == '.'){
				ints.push_back(0);
			}else if(s[i] == '@'){
				ints.push_back(1);
			}else{
				nfg = true;
				break;
			}
		}
		data[r]=ints;
	}
	bool nfg=false;

	void print() {
		for(auto row : data) {
			for(auto elem : row) {
				std::cout << (int)elem;
			}
			std::cout << std::endl;
		}
		std::cout << std::endl;
	}


	bool check() {
		unsigned int firstrow = data.empty() ? 0 : data[0].size();
		if(!firstrow) return false;
		for(unsigned int i = 0; i < data.size(); i++)
			if(data[i].size() != firstrow)
				return false;
		return true;
	}

	friend bool operator== (const letter& l1, const letter& l2){
		if(l1.data.size()!=l2.data.size()) return false; // y dim doesn'nt match
		if(l1.data[0].size()!=l2.data[0].size()) return false; // x dim doesn't match
		for(unsigned int i = 0; i < l1.data.size(); i++)
		for(unsigned int j = 0; j < l1.data[0].size(); j++)
			if(l1.data[i][j]!=l2.data[i][j])
				return false;
		return true;
	}
};

class letter_selector {
public:
	void write(int x, int y, int z, unsigned char color[4], std::vector<unsigned char> &data, int dim) {

		// this function has to do bounds checking
		if(x < 0 || x >= dim || y < 0 || y >= dim || z < 0 || z >= dim) return;

		int index = 4 * (x + y*dim + z*dim*dim);
		data[index+0]=color[0];
		data[index+1]=color[1];
		data[index+2]=color[2];
		data[index+3]=color[3];
	}

	void stamp(letter l, int x, int y, int z, int dir, int scale, unsigned char color[4], std::vector<unsigned char> &data, int dim) {
		// use the write function to write at all voxels where the currently selected letter has 1's

		int xdim = l.data.size();
		int ydim = l.data[1].size();

		std::cout << "stamping at " << x << " " << y << " with dimensions " << xdim << " " << ydim << std::endl;
		for(int xx = 0; xx < xdim; xx++)
		for(int yy = 0; yy < ydim; yy++)
		for(int xs = 0; xs < scale; xs++)
		for(int ys = 0; ys < scale; ys++)
		for(int zs = 0; zs < scale; zs++)
			switch(dir) {
				case 1:
					if(l.data[xx][yy]==1)
						write(x+xx*scale+xs, y+yy*scale+ys, z+zs,color,data,dim);
					break;

				case 2:
					if(l.data[xx][yy]==1)
						write(x-xx+xs, y+yy*scale+ys, z+zs,color,data,dim);
					break;

				case 3:
					if(l.data[xx][yy]==1)
						write(x+xx*scale+xs, y+yy*scale+ys, z+zs,color,data,dim);
					break;

				case 4:
					if(l.data[xx][yy]==1)
						write(x+xx*scale+xs, y-yy+ys, z+zs,color,data,dim);
					break;

				case 5:
					if(l.data[xx][yy]==1)
						write(x+xs, y+yy*scale+ys, z+xx*scale+zs,color,data,dim);
					break;

				case 6:
					if(l.data[xx][yy]==1)
						write(x+xs, y+yy*scale+ys, z-xx+zs,color,data,dim);
					break;

				default:
					break;
			}
	}

	std::vector<letter> letters;
	void populate(std::vector<unsigned char> &data, int dim, int count, int num_dirs, float color[4]){
		// load the model into the list of letters
		std::ifstream ii("./optimized.json");
		// std::ifstream i("monolithic_model.json");
		json jj; ii >> jj;


		// get the new font, add it to the letters list
		json j;
		// std::ifstream i("./trs-80/trs80-coco3-rom.draw");
		std::ifstream i("./dingo.json");
		i >> j;
		for (auto& element : j) { // per font
			// std::cout << element["label"] << '\n';
			for(unsigned int i = 0; i < element["num_glyphs"]; i++) { // per glyph in the font
				letter temp;
				int y=0;
				temp.data.resize(element["glyph"+std::to_string(i)+"data"].size());
				for(auto& row : element["glyph"+std::to_string(i)+"data"]) { // per row in the data
					temp.buildrow(y, row);
					y++;
				}
				if(!temp.nfg) letters.push_back(temp);
				// else std::cout << element["label"] << " " << i << " bad data " << endl; // this needs to be fixed in the actual data at some point
			}
		}


		// get all the other glyphs in the optimized list
		for (auto& element : jj) { // per character
			letter temp; int y = 0;
			temp.data.resize(element.size());
			for(auto& row : element) {
				temp.buildrow(y, row);
				y++;
			}
			if(!temp.nfg) letters.push_back(temp);
		}
		std::cout << "loaded " << letters.size() << " letters from " << jj.size() << " fonts." << std::endl;

		// remove duplicates
		// for( int i = 0; i < letters.size(); i++ ) {
		// 	for( int j = i + 1; j < letters.size(); j++ ) {
		// 		if( letters[ i ] == letters[ j ] ) {
		// 			letters.erase( letters.begin() + j );
		// 			letters[ j ].print();
		// 			std::cout << " removed" << std::endl;
		// 			j--;
		// 		}
		// 	}
		// }

		// std::cout << "letters remaining: " << letters.size() << std::endl;



	json jjj; int index = 0;
	for(auto lett : letters) {
		std::vector< std::string > rows;
		rows.resize( lett.data.size() );
		for(unsigned int i = 0; i < lett.data.size(); i++)
			for(unsigned int j = 0; j < lett.data[ 0 ].size(); j++)
				rows[ i ].append( 1, lett.data[ i ][ j ] ? '@' : '.' ); // back to the expected format
		jjj[ std::to_string( index ) ] = rows;
		index++;
	}

		// dump new optimized file
		std::ofstream o("./optimized.json");
		o << jjj.dump(1);

























		// if(data.size()==0) return; //
		//
		// std::mt19937_64 gen;
		// std::random_device r;
		// std::seed_seq s{r(), r(), r(), r(), r(), r(), r(), r(), r()};
		// gen = std::mt19937_64(s);
		//
		// std::uniform_int_distribution<int> colo(-15, 15);
		// std::uniform_int_distribution<int> pick(0, letters.size());
		// std::uniform_int_distribution<int> loc(0, dim);
		// std::uniform_int_distribution<int> dir(1, (num_dirs%6)+1);
		// std::uniform_int_distribution<int> sca(1, (num_dirs/6)+1);
		//
		// unsigned char col[4];
		//
		// // begin to pick letters
		// for(int i = 0; i < count; i++) {
		//
		// 	// randomize position and color (a small amount with the <random> distributions)
		// 	for(int j = 0; j < 4; j++)
		// 		col[j] = (color[j]*255)+colo(gen);
		//
		// 	stamp(letters[pick(gen)], loc(gen), loc(gen), loc(gen), dir(gen), sca(gen), col, data, dim);
		// }
	}
};

#endif
