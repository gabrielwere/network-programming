#define ETH_ALEN 6 //ethernet address length
#define ETH_PACKET_LEN 14 //ethernet packet length

#define NUMBER_OF_PACKETS 5
#define PACKET_SIZE 4096
#define PROMISC 1
#define TIMEOUT 0

//TCP FLAGS
#define FIN 0x01
#define SYN 0x02
#define RST 0x04
#define PUSH 0x08
#define ACK 0x10
#define URG 0x20

struct ethernet_header{

	unsigned char destination_address[ETH_ALEN];
	unsigned char source_address[ETH_ALEN];
	unsigned short packet_type;

};

struct ip_header{

	unsigned char version_and_length;
	unsigned char tos;
	unsigned short tol;
	unsigned short id;
	unsigned short frag_off;
	unsigned char ttl;
	unsigned char protocol;
	unsigned short checksum;
	unsigned int source_address;
	unsigned int destination_address;

};

struct tcp_header{

	unsigned short source_port;
	unsigned short destination_port;
	unsigned int seq_number;
	unsigned int ack_number;
	unsigned char data_offset;
	unsigned char flags;
	unsigned short window;
	unsigned short checksum;
	unsigned short urgent_pointer;

};


