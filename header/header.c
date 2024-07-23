#include <stdio.h>
#include <pcap.h>
#include <arpa/inet.h>
#include "structs.h"

void caught_packet(unsigned char *options,const struct pcap_pkthdr *header,const unsigned char *packet);
void decode_ethernet(const unsigned char *packet);
void decode_ip(const unsigned char *packet);
void decode_tcp(const unsigned char *packet);

int main(){

	char *device;
	char errbuf[PCAP_ERRBUF_SIZE];
	pcap_t *pcap_handle;

	if((device = pcap_lookupdev(errbuf)) == NULL){
		fprintf(stderr,"%s\n",errbuf);
		return -1;
	}

	if((pcap_handle = pcap_open_live(device,PACKET_SIZE,PROMISC,TIMEOUT,errbuf)) == NULL){
		fprintf(stderr,"%s\n",errbuf);
		return -1;
	}

	pcap_loop(pcap_handle,NUMBER_OF_PACKETS,caught_packet,NULL);

	pcap_close(pcap_handle);


	return 0;
}

void caught_packet(unsigned char *options,const struct pcap_pkthdr *header,const unsigned char *packet){

	printf("Received a %d length packet\n",header->len);

	decode_ethernet(packet);
	decode_ip(packet + ETH_PACKET_LEN);
	decode_tcp(packet + ETH_PACKET_LEN + sizeof(struct ip_header));

	printf("\n\n");
}


void decode_ethernet(const unsigned char *packet){

	const struct ethernet_header *header;
	int i;

	header = (struct ethernet_header*)packet;

	printf("[[  Ethernet Header ]]\n");

	printf("[[ Destination mac address : %02x",header->destination_address[0]);

	for(i = 1;i < ETH_ALEN;i++)
		printf(":%02x",header->destination_address[i]);

	printf("  ]]\t");
	printf("[[  Source mac address : %02x",header->source_address[0]);

	for(i = 1;i < ETH_ALEN;i++)
		printf(":%02x",header->source_address[i]);

	printf("  ]]\n");
}

void decode_ip(const unsigned char *packet){

	const struct ip_header *header;

	header = (struct ip_header*)packet;

	struct in_addr source_address;
	struct in_addr destination_address;

	source_address.s_addr = (long)(header->source_address);
	destination_address.s_addr = (long)(header->destination_address);

	printf("((  IP Header  ))\n");
	printf("((  Destination ip address : %s  ))",inet_ntoa(destination_address));
	printf("\t");
	printf("((  Source ip address : %s  ))",inet_ntoa(source_address));
	printf("\n");

}

void decode_tcp(const unsigned char *packet){

	const struct tcp_header *header;

	header = (struct tcp_header*)packet;

	printf("{{  TCP Header }}\n");
	printf("{{  Destination port : %d  }}",ntohs(header->destination_port));
	printf("\t");
	printf("{{  Source port : %d  }}",ntohs(header->source_port));
	printf("\t");
	printf("{{  Flags : ");

	if(header->flags & FIN)
		printf("FIN ");

	if(header->flags & SYN)
		printf("SYN ");

	if(header->flags & RST)
		printf("RST ");

	if(header->flags & PUSH)
		printf("PUSH ");

	if(header->flags & ACK)
		printf("ACK ");

	if(header->flags & URG)
		printf("URG ");

	printf(" }}");

}
