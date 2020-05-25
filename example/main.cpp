#include "radiuspp.hpp"

void on_res( const RadiusDict &dict, std::vector<uint8_t> avp ) {
    auto res = deserialize<RadiusResponse>( dict, avp );
    std::cout << "Framed-IP: " << res.framed_ip.to_string() << std::endl;
    std::cout << "DNS1: " << res.dns1.to_string() << std::endl;
    std::cout << "DNS2: " << res.dns2.to_string() << std::endl;
}

void on_err( const std::string &err ) {
    std::cout << "Error: " << err << std::endl;
}

int main( int argc, char* argv[] ) {
    std::vector<std::string> files = {
        "/usr/share/freeradius/dictionary.rfc2865",
        "/usr/share/freeradius/dictionary.rfc2869",
        "/usr/share/freeradius/dictionary.ericsson.ab"
    };

    auto main_dict = RadiusDict( files );

    RadiusRequest req;
    req.username = "user";
    req.password = "pass";
    req.nas_id = "vBNG test";
    req.framed_protocol = "PPP";
    req.service_type = "Framed-User";
    req.calling_station_id = "00:aa:bb:11:22:33";
    req.nas_port_id = "vlan200";

    io_service io;
    AuthClient udp( io, address_v4::from_string( "127.0.0.1" ), 1812, "testing123", main_dict );
    udp.request<RadiusRequest>( req, std::bind( on_res, main_dict, std::placeholders::_1 ), on_err );

    io.run();

    return 0;
}