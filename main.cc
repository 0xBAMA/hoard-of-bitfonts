#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>

// Goal here is to construct a monolitic JSON file out of this data, which will
// format the data like this:
// 	Total # of fonts
// 	Font 1
// 		Font 1 Label (filename)
// 		Font 1 # glyphs
// 		Glyph 1
// 			Glyph 1 Label
// 			Glyph x dim
// 			Glyph y dim
// 			Glyph bitmap data
// 		Glyph 2
// 			...
// 	Font 2
// 		...

// This is to facilitate the selection of random character glyphs, for a matrix
// glyph waterfall type effect, inside a volume dataset. By writing stripes of
// these at different orientations through the volume, you could get some pretty
// cryptic looking stuff pretty quickly

#include "json.h"
using json = nlohmann::json;
json j; // global json model

int num_fonts = 0;
int global_glyphcount = 0;

std::string get_label() {
	std::stringstream ss;
	ss << "font" << num_fonts;
	return ss.str();
}

std::string load_path(std::filesystem::path p) {
	std::ifstream ifs(p);
	return std::string(std::istreambuf_iterator<char>(ifs),std::istreambuf_iterator<char>());
}

std::size_t replace_all(std::string& inout, std::string_view what, std::string_view with){
    std::size_t count{};
    for (std::string::size_type pos{};
         inout.npos != (pos = inout.find(what.data(), pos, what.length()));
         pos += with.length(), ++count) {
        inout.replace(pos, what.length(), with.data(), with.length());
    }
    return count;
}

void load_yaff(std::filesystem::path p) {
	int glyph_count = 0;
	std::stringstream s(load_path(p));

	p.replace_extension(""); // strip the extension off
	j[get_label()]["label"] = std::string(p.c_str()+2); // get rid of './'
	j[get_label()]["format"] = std::string("yaff"); // do I care about this?

// load all the glyphs, with labels as possible
	// glyph needs to know:
		// label
		// x dimension, y dimension
		// the data, an int array of size x*y

	// the YAFF format starts with a header - skip it, till the beginning of the first comment
	std::string temp; // temporary storage
	for(;;){
		temp.clear();
		if(std::getline(s, temp, '\n')){
			if(temp[0] == '#') // start of first comment, continue
				break;
		}else{
			break;
		}
	}

	for(;;) {
		// first comment is first label
		std::string index = "glyph"+std::to_string(glyph_count)+"label";
		std::string data = "glyph"+std::to_string(glyph_count)+"data";
		j[get_label()][index] = temp;
		temp.clear();
		for(;;) {
			std::getline(s, temp, '\n');
			if(temp[0]=='u') continue; // these two characters indicate a line which contains a value I'm not interested in
			if(temp[0]=='0') continue;
			break; // fall out with the first row of the data in hand
		}

		// we are now inside the data for a character
		std::vector<std::string> hold;
		for(;;) {
			// remove whitespace
			temp.erase(remove(temp.begin(),temp.end(),' '),temp.end());
			temp.erase(remove(temp.begin(),temp.end(),'\t'),temp.end());
			temp.erase(remove(temp.begin(),temp.end(),'\n'),temp.end());
			// is it empty?
			if(temp.length()==0) break; // line had no data
			else
				hold.push_back(temp);

			std::getline(s, temp, '\n');
		}

		j[get_label()][data]=hold;
		temp.clear();
		while(s.rdbuf()->in_avail()>0) { // while there is file left
			std::getline(s, temp, '\n');
			temp.erase(remove(temp.begin(),temp.end(),' '),temp.end()); // remove whitespace
			temp.erase(remove(temp.begin(),temp.end(),'\t'),temp.end());
			temp.erase(remove(temp.begin(),temp.end(),'\n'),temp.end());
			if(temp.empty()) continue;
			break;
		}

		glyph_count++;
		if(s.rdbuf()->in_avail()==0) break;
	}

	j[get_label()]["num_glyphs"] = glyph_count;
	global_glyphcount += glyph_count;
	num_fonts++;
}








void load_draw(std::filesystem::path p) {
	// std::cout << p << std::endl;
	int glyph_count = 0;
	std::stringstream s(load_path(p));

	p.replace_extension("");
	j[get_label()]["label"] = std::string(p.c_str()+2);
	j[get_label()]["format"] = std::string("draw");

	// get rid of leading comments
	std::string garbage;
	while(s.peek()=='#') std::getline(s,garbage,'\n');

	// .draw fonts are actually much simpler to parse
	while(!s.rdbuf()->in_avail()==0) { // while there's file left
		std::string label, temp;
		std::string index = "glyph"+std::to_string(glyph_count)+"label";
		std::string data = "glyph"+std::to_string(glyph_count)+"data";

		s >> label; // std::cout << label << std::endl;
		std::getline(s, temp, '\n'); // read in the first of the new data
		temp.erase(remove(temp.begin(),temp.end(),' '),temp.end());
		temp.erase(remove(temp.begin(),temp.end(),'\t'),temp.end());
		temp.erase(remove(temp.begin(),temp.end(),'\n'),temp.end());
		if(temp.empty())
			std::getline(s, temp, '\n');
		std::vector<std::string> hold;

		for(;;) {
			temp.erase(remove(temp.begin(),temp.end(),' '),temp.end());
			temp.erase(remove(temp.begin(),temp.end(),'\t'),temp.end());
			temp.erase(remove(temp.begin(),temp.end(),'\n'),temp.end());

			if(temp.empty()) // was this a blank line or a comment?
				break;

			hold.push_back(temp); // keep lines
			std::getline(s, temp, '\n');
		}

		// put the data in the list
		j[get_label()][index] = label;
		j[get_label()][data] = hold;
		glyph_count++;
	}

	j[get_label()]["num_glyphs"] = glyph_count;
	global_glyphcount += glyph_count;
	num_fonts++;
}










int main(int argc, char const *argv[]) {
	std::filesystem::path working_dir{"."};
	for(auto const& dir_entry: std::filesystem::recursive_directory_iterator{working_dir}) {

		// load a YAFF font
		if(dir_entry.path().extension() == ".yaff")
			load_yaff(dir_entry);

		// load a DRAW font
		if(dir_entry.path().extension() == ".draw")
			load_draw(dir_entry);
	}

	// output the constructed JSON model
	j["count"] = num_fonts;
	std::cout << std::endl << j.dump(2) << std::endl << std::endl;

	std::ofstream o("monolithic_model.json");
	o << std::setw(4) << j << std::endl;

	std::cout << " > Total character count: " << global_glyphcount << " across " << num_fonts << " fonts." << std::endl;

	return 0;
}
