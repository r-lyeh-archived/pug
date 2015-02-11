// pugview: image viewer. public domain.
// - rlyeh

#include <iostream>
#include "spot/spot.hpp"

#define VERSION "1.0.1"

void display( const spot::image &pic, const char *title = "" );

int main( int argc, char **argv ) {
    if( argc < 2 ) {
        std::cout << argv[0] << " v" VERSION " - quick image viewer. https://github.com/r-lyeh/pug" << std::endl << std::endl;
        std::cout << "Usage: " << argv[0] << " image [image [...]]" << std::endl;
        return -1;
    }

    for( int i = 1; i < argc; ++i ) {
        spot::image base( argv[i] );
        display( base, argv[i] );        
    }

    return 0;
}

// CImg.h code following
#include "spot/samples/cimg.h"
#pragma comment(lib,"shell32.lib")
#pragma comment(lib,"user32.lib")
#pragma comment(lib,"gdi32.lib")
void display( const spot::image &pic, const char *title ) {
    if( pic.size() ) {
        using namespace spot;
        // check pattern that covers whole image.
        spot::image check( pic.w, pic.h );
        // cell size is 10% of image, check pattern is 10 cells wide.
        int cell_size = pic.w * 0.10f;
        for( size_t y = 0; y < check.h; ++y ) {
            for( size_t x = 0; x < check.w; ++x ) {
                int xc = x / cell_size;
                int yc = y / cell_size;
                float light = 0.5f + 0.5f * (( xc % 2 ) ^ ( yc % 2 ));
                check.at(x,y) = spot::hsla( 0.f, 0.f, light, 1.f );
            }
        }
        // blend image
        for( size_t p = 0, pend = pic.size(); p < pend; ++p ) {
            float a = pic.at(p).a;
            check.at( p ) = pic.at( p ) * a + check.at( p ) * (1-a);
            check.at( p ).a = 1;
        }
        auto &blend = check;
        // display blend
        cimg_library::CImg<unsigned char> ctexture( blend.w, blend.h, 1, 4, 0 );
        for( size_t y = 0; y < blend.h; ++y ) {
            for( size_t x = 0; x < blend.w; ++x ) {
                spot::pixel pix = blend.at( x, y );
                ctexture( x, y, 0 ) = pix.r;
                ctexture( x, y, 1 ) = pix.g;
                ctexture( x, y, 2 ) = pix.b;
                ctexture( x, y, 3 ) = pix.a;
            }
        }
        ctexture.display( title, false );
    }
}
