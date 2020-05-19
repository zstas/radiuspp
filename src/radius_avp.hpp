#ifndef RADIUS_AVP_HPP
#define RADIUS_AVP_HPP

#include "radius_dict.hpp"

struct AVP {
    uint8_t type;
    uint8_t length;
    std::vector<uint8_t> value;

    explicit AVP( const RadiusDict &dict, const std::string &attr, BE32 v ):
        length( sizeof( type) + sizeof( length ) + sizeof( v ) )
    {
        type = dict.getIdByName( attr );
        value.resize( sizeof( BE32 ) );
        *reinterpret_cast<uint32_t*>( value.data() ) = v.raw();
    }

    explicit AVP( const RadiusDict &dict, const std::string &attr, BE16 v ):
        length( sizeof( type) + sizeof( length ) + sizeof( v ) )
    {
        type = dict.getIdByName( attr );
        value.resize( sizeof( BE16 ) );
        *reinterpret_cast<uint16_t*>( value.data() ) = v.raw();
    }

    explicit AVP( const RadiusDict &dict, const std::string &attr, const std::string &s ):
        length( sizeof( type) + sizeof( length ) + s.size() ),
        value( s.begin(), s.end() )
    {
        type = dict.getIdByName( attr );
    }

    explicit AVP( const std::vector<uint8_t> &v, std::vector<uint8_t>::iterator it ) {
        if( ( v.end() - it ) < 2 ) {
            return;
        }

        type = *it;
        it++;
        length = *it;
        it++;
        value = { it, it + length - 2 };
    }

    size_t getSize() const {
        return sizeof( type ) + sizeof( length ) + value.size();
    }

    template<typename T>
    std::tuple<T, bool> getVal() const;

    bool operator<( const AVP &r ) const {
        return type < r.type;
    }

    std::vector<uint8_t> serialize() const {
        std::vector<uint8_t> ret;
        ret.reserve( sizeof( type) + sizeof( length) + length );
        ret.push_back( type );
        ret.push_back( length );
        ret.insert( ret.end(), value.begin(), value.end() );
        return ret;
    }
};

template<>
std::tuple<std::string, bool> AVP::getVal<std::string>() const;

template<>
std::tuple<BE32, bool> AVP::getVal<BE32>() const;

template<>
std::tuple<BE16, bool> AVP::getVal<BE16>() const;

std::vector<AVP> parseAVP( std::vector<uint8_t> &v );
std::string printAVP( const RadiusDict &dict, const AVP &avp );

std::vector<uint8_t> serializeAVP( const std::set<AVP> &avp );

#endif