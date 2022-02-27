#include <algorithm>
#include <iostream>
#include <vector>
#include <random>

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

uint64_t RowToUint64_t ( std::vector< unsigned char > inputRow ) {
	uint64_t accumulator = 0;
	for ( int i = 1; i <= inputRow.size(); i++ ) {
		accumulator += ( inputRow[ inputRow.size() - i ] == 1 ) ? ( uint64_t( 1 ) << i ) : 0;
	}
	return accumulator;
}

json output;
void AddUintGlyph ( letter in ) {
	static int num = 0;
	output[ std::to_string( num ) ][ "x" ] = in.data[ 1 ].size();
	output[ std::to_string( num ) ][ "y" ] = in.data.size();
	std::vector< uint64_t > data;
	for ( int i = 0; i < in.data.size(); i++ ) {
		data.push_back( RowToUint64_t( in.data[ i ] ) );
	}
	output[ std::to_string( num ) ][ "d" ] = data;
	num++;
}

void WriteModelAsUints () {
	// shuffle around the entries
	auto rd = std::random_device {};
	auto rng = std::default_random_engine { rd() };
	std::shuffle( std::begin( glyphs ), std::end( glyphs ), rng );

	// add to json object
	for ( int i = 0; i < glyphs.size(); i++ ) {
		AddUintGlyph( glyphs[ 1 ] );
	}

	// save json to file
	std::ofstream o ( "uintEncoded.json" );
	o << output.dump( 0 ) << std::endl;

	// ended up not being smaller
	// std::vector< uint8_t > convertedToBSON = json::to_bson( output );
	// for ( int i = 0; i < convertedToBSON.size(); i++ ) {
		// o << convertedToBSON[ i ];
	// }
}


int main ( int argc, char const *argv[] ) {
	PopulateList();

	// int maxRowSize = 0;
	// int maxRowSizeIndex = -1;
	// for ( int i = 0; i < glyphs.size(); i++ ) {
	// 	if ( glyphs[ i ].data[ 1 ].size() > maxRowSize ) {
	// 		maxRowSize = glyphs[ i ].data[ 1 ].size();
	// 		maxRowSizeIndex = i;
	// 	}
	// }
	//
	// std::cout << "maximum observed width in the data: " << maxRowSize << " at " << maxRowSizeIndex << std::endl;
	// glyphs[ maxRowSizeIndex ].print();

	WriteModelAsUints();

	return 0;
}
