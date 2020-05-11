#include "main.hpp"

using attributes_t = std::map<std::string,std::string>;

std::vector<AVP> parseAVP( std::vector<uint8_t> &v ) {
    std::vector<AVP> ret;
    auto it = v.begin();
    while( it != v.end() ) {
        ret.emplace_back( v, it );
        it += ret.back().getSize();
    }

    return ret;
}

int main( int argc, char* argv[] ) {
    std::string main_dict_path { "/usr/share/freeradius/dictionary.rfc2865" };

    attributes_t main_dict;

    std::ifstream file { main_dict_path };
    if( !file.is_open() ) {
        return -1;
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
        main_dict.emplace( out[1], out[3] );
    }

    for( auto const &[ attr, type ]: main_dict ) {
        std::cout << "ATTR: " << attr << " TYPE: " << type << std::endl;
    }

    std::vector<uint8_t> radius_avp = { 
        0x01, 0x07, 0x73, 0x74, 0x65, 0x76, 0x65, 0x02, 0x12, 0xdb, 0xc6, 0xc4, 0xb7, 0x58, 0xbe, 0x14, 0xf0, 0x05, 0xb3,
        0x87, 0x7c, 0x9e, 0x2f, 0xb6, 0x01, 0x04, 0x06, 0xc0, 0xa8, 0x00, 0x1c, 0x05, 0x06, 0x00, 0x00, 0x00, 0x7b, 0x50, 
        0x12, 0x5f, 0x0f, 0x86, 0x47, 0xe8, 0xc8, 0x9b, 0xd8, 0x81, 0x36, 0x42, 0x68, 0xfc, 0xd0, 0x45, 0x32, 0x4f, 0x0c, 
        0x02, 0x66, 0x00, 0x0a, 0x01, 0x73, 0x74, 0x65, 0x76, 0x65
    };

    auto avp_set = parseAVP( radius_avp );
    for( auto const &avp: avp_set ) {
        switch( avp.type ) {
        case 1:
            std::cout << "Username: ";
            if( auto const &[ ret, success ] = avp.getVal<std::string>(); success ) {
                std::cout << ret << std::endl;
            } else {
                std::cout << "ERROR" << std::endl;
            }
            break;
        case 2:
            std::cout << "Password: ";
            if( auto const &[ ret, success ] = avp.getVal<std::string>(); success ) {
                std::cout << ret << std::endl;
            } else {
                std::cout << "ERROR" << std::endl;
            }
            break;
        case 4:
            std::cout << "IP-Address: ";
            if( auto const &[ ret, success ] = avp.getVal<BE32>(); success ) {
                std::cout << ret << std::endl;
            } else {
                std::cout << "ERROR" << std::endl;
            }
            break;
        default:
            break;
        }
    }

    return 0;
}