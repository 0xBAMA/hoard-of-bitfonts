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
			temp.print();
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
	// // shuffle around the entries
	// auto rd = std::random_device {};
	// auto rng = std::default_random_engine { rd() };
	// std::shuffle( std::begin( glyphs ), std::end( glyphs ), rng );

	// add each glyph to json object
	for ( int i = 0; i < glyphs.size(); i++ ) {
		AddUintGlyph( glyphs[ i ] );
	}

	// save json object to file
	std::ofstream o ( "uintEncoded.json" );
	o << output.dump( 0 ) << std::endl;
}

int main ( int argc, char const *argv[] ) {
	PopulateList();
	WriteModelAsUints();

	glyphs.clear();

	ReadUintModel();

	return 0;
}
