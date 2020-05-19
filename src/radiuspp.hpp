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

using boost::asio::ip::udp;
using address_v4 = boost::asio::ip::address_v4;
using io_service = boost::asio::io_service;

#include "utils.hpp"
#include "net_integer.hpp"
#include "radius_avp.hpp"
#include "radius_dict.hpp"
#include "packet.hpp"
#include "request_response.hpp"
#include "auth_client.hpp"