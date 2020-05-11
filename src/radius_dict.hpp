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

using radius_attribute_t = std::pair<std::string,RADIUS_TYPE_T>;
using attributes_t = std::map<uint8_t,radius_attribute_t>;

class RadiusDict {
public:
    RadiusDict( const std::string &path );
    uint8_t getIdByName( const std::string &attr ) const;
    radius_attribute_t getAttrById( uint8_t id ) const;
private:
    attributes_t attrs;
};

#endif