#include <map>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <tuple>
#include <set>
#include <boost/asio.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/asio/ip/address.hpp>

#include "utils.hpp"
#include "net_integer.hpp"
#include "radius_avp.hpp"
#include "radius_dict.hpp"
#include "packet.hpp"

using address_v4 = boost::asio::ip::address_v4;

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