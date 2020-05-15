#ifndef RADIUS_DICT_HPP
#define RADIUS_DICT_HPP

enum class RADIUS_TYPE_T : uint8_t {
    STRING,
    INTEGER,
    IPADDR,
    OCTETS,
    VSA,
    ERROR
};

struct radius_attribute_t {
    std::string name;
    RADIUS_TYPE_T type;
    std::map<std::string,uint8_t> values;

    radius_attribute_t( std::string n, RADIUS_TYPE_T t ):
        name( std::move( n ) ),
        type( t )
    {}
};

using attributes_t = std::map<uint8_t,radius_attribute_t>;

class RadiusDict {
public:
    RadiusDict( const std::string &path );
    uint8_t getIdByName( const std::string &attr ) const;
    std::pair<std::string,RADIUS_TYPE_T> getAttrById( uint8_t id ) const;
private:
    attributes_t attrs;
};

#endif