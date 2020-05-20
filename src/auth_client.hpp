#ifndef AUTH_CLIENT_HPP
#define AUTH_CLIENT_HPP

using ResponseHandler = std::function<void( std::vector<uint8_t> )>;

template<typename T>
std::vector<uint8_t> serialize( const RadiusDict &dict, const T &v, const authenticator_t &a, const std::string &secret );

template<typename T>
T deserialize( const RadiusDict &dict, std::vector<uint8_t> &v );

struct response_t {
    ResponseHandler response;
    authenticator_t auth;

    response_t( ResponseHandler r, authenticator_t a ):
        response( std::move( r ) ),
        auth( std::move( a ) )
    {}
};

class AuthClient
{
public:
	AuthClient( io_service& i, const address_v4& ip_address, uint16_t port, std::string s, RadiusDict d );
	~AuthClient();

    template<typename T>
    void request( const T &req, ResponseHandler handler ) {
        last_id++;
        std::vector<uint8_t> pkt;
        pkt.resize( sizeof( RadiusPacket ) );
        auto pkt_hdr = reinterpret_cast<RadiusPacket*>( pkt.data() );
        pkt_hdr->code = RADIUS_CODE::ACCESS_REQUEST;
        pkt_hdr->id = last_id;
        pkt_hdr->authenticator = generateAuthenticator();

        auto seravp = serialize( dict, req, pkt_hdr->authenticator, secret );
        pkt.insert( pkt.end(), seravp.begin(), seravp.end() );

        pkt_hdr = reinterpret_cast<RadiusPacket*>( pkt.data() );
        pkt_hdr->length = pkt.size();

        callbacks.emplace( std::piecewise_construct, std::forward_as_tuple( last_id ), std::forward_as_tuple( std::move( handler ), pkt_hdr->authenticator ) );
        send( pkt );
    }

private:
    void on_rcv( boost::system::error_code ec, size_t size );
    bool checkRadiusAnswer( const authenticator_t &req_auth, const authenticator_t &res_auth, const std::vector<uint8_t> &avp );
	void send( const std::vector<uint8_t> &msg );
    void receive();

    uint8_t last_id;
    RadiusDict dict;
    std::string secret;
    std::map<uint8_t,response_t> callbacks;
    std::array<uint8_t,1500> buf;
	io_service &io;
	udp::socket socket;
	udp::endpoint endpoint;
};

#endif