// pugify: png<=>pug conversion. public domain.
// - rlyeh.

#include <cctype>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "spot/spot.hpp"

#define VERSION "1.0.1"

std::string readfile( const std::string &stream ) {
    std::ifstream ifs( stream, std::ios::binary );
    std::stringstream ss;
    ss << ifs.rdbuf();
    return ss.str();
}

std::string mkfooter( const std::string &color24, const std::string &alpha08 ) {
    // @todo: endianness and alignment
    std::stringstream ss;
    std::int32_t size24( color24.size() );
    std::int32_t size08( alpha08.size() );
    ss.write( (const char *)&size24, 4 );
    ss.write( (const char *)&size08, 4 );
    ss.write( "pug1", 4 );
    return ss.good() ? ss.str() : std::string();
}

bool rdfooter( const std::string &filename, std::int32_t &off24, std::int32_t &len24, std::int32_t &off08, std::int32_t &len08 ) {
    std::ifstream ifs( filename.c_str(), std::ios::binary|std::ios::ate );
    if( ifs.good() ) {
        std::int32_t FEOF = std::int32_t( ifs.tellg() );
        if( FEOF >= 12 ) {
            ifs.seekg( -12, std::ios_base::end );
            ifs.read( (char *)&len24, 4 );
            ifs.read( (char *)&len08, 4 );
            char info[5] = {};
            ifs.read( (char *)info, 4 );
            if( !strcmp( info, "pug1" ) ) {
                off24 = FEOF - 12 - len08 - len24;
                off08 = FEOF - 12 - len08;
                return true;
            }
        }
    }
    off24 = len24 = off08 = len08 = 0;
    return false;
}

int main( int argc, const char **_argv ) {

    if( argc < 3 ) {
        std::cout
            << _argv[0] << " v" VERSION " - https://github.com/r-lyeh/pug"   << std::endl << std::endl
            << "Usage: " << _argv[0] << " input.png output.pug [0..100]        (default quality: 70)" << std::endl
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

        std::string data = readfile( argv[1] );

        std::int32_t off24, len24;
        std::int32_t off08, len08;
        if( !rdfooter( argv[1], off24, len24, off08, len08 ) ) {
            return std::cout << "bad input file: " << argv[1] << std::endl, -1;            
        }

        spot::image color( &data[off24], len24 );
        spot::image alpha( &data[off08], len08 );

        for( unsigned h = 0; h < color.h; ++h ) {
            for( unsigned w = 0; w < color.w; ++w ) {
                color.at(w,h).a = alpha.at(w,h).l;
            }
        }

        return color.save_as_png( argv[2] ) ? 0 : -1;
    }
    else
    {
        spot::image color( argv[1] );
        color = color.bleed();

        std::string color24 = color.encode_as_jpg( quality );
        std::string alpha08 = color.encode_as_png( false, false, 8 );

        std::ofstream ofs( argv[2], std::ios::binary );
        if( ofs.good() ) {
            ofs << color24 << alpha08 << mkfooter( color24, alpha08 );
        }

        return ofs.good() ? 0 : -1;
    }

    std::cout << "cannot determine input file" << std::endl;
    return -1;
}
