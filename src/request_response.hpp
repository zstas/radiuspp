#ifndef REQUEST_RESPONSE_HPP
#define REQUEST_RESPONSE_HPP

struct RadiusRequest {
    std::string username;
    std::string password;
    std::string nas_id;
    std::string service_type;
    std::string framed_protocol;
    std::string calling_station_id;
    std::string nas_port_id;
};

struct RadiusResponse {
    address_v4 framed_ip;
    address_v4 dns1;
    address_v4 dns2;
};

#endif