#include <iostream>
#include <algorithm>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define STB_RECT_PACK_IMPLEMENTATION
#include "stb_rect_pack.h"

#include "letters.h"

std::vector< letter > glyphs;
std::string UintToString ( uint64_t data, int width ) {
	std::string temp;
	for ( int i = 0; i <= width; i++ ) {
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

void GeneratePackedVis ( ) {
	size_t max_vertical_size = 0;
	size_t max_horizontal_size = 0;

	for ( auto& letter : glyphs ) {
		max_vertical_size = std::max( letter.data.size(), max_vertical_size );
	}

	for ( auto& letter : glyphs ) {
		max_horizontal_size = std::max( letter.data[ 0 ].size(), max_horizontal_size );
	}

	std::cout << " height is going to be " << max_vertical_size + 2 << std::endl;
	std::cout << " width is going to be " << max_horizontal_size + 2 << std::endl;

	constexpr int num_wide = 69; // how many side to side
	const int num_rows = int( std::ceil( static_cast< float >( glyphs.size() ) / static_cast< float >( num_wide ) ) );

	int width_pixels = num_wide * max_horizontal_size;
	int height_pixels = num_rows * max_horizontal_size;

	// this is scratch space memory, from what I understand
	std::vector< stbrp_node > nodes;
	nodes.resize( glyphs.size() * 2 );
	stbrp_context c;
	stbrp_init_target( &c, width_pixels, height_pixels, &nodes[ 0 ], glyphs.size() * 2 );

	int k = 0;
	std::vector< stbrp_rect > rects;
	for ( auto& letter : glyphs ) {
		stbrp_rect r;
		r.w = letter.data[ 0 ].size() + 2; // padding
		r.h = letter.data.size() + 2;
		r.index = k;
		k++;
		rects.push_back( r );
	}

	if ( stbrp_pack_rects( &c, &rects[ 0 ], glyphs.size() ) ) {
		std::cout << "success" << std::endl;
	}

	width_pixels += 14;
	int ymax = 0;
	std::vector< unsigned char > image;
	image.resize( height_pixels * width_pixels * 4, 255 );

	std::random_device r;
	std::seed_seq s{ r(), r(), r(), r(), r(), r(), r(), r(), r() };
	std::mt19937_64 gen = std::mt19937_64( s );

	std::uniform_int_distribution<int> color( 60, 189 );

	for( auto& rect : rects ) {
		int col = color( gen );
		for( int x = 0; x < rect.w-2; x++ ) // compensate for padding
		for( int y = 0; y < rect.h-2; y++ ) {
			int base = ( ( x + rect.x + 7 ) + width_pixels * ( y + rect.y + 7 ) ) * 4;
			if( glyphs[ rect.index ].data[ y ][ x ] == 1 ) {
				image[ base + 0 ] = 0;
				image[ base + 1 ] = 0;
				image[ base + 2 ] = 0;
				image[ base + 3 ] = 255;
			}else{
				image[ base + 0 ] = col;
				image[ base + 1 ] = col;
				image[ base + 2 ] = col;
				image[ base + 3 ] = 255;
			}
		}
		ymax = std::max( rect.y + rect.h + 14, ymax );
	}
	stbi_write_png( "rectPack.png", width_pixels, ymax, 4, &image[ 0 ], width_pixels * 4 );
}

int main ( int argc, char const *argv[] ) {
	ReadUintModel();
	std::cout << "loaded " << glyphs.size() << std::endl;
	GeneratePackedVis();
	return 0;
}
