#ifndef REQUEST_RESPONSE_HPP
#define REQUEST_RESPONSE_HPP

struct RadiusRequest {
    std::string username;
    std::string password;
    std::string nas_id;
    std::string service_type;
    std::string framed_protocol;
};

struct RadiusResponse {
    address_v4 framed_ip;
};

#endif