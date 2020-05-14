#ifndef UTILS_HPP
#define UTILS_HPP

using authenticator_t = std::array<uint8_t,16>;

authenticator_t generateAuthenticator();
std::string md5( const std::string &v );
std::string md5_hex( const std::string &v );
std::string password_pap_process( const authenticator_t &auth, const std::string secret, std::string pass );

#endif