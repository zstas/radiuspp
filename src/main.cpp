#include "main.hpp"

using boost::asio::ip::udp;
using io_service = boost::asio::io_service;

std::string std::to_string( const RADIUS_CODE &code ) {
    switch( code ) {
    case RADIUS_CODE::ACCESS_REQUEST:
        return "ACCESS_REQUEST";
    case RADIUS_CODE::ACCESS_ACCEPT:
        return "ACCESS_ACCEPT";
    case RADIUS_CODE::ACCESS_REJECT:
        return "ACCESS_REJECT";
    case RADIUS_CODE::ACCOUNTING_REQUEST:
        return "ACCOUNTING_REQUEST";
    case RADIUS_CODE::ACCOUNTING_RESPONSE:
        return "ACCOUNTING_RESPONSE";
    case RADIUS_CODE::ACCESS_CHALLENGE:
        return "ACCESS_CHALLENGE";
    default:
        break;
    }
    return {};
}

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
        receive();
	}

    void receive() {
        socket_.async_receive_from( boost::asio::buffer( buf, buf.size() ), endpoint_, std::bind( &UDPClient::on_rcv, this, std::placeholders::_1, std::placeholders::_2 ) );
    }

    void on_rcv( boost::system::error_code ec, size_t size ) {
        if( ec ) {
            std::cout << ec.message() << std::endl;
        }
        auto pkt = reinterpret_cast<Packet*>( buf.data() );
        std::cout << pkt->to_string() << std::endl;
    }

private:
    std::array<uint8_t,1500> buf;
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

    io.run();

    return 0;
}