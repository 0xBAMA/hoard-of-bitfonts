#include <iostream>
#include <vector>

#include "letters.h"
#include "json.h"
using json = nlohmann::json;

std::vector< letter > glyphs;
void PopulateBigList() { // get the glyphs into the glyphs array

	// optimized list
	std::ifstream i( "optimized.json" );
	json j; i >> j;
	for ( auto& element : j ) { // per character
		letter temp;
		int y = 0;
		temp.data.resize( element.size() );
		for ( auto& row : element ) {
			temp.buildrow( y, row );
			y++;
		}
		if( !temp.nfg ) {
			glyphs.push_back( temp );
		}
	}

	// non-optimized list
	std::ifstream i2( "monolithic_model.json" );
	json j2; i2 >> j2;
	for ( auto& element : j2 ) { // per font
		for ( unsigned int i = 0; i < element[ "num_glyphs" ]; i++ ) { // per glyph in the font
			letter temp;
			int y = 0;
			temp.data.resize( element[ "glyph" + std::to_string( i ) + "data" ].size() );
			for ( auto& row : element[ "glyph" + std::to_string( i ) + "data" ] ) { // per row in the data
				temp.buildrow( y, row );
				y++;
			}
			if( !temp.nfg && y != 0 ) {
				glyphs.push_back( temp );
			}
		}
	}
	std::cout << "Loaded " << glyphs.size() << " glyphs\n";
}

bool ValidityCheck ( letter l ) {
	int firstLineLength = l.data[ 1 ].size();
	for ( int y = 0; y < l.data.size(); y++ ) {
		for ( int x = 0; x < l.data[ y ].size(); x++ ) {
			if ( l.data[ y ].size() != firstLineLength ) {
				return false; // matching line length check
			}
			if ( l.data[ y ][ x ] != 0 && l.data[ y ][ x ] != 1 ) {
				return false; // valid letter check
			}
		}
	}
	// tests passed
	return true;
}

void OutputNewModel () {
	json j; int i = 0;
	for ( auto glyph : glyphs ) {
		std::vector< std::string > rows;
		rows.resize( glyph.data.size() );
		for ( int i = 0; i < glyph.data.size(); i++ ) {
			for ( int j = 0; j < glyph.data[ 0 ].size(); j++ ) {
				rows[ i ].append( 1, glyph.data[ i ][ j ] ? '@' : '.' ); // to the expected format
			}
		}
		j[ std::to_string( i ) ] = rows;
		i++;
	}
	std::ofstream o ( "optimized2electricboogaloo.json" );
	o << j.dump( 1 ) << std::endl;
}

int main ( int argc, char const *argv[] ) {
	// grab both the original and the optimized list
	PopulateBigList();

	// number of glyphs before processing
	int beginSize = glyphs.size();

	// valid glyph check
	std::cout << std::endl << "Begin Validity Check" << std::endl;
	for ( int l = 0; l < glyphs.size(); l++ ) {
		std::cout << "\r" << l << "/" << glyphs.size() << " (" << ( float( l ) / float( glyphs.size() - 1 ) ) * 100.0f << "%)                ";
		if( !ValidityCheck( glyphs[ l ] ) ) {
			glyphs[ l ].print();
			std::cout << "Failed Validity check at index " << l << std::endl;
			// remove invalid glyph from the list
			glyphs.erase( glyphs.begin() + l );
		}
	}

	// duplicate glyph check
	int numDuplicates = 0;
	std::cout << std::endl << "Begin Duplicate Check" << std::endl;
	for ( int letterIndex1 = 0; letterIndex1 < glyphs.size(); letterIndex1++ ) {
		std::cout << "\r" << letterIndex1 << "/" << glyphs.size() << " (" << ( float( letterIndex1 ) / float( glyphs.size() - 1 ) ) * 100.0f << "%)            ";
		for ( int letterIndex2 = letterIndex1 + 1; letterIndex2 < glyphs.size(); letterIndex2++ ) {
			if ( glyphs[ letterIndex1 ] == glyphs[ letterIndex2 ] ) {
				numDuplicates++;
				// remove invalid glyph from the list
				glyphs.erase( glyphs.begin() + letterIndex2 );
				// decrement letterIndex2 to compensate
				letterIndex2--;
			}
		}
	}

	// number of glyphs after processing
	int endSize = glyphs.size();

	std::cout << "found " << numDuplicates << " duplicates" << std::endl;
	std::cout << "total glyphs: " << std::endl;
	std::cout << " initial: " << std::endl << "  " << beginSize << std::endl << std::endl;
	std::cout << " after processing: " << std::endl << "  " << endSize << std::endl << std::endl;

	// dump the newly optimized list to another file
	OutputNewModel();

	return 0;
}
