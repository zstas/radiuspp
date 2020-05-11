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
        if( line.find( "ATTRIBUTE" ) == std::string::npos ) {
            continue;
        }
        std::vector<std::string> out;
        boost::split( out, line, boost::is_any_of(" \t") );
        std::remove_if( out.begin(), out.end(), []( const std::string &r )->bool { return r.size() == 0; } );
        if( out.size() < 4) {
            continue;
        }
        uint8_t attr_id = std::stoi( out[2] );
        RADIUS_TYPE_T type { RADIUS_TYPE_T::ERROR };
        if( out[ 3 ].find( "string" ) == 0 ) { type = RADIUS_TYPE_T::STRING; }
        if( out[ 3 ].find( "octets" ) == 0 ) { type = RADIUS_TYPE_T::OCTETS; }
        if( out[ 3 ].find( "ipaddr" ) == 0 ) { type = RADIUS_TYPE_T::IPADDR; }
        if( out[ 3 ].find( "integer" ) == 0 ) { type = RADIUS_TYPE_T::INTEGER; }
        if( out[ 3 ].find( "vsa" ) == 0 ) { type = RADIUS_TYPE_T::VSA; }
        attrs.emplace( std::piecewise_construct, std::forward_as_tuple( attr_id ), std::forward_as_tuple( out[1], type ) );
    }
}

uint8_t RadiusDict::getIdByName( const std::string &attr ) const {
    for( auto const &[ k, v ]: attrs ) {
        if( v.first == attr ) {
            return k;
        }
    }
    return 0;
}

radius_attribute_t RadiusDict::getAttrById( uint8_t id ) const {
    if( auto const &it = attrs.find( id ); it != attrs.end() ) {
        return it->second;
    }
    return { {}, RADIUS_TYPE_T::ERROR };
}