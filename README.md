# Embedded Software and Smart Mobility Project #1
## Character Device Driver
### Problem 
- Stack을 활용하여 Character Device Driver를 만들고 이를 테스트하기 위해 driver를 활용하는 application을 구현하라.
- Stack의 크기는 256이고 application은 쓰여진, 읽을 데이터의 순서를 확인하는 방식으로 동작한다.
### Implementation
- dummmy_driver.c
  - Instack(), Destack() : Stack에 데이터를 넣고 빼는 코드 구현. Return 조건에 맞게 코드를 구현해야 함.
  - dummy_read() : 사용자가 디바이스 파일로부터 데이터를 읽으려 할 때 호출한다. 스택에서 데이터를 읽어 사용자 공간으로 데이터를 전송한다.
  - dummy_write() : 사용자가 디바이스 파일에 데이터를 쓰려 할 때 호출한다. 사용자로부터 데이터를 받아 스택에 저장한다.
- application.c
  - main() : wbuf에 자신의 이름과 학번을 할당할 것.
- Makefile
  - default와 clean macro를 포함하는 Makefile을 구현할 것.
### Main Function
dummy_init: 모듈이 로드될 때 호출되는 함수입니다. 디바이스를 초기화하고, 디바이스 파일을 생성합니다.
dummy_exit: 모듈이 제거될 때 호출되는 함수입니다. 디바이스 파일을 제거하고, 디바이스를 정리합니다.
dummy_open: 사용자가 디바이스 파일을 열 때 호출됩니다.
dummy_read: 사용자가 디바이스 파일로부터 데이터를 읽으려 할 때 호출됩니다. 스택에서 데이터를 읽어 사용자 공간으로 데이터를 전송합니다.
dummy_write: 사용자가 디바이스 파일에 데이터를 쓰려 할 때 호출됩니다. 사용자로부터 데이터를 받아 스택에 저장합니다.
dummy_release: 사용자가 디바이스 파일을 닫을 때 호출됩니다.
dummy_ioctl: 사용자 공간에서 특정 명령을 디바이스 드라이버에 전달할 때 사용됩니다. 여기서는 스택을 초기화하는 명령을 처리합니다.
dummy_clean: 스택의 모든 요소를 삭제하고 초기 상태로 되돌립니다.
### Logic
#### dummy_driver.c
1. 초기화 및 종료: dummy_init 함수에서는 디바이스를 등록하고 스택을 초기화한다. dummy_exit 함수에서는 디바이스의 등록을 해제한다.
2. 스택 구조체: ST_t 구조체는 스택을 구현하기 위해 사용된다. head, tail, 그리고 item 배열을 포함하고 있으며 여기에서는 배열을 사용하여 간단하게 스택을 구현한다.
3. 데이터 전송: dummy_read와 dummy_write 함수에서는 copy_to_user와 copy_from_user 함수를 사용해 커널 공간과 사용자 공간 사이의 데이터를 전송한다. 이 과정에서 스택의 상태(가득 찼는지, 비었는지)도 확인해야 한다.
  - copy_to_user
    unsigned long copy_to_user(void __user *to, const void *from, unsigned long n);
      to: 사용자 공간의 목적지 주소를 가리킵니다.
      from: 커널 공간의 소스 데이터 주소를 가리킵니다.
      n: 복사할 데이터의 바이트 수입니다.
  - copy_from_user
    unsigned long copy_from_user(void *to, const void __user *from, unsigned long n);
      to: 커널 공간의 목적지 주소를 가리킵니다.
      from: 사용자 공간의 소스 데이터 주소를 가리킵니다.
      n: 복사할 데이터의 바이트 수입니다.
  - 두 함수 모두 복사되지 않은 바이트 수를 반환하기 때문에 완벽하게 복사가 성공했다면 0을 반환하게 된다. 사용자 공간의 포인터가 유효하지 않아 접근이 불가능하다면 복사가 실패하기 때문에 복사되지 않은 바이트 수를 반환한다.
  - dummy_read는 stack에서 값을 꺼내어 사용자에게 전달하고, dummy_write는 사용자로부터 받아와서 stack에 값을 넣는다.

stack_buffer: 이 변수는 장치 드라이버에서 사용되는 스택 구조체를 참조합니다. 스택은 후입선출(LIFO, Last In First Out) 방식의 자료 구조로, 여기서는 장치 드라이버가 사용자로부터 데이터를 받아 저장하거나 사용자에게 데이터를 전달할 때 중간 저장소의 역할을 합니다. stack_buffer는 이 스택의 상태(예: 현재 스택에 저장된 데이터의 개수나 스택의 인덱스 위치 등)를 관리합니다.

device_buf: 이 버퍼는 커널 공간에서 동적으로 할당된 메모리 영역입니다. kmalloc을 통해 메모리가 할당되며, 이 공간은 사용자 공간으로부터 데이터를 받아 스택에 저장하기 전(dummy_write 함수에서) 또는 스택에서 데이터를 꺼내 사용자 공간으로 전달하기 전(dummy_read 함수에서) 임시 저장소 역할을 합니다. dummy_write 함수에서는 사용자로부터 받은 데이터를 device_buf에 저장한 다음, Instack 함수를 통해 이 데이터를 stack_buffer에 저장합니다. 반대로, dummy_read 함수에서는 Destack 함수를 사용하여 stack_buffer에서 데이터를 꺼내 device_buf에 저장한 후, 이를 사용자 공간으로 전달합니다.

buffer: 이 변수는 사용자 공간의 메모리 주소를 가리킵니다. dummy_read 함수에서는 copy_to_user를 통해 device_buf에서 이 buffer로 데이터를 복사하여 사용자에게 데이터를 전달합니다. 반대로, dummy_write 함수에서는 copy_from_user를 통해 사용자 공간의 이 buffer에서 데이터를 device_buf로 복사하여 커널 공간으로 가져옵니다.

요약하자면, stack_buffer는 스택의 상태를 관리하는 구조체, device_buf는 커널 공간과 사용자 공간 사이에서 데이터를 임시로 저장하는 버퍼, 그리고 buffer는 사용자 공간의 메모리 주소를 참조하는 변수입니다. dummy_write 함수에서는 buffer로부터 데이터를 device_buf로 가져온 다음, 그 데이터를 stack_buffer에 저장합니다. dummy_read 함수에서는 stack_buffer에서 데이터를 device_buf로 가져온 다음, 그 데이터를 다시 buffer로 전달하여 사용자에게 데이터를 제공합니다.

4. 디바이스 파일: 이 드라이버는 /dev 디렉토리에 디바이스 파일을 생성하지 않기 때문에 사용자는 mknod 명령어를 사용해 디바이스 파일을 수동으로 생성해야 한다.
  - 디바이스 드라이버를 사용하는 디바이스를 파일 형태로 만들고, 드라이버를 커널에 적재한 후 application으로 디바이스에 접근하는 방식이다.
  - 즉, 소스파일들을 컴파일 하고 디바이스 파일을 만든 후, 드라이버를 커널에 적재하고(insmod), dmesg로 여러 커널 메시지를 확인한 후 application을 동작하면 된다.
