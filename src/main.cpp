#include "main.hpp"

using boost::asio::ip::udp;
using io_service = boost::asio::io_service;

class UDPClient
{
public:
	UDPClient( io_service& io_service, const address_v4& ip_address, uint16_t port ): 
        io_service_( io_service ), 
        socket_( io_service, udp::endpoint(udp::v4(), 0) ),
        endpoint_( ip_address, port )
    {}

	~UDPClient()
	{
		socket_.close();
	}

	void send( const std::vector<uint8_t> &msg ) {
		socket_.send_to( boost::asio::buffer( msg, msg.size() ), endpoint_ );
	}

private:
	boost::asio::io_service& io_service_;
	udp::socket socket_;
	udp::endpoint endpoint_;
};

int main( int argc, char* argv[] ) {
    auto main_dict = RadiusDict( "/usr/share/freeradius/dictionary.rfc2865" );    

    std::set<AVP> avp_set { 
        AVP { main_dict, "User-Name", "zstas" },
        AVP { main_dict, "User-Password", "12345" }
    };

    for( auto const &avp: avp_set ) {
        std::cout << printAVP( main_dict, avp ) << std::endl;
    }

    std::vector<uint8_t> pkt;
    pkt.resize( sizeof( Packet ) );
    auto pkt_hdr = reinterpret_cast<Packet*>( pkt.data() );
    pkt_hdr->code = RADIUS_CODE::ACCESS_REQUEST;
    pkt_hdr->id = 1;
    pkt_hdr->authentificator = { 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10 };

    auto seravp = serializeAVP( avp_set );
    pkt.insert( pkt.end(), seravp.begin(), seravp.end() );

    pkt_hdr = reinterpret_cast<Packet*>( pkt.data() );
    pkt_hdr->length = pkt.size();

    io_service io;
    UDPClient udp( io, address_v4::from_string( "127.0.0.1" ), 1812 );
    udp.send( pkt );

    return 0;
}