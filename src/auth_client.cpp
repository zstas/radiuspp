#include "main.hpp"

AuthClient::AuthClient( io_service& i, const address_v4& ip_address, uint16_t port, std::string s, RadiusDict d ): 
    io( i ), 
    socket( i, udp::endpoint(udp::v4(), 0) ),
    endpoint( ip_address, port ),
    secret( std::move( s )),
    dict( std::move( d ) ),
    last_id( 0 )
{}

AuthClient::~AuthClient()
{
	socket.close();
}

void AuthClient::send( const std::vector<uint8_t> &msg ) {
	socket.send_to( boost::asio::buffer( msg, msg.size() ), endpoint );
    receive();
}

void AuthClient::receive() {
    socket.async_receive_from( boost::asio::buffer( buf, buf.size() ), endpoint, std::bind( &AuthClient::on_rcv, this, std::placeholders::_1, std::placeholders::_2 ) );
}

// Incoming packet should be in buf
bool AuthClient::checkRadiusAnswer( const authenticator_t &req_auth, const authenticator_t &res_auth, const std::vector<uint8_t> &avp ) {
    std::string check { buf.begin(), buf.begin() + 4 };
    check.reserve( 128 );
    check.insert( check.end(), req_auth.begin(), req_auth.end() );
    check.insert( check.end(), avp.begin(), avp.end() );
    check.insert( check.end(), secret.begin(), secret.end() );
    auto hash_str = md5( check );
    std::vector<uint8_t> hash_to_check{ hash_str.begin(), hash_str.end() };

    if( std::equal( res_auth.begin(), res_auth.end(), hash_to_check.begin() ) ) {
        return true;
    } 
    return false;
}

void AuthClient::on_rcv( boost::system::error_code ec, size_t size ) {
    if( ec ) {
        std::cerr << ec.message() << std::endl;
    }

    auto pkt = reinterpret_cast<Packet*>( buf.data() );
    std::cout << pkt->to_string() << std::endl;

    auto const &it = callbacks.find( pkt->id );
    if( it == callbacks.end() ) {
        return;
    }
    auto &auth_authenticator = it->second.auth;

    std::vector<uint8_t> avp_buf { buf.begin() + sizeof( Packet ), buf.begin() + pkt->length.native() };

    if( !checkRadiusAnswer( it->second.auth, pkt->authenticator, avp_buf ) ) {
        std::cerr << "Answer is not correct, check the RADIUS secret" << std::endl;
        return;
    }

    auto res = deserialize<RadiusResponse>( dict, avp_buf );
    it->second.response( res );
}