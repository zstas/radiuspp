#include "main.hpp"

using boost::asio::ip::udp;
using io_service = boost::asio::io_service;
using RadiusResponseHandler = std::function<void( RadiusResponse )>;

struct response_t {
    RadiusResponseHandler response;
    authenticator_t auth;

    response_t( RadiusResponseHandler r, authenticator_t a ):
        response( std::move( r ) ),
        auth( std::move( a ) )
    {}
};

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
	UDPClient( io_service& io_service, const address_v4& ip_address, uint16_t port, std::string s, RadiusDict d ): 
        io_service_( io_service ), 
        socket_( io_service, udp::endpoint(udp::v4(), 0) ),
        endpoint_( ip_address, port ),
        secret( std::move( s )),
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

    // Incoming packet should be in buf
    bool checkRadiusAnswer( const authenticator_t &req_auth, const authenticator_t &res_auth, const std::vector<uint8_t> &avp ) {
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

    void on_rcv( boost::system::error_code ec, size_t size ) {
        if( ec ) {
            std::cerr << ec.message() << std::endl;
        }

        RadiusResponse res;
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

        auto avp_set = parseAVP( avp_buf );
        for( auto const &avp: avp_set ) {
            auto const &attr = dict.getAttrById( avp.type ); 
            if( attr.first == "Framed-IP-Address" ) {
                if( auto const &[ ip, success ] = avp.getVal<BE32>(); success ) {
                    res.framed_ip = address_v4{ ip.native() };
                } 
            }
        }

        it->second.response( res );
    }

    void request( const RadiusRequest &req, RadiusResponseHandler handler ) {
        last_id++;
        std::vector<uint8_t> pkt;
        pkt.resize( sizeof( Packet ) );
        auto pkt_hdr = reinterpret_cast<Packet*>( pkt.data() );
        pkt_hdr->code = RADIUS_CODE::ACCESS_REQUEST;
        pkt_hdr->id = last_id;
        pkt_hdr->authenticator = generateAuthenticator();

        auto encrypted_pass = password_pap_process( pkt_hdr->authenticator, secret, req.password );

        std::set<AVP> avp_set { 
            AVP { dict, "User-Name", req.username },
            AVP { dict, "User-Password", encrypted_pass }
        };

        auto seravp = serializeAVP( avp_set );
        pkt.insert( pkt.end(), seravp.begin(), seravp.end() );

        pkt_hdr = reinterpret_cast<Packet*>( pkt.data() );
        pkt_hdr->length = pkt.size();

        callbacks.emplace( std::piecewise_construct, std::forward_as_tuple( last_id ), std::forward_as_tuple( std::move( handler ), pkt_hdr->authenticator ) );
        send( pkt );
    }

private:
    RadiusDict dict;
    std::string secret;
    uint8_t last_id;
    std::map<uint8_t,response_t> callbacks;
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
    req.password = "1234567890123456789012345";

    io_service io;
    UDPClient udp( io, address_v4::from_string( "127.0.0.1" ), 1812, "testing123", main_dict );
    udp.request( req, on_res );

    io.run();

    return 0;
}