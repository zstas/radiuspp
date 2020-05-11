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

#include "net_integer.hpp"
#include "radius_avp.hpp"
#include "radius_dict.hpp"
#include "packet.hpp"

using address_v4 = boost::asio::ip::address_v4;