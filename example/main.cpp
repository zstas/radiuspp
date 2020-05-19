#include "radiuspp.hpp"

void on_res( const RadiusDict &dict, std::vector<uint8_t> avp ) {
    auto res = deserialize<RadiusResponse>( dict, avp );
    std::cout << "Response: " << res.framed_ip.to_string() << std::endl;
}

int main( int argc, char* argv[] ) {
    auto main_dict = RadiusDict( "/usr/share/freeradius/dictionary.rfc2865" );

    RadiusRequest req;
    req.username = "zstas";
    req.password = "1234567890123456789012345";
    req.nas_id = "vBNG test";
    req.framed_protocol = "PPP";
    req.service_type = "Framed-User";

    io_service io;
    AuthClient udp( io, address_v4::from_string( "127.0.0.1" ), 1812, "testing123", main_dict );
    udp.request<RadiusRequest>( req, std::bind( on_res, main_dict, std::placeholders::_1 ) );

    io.run();

    return 0;
}