#include "main.hpp"
#include <boost/random/random_device.hpp>
#include <boost/uuid/detail/md5.hpp>
#include <boost/algorithm/hex.hpp>

using md5_t = boost::uuids::detail::md5;

authenticator_t generateAuthenticator() {
    boost::random::random_device rng;
    authenticator_t ret;

    rng.generate( ret.begin(), ret.end() );

    return ret;
}

std::string md5( const std::vector<uint8_t> &v ) {
    md5_t hash;
    md5_t::digest_type digest;

    hash.process_bytes( v.data(), v.size() );
    hash.get_digest( digest );

    const auto charDigest = reinterpret_cast<const char *>(&digest);
    std::string result;
    boost::algorithm::hex( charDigest, charDigest + sizeof( md5_t::digest_type ), std::back_inserter( result ) );

    return result;
}