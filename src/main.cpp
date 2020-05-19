#include "main.hpp"

void on_res( RadiusResponse res ) {
    std::cout << "Response: " << res.framed_ip.to_string() << std::endl;
}

int main( int argc, char* argv[] ) {
    auto main_dict = RadiusDict( "/usr/share/freeradius/dictionary.rfc2865" );

    RadiusRequest req;
    req.username = "zstas";
    req.password = "1234567890123456789012345";

    io_service io;
    AuthClient udp( io, address_v4::from_string( "127.0.0.1" ), 1812, "testing123", main_dict );
    udp.request( req, on_res );

    io.run();

    return 0;
}