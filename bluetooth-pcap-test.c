#include <stdio.h>
#include <stdint.h>
#include <pcap.h>
#include <arpa/inet.h> // For ntohs, htons
#include <unistd.h>
#include <ctype.h> // for isprint

// L2CAP 헤더 구조 (Length + CID)
typedef struct {
    uint16_t length;   // Length of payload
    uint16_t cid;      // Channel ID
    uint8_t payload[]; // Variable length payload 가변
} l2cap_packet_t;

// L2CAP 데이터 처리 함수 (OBEX 또는 ATT로 분기)
void process_l2cap(const uint8_t *data, uint32_t length) {
    if (length < 4) { // 최소 L2CAP 헤더 크기
        printf("L2CAP: Insufficient length\n");
        return;
    }

    l2cap_packet_t *l2cap = (l2cap_packet_t *)data;
    uint16_t cid = ntohs(l2cap->cid); // L2CAP Channel ID

    // L2CAP 페이로드 추출
    const uint8_t *payload = l2cap->payload;
    uint32_t payload_length = length - 4; // L2CAP 헤더 크기(4바이트) 제외

    // 페이로드 출력 (16진수)
    printf("L2CAP Data (CID: 0x%04X) Hex: ", cid);
    for (uint32_t i = 0; i < payload_length; i++) {
        printf("%02X ", payload[i]);
    }
    printf("\n");

    // 페이로드 출력 (스트링)
    printf("L2CAP Data (CID: 0x%04X) String: ", cid);
    for (uint32_t i = 0; i < payload_length; i++) {
        if (isprint(payload[i])) {
            putchar(payload[i]); // 출력 가능한 문자
        } else {
            putchar('.'); // 비 ASCII 문자는 .으로 대체
        }
    }
    printf("\n");
}

// HCI 패킷 타입 검사 및 처리

int count;

void process_packet(const uint8_t *data, uint32_t length) {


    
    if (length < 5) { // 최소 패킷 길이 확인
        printf("Packet too short to determine type\n");
        return;
    }

    // 데이터 오프셋 조정 (앞의 4바이트 스킵)
    //Raw Packet Data: 00 00 00 01 04 여기서 보면 04앞에 00 00 00 01이 자꾸 들어감
    const uint8_t *adjusted_data = data + 4;
    uint8_t packet_type = adjusted_data[0];
    count++;

    if (packet_type == 0x02) { // HCI_ACL 데이터만 처리
        printf("HCI_ACL Packet Detected and count : %d\n",count);

    	// HCI_ACL 헤더 (4 bytes) 제거 후 L2CAP 데이터 처리
    	const uint8_t *l2cap_data = adjusted_data + 4; // HCI_ACL 헤더 크기(4바이트 제거)
    	uint32_t l2cap_length = length - 8; // 앞의 4바이트와 HCI_ACL 헤더(4바이트) 제거

        if (l2cap_length >= 4) { // L2CAP 최소 길이 확인
            process_l2cap(l2cap_data, l2cap_length);
        } else {
            printf("HCI_ACL Packet Detected but insufficient L2CAP data length.\n");
        }
    } 
    /*
    else {
        printf("Non-L2CAP Packet (Type: 0x%02X) Skipped and count : %d\n", packet_type,count);
    }*/
    
}

// main 함수
int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <fifo pcap file>\n", argv[0]);
        return -1;
    }

    const char *fifo_file = argv[1];
    char errbuf[PCAP_ERRBUF_SIZE];

    // pcap 파일 열기
    pcap_t *handle = pcap_open_offline(fifo_file, errbuf);
    if (!handle) {
        fprintf(stderr, "Error opening pcap file: %s\n", errbuf);
        return -1;
    }

    struct pcap_pkthdr *header;
    const uint8_t *data;

    printf("Listening for new packets in: %s\n", fifo_file);

    // 실시간 파일 읽기 루프
    while (1) {
        int ret = pcap_next_ex(handle, &header, &data);
        if (ret == 1) { // 패킷 성공적으로 읽음
            process_packet(data, header->caplen);
        } else if (ret == PCAP_ERROR_BREAK || ret == 0) { // EOF 또는 타임아웃
           break;
        } else if (ret == PCAP_ERROR) { // 에러 발생
            fprintf(stderr, "Error reading packet: %s\n", pcap_geterr(handle));
            break;
        }
    }

    pcap_close(handle);
    return 0;
}

