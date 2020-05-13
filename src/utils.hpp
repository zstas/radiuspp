#ifndef UTILS_HPP
#define UTILS_HPP

using authenticator_t = std::array<uint8_t,16>;

authenticator_t generateAuthenticator();
std::string md5( const std::vector<uint8_t> &v );

#endif