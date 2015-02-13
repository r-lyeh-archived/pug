// pugify: png<=>pug conversion. public domain.
// - rlyeh.

#include <stddef.h>
#include <string.h>
#include <cctype>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "deps/lodepng.h"
#include "deps/lodepng.cpp"
#include "deps/jpge.h"
#include "deps/jpge.cpp"
#include "deps/stb_image.c"

#define VERSION "1.0.2"

struct image {
    unsigned w, h;
    std::vector<unsigned char> pixels;
};

image decode_png( const void *data, unsigned size );
image decode_pug( const void *data, unsigned size );

std::string encode_png( unsigned w, unsigned h, const void *data, unsigned stride  );
std::string encode_pug( unsigned w, unsigned h, const void *data, unsigned quality );

int main( int argc, const char **argv ) {

    auto read_file = []( std::string &input, const std::string &stream ) -> bool {
        std::stringstream ss;
        std::ifstream ifs( stream, std::ios::binary );
        return ss << ifs.rdbuf() ? (input = ss.str(), true) : (input = std::string(), false);
    };

    auto write_file = []( const std::string &filename, const std::string &data ) -> bool {
        if( !data.empty() ) {
            std::ofstream ofs( filename.c_str(), std::ios::binary );
            ofs.write( &data[0], data.size() );
            return ofs.good();                
        }
        return false;
    };

    if( argc < 3 ) {
        std::cout
            << argv[0] << " v" VERSION " - https://github.com/r-lyeh/pug" << std::endl << std::endl
            << "Usage: " << argv[0] << " input.png output.pug [0..100]        (default quality: 70)" << std::endl
            << "Usage: " << argv[0] << " input.pug output.png" << std::endl;
        return -1;
    }

    unsigned quality = 70;
    if( argc >= 4 ) quality = std::strtoul( argv[3], 0, 0 );
    if( quality > 100 ) quality = 100;

    std::string args[] = { argv[0], argv[1], argv[2] };
    std::string ext = args[1].substr( args[1].find_last_of('.') + 1 );

    image img;
    std::string input;
    std::string output;

    if( !read_file( input, args[1] ) ) {
        std::cerr << "cannot read input file" << std::endl;
        return -1;
    }

    if( ext == "pug" ) {
        img = decode_pug( &input[0], input.size() );
        output = encode_png( img.w, img.h, img.pixels.data(), 4 );
    }
    else {
        img = decode_png( &input[0], input.size() );
        output = encode_pug( img.w, img.h, img.pixels.data(), quality );
    }

    if( !write_file( args[2], output) ) {
        std::cerr << "cannot write output file" << std::endl;
        return -1;
    }

    return 0;
}

// libpug

int32_t swapbe( int32_t v ) {
    // swap bytes (on big endian architectures)
    union autodetect {
        int word;
        char byte[ sizeof(int) ];
        autodetect() : word(1) 
        {}
    } _;
    bool is_big = _.byte[0] == 0;
    if( is_big ) {
        unsigned char *p = (unsigned char *)&v;
        std::swap( p[0], p[3] );
        std::swap( p[1], p[2] );
    }
    return v;
};

image decode_png( const void *ptr, size_t size ) {
    int w = 0, h = 0, bpp = 0;
    stbi_uc *imageuc = stbi_load_from_memory( (const unsigned char *)ptr, size, &w, &h, &bpp, 4 );
    if( imageuc ) {
        // convert to image class
        image img { w, h, std::vector<unsigned char>( w * h * 4 ) };
        memcpy( img.pixels.data(), imageuc, w * h * 4 );
        stbi_image_free( imageuc );
        return img;
    } else {
        return image { 0, 0 };        
    }
}

image decode_jpg( const void *ptr, size_t size ) {
    return decode_png( ptr, size );
}

image decode_pug( const void *ptr, size_t size ) {
    image img = decode_jpg( ptr, size );
    // if .pug file, then decode alpha
    if( img.pixels.size() && 0 == memcmp( "pug1", (const char *)ptr + size - 4, 4 ) ) {
        const int32_t color_size = swapbe( *(const int32_t *)((const char *)ptr + size - 12) );
        const int32_t alpha_size = swapbe( *(const int32_t *)((const char *)ptr + size - 8) );
        int w2 = 0, h2 = 0, bpp2 = 0;
        stbi_uc *alpha = stbi_load_from_memory( (const unsigned char *)ptr + color_size, alpha_size, &w2, &h2, &bpp2, 1 );
        if( alpha ) {
            for( unsigned it = 0, end = w2 * h2; it < end; ++it ) {
                img.pixels[ it * 4 + 3 ] = alpha[ it ];
            }
            stbi_image_free( alpha );                        
        }
    }                
    return img;
}

std::string encode_png( unsigned w, unsigned h, const void *data, unsigned stride ) {
    if( w && h && data && stride ) {
        auto mode = LCT_RGBA;
        /**/ if( stride == 3 ) mode = LCT_RGB;
        else if( stride == 2 ) mode = LCT_GREY_ALPHA;
        else if( stride == 1 ) mode = LCT_GREY;
        unsigned char* png;
        size_t pngsize;
        unsigned bpp = 8;
        unsigned error = lodepng_encode_memory_std( &png, &pngsize, (const unsigned char *)data, w, h, mode, bpp );
        if( !error && pngsize ) {
            std::string buf;
            buf.resize(pngsize);
            memcpy(&buf[0],png,pngsize);
            free(png);
            return buf;
        }
    }
    return std::string();
}

std::string encode_jpg( unsigned w, unsigned h, const void *data, unsigned quality ) {
    if( w && h && data && quality ) {
        std::string buf( 1024 + w * h * 3, '\0' );
        jpge::params p;
        p.m_quality = (int)quality;
        p.m_two_pass_flag = true; // slower but slighty smaller
        int buf_size = (int)buf.size();
        if( jpge::compress_image_to_jpeg_file_in_memory(&buf[0], buf_size, w, h, 4, (const jpge::uint8 *)data, p) ) {
            if( buf_size > 0 ) {
                buf.resize((unsigned)(buf_size));
                return buf;
            }
        }
    }
    return std::string();
}

std::string encode_pug( unsigned w, unsigned h, const void *data, unsigned quality ) {
    if( w && h && data && quality ) {
        // encode color as jpg
        std::string jpg = encode_jpg( w, h, data, quality );
        // encode alpha as png (gray, lum8)
        std::vector<unsigned char> alpha( w * h );
        unsigned char *ptr = ((unsigned char *)data) + 3;
        for( unsigned x = 0; x < w * h; ++x, ptr += 4 ) alpha[ x ] = *ptr;
        std::string png = encode_png( w, h, alpha.data(), 1 );
        // glue and footer
        int32_t size24 = swapbe( int32_t(jpg.size()) );
        int32_t size08 = swapbe( int32_t(png.size()) );
        if( size24 && size08 ) {
            std::stringstream ss;
            ss.write( &jpg[0], size24 );
            ss.write( &png[0], size08 );
            ss.write( (const char *)&size24, 4 );
            ss.write( (const char *)&size08, 4 );
            ss.write( "pug1", 4 );
            if( ss.good() ) {
                return ss.str();
            }
        }
    }
    return std::string();
}     


