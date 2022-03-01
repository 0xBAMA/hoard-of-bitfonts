#include <algorithm>
#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include "letters.h"
#include "lodepng.h"

// starting with 8x8 glyphs, since they have good coverage over all bins
// good options are: 8x8, 6x8, 8x16, 8x12, 7x8
int glyphX = 8;
int glyphY = 8;

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

// starting with 10 bins
constexpr int numBins = 24;
std::vector< letter > binnedGlyphs[ numBins ];
void BinGlyphs () {
	for ( int i = 0; i < glyphs.size(); i++ ) {
		if ( glyphs[ i ].data.size() == glyphY && glyphs[ i ].data[ 1 ].size() == glyphX ) {
			binnedGlyphs[ std::clamp( int( numBins * glyphs[ i ].fill() ), 0, numBins - 1 ) ].push_back( glyphs[ i ] );
		}
	}
}

// input
std::vector< unsigned char > inputImage;
unsigned inputImageWidth, inputImageHeight;

unsigned char getImageDataAt ( int x, int y, int c ) {
	// std::cout << " reading image data" << std::endl << std::flush;
	int index = ( x + y * inputImageWidth ) * 4;
	return inputImage[ index + c ];
}

void LoadImage ( std::string filename ) {
	unsigned error = lodepng::decode( inputImage, inputImageWidth, inputImageHeight, filename.c_str() );
	if ( error ) std::cout << "decode error during load(\" " + filename + " \") " << error << ": " << lodepng_error_text( error ) << std::endl;
}

// output
std::vector< unsigned char > outputImage;

void writeImageDataAt ( int x, int y, int c, unsigned char value ) {
	// std::cout << "  writing pixel" << std::endl << std::flush;
	// int index = ( ( glyphX * x ) + ( glyphY * y ) * ( glyphY * inputImageWidth ) ) * 4;
	int index = ( x + y * inputImageWidth * glyphX ) * 4;
	if ( index < outputImage.size() ) {
		outputImage[ index + c ] = value;
		// outputImage[ index + 3 ] = 255;
	} else {
		std::cout << " out of bounds write requested " << x << " " << y << " " << c << std::endl << std::flush;
	}
}

void writeGlyph ( int xin, int yin, int c, letter l ) {
	// x and y are in terms of the input pixels
	// std::cout << " writing glyph" << std::endl << std::flush;
	int baseX = glyphX * xin;
	int baseY = glyphY * yin;

	long unsigned int seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::default_random_engine engine{ seed };
	// std::uniform_int_distribution< int > distribution { 128, 255 };
	std::uniform_int_distribution< int > distribution { 245, 255 };

	for ( int y = 0; y < glyphY; y++ ) {
		for ( int x = 0; x < glyphX; x++ ) {
			writeImageDataAt( x + baseX, y + baseY, c, l.data[ y ][ x ] * distribution( engine ) );
		}
	}
}

void GenerateOutput () {
	outputImage.resize( ( glyphX * inputImageWidth ) * ( glyphY * inputImageHeight ) * 4, 255 );
	std::string filename = std::string( "out.png" );

	long unsigned int seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::default_random_engine engine{ seed };


	for ( int y = 0; y < inputImageHeight; y++ ) {
		for ( int x = 0; x < inputImageWidth; x++ ) {
			for( int c = 0; c < 4; c++ ) {
				// std::cout << "x is " << x << " y is " << y << " c is " << c << std::endl << std::flush;
				unsigned char imageData = getImageDataAt( x, y, c );
				int binPick = int( ( numBins - 1 ) * float( imageData ) / 255.0 );
				std::uniform_int_distribution< int > distribution { 0, int( binnedGlyphs[ binPick ].size() - 1 ) };
				int glyphPick = distribution( engine );
				writeGlyph( x, y, c, binnedGlyphs[ binPick ][ glyphPick ] );
			}
		}
		std::cout << "\r" << 100.0 * float( y ) / float( inputImageHeight ) << " percent      ";
	}

	std::cout << "\rwriting output to " << filename << "      " << std::endl;
	unsigned error = lodepng::encode( filename.c_str(), outputImage, inputImageWidth * glyphX, inputImageHeight * glyphY );
	if ( error ) std::cout << "encode error during save(\" " + filename + " \") " << error << ": " << lodepng_error_text( error ) << std::endl;
}

int main ( int argc, char const *argv[] ) {
	PopulateList();
	BinGlyphs();
	// LoadImage( std::string( argv[ 1 ] ) );
	LoadImage( std::string( "in.png" ) );
	GenerateOutput();

	return 0;
}
