#include <iostream>
#include <algorithm>
#include <random>

#include "lodepng.h"
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

	// example: draw some glyphs onto an image
	std::vector< unsigned char > imageData;
	int imageDim = 256;
	int glyphCount = 420;
	imageData.resize( 4 * imageDim * imageDim, 255 );

	auto rd = std::random_device {};
	auto rng = std::default_random_engine { rd() };
	auto glyphPick = std::uniform_int_distribution< int > { 0, int( glyphs.size() - 1 ) };
	auto locationPick = std::uniform_int_distribution< int > { 0, imageDim };
	auto channelPick = std::uniform_int_distribution< int > { 0, 2 };

	for ( int i = 0; i < glyphCount; i++ ) {
		letter selected = glyphs[ glyphPick( rng ) ];
		int xOffset = locationPick( rng );
		int yOffset = locationPick( rng );
		int c = channelPick( rng );
		for ( int y = 0; y < selected.data.size(); y++ ) {
			for ( int x = 0; x < selected.data[ 1 ].size(); x++ ) {
				int index = ( ( xOffset + x ) + ( yOffset + y ) * imageDim ) * 4 + c;
				if ( index < imageData.size() && selected.data[ y ][ x ] == 1 ) {
					imageData[ index ] = 0;
				}
			}
		}
	}

	std::string filename( "out.png" );
	unsigned error = lodepng::encode( filename.c_str(), imageData, imageDim, imageDim );
	if ( error ) {
		std::cout << "encode error during save(\" " + filename + " \") " << error << ": " << lodepng_error_text( error ) << std::endl;
	}

	return 0;
}
