#include <iostream>
#include <algorithm>

#include "letters.h"

std::vector< letter > glyphs;
std::string UintToString ( uint64_t data, int width ) {
	std::string temp;
	for ( int i = 0; i < width; i++ ) {
		temp += ( data % 2 ) ? '@' : '.';
		data = data >> 1;
	}
	reverse( temp.begin(), temp.end() );
	return temp;
}

void ReadUintModel () {
	std::ifstream i( "uintEncoded.json" );
	json j; i >> j;
	for ( auto& element : j ) { // per character
		int loadX = element[ "x" ];
		int loadY = element[ "y" ];
		int currentY = 0;
		letter temp;
		temp.data.resize( element[ "d" ].size() );
		for ( int i = 0; i < loadY; i++ ) {
			temp.buildrow( currentY, UintToString( element[ "d" ][ i ], loadX ) );
			currentY++;
		}
		if( !temp.nfg ) {
			glyphs.push_back( temp );
		}
	}
}


int main ( int argc, char const *argv[] ) {
	ReadUintModel();
	std::cout << "loaded " << glyphs.size() << " glyphs" << std::endl << std::endl;

	// print a glyph
	glyphs[ 99 ].print();

	// letter class data access:
		// glyphs[ n ].data is a 2d array of unsigned chars, which are either 0 or 1
		// accessing a particular 'pixel' on the glyph is done like this:
			// glyphs[ n ].data[ row ][ column ];

	return 0;
}
