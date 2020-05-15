#include "main.hpp"

RadiusDict::RadiusDict( const std::string &path ) {
    std::ifstream file { path };
    if( !file.is_open() ) {
        return;
    }

    std::string line; 
    while( getline( file, line ) ) {
        if( line.front() == '#' ) {
            continue;
        }
        if( line.find( "ATTRIBUTE" ) == std::string::npos &&
            line.find( "VALUE") == std::string::npos ) {
            continue;
        }
        std::vector<std::string> out;
        boost::split( out, line, boost::is_any_of(" \t") );
        std::remove_if( out.begin(), out.end(), []( const std::string &r )->bool { return r.size() == 0; } );
        if( out.size() < 4) {
            continue;
        }

        if( out[ 0 ].find( "ATTRIBUTE" ) == 0 ) {
            uint8_t attr_id = std::stoi( out[ 2 ] );
            RADIUS_TYPE_T type { RADIUS_TYPE_T::ERROR };
            if( out[ 3 ].find( "string" ) == 0 ) { type = RADIUS_TYPE_T::STRING; }
            if( out[ 3 ].find( "octets" ) == 0 ) { type = RADIUS_TYPE_T::OCTETS; }
            if( out[ 3 ].find( "ipaddr" ) == 0 ) { type = RADIUS_TYPE_T::IPADDR; }
            if( out[ 3 ].find( "integer" ) == 0 ) { type = RADIUS_TYPE_T::INTEGER; }
            if( out[ 3 ].find( "vsa" ) == 0 ) { type = RADIUS_TYPE_T::VSA; }
            attrs.emplace( std::piecewise_construct, std::forward_as_tuple( attr_id ), std::forward_as_tuple( out[1], type ) );
        } else if( out[ 0 ].find( "VALUE" ) == 0 ) {
            uint8_t attr_val = std::stoi( out[ 3 ] );
            for( auto &[ k, v ]: attrs ) {
                if( out[ 1 ].find( v.name ) == 0 ) {
                    v.values.emplace( out[ 2 ], attr_val );
                }
            }
        }
    }
}

uint8_t RadiusDict::getIdByName( const std::string &attr ) const {
    for( auto const &[ k, v ]: attrs ) {
        if( v.name == attr ) {
            return k;
        }
    }
    return 0;
}

std::pair<std::string,RADIUS_TYPE_T> RadiusDict::getAttrById( uint8_t id ) const {
    if( auto const &it = attrs.find( id ); it != attrs.end() ) {
        return { it->second.name, it->second.type };
    }
    return { {}, RADIUS_TYPE_T::ERROR };
}