#include <algorithm>
#include <iostream>
#include <vector>
#include "letters.h"

std::vector< letter > glyphs;
void PopulateList() { // get the glyphs into the glyphs array
	// new optimized list
	std::ifstream i( "optimized2electricboogaloo.json" );
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
}

struct dimensions { int x, y, count; std::vector< float > fills; };
std::vector< dimensions > sizes;
void CountDimensions () {
	sizes.clear();
	for ( int i = 0; i < glyphs.size(); i++ ) {
		// get the dimensions
		dimensions current;
		current.x = glyphs[ i ].data[ 1 ].size();
		current.y = glyphs[ i ].data.size();
		current.count = 1;
		current.fills.push_back( glyphs[ i ].fill() );
		bool dimensionFound = false;
		for ( int j = 0; j < sizes.size(); j++ ) {
			if ( sizes[ j ].x == current.x && sizes[ j ].y == current.y ) {
				sizes[ j ].count++;
				sizes[ j ].fills.push_back( current.fills[ 0 ] );
				dimensionFound = true;
				break;
			}
		}
		if ( !dimensionFound ) {
			sizes.push_back( current );
		}
	}
}

void Distribution ( dimensions d ) {
	int bins[ 10 ] = { 0 };
	// for ( int i = 0; i < 10; i++ ) {
		// bins[ i ] = 0;
	// }

	for ( int i = 0; i < d.fills.size(); i++ ) {
		bins[ std::clamp( int( 10 * d.fills[ i ] ), 0, 9 ) ]++;
	}
	std::cout << "| ";
	for ( int i = 0; i < 10; i++ ) {
		std::cout << bins[ i ] << " | ";
	}
	std::cout << std::endl << std::endl;
}

void EnumerateDimensions () {
	std::cout << "there are " << sizes.size() << " distinct glyph sizes" << std::endl;
	for ( int i = 0; i < sizes.size(); i++ ) {
		std::cout << sizes[ i ].x << " by " << sizes[ i ].y << " showed up " << sizes[ i ].count << " times\n";
		Distribution( sizes[ i ] );
	}
}

int main ( int argc, char const *argv[] ) {
	PopulateList();
	CountDimensions();
	EnumerateDimensions();

	return 0;
}
