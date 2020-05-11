#include <map>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <tuple>
#include <boost/asio.hpp>
#include <boost/algorithm/string.hpp>


constexpr auto bswap( uint16_t val ) noexcept {
    return __builtin_bswap16( val );
}

constexpr auto bswap( uint32_t val ) noexcept {
    return __builtin_bswap32( val );
}

template<typename T>
struct Raw {
    T value;
};

template<typename T>
Raw(T) -> Raw<T>;

template<typename T>
class NetInt {
    using Underlying = T;
    using Native = T;
public:
    constexpr NetInt() = default;

    constexpr explicit NetInt( Native v ) noexcept :
        value { bswap( v ) }
    {}

    constexpr NetInt( Raw<Underlying> v ) noexcept :
        value { v }
    {}

    constexpr Native native() const {
        return bswap( value );
    }
    constexpr Underlying raw() const {
        return value;
    }

    friend std::ostream&
    operator<<(std::ostream& out, const NetInt& value) {
        return out << value.native();
    }

private:
    Underlying value;
};

using BE16 = NetInt<uint16_t>;
using BE32 = NetInt<uint32_t>;

struct AVP {
    uint8_t type;
    uint8_t length;
    std::vector<uint8_t> value;

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
};

template<>
std::tuple<std::string, bool> AVP::getVal<std::string>() const {
    return { { value.begin(), value.end() }, true };
}

template<>
std::tuple<BE32, bool> AVP::getVal<BE32>() const {
    if( value.size() != 4 ) {
        return { BE32( 0 ), false };
    } 
    return { BE32( *reinterpret_cast<const uint32_t*>( value.data() ) ), true };
}

template<>
std::tuple<BE16, bool> AVP::getVal<BE16>() const {
    if( value.size() != 4 ) {
        return { BE16( 0 ), false };
    } 
    return { BE16( *reinterpret_cast<const uint16_t*>( value.data() ) ), true };
}