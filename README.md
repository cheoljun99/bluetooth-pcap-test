# bluetooth-pcap-test

bluetooth-pcap-test

## Android Debug Bridge 설치

```
    sudo apt update
    sudo apt install adb
```

## adb device 인식 및 실행

```
    adb kill-server
    adb start-server
    adb devices
```

```
find / -name "btsnoop_hci.log"
해당 로그 파일을 pcap으로 변환하여 와이어샤크에서 열게되면 패킷을 분석할 수 있다.
하지만 실시간 데이터가 아니므로 실시간으로 패킷을 분석하기 위해서는
btsnoop_hci.log를 와이어샤크에 tail로 연결하여 실시간으로 log파일에 추가되는 데이터를 와이어샤크로 분석할 수 있다.
이점을 이용해서 만약 해당 로그파일에 실시간으로 pcap같은 것을 통해 열게 된다면 패킷 캡쳐 코드를 작성하여
내가 원하는 패킷을 스니핑할 수 있을 것이라고 생각했다. 하지만 해당 로그파일이 루팅을 하지 않으면 접근 권한 문제로 접근에
문제가 있었다.
```

```
BTSnoop HCI 로그 데이터를 androiddump가 ADB를 통해 실시간으로 Wireshark에 스트리밍(piping)하여,
Wireshark가 이를 인터페이스로 표시하는 방식을 사용하고자한다.

Bluetooth HCI Snoop Log 활성화:
	- 안드로이드 기기에서 HCI 데이터를 캡처할 수 있도록 설정.
ADB 연결:
	- PC에서 ADB를 통해 디바이스와 연결.
AndroidDump 실행:
	- androiddump가 ADB를 통해 BTSnoop HCI 데이터를 요청. 데이터를 Wireshark로 스트리밍.
Wireshark 인터페이스 표시:
	- Wireshark가 데이터를 인터페이스로 등록하고 실시간 캡처 시작.


androiddump가 와이어샤크 설치 폴더에 실행파일이 존재하므로 환경 변수 조작

nano ~/.bashrc // 편집기 오픈
export PATH=$PATH:/usr/lib/x86_64-linux-gnu/wireshark/extcap // 마지막 줄에 추가
source ~/.bashrc // 설정 적용
androiddump --extcap-interfaces // 아무 디렉토리에서 실행해서 테스트


sudo strace -f -e execve wireshark 2>&1 | grep androiddump
//와이어샤크에서 어떻게 파이프를 사용해서 pcap 파일을 연결하는 지 확인
sudo strace -f -s 500 -e execve wireshark 2>&1 | grep androiddump
// 내용 많이 나옴


[pid 16187] execve("/usr/lib/x86_64-linux-gnu/wireshark/extcap/androiddump", ["/usr/lib/x86_64-linux-gnu/wiresh"..., "--capture", "--extcap-interface", "android-bluetooth-btsnoop-net-R5"..., "--fifo", "/tmp/wireshark_extcap_android-bl"..., "--adb-server-ip", "127.0.0.1", "--adb-server-tcp-port", "5037"], 0x7fff7f647688 /* 27 vars */) = 0

/usr/lib/x86_64-linux-gnu/wireshark/extcap/androiddump \
--capture \
--extcap-interface android-bluetooth-btsnoop-net-R5... \
--fifo /tmp/wireshark_extcap_android-bl... \
--adb-server-ip 127.0.0.1 \
--adb-server-tcp-port 5037

좀 수정해서

androiddump --extcap-interface android-bluetooth-btsnoop-net-R5CT22G815P \
--adb-server-ip 127.0.0.1 --adb-server-tcp-port 5037 \
--capture --fifo bluetooth-pcap-test/bluetooth.pcap (이때는 pcap파일)

를 통해 실시간으로 패킷을 주고 받는 내용이 담기는 pcap 파일 생성

이 방법 좀 아쉬운게 실시간으로 pcap파일 내용이 변하기는 하지만 코드에서 읽을 때
pcap_open_offline를 사용해야해서 파일의 끝에 도달하면 갱신이되지 않고 종료됨

해결법으로


mkfifo bluetooth.pcap (.pcap파일 아님 파이프 파일임)

wireshark -k -i bluetooth.pcap (이건 파이프 파일을 강제로 wireshark로 읽을 때)

즉 파이프 파일을 하나 생성하고 실시간으로 read를 할 경우 데이터가 파이프 파일에 들어올 경우


androiddump --extcap-interface android-bluetooth-btsnoop-net-R5CT219X7XY \
--adb-server-ip 127.0.0.1 --adb-server-tcp-port 5037 \
--capture --fifo bluetooth-pcap-test/bluetooth.pcap


androiddump --extcap-interface android-bluetooth-btsnoop-net-R5CT219X7XY \
--adb-server-ip 127.0.0.1 --adb-server-tcp-port 5037 \
--capture --fifo bluetooth.pcap
```

- 파이프 파일이란
  파이프 파일은 유닉스 기반 시스템에서 프로세스 간 통신(IPC, Inter-Process Communication)을 위해 사용되는 특수 파일이다. `mkfifo` 명령어를 통해 생성할 수 있으며, 일반 파일과 달리 데이터가 영구적으로 저장되지 않고 프로세스 간의 데이터 전달을 위한 일시적인 경로 역할을 한다.

  ### 파이프 파일의 특징

  1. **특수 파일**: 일반 파일과는 달리 디스크에 저장된 데이터를 갖지 않으며, 커널에서 관리하는 버퍼를 통해 데이터가 전달된다.
  2. **단방향 데이터 스트림**: 기본적으로 한쪽 프로세스가 데이터를 쓰면 다른 쪽 프로세스가 이를 읽는다.
  3. **차단(Blocking)**: 파이프 파일은 기본적으로 차단 모드로 동작하며, 데이터가 쓰이거나 읽히지 않으면 해당 작업이 완료될 때까지 프로세스가 대기한다.

  ### `androiddump --extcap-interface`와 파이프 파일의 관계

  `androiddump --extcap-interface`는 외부 캡처 인터페이스를 통해 실시간으로 네트워크 패킷 데이터를 생성한다. 이 데이터를 파이프 파일로 출력하면, `pcap_open_offline`으로 해당 파이프 파일을 읽어 들일 수 있다.

  ### `pcap_open_offline`이 작동하는 이유

  `pcap_open_offline` 함수는 일반적으로 파일에서 데이터를 읽기 위해 사용된다. 하지만, 파이프 파일을 입력으로 제공하면 다음과 같은 이유로 실시간 처리가 가능하다.

  1. **EOF(End of File) 처리**:
     - 파이프 파일에서는 데이터가 계속 쓰여지는 한, 읽는 프로세스는 EOF에 도달하지 않는다.
     - 쓰는 프로세스가 종료하거나 쓰기를 중단하면 읽는 프로세스는 EOF를 만나게 된다.
  2. **실시간 스트리밍**:
     - `androiddump`가 데이터를 지속적으로 생성하여 파이프 파일에 쓰기 때문에 `pcap_open_offline`은 이를 끊임없이 읽어들일 수 있다.
     - 내부적으로 `pcap_open_offline`은 입력 스트림에서 데이터를 순차적으로 읽으며, 스트림이 끝나지 않으면 계속 대기 상태를 유지한다.
  3. **파이프의 차단 특성**:
     - 데이터를 쓰는 프로세스와 읽는 프로세스 간의 속도가 다를 경우, 파이프는 자동으로 동기화 역할을 한다.
     - 데이터가 없으면 읽는 프로세스는 대기하며, 새로운 데이터가 쓰여지면 이를 처리한다.

  ### 결론

  파이프 파일은 일반 파일처럼 보이지만, 데이터가 지속적으로 스트리밍되기 때문에 `pcap_open_offline`은 이를 실시간으로 처리할 수 있다. 이는 파이프 파일이 데이터가 계속 쓰여질 때 EOF 상태에 도달하지 않기 때문이다. `androiddump`와 같은 데이터 생성기가 파이프에 데이터를 공급하는 한, 읽는 프로세스는 중단 없이 데이터를 처리할 수 있다.

  - 발견한 패킷 프로토콜 종류

    ### **HCI란?**

    HCI (Host Controller Interface)는 Bluetooth 장치의 **호스트(Host)**와 **컨트롤러(Controller)** 간의 통신을 담당하는 계층이다.

    - **호스트**: 운영체제에서 실행되는 Bluetooth 스택(예: L2CAP, ATT 등).
    - **컨트롤러**: 물리적인 Bluetooth 칩과 RF 하드웨어.

    ### HCI 역할

    - **호스트와 컨트롤러 간 명령 및 이벤트 교환**
      - HCI_CMD: 호스트 → 컨트롤러로 명령 전송.
      - HCI_EVT: 컨트롤러 → 호스트로 이벤트 전달.
    - **L2CAP 데이터를 전달하지 않음**:
      - HCI 자체는 L2CAP 페이로드를 처리하지 않고, 단순히 데이터 전달 경로를 제공한다.
      - L2CAP 데이터는 ACL (Asynchronous Connection-Less) 패킷 형태로 전달된다.

    ### **HCI_CMD: HCI Command**

    ### 정의

    - **호스트가 컨트롤러에 Bluetooth 작업을 요청**할 때 사용하는 명령 메시지.
    - 예: Bluetooth 연결 설정, 스캔 요청, 디바이스 이름 변경 등.

    ### 구조 (간단히)

    | 필드 이름     | 크기     | 설명                       |
    | ------------- | -------- | -------------------------- |
    | Opcode        | 2 바이트 | 명령 코드 (명령 종류 정의) |
    | Parameter Len | 1 바이트 | 추가 매개변수 길이         |
    | Parameters    | 가변     | 명령 매개변수 데이터       |

    ### **HCI_EVT: HCI Event**

    ### 정의

    - **컨트롤러가 호스트에게 특정 이벤트를 알릴 때 사용하는 메시지.**
    - 예: 연결 완료, 스캔 결과, 오류 상태 등.

    ### 구조 (간단히)

    | 필드 이름     | 크기     | 설명                           |
    | ------------- | -------- | ------------------------------ |
    | Event Code    | 1 바이트 | 이벤트 코드 (이벤트 종류 정의) |
    | Parameter Len | 1 바이트 | 추가 매개변수 길이             |
    | Parameters    | 가변     | 이벤트 매개변수 데이터         |

    ### **HCI_CMD와 HCI_EVT의 데이터 전달**

    이들은 데이터 전송보다는 **제어 메시지** 교환에 사용된다. 따라서 L2CAP처럼 데이터를 담아 전송하는 것이 아니라, Bluetooth 컨트롤러와 관련된 상태나 명령을 주고받는 데 사용된다.

    ### **L2CAP과의 연관성**

    - HCI_CMD와 HCI_EVT는 **L2CAP과 직접 연관이 없다.**
    - L2CAP 데이터는 **HCI_ACL**(HCI Asynchronous Connection-Less) 데이터 채널을 통해 전달된다.
    - HCI_CMD와 HCI_EVT는 Bluetooth 장치를 설정하거나 제어할 때 사용되며, 데이터 흐름은 포함되지 않는다.

    ### **HCI_CMD와 HCI_EVT의 활용 예**

    ### Bluetooth 장치를 스캔하고 결과를 처리

    1. **HCI_CMD: Inquiry Command**
       - 호스트가 컨트롤러에 Inquiry 명령(HCI_CMD)을 전송.
       - 컨트롤러는 주변 Bluetooth 장치를 스캔한다.
    2. **HCI_EVT: Inquiry Result Event**
       - 컨트롤러가 주변 장치를 발견하면 HCI_EVT로 스캔 결과를 호스트에 알림.

    ### **HCI_CMD와 HCI_EVT는 데이터를 담지 않는가?**

    ### 데이터의 정의에 따라 다름:

    - HCI_CMD와 HCI_EVT는 데이터 자체(예: 파일, 페이로드)를 전송하지는 않는다.
      - 이들은 제어 메시지를 주고받는 역할을 한다.
    - 하지만 **HCI_EVT의 이벤트 매개변수**에는 특정 장치 정보(예: MAC 주소, RSSI 값)나 연결 상태와 같은 데이터를 포함할 수 있다.

    ### 요약

    | **프로토콜** | **역할**                                            | **데이터 포함 여부**                   | **L2CAP과 연관성**  |
    | ------------ | --------------------------------------------------- | -------------------------------------- | ------------------- |
    | **HCI_CMD**  | 호스트 → 컨트롤러로 명령 전달                       | 제어 데이터 포함 (예: 명령 매개변수)   | 직접적인 연관 없음  |
    | **HCI_EVT**  | 컨트롤러 → 호스트로 이벤트 전달                     | 이벤트 데이터 포함 (예: 상태, 결과 등) | 직접적인 연관 없음  |
    | **L2CAP**    | HCI_ACL을 통해 상위 프로토콜 데이터(ATT, OBEX) 전달 | 페이로드 데이터 포함                   | HCI_ACL 위에서 동작 |

- L2CAP 프로토콜과 상위 프로토콜의 역할 및 개요
  ### 1. **L2CAP의 역할**
  L2CAP(Logical Link Control and Adaptation Protocol)은 Bluetooth 프로토콜 스택에서 **데이터 링크 계층**의 일부로 동작하며, Bluetooth 장치 간 데이터를 **다중화(Multiplexing)**하고 **상위 계층으로 전달**하는 중요한 역할을 한다.
  ### 주요 기능
  1. **데이터 다중화(Multiplexing)**:
     - 여러 상위 프로토콜(예: ATT, OBEX 등)의 데이터를 CID(Channel Identifier)를 통해 구분하고 처리.
  2. **데이터 프래그멘테이션(Fragmentation) 및 재조립(Reassembly)**:
     - 데이터가 Bluetooth 연결의 MTU(Maximum Transmission Unit)를 초과하면 데이터를 조각(Fragment)으로 나눠 전송.
     - 수신 측에서는 조각을 재조립하여 상위 계층으로 전달.
  3. **QoS 및 흐름 제어 지원**:
     - 실시간 데이터를 다룰 때 품질 보증(Quality of Service)을 위한 메커니즘 제공.
  4. **상위 프로토콜 데이터 전달**:
     - 상위 프로토콜의 데이터(페이로드)를 캡슐화하고 전송.
  ***
  ### L2CAP 데이터 구조
  L2CAP 데이터는 다음과 같은 구조를 가진다:
  | 필드    | 크기     | 설명                            |
  | ------- | -------- | ------------------------------- |
  | Length  | 2 바이트 | 페이로드 크기                   |
  | CID     | 2 바이트 | 채널 식별자(Channel Identifier) |
  | Payload | 가변     | 상위 프로토콜 데이터 (페이로드) |
  ***
  ### 2. **L2CAP 위의 상위 프로토콜**
  L2CAP은 상위 프로토콜 데이터를 캡슐화하여 전송하며, CID를 통해 각 프로토콜 데이터를 구분한다.
  ### 주요 상위 프로토콜
  1. **ATT (Attribute Protocol)**:
     - Bluetooth Low Energy(BLE) 환경에서 사용.
     - GATT(Generic Attribute Profile)의 기반이 되는 프로토콜.
     - **역할**:
       - 장치 간 Attribute(속성) 데이터를 요청하고 응답.
       - 예: 센서 데이터, 상태 정보 전송.
  2. **OBEX (Object Exchange Protocol)**:
     - 파일 전송 및 데이터 동기화에 사용.
     - Bluetooth 프로파일 중 OPP(Object Push Profile)와 FTP(File Transfer Profile)에서 활용.
     - **역할**:
       - 장치 간 파일, 메시지 등 객체(Object) 교환.
  3. **RFCOMM (Radio Frequency Communication)**:
     - 시리얼 포트 에뮬레이션 프로토콜.
     - Bluetooth 프로파일 중 SPP(Serial Port Profile)에서 사용.
     - **역할**:
       - 데이터 스트림 전송, 예: Bluetooth 시리얼 연결.
  4. **SDP (Service Discovery Protocol)**:
     - Bluetooth 장치의 서비스 검색.
     - **역할**:
       - Bluetooth 장치가 제공하는 프로파일과 서비스 정보를 탐색.
  5. **AVDTP (Audio/Video Distribution Transport Protocol)**:
     - 오디오 및 비디오 데이터 스트리밍.
     - A2DP(Advanced Audio Distribution Profile) 기반.
     - **역할**:
       - 고품질 오디오 및 비디오 스트리밍 지원.
  6. **AVCTP (Audio/Video Control Transport Protocol)**:
     - 오디오 및 비디오 기기 제어.
     - 예: Bluetooth 리모컨.
     - **역할**:
       - 기기 간 재생, 정지, 볼륨 제어 등 지원.
  ***
  ### 3. **상위 프로토콜의 주요 역할 요약**
  | 프로토콜   | 역할                                  | 예시                                  |
  | ---------- | ------------------------------------- | ------------------------------------- |
  | **ATT**    | Attribute 데이터 요청/응답 (BLE 기반) | 센서 데이터 전송, GATT 기반 서비스    |
  | **OBEX**   | 파일 및 데이터 전송                   | 파일 전송, 연락처 교환                |
  | **RFCOMM** | 시리얼 데이터 스트림 전송             | Bluetooth 시리얼 포트                 |
  | **SDP**    | 장치의 서비스 탐색                    | 장치가 지원하는 프로파일 검색         |
  | **AVDTP**  | 오디오/비디오 데이터 스트리밍         | 무선 스피커, 헤드셋                   |
  | **AVCTP**  | 오디오/비디오 기기 제어               | Bluetooth 리모컨, 재생/정지/볼륨 제어 |
  ***
  ### 4. **L2CAP과 상위 프로토콜의 관계**
  L2CAP은 **상위 프로토콜 데이터를 전달하는 통로** 역할을 한다. CID 값을 기반으로 상위 프로토콜을 구분하며, 상위 프로토콜의 데이터는 L2CAP 페이로드에 포함된다.
  ### 관계 예시
  - **ATT 데이터 흐름**:
    - ATT는 L2CAP 위에서 동작하며, CID `0x0040`으로 데이터 구분.
    - L2CAP은 ATT 데이터를 캡슐화하여 전송.
  - **OBEX 데이터 흐름**:
    - OBEX는 L2CAP 위에서 동작하며, CID `0x0003`으로 데이터 구분.
    - 파일 전송 시 OBEX 데이터를 L2CAP 페이로드에 포함.
  ***
  ### 5. **L2CAP과 상위 프로토콜 간 데이터 흐름 예**
  ### 데이터 전송 흐름
  1. **HCI**:
     - Bluetooth 컨트롤러와 호스트 간 데이터 전달.
  2. **L2CAP**:
     - CID로 상위 프로토콜 데이터 구분.
     - 데이터 프래그멘테이션 및 재조립.
  3. **상위 프로토콜 (예: ATT)**:
     - L2CAP 페이로드를 해석하여 실제 데이터를 처리.
  ### 흐름 예제
  - 장치 A가 장치 B로 센서 데이터를 전송하는 경우:
    1. **ATT**: 센서 데이터(예: 온도)를 Attribute로 캡슐화.
    2. **L2CAP**: ATT 데이터를 CID `0x0040`으로 캡슐화.
    3. **HCI**: L2CAP 데이터를 물리 계층으로 전송.
  ***
  ### 6. **L2CAP과 상위 프로토콜 요약**
  | **L2CAP**의 역할                       | **상위 프로토콜**의 역할                      |
  | -------------------------------------- | --------------------------------------------- |
  | 데이터 다중화 및 프로토콜 구분         | 특정 기능(파일 전송, 데이터 스트림 등) 구현   |
  | CID를 통해 상위 프로토콜 데이터 전달   | 프로토콜별 규격에 따라 데이터 처리            |
  | 페이로드 캡슐화 및 프래그멘테이션 처리 | 애플리케이션 또는 서비스 수준에서 데이터 활용 |
  ***
  ### 결론
  L2CAP은 Bluetooth 프로토콜에서 **데이터 전달 및 다중화 계층**으로, 상위 프로토콜 데이터를 전송하기 위한 기반을 제공한다. ATT, OBEX, RFCOMM 등은 L2CAP 위에서 동작하며, 각각의 프로토콜은 Bluetooth 장치 간 다양한 기능(예: 센서 데이터 교환, 파일 전송)을 구현한다. L2CAP과 상위 프로토콜의 역할은 서로 명확히 구분되며, 이러한 설계를 통해 Bluetooth는 다양한 서비스와 프로파일을 지원할 수 있다.
