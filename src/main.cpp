#include "main.hpp"

using boost::asio::ip::udp;
using io_service = boost::asio::io_service;
using RadiusResponseHandler = std::function<void( RadiusResponse )>;

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
	UDPClient( io_service& io_service, const address_v4& ip_address, uint16_t port, RadiusDict d ): 
        io_service_( io_service ), 
        socket_( io_service, udp::endpoint(udp::v4(), 0) ),
        endpoint_( ip_address, port ),
        dict( std::move( d ) )
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
        RadiusResponse res;
        auto pkt = reinterpret_cast<Packet*>( buf.data() );
        std::cout << pkt->to_string() << std::endl;

        std::vector<uint8_t> avp_buf { buf.begin() + sizeof( Packet), buf.begin() + pkt->length.native() - sizeof( Packet ) };

        auto avp_set = parseAVP( avp_buf );
        for( auto const &avp: avp_set ) {
            auto const &attr = dict.getAttrById( avp.type ); 
            if( attr.first == "Framed-IP-Address" ) {
                if( auto const &[ ip, success ] = avp.getVal<BE32>(); success ) {
                    res.framed_ip == address_v4{ ip.native() };
                } 
            }
        }
        if( auto const &it = callbacks.find( pkt->id ); it != callbacks.end() ) {
            it->second( res );
        }
    }

    void request( const RadiusRequest &req, RadiusResponseHandler handler ) {
        last_id++;
        std::vector<uint8_t> pkt;
        pkt.resize( sizeof( Packet ) );
        auto pkt_hdr = reinterpret_cast<Packet*>( pkt.data() );
        pkt_hdr->code = RADIUS_CODE::ACCESS_REQUEST;
        pkt_hdr->id = last_id;
        //pkt_hdr->authenticator = { 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10 };
        pkt_hdr->authenticator = generateAuthenticator();

        std::set<AVP> avp_set { 
            AVP { dict, "User-Name", req.username },
            AVP { dict, "User-Password", req.password }
        };

        auto seravp = serializeAVP( avp_set );
        pkt.insert( pkt.end(), seravp.begin(), seravp.end() );

        pkt_hdr = reinterpret_cast<Packet*>( pkt.data() );
        pkt_hdr->length = pkt.size();

        callbacks.emplace( last_id, std::move( handler ) );
        send( pkt );
    }

private:
    RadiusDict dict;
    uint8_t last_id;
    std::map<uint8_t,RadiusResponseHandler> callbacks;
    std::array<uint8_t,1500> buf;
	boost::asio::io_service& io_service_;
	udp::socket socket_;
	udp::endpoint endpoint_;
};

void on_res( RadiusResponse res ) {
    std::cout << "Response: " << res.framed_ip.to_string() << std::endl;
}

int main( int argc, char* argv[] ) {
    auto main_dict = RadiusDict( "/usr/share/freeradius/dictionary.rfc2865" );

    RadiusRequest req;
    req.username = "zstas";
    req.password = "12345";

    io_service io;
    UDPClient udp( io, address_v4::from_string( "127.0.0.1" ), 1812, main_dict );
    udp.request( req, on_res );

    io.run();

    return 0;
}