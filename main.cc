#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>

#include "letters.h"

// image output
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"


#define STB_RECT_PACK_IMPLEMENTATION
#include "stb_rect_pack.h"

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

// int num_fonts = 0;
// int global_glyphcount = 0;
//
// std::string get_label() {
// 	std::stringstream ss;
// 	ss << "font" << num_fonts;
// 	return ss.str();
// }
//
// std::string load_path(std::filesystem::path p) {
// 	std::ifstream ifs(p);
// 	return std::string(std::istreambuf_iterator<char>(ifs),std::istreambuf_iterator<char>());
// }
//
// std::size_t replace_all(std::string& inout, std::string_view what, std::string_view with){
//     std::size_t count{};
//     for (std::string::size_type pos{};
//          inout.npos != (pos = inout.find(what.data(), pos, what.length()));
//          pos += with.length(), ++count) {
//         inout.replace(pos, what.length(), with.data(), with.length());
//     }
//     return count;
// }

// void load_yaff(std::filesystem::path p) {
// 	int glyph_count = 0;
// 	std::stringstream s(load_path(p));
//
// 	p.replace_extension(""); // strip the extension off
//
// // load all the glyphs, with labels as possible
// 	// glyph needs to know:
// 		// label
// 		// the data, an array of characters
//
// 	// the YAFF format starts with a header - skip it, till the beginning of the first comment
// 	std::string temp; // temporary storage
// 	for(;;){
// 		temp.clear();
// 		if(std::getline(s, temp, '\n')){
// 			if(temp[0] == '#') // start of first comment, continue
// 				break;
// 		}else{
// 			break;
// 		}
// 	}
//
// 	for(;;) {
// 		// first comment is first label
// 		std::string index = "glyph"+std::to_string(glyph_count)+"label";
// 		std::string data = "glyph"+std::to_string(glyph_count)+"data";
// 		j[get_label()][index] = temp;
// 		temp.clear();
// 		for(;;) {
// 			std::getline(s, temp, '\n');
// 			if(temp[0]=='u') continue; // these two characters indicate a line which contains a value I'm not interested in
// 			if(temp[0]=='0') continue;
// 			break; // fall out with the first row of the data in hand
// 		}
//
// 		// we are now inside the data for a character
// 		std::vector<std::string> hold;
// 		for(;;) {
// 			// remove whitespace
// 			temp.erase(remove(temp.begin(),temp.end(),' '),temp.end());
// 			temp.erase(remove(temp.begin(),temp.end(),'\t'),temp.end());
// 			temp.erase(remove(temp.begin(),temp.end(),'\n'),temp.end());
// 			// is it empty?
// 			if(temp.length()==0) break; // line had no data
// 			else
// 				hold.push_back(temp);
//
// 			std::getline(s, temp, '\n');
// 		}
//
// 		j[get_label()][data]=hold;
// 		temp.clear();
// 		while(s.rdbuf()->in_avail()>0) { // while there is file left
// 			std::getline(s, temp, '\n');
// 			temp.erase(remove(temp.begin(),temp.end(),' '),temp.end()); // remove whitespace
// 			temp.erase(remove(temp.begin(),temp.end(),'\t'),temp.end());
// 			temp.erase(remove(temp.begin(),temp.end(),'\n'),temp.end());
// 			if(temp.empty()) continue;
// 			break;
// 		}
//
// 		glyph_count++;
// 		if(s.rdbuf()->in_avail()==0) break;
// 	}
//
// 	j[get_label()]["label"] = std::string(p.c_str()+2); // get rid of './'
// 	j[get_label()]["format"] = std::string("yaff"); // do I care about this?
// 	j[get_label()]["num_glyphs"] = glyph_count;
// 	global_glyphcount += glyph_count;
// 	num_fonts++;
// }
//
//
//
//
//
//
//
//
// void load_draw(std::filesystem::path p) {
// 	// std::cout << p << std::endl;
// 	int glyph_count = 0;
// 	std::stringstream s(load_path(p));
//
// 	p.replace_extension("");
//
// 	// get rid of leading comments
// 	std::string garbage;
// 	while(s.peek()=='#') std::getline(s,garbage,'\n');
//
// 	// .draw fonts are actually much simpler to parse
// 	while(!s.rdbuf()->in_avail()==0) { // while there's file left
// 		std::string label, temp;
// 		std::string index = "glyph"+std::to_string(glyph_count)+"label";
// 		std::string data = "glyph"+std::to_string(glyph_count)+"data";
//
// 		s >> label; // std::cout << label << std::endl;
// 		std::getline(s, temp, '\n'); // read in the first of the new data
// 		temp.erase(remove(temp.begin(),temp.end(),' '),temp.end());
// 		temp.erase(remove(temp.begin(),temp.end(),'\t'),temp.end());
// 		temp.erase(remove(temp.begin(),temp.end(),'\n'),temp.end());
// 		if(temp.empty())
// 			std::getline(s, temp, '\n');
// 		std::vector<std::string> hold;
//
// 		for(;;) {
// 			temp.erase(remove(temp.begin(),temp.end(),' '),temp.end());
// 			temp.erase(remove(temp.begin(),temp.end(),'\t'),temp.end());
// 			temp.erase(remove(temp.begin(),temp.end(),'\n'),temp.end());
//
// 			if(temp.empty()) // was this a blank line or a comment?
// 				break;
//
// 			replace_all(temp, "-", ".");
// 			replace_all(temp, "#", "@");
//
// 			hold.push_back(temp); // keep lines
// 			std::getline(s, temp, '\n');
// 		}
//
// 		// put the data in the list
// 		j[get_label()]["label"] = std::string(p.c_str()+2);
// 		j[get_label()]["format"] = std::string("draw");
// 		j[get_label()][index] = label;
// 		j[get_label()][data] = hold;
// 		glyph_count++;
// 	}
//
// 	j[get_label()]["num_glyphs"] = glyph_count;
// 	global_glyphcount += glyph_count;
// 	num_fonts++;
// }










int main(int argc, char const *argv[]) {
	// std::filesystem::path working_dir{"."};
	// for(auto const& dir_entry: std::filesystem::recursive_directory_iterator{working_dir}) {
	//
	// 	// load a YAFF font
	// 	if(dir_entry.path().extension() == ".yaff")
	// 		load_yaff(dir_entry);
	//
	// 	// load a DRAW font
	// 	if(dir_entry.path().extension() == ".draw")
	// 		load_draw(dir_entry);
	// }
	//
	// // output the constructed JSON model
	// j["count"] = num_fonts;
	// std::cout << std::endl << j.dump(2) << std::endl << std::endl;
	//
	// std::ofstream o("monolithic_model.json");
	// o << std::setw(4) << j << std::endl;
	//
	// std::cout << " > Total character count: " << global_glyphcount << " across " << num_fonts << " fonts." << std::endl;
	//
	// return 0;




// - ----------------------------------
	// optimizing letter list, so it doesn't seg fault

// 	letter_selector l;
// 	std::vector<unsigned char> loaded_bytes;
// 	std::vector<letter> unsorted_letters;
//
//
// 	l.populate(loaded_bytes, 0, 0, nullptr); // no block, just letters
//
// int num = 0;
// 	for(int i = 0; i < l.letters.size(); i++) {
// 		if(!l.letters[i].check()) {
// 			l.letters.erase(l.letters.begin()+i);
// 			i--;
// 			num++;
// 		}
// 		std::cout << float(i)/float(l.letters.size()) << "\r";
// 	}
//
// 	std::cout << std::endl << num << " bad letters removed" << std::endl;
//
// 	// remove duplicates
// 	num = 0;
// 	for(int i = 0; i < l.letters.size(); i++)
// 	for(int j = i+1; j < l.letters.size(); j++)
// 		if(l.letters[i]==l.letters[j]) {
// 			l.letters.erase(l.letters.begin()+j);
// 			j--;
// 			num++;
// 			std::cout << float(i)/float(l.letters.size()) << "\r";
// 		}
//
// 	std::cout << num << " duplicates found" << std::endl;
//
// 	std::cout << l.letters.size() << " letters remaining" << std::endl;
//
// 	json j; int i = 0;
// 	for(auto lett : l.letters) {
// 		std::vector<std::string> rows;
// 		rows.resize(lett.data.size());
// 		for(int i = 0; i < lett.data.size(); i++)
// 			for(int j = 0; j < lett.data[0].size(); j++)
// 				rows[i].append(1, lett.data[i][j] ? '@' : '.'); // back to the expected format
//
// 		j[std::to_string(i)] = rows;
// 		i++;
// 	}
//
// 	// std::cout << std::endl << j.dump(2) << std::endl;
//
// 	std::ofstream o("optimized.json");
// 	o << j.dump(2) << std::endl;


  		std::ifstream i("optimized.json");
  		json j; i >> j;
	     std::vector<letter> letters;

      if(letters.size() == 0){
        for (auto& element : j) { // per character
          letter temp; int y = 0;
          temp.data.resize(element.size());
          for(auto& row : element) {
            temp.buildrow(y, row);
            y++;
          }
          if(!temp.nfg) letters.push_back(temp);
        }

        std::cout << "loaded " << letters.size() << " letters from " << j.size() << " fonts." << std::endl;
      }

      size_t max_vertical_size = 0;
      size_t max_horizontal_size = 0;
    for(auto& letter : letters)
      max_vertical_size = std::max(letter.data.size(), max_vertical_size);

    for(auto& letter : letters)
      max_horizontal_size = std::max(letter.data[0].size(), max_horizontal_size);

    std::cout << " height is going to be " << max_vertical_size + 1 << std::endl;
    std::cout << " width is going to be " << max_horizontal_size + 1 << std::endl;

    constexpr int num_wide = 69; // how many side to side
    const int num_rows = int(std::ceil(static_cast<float>(letters.size())/static_cast<float>(num_wide)));

    int width_pixels = num_wide * max_horizontal_size;
    int height_pixels = num_rows * max_horizontal_size;

    // this is scratch space memory, from what I understand
    std::vector<stbrp_node> nodes; nodes.resize(letters.size()*2);
    stbrp_context c;
    stbrp_init_target(&c, width_pixels, height_pixels, &nodes[0], letters.size()*2);

    std::vector<stbrp_rect> rects; int k = 0;
    for(auto& letter : letters) {
      stbrp_rect r;

      r.w = letter.data[0].size()+1; // padding
      r.h = letter.data.size()+1;
      r.index = k; k++;

      rects.push_back(r);
    }

    if(stbrp_pack_rects(&c, &rects[0], letters.size()))
      std::cout << "success" << std::endl;

    width_pixels += 14;
    int ymax = 0;
    std::vector<unsigned char> image;
    image.resize(height_pixels * width_pixels * 4, 255);

    for(auto& rect : rects) {
      for(int x = 0; x < rect.w-1; x++) // compensate for padding
      for(int y = 0; y < rect.h-1; y++) {
        int base = ((x+rect.x+7) + width_pixels * (y+rect.y+7)) * 4;
        if(letters[rect.index].data[y][x] == 1) {
          image[base+0] = 0;
          image[base+1] = 0;
          image[base+2] = 0;
          image[base+3] = 255;
        }
      }
      ymax = std::max(rect.y+rect.h+14, ymax);
    }



      stbi_write_png("test2.png", width_pixels, ymax, 4, &image[0], width_pixels * 4);
}
