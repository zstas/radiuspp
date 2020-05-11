#ifndef PACKET_HPP
#define PACKET_HPP

enum class RADIUS_CODE : uint8_t {
    ACCESS_REQUEST = 1,
    ACCESS_ACCEPT = 2,
    ACCESS_REJECT = 3,
    ACCOUNTING_REQUEST = 4,
    ACCOUNTING_RESPONSE = 5,
    ACCESS_CHALLENGE = 11,
    RESERVED = 255
};

struct Packet {
    RADIUS_CODE code;
    uint8_t id;
    BE16 length;
    std::array<uint8_t,16> authentificator;
}__attribute__((__packed__));

#endif