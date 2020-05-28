#include "radiuspp.hpp"

void on_err( const std::string &err ) {
    std::cout << "Error: " << err << std::endl;
}

void on_res_acct( const RadiusDict &dict, RADIUS_CODE code, std::vector<uint8_t> avp ) {
}

void on_res_auth( AuthClient &acct_radius, RadiusDict &dict, RADIUS_CODE code, std::vector<uint8_t> avp ) {
    if( code == RADIUS_CODE::ACCESS_REJECT ) {
        std::cout << "Request rejected!" << std::endl;
    } else if( code == RADIUS_CODE::ACCESS_ACCEPT ) {
        std::cout << "Request accepted!" << std::endl;
    }
    auto res = deserialize<RadiusResponse>( dict, avp );
    std::cout << "Framed-IP: " << res.framed_ip.to_string() << std::endl;
    std::cout << "DNS1: " << res.dns1.to_string() << std::endl;
    std::cout << "DNS2: " << res.dns2.to_string() << std::endl;

    AcctRequest req;
    req.username = "user";
    req.nas_id = "vBNG test";
    req.calling_station_id = "00:aa:bb:11:22:33";
    req.nas_port_id = "vlan200";
    req.acct_status_type = "Start";
    req.in_pkts = 0;
    req.out_pkts = 0;

    acct_radius.acct_request<AcctRequest>( req, std::bind( on_res_acct, dict, std::placeholders::_1, std::placeholders::_2 ), on_err );
}

int main( int argc, char* argv[] ) {
    std::vector<std::string> files = {
        "/usr/share/freeradius/dictionary.rfc2865",
        "/usr/share/freeradius/dictionary.rfc2866",
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
    AuthClient auth_radius( io, address_v4::from_string( "127.0.0.1" ), 1812, "testing123", main_dict );
    AuthClient acct_radius( io, address_v4::from_string( "127.0.0.1" ), 1813, "testing123", main_dict );
    auth_radius.request<RadiusRequest>( req, std::bind( on_res_auth, std::ref( acct_radius ), std::ref( main_dict ), std::placeholders::_1, std::placeholders::_2 ), on_err );

    io.run();

    return 0;
}