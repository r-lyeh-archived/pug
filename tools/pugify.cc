// pugify: png<=>pug conversion. public domain.
// - rlyeh.

#include <cctype>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "spot/spot.hpp"

std::string read( const std::string &stream ) {
    std::ifstream ifs( stream, std::ios::binary );
    std::stringstream ss;
    ss << ifs.rdbuf();
    return ss.str();
}

std::string footer( const std::string &rgb24, const std::string &alpha8 ) {
    // @todo: endianness and alignment
    std::stringstream ss;
    std::int32_t size24(  rgb24.size() );
    std::int32_t size08( alpha8.size() );
    ss.write( (const char *)&size24, 4 );
    ss.write( (const char *)&size08, 4 );
    ss.write( "pug1", 4 );
    return ss.good() ? ss.str() : std::string();
}

int main( int argc, const char **_argv ) {

    if( argc < 3 ) {
        std::cout
            << _argv[0] << " v1.0.0 - https://github.com/r-lyeh/pug"   << std::endl << std::endl
            << "Usage: " << _argv[0] << " input.png output.pug [0..100]" << std::endl
            << "Usage: " << _argv[0] << " input.pug output.png" << std::endl;
        return -1;
    }

    unsigned quality = 70;
    if( argc >= 4 ) quality = std::strtoul( _argv[3], 0, 0 );
    if( quality > 100 ) quality = 100;

    std::string argv[] = { _argv[0], _argv[1], _argv[2] };
    std::string ext = argv[1].substr( argv[1].find_last_of('.') + 1 );

    if( ext.size() == 3 &&
        std::tolower( ext[0] ) == 'p' &&
        std::tolower( ext[1] ) == 'u' &&
        std::tolower( ext[2] ) == 'g' ) {

        char info[5] = {};
        std::int32_t RGB24, A8, FEOF;
        std::string data;
        {
            std::ifstream ifs( argv[1], std::ios::binary );
            std::stringstream ss;
            ss << ifs.rdbuf();
            data = ss.str();
        }

        {
            std::ifstream ifs( argv[1], std::ios::binary|std::ios::ate );
            FEOF = std::int32_t( ifs.tellg() );
            ifs.seekg( -12, std::ios_base::end );
            ifs.read( (char *)&RGB24, 4 );
            ifs.read( (char *)&A8, 4 );
            ifs.read( (char *)info, 4 );
        }

        if( strcmp( info, "pug1" ) )
            return std::cout << "bad input file: " << info << std::endl, -1;

        spot::image image( &data[FEOF - 12 - A8 - RGB24], RGB24 );
        spot::image alpha( &data[FEOF - 12 - A8], A8 );

        for( unsigned h = 0; h < image.h; ++h ) {
            for( unsigned w = 0; w < image.w; ++w ) {
                image.at(w,h).a = alpha.at(w,h).l;
            }
        }

        image.save_as_png( argv[2] );
        return 0;
    }
    else
    {
        spot::image image( argv[1] ), alpha( image.w, image.h );

        image = image.bleed();

        for( unsigned h = 0; h < image.h; ++h ) {
            for( unsigned w = 0; w < image.w; ++w ) {
                alpha.at(w,h) = image.at(w,h) * spot::color(0,0,0,1);
                image.at(w,h) = image.at(w,h) * spot::color(1,1,1,0);
            }
        }

        image.to_rgba().save_as_jpg( "$diffuse.jpg", quality );
        std::string RGB24 = read( "$diffuse.jpg" );
        unlink( "$diffuse.jpg" );

        std::string A8 = alpha.to_rgba().encode_as_png( false, false, 8 );

        std::ofstream ofs( argv[2], std::ios::binary );
        ofs << RGB24 << A8 << footer( RGB24, A8 );

        return 0;
    }

    std::cout << "cannot determine input file" << std::endl;
    return -1;
}
