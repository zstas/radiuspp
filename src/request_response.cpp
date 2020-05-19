#include "main.hpp"

template<>
std::vector<uint8_t> serialize<RadiusRequest>( const RadiusDict &dict, const RadiusRequest &req, const authenticator_t &a, const std::string &secret ) {
    std::set<AVP> avp_set { 
        AVP { dict, "User-Name", req.username },
        AVP { dict, "User-Password", password_pap_process( a, secret, req.password ) }
    };

    return serializeAVP( avp_set );
}

template<>
RadiusResponse deserialize<RadiusResponse>( const RadiusDict &dict, std::vector<uint8_t> &v ) {
    RadiusResponse res;

    auto avp_set = parseAVP( v );
    for( auto const &avp: avp_set ) {
        auto const &attr = dict.getAttrById( avp.type ); 
        if( attr.first == "Framed-IP-Address" ) {
            if( auto const &[ ip, success ] = avp.getVal<BE32>(); success ) {
                res.framed_ip = address_v4{ ip.native() };
            } 
        }
    }
    return res;
}