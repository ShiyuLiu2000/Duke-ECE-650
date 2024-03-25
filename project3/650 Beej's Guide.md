# 650 Beej's Guide

## 字节顺序

大多数当你在打造数据包或填写数据结构时，你需要确认你的两个数字跟四个数字都是 Network Byte Order。只是如果你不知道本地端的 Host Byte Order，那该怎麽做呢？

好消息是你只需要假设 Host Byte Order 不正确，然後每次都透过一个函数将值设定为 Network Byte Order。如果有必要，该函数会进行魔法的转换，而这个方式可以让你的代码能方便的移植到不同 endian 的机器上。

你可以转换两种型别的数值：short［两个 bytes］与 long［四个 bytes］。这些函数也可以用在 unsigned 变量。比如说，你想要将 short 从 Host Byte Order 转换为 Network Byte Order，用＂h＂代表＂host＂，用＂n＂代表＂network＂，而＂s＂代表＂short＂，所以是：h-to-n-s，或者htons()［读做：＂Host to Network Short＂］。

这真是太简单了…

你可以用任何你想要的方式来组合＂n＂丶＂h＂丶＂s＂与＂l＂，不过别用太蠢的组合，比如：没有这样的函数 stolh()［＂Short to Long Host＂］，没有这种东西，不过有：

```
htons() host to network short
htonl() host to network long
ntohs() network to host short
ntohl() network to host long
```

基本上，你需要在送出以前将数值转换为 Network Byte Order，并在收到之後将数值转回 Host Byte Order。

## 数据结构

首先是最简单的：socket descriptor，型别如下：

```
int
```

也用在主机名（host name）及服务名（service name）的查询。当我们之後开始实际应用时，才会开始觉得比较靠谱，现在只需要知道你在建立连接调用时会用到这个数据结构。

```cpp
struct addrinfo {
    int ai_flags; // AI_PASSIVE, AI_CANONNAME 等。
    int ai_family; // AF_INET, AF_INET6, AF_UNSPEC
    int ai_socktype; // SOCK_STREAM, SOCK_DGRAM
    int ai_protocol; // 用 0 當作 "any"
    size_t ai_addrlen; // ai_addr 的大小，單位是 byte
    struct sockaddr *ai_addr; // struct sockaddr_in 或 _in6
    char *ai_canonname; // 典型的 hostname
    struct addrinfo *ai_next; // 鏈結串列、下個節點
};
```

你可以载入这个数据结构，然後调用 getaddrinfo()。它会返回一个指针，这个指针指向一个新的链表，这个链表有一些数据结构，而数据结构的内容记载了你所需的东西。

总之，struct sockaddr 记录了很多 sockets 类型的 socket 的地址资料。sa_family 可以是任何东西，不过在这份教程中我们会用到的是 AF_INET［IPv4］或 AF_INET6［IPv6］。sa_data 包含一个 socket 的目地地址与 port number。这样很不方便，因为你不会想要手动的将地址封装到 sa_data 里。

```cpp
struct sockaddr {
    unsigned short sa_family; // address family, AF_xxx
    char sa_data[14]; // 14 bytes of protocol address
};
```

为了处理 struct sockaddr，程序设计师建立了对等平行的数据结构：struct sockaddr_in［＂in＂是代表＂internet＂］用在 IPv4。

而这有个重点：指向 struct sockaddr_in 的指针可以转型（cast）为指向 struct sockaddr 的指针，反之亦然。所以即使 connect() 需要一个 struct sockaddr *，你也可以用 struct sockaddr_in，并在最後的时候对它做型别转换！这个数据结构让它很容易可以参考（reference）socket 地址的成员。要注意的是 sin_zero［这是用来将数据结构补足符合 struct sockaddr 的长度］，应该要使用 memset()函数将 sin_zero 整个清为零。还有，sin_family 是对应到 struct sockaddr 中的 sa_family，并应该设定为＂AF_INET＂。最後，sin_port 必须是 Network Byte Order［利用 htons()］。

```cpp
// （IPv4 專用-- IPv6 請見 struct sockaddr_in6）
struct sockaddr_in {
    short int sin_family; // Address family, AF_INET
    unsigned short int sin_port; // Port number
    struct in_addr sin_addr; // Internet address
    unsigned char sin_zero[8]; // 與 struct sockaddr 相同的大小
};
```

你可以在 sruct in_addr 里看到 sin_addr 栏位。

```cpp
// (仅限 IPv4 — IPv6 请参考 struct in6_addr)
// Internet address (a structure for historical reasons)
struct in_addr {
    uint32_t s_addr; // that's a 32-bit int (4 bytes)
};
```

# IP address，Part II

咱们说，你有一个 struct sockaddr_in ina，而且你有一个 ＂10.12.110.57＂ 或 ＂2001:db8:63b3:1::3490＂ 这样的一个 IP address 要储存。你想要使用 inet_pton() 函数将 IP address 转换为数值与句号的符号，并依照你指定的 AF_INET 或 AF_INET6 来决定要储存在 struct in_addr 或 struct in6_addr。［＂pton＂的意思是＂presentation to network＂，你可以称之为＂printable to network＂，如果这样会比较好记的话］。

```cpp
struct sockaddr_in sa; // IPv4
struct sockaddr_in6 sa6; // IPv6
inet_pton(AF_INET, "192.0.2.1", &(sa.sin_addr)); // IPv4
inet_pton(AF_INET6, "2001:db8:63b3:1::3490", &(sa6.sin6_addr)); // IPv6
```

如果你有一个 struct in_addr 且你想要以数字与句号印出来的话呢？

［呵呵，或者如果你想要以＂十六进制与冒号＂打印出 struct in6_addr］。在这个例子中，你会想要使用 inet_ntop()函数［＂ntop＂意谓＂network to presentation＂－如果有比较好记的话，你可以称它为＂network to prinable＂］，像是这样：

```cpp
// IPv4:
char ip4[INET_ADDRSTRLEN]; // 储存 IPv4 字符串的空间
struct sockaddr_in sa; // 假這會由某個東西載入
inet_ntop(AF_INET, &(sa.sin_addr), ip4, INET_ADDRSTRLEN);
printf("The IPv4 address is: %s\n", ip4);
// IPv6:
char ip6[INET6_ADDRSTRLEN]; // 储存 IPv6 字符串的空间
struct sockaddr_in6 sa6; // 假這會由某個東西載入
inet_ntop(AF_INET6, &(sa6.sin6_addr), ip6, INET6_ADDRSTRLEN);
printf("The address is: %s\n", ip6);
```

## system call / Bust

### getaddrinfo()－准备开始！

它帮你设定之後需要的 struct。

```cpp
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

int getaddrinfo(const char *node, // 例如： "www.example.com" 或 IP
                const char *service, // 例如： "http" 或 port number
                const struct addrinfo *hints,
                struct addrinfo **res);
```

你给这个函数三个输入参数，结果它会返回一个指向链表的指针给你 － res。

*node* 参数是要连接的主机名，或者一个 IP address（地址）。

下一个参数是 *service*，这可以是 port number，像是 ＂80＂，或者特定的服务名［可以在你 UNIX 系统上的 IANA Port List [17] 或 /etc/services 文件中找到］，像是 ＂http＂ 或 ＂ftp＂ 或 ＂telnet＂ 或 ＂smtp＂ 诸如此类的。

最後，*hints* 参数指向一个你已经填好相关资料的 struct addrinfo。

```cpp
int status;
struct addrinfo hints;c
struct addrinfo *servinfo; // 将指向结果

memset(&hints, 0, sizeof hints); // 确保 struct 为空
hints.ai_family = AF_UNSPEC;  // 不用管是 IPv4 或 IPv6
hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
hints.ai_flags = AI_PASSIVE; // 幫我填好我的 IP 

if ((status = getaddrinfo(NULL, "3490", &hints, &servinfo)) != 0) {
  fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
  exit(1);
}

// servinfo 目前指向一个或多个 struct addrinfos 的链表

// ... 做每件事情，一直到你不再需要 servinfo  ....

freeaddrinfo(servinfo); // 释放这个链表
```

注意一下，我将 *ai_family* 设置为 AF_UNSPEC，这样代表我不用管我们用的是 IPv4 或 IPv6 address。如果你想要指定的话，你可以将它设置为 AF_INET 或 AF_INET6。

还有，你会在这里看到 AI_PASSIVE flag；这个会告诉 getaddrinfo() 要将我本地端的地址（address of local host）指定给 socket structure。这样很好，因为你就不用写固定的地址了［或者你可以将特定的地址放在 getaddrinfo() 的第一个参数中，我现在写 NULL 的那个参数］。

然後我们进行调用，若有错误发生时［getaddrinfo 会返回非零的值］，如你所见，我们可以使用 gai_strerror() 函数将错误打印出来。若每件事情都正常运作，那麽 *serinfo* 就会指向一个 struct addrinfos 的链表，表中的每个成员都会包含一个我们之後会用到的某种 struct sockaddr。

最後，当我们终於使用 getaddrinfo() 配置的链表完成工作後，我们可以［也应该］要调用 freeaddrinfo() 将链表全部释放。

这边有一个调用示例，如果你是一个想要连接到特定 server 的 client，比如是：＂www.example.net＂的 port 3490。再次强调，这里并没有真的进行连接，它只是设置我们之後要用的 structure。

```cpp
int status;
struct addrinfo hints;
struct addrinfo *servinfo; // 将指向结果

memset(&hints, 0, sizeof hints); // 确保 struct 为空
hints.ai_family = AF_UNSPEC; // 不用管是 IPv4 或 IPv6
hints.ai_socktype = SOCK_STREAM; // TCP stream sockets

// 准备好连接
status = getaddrinfo("www.example.net", "3490", &hints, &servinfo);

// servinfo 现在指向有一个或多个 struct addrinfos 的链表

我一直说 serinfo 是一个链表，它有各种的地址资料。让我们写一个能快速 demo 的程序，来呈现这个资料。这个小程序 [18] 会打印出你在命令行中所指定的主机 IP address：
** showip.c -- 顯示命令列中所給的主機 IP address
*/
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int main(int argc, char *argv[])
{
  struct addrinfo hints, *res, *p;
  int status;
  char ipstr[INET6_ADDRSTRLEN];

  if (argc != 2) {
    fprintf(stderr,"usage: showip hostname\n");
    return 1;
  }

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC; // AF_INET 或 AF_INET6 可以指定版本
  hints.ai_socktype = SOCK_STREAM;

  if ((status = getaddrinfo(argv[1], NULL, &hints, &res)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
    return 2;
  }

  printf("IP addresses for %s:\n\n", argv[1]);

  for(p = res;p != NULL; p = p->ai_next) {
    void *addr;
    char *ipver;

    // 取得本身地址的指针
     // 在 IPv4 与 IPv6 中的栏位不同：
    if (p->ai_family == AF_INET) { // IPv4
      struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
      addr = &(ipv4->sin_addr);
      ipver = "IPv4";
    } else { // IPv6
      struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
      addr = &(ipv6->sin6_addr);
      ipver = "IPv6";
    }

    // convert the IP to a string and print it:
    inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
    printf(" %s: %s\n", ipver, ipstr);
  }

  freeaddrinfo(res); // 释放链表

  return 0;
}
```

如你所见，代码使用你在命令行输入的参数调用 getaddrinfo()，它填好 *res* 所指的链表，并接着我们就能重复那行并打印出东西或做点类似的事。

［有点不好意思！我们在讨论 struct sockaddrs 它的型别差异是因 IP 版本而异之处有点鄙俗。我不确定是否有较优雅的方法。］

在下面运行示例！来看看大家喜欢看的运行画面：

```
$ showip www.example.net

IP addresses for www.example.net:

  IPv4: 192.0.2.88


$ showip ipv6.example.com

IP addresses for ipv6.example.com:

  IPv4: 192.0.2.101

  IPv6: 2001:db8:8c00:22::171
```

### socket()－取得 File Descriptor！

```cpp
#include <sys/types.h>
#include <sys/socket.h>

int socket(int domain, int type, int protocol);
```

［*domain* 是 PF_INET 或 PF_INET6，*type* 是 SOCK_STREAM 或 SOCK_DGRAM，而 *protocol* 可以设置为 0，用来帮给予的 type 选择适当的协议。或者你可以调用 getprotobyname() 来查询你想要的协议，＂tcp＂或＂udp＂］。

PF_INET 就是你在初始化 struct sockaddr_in 的 *sin_family* 栏位会用到的，它是 AF_INET 的亲戚。实际上，它们的关系很接近，所以其实它们的值也都一样，而许多程序设计师会调用 socket()，并以 AF_INET 替换 PF_INET 来做为第一个参数传递。struct sockaddr_in 中使用 AF_INET，而在调用 socket() 时使用 PF_INET。

你真的该做的只是使用调用 **getaddrinfo()** 得到的值，并将这个值直接提供给 socket()，像这样：

```cpp
int s;
struct addrinfo hints, *res;

// 运行查询
// [假装我们已经填好 "hints" struct]
getaddrinfo("www.example.com", "http", &hints, &res);

// [再来，你应该要对 getaddrinfo() 进行错误检查, 并走到 "res" 链表查询能用的资料，
// 而不是假设第一笔资料就是好的［像这些示例一样］
// 实际的示例请参考 client/server 章节。
s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
```

### bind()－ 我在哪個 port？

一旦你有了一个 socket，你会想要将这个 socket 与你本地端的 port 进行关联［如果你正想要 listen() 特定 port 进入的连接，通常都会这样做，比如：多人网路连线游戏在它们告诉你＂连接到 192.168.5.10 port 3490＂时这麽做］。port number 是用来让 kernel 可以比对出进入的数据包是属於哪个 process 的 socket descriptor。

```cpp
#include <sys/types.h>
#include <sys/socket.h>

int bind(int sockfd, struct sockaddr *my_addr, int addrlen);
```

*sockfd* 是 socket() 传回的 socket file descriptor。*my_addr* 是指向包含你的地址资料丶名称及 IP address 的 struct sockaddr 之指针。*addrlen* 是以 byte 为单位的地址长度。

我们来看一个示例，它将 socket bind（绑定）到运行程序的主机上，port 是 3490：

```cpp
struct addrinfo hints, *res;
int sockfd;

// 首先，用 getaddrinfo() 载入地址结构：

memset(&hints, 0, sizeof hints);
hints.ai_family = AF_UNSPEC; // use IPv4 or IPv6, whichever
hints.ai_socktype = SOCK_STREAM;
hints.ai_flags = AI_PASSIVE; // fill in my IP for me

getaddrinfo(NULL, "3490", &hints, &res);

// 建立一个 socket：

sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

// 将 socket bind 到我们传递给 getaddrinfo() 的 port：

bind(sockfd, res->ai_addr, res->ai_addrlen);
```

使用 AI_PASSIVE flag，我可以跟程序说要 bind 它所在主机的 IP。如果你想要 bind 到指定的本地端 IP address，舍弃 AI_PASSIVE，并改放一个地址到 getaddrinfo() 的第一个参数。 bind() 在错误时也会返回 -1，并将 errno 设置为该错误的值。

另一件调用 bind() 时要小心的事情是：不要用太小的 port number。全部 1024 以下的 ports 都是保留的［除非你是系统管理员］！你可以使用任何 1024 以上的 port number，最高到 65535［提供尚未被其它程序使用的］。 你可能有注意到，有时候你试着重新运行 server，而 bind() 却失败了，它声称＂Address already in use.＂（地址正在使用）。这是什麽意思呢？很好，有些连接到 socket 的连接还悬在 kernel 里面，而它占据了这个 port。你可以等待它自行清除［一分钟之类］，或者在你的程序中新增代码，让它重新使用这个 port，类似这样：

```cpp
int yes=1;
//char yes='1'; // Solaris 的用户使用这个

// 可以跳过 "Address already in use" 错误信息

if (setsockopt(listener,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1) {
  perror("setsockopt");
  exit(1);
}
```

最後一个对 bind() 的额外小提醒：在你不愿意调用 bind() 时。若你正使用 connect() 连接到远端的机器，你可以不用管 local port 是多少（以 telnet 为例，你只管远端的 port 就好），你可以单纯地调用 connect()，它会检查 socket 是否尚未绑定（unbound），并在有需要的时候自动将 socket bind() 到一个尚未使用的 local port。

### connect()，嘿!你好。

connect() call 如下：

```cpp
#include <sys/types.h>
#include <sys/socket.h>

int connect(int sockfd, struct sockaddr *serv_addr, int addrlen);
```

sockfd 是我们的好邻居 socket file descriptor，如同 socket() 调用所返回的，serv_addr 是一个 struct sockaddr，包含了目的 port 及 IP 地址，而 addrlen 是以 byte 为单位的 server 地址结构之长度。 全部的资料都可以从 getaddrinfo() 调用中取得，它很好用。 我们有个示例，这边我们用 socket 连接到 ＂www.example.com＂ 的 port 3490：

```cpp
struct addrinfo hints, *res;
int sockfd;

// 首先，用 getaddrinfo() 载入 address structs：

memset(&hints, 0, sizeof hints);
hints.ai_family = AF_UNSPEC;
hints.ai_socktype = SOCK_STREAM;

getaddrinfo("www.example.com", "3490", &hints, &res);

// 建立一个 socket：

sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

// connect!
connect(sockfd, res->ai_addr, res->ai_addrlen);
```

老学校的程序再次填满了它们自己的 struct sockaddr_ins 并传给 connect()。如果你愿意的话，你可以这样做。请见上面 bind() 章节中类似的提点。 要确定有检查 connect() 返回的值，它在错误时会返回 -1，并设定 errno 变量。 还要注意的是，我们不会调用 bind()。基本上，我们不用管我们的 local port number；我们只在意我们的目地［远端 port］。Kernel 会帮我们选择一个 local port，而我们要连接的站台会自动从我们这里取得资料，不用担心。

### listen()－有人会调用我吗

你想要等待进入的连接，并以某种方式处理它们。 这个过程有两个步骤：你要先调用 listen()，接着调用 accept()［参考下一节］。 listen 调用相当简单，不过需要一点说明：

```cpp
int listen(int sockfd, int backlog);
```

*sockfd* 是来自 socket() system call 的一般 socket file descriptor。*backlog* 是进入的队列（incoming queue）中所允许的连接数目。这代表什麽意思呢？好的，进入的连接将会在这个队列中排队等待，直到你 accept() 它们（请见下节），而这限制了排队的数量。多数的系统默认将这个数值限制为 20；你或许可以一开始就将它设置为 5 或 10。 再来，如同往常，listen() 会返回 -1 并在错误时设置 errno。 好的，你可能会想像，我们需要在调用 listen() 以前调用 bind()，让 server 可以在指定的 port 上运行。［你必须能告诉你的好朋友要连接到哪一个 port！］所以如果你正在 listen 进入的连接，你会运行的 system call 顺序是：

```cpp
getaddrinfo();
socket();
bind();
listen();
/* accept() 从这里开始 */
```

### accept()－"谢谢你调用 port 3490"

很远的人会试着 connect() 到你的电脑正在 listen() 的 port。他们的连接会排队等待被 accept()。你调用 accept()，并告诉它要取得搁置的（pending）连接。它会返回专属这个连接的一个新 socket file descriptor 给你！那是对的，你突然有了两个 *socket file descriptor*！原本的 socket file descriptor 仍然正在 listen 之後的连线，而新建立的 socket file descriptor 则是在最後要准备给 send() 与 recv() 用的。

```cpp
#include <sys/types.h>
#include <sys/socket.h>

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
```

*sockfd* 是正在进行 listen() 的 socket descriptor。很简单，*addr* 通常是一个指向 local struct sockaddr_storage 的指针，关於进来的连接将往哪里去的资料［而你可以用它来得知是哪一台主机从哪一个 port 调用你的］。*addrlen* 是一个 local 的整数变量，应该在将它的地址传递给 accept() 以前，将它设置为 sizeof(struct sockaddr_storage)。accept() 不会存放更多的 bytes（字节）到 *addr*。若它存放了较少的 bytes 进去，它会改变 *addrlen* 的值来表示。

```cpp
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MYPORT "3490" // 使用者将连接的 port
#define BACKLOG 10 // 在队列中可以有多少个连接在等待

int main(void)
{
　　struct sockaddr_storage their_addr;
　　socklen_t addr_size;
　　struct addrinfo hints, *res;
　　int sockfd, new_fd;

　　// !! 不要忘了帮这些调用做错误检查 !!

　　// 首先，使用 getaddrinfo() 载入 address struct：

　　memset(&hints, 0, sizeof hints);
　　hints.ai_family = AF_UNSPEC; // 使用 IPv4 或 IPv6，都可以
　　hints.ai_socktype = SOCK_STREAM;
　　hints.ai_flags = AI_PASSIVE; // 帮我填上我的 IP 

　　getaddrinfo(NULL, MYPORT, &hints, &res);

　　// 产生一个 socket，bind socket，并 listen socket：

　　sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
　　bind(sockfd, res->ai_addr, res->ai_addrlen);
　　listen(sockfd, BACKLOG);

　　// 现在接受一个进入的连接：

　　addr_size = sizeof their_addr;
　　new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);

　　// 准备好与 new_fd 这个 socket descriptor 进行沟通！
　　.
　　.
　　.
```

一样，我们会将 *new_fd* socket descriptor 用於 send() 与 recv() 调用。若你只是要取得一个连接，你可以用 close() 关闭正在 listen 的*sockfd*，以避免有更多的连接进入同一个 port，若你有这个需要的话。

### send() 与 recv()－ 宝贝，跟我说说话！

这两个用来通讯的函数是透过 stream socket 或 connected datagram ssocket。若你想要使用常规的 unconnected datagram socket，你会需要参考底下的 sendto() 及 recvfrom() 的章节。

```cpp
int send(int sockfd, const void *msg, int len, int flags);
```

*sockfd* 是你想要送资料过去的 socket descriptor［不论它是不是 socket() 返回的，或是你用 accept() 取得的］。*msg* 是一个指向你想要传送资料之指标，而 *len* 是以 byte 为单位的资料长度。而 *flags* 设置为 0 就好。［更多相关的 flag 资料请见 send() man 手册］。

```cpp
char *msg = "Beej was here!";
int len, bytes_sent;
.
.
.
len = strlen(msg);
bytes_sent = send(sockfd, msg, len, 0);
.
.
.
```

要记住，如果 send() 返回的值与 *len* 的值不符合的话，你就需要再送出字串剩下的部分。好消息是：如果数据包很小［比 1K 还要小这类的］，或许有机会一次就送出全部的东西。 一样，错误时会返回 -1，并将 errno 设置为错误码（error number）。

```cpp
int recv(int sockfd, void *buf, int len, int flags);
```

*sockfd* 是要读取的 socket descriptor，*buf* 是要记录读到资料的缓冲区（buffer），*len* 是缓冲区的最大长度，而 *flags* 可以再设置为 0。［关於 flag 资料的细节请参考 recv() 的 man 手册］。 recv() 返回实际读到并写入到缓冲区的 byte 数目，而错误时返回 -1［并设置相对的 errno］。 等等！ recv() 会返回 0，这只能表示一件事情：远端那边已经关闭了你的连接！recv() 返回 0 的值是让你知道这件事情。

### close() 与 shutdown()－从我面前消失吧

呼！你已经成天都在 send() 与 recv()了。你正准备要关闭你 socket descriptor 的连接，这很简单，你只要使用常规的 UNIX file descriptor close() 函数：

```cpp
close(sockfd);
```

```cpp
int shutdown(int sockfd, int how);
```

*sockfd* 是你想要 shutdown 的 socket file descriptor，而 *how* 是下列其中一个值：

```
0 不允许再接收数据

1 不允许再传送数据

2 不允许再传送与接收数据［就像 close()］
```

shutdown() 成功时返回 0，而错误时返回 -1（设置相对的 errno）。

重要的是 shutdown() 实际上没有关闭 file descriptor，它只是改变了它的可用性。如果要释放 socket descriptor，你还是需要使用 close()。

### getpeername()－你是谁？

getpeername() 函数会告诉你另一端连接的 stream socket 是谁，函数原型如下：

```cpp
#include <sys/socket.h>
int getpeername(int sockfd, struct sockaddr *addr, int *addrlen);
```

*sockfd* 是连接的 stream socket 之 descriptor，*addr* 是指向 struct sockaddr［或 struct sockaddr_in］的指针，这个数据结构储存了连线另一端的资料，而 *addrlen* 则是指向 int 的指针，应该将它初始化为 sizeof *addr 或 sizeof(struct sockaddr)。 函数在错误时返回 -1，并设置相对的 errno。 一旦你取得了它们的地址，你就可以用 inet_ntop()丶getnameinfo() 或 gethostbyaddr() 印出或取得更多的资料。不过你无法取得它们的登录帐号。

### gethostname()－我是誰？

比 getpeername() 更简单的函数是 gethostname()，它会返回你运行程序的电脑名，这个名称之後可以用在 gethostbyname()，用来定义你本地端电脑的 IP address。

```cpp
#include <unistd.h>
int gethostname(char *hostname, size_t size);
```

参数很简单：*hostname* 是指向字符数组（array of chars）的指针，它会储存函数返回的主机名（hostname），而 *size* 是以 byte 为单位的主机名长度。

函数在运行成功时返回 0，在错误时返回 -1，并一样设置 errno。

## Client-Server 基础

宝贝，这是个 client-server（客户-服务器）的世界。单纯与网路处理 client processes（客户进程）及 server processes（服务器进程）通讯的每件事情有关，反之亦然。以 telnet 为例，当你用 telnet（client）连接到远端主机的 port 23 时，主机上的程序（称为 telnetd server）就开始动了起来，它会处理进来的 telnet 连接，并帮你设定一个登录提示符等。

一台设备上通常只会有一个 server，而该 server 会利用 fork()来处理多个 clients。基本的例程（routine）是：server 会等待连接丶accept() 连接，并且 fork() 一个 child process（子进程）来处理此连接。这就是我们在下一节的 server 示例所做的事情。

### 简易的 Stream Server

这个 server 所做的事情就是透过 stream connection（串流连接）送出"Hello, World!\n"字符串。你所需要做就是用一个窗口来测试执行 server，并用另一个窗口来 telnet 到 server： $ telnet remotehostname 3490 这里的 remotehostname 就是你运行 server 的主机名。

```cpp
/*
** server.c － 展示一个stream socket server
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#define PORT "3490" // 提供给用戶连接的 port
#define BACKLOG 10 // 有多少个特定的连接队列（pending connections queue）

void sigchld_handler(int s)
{
  while(waitpid(-1, NULL, WNOHANG) > 0);
}

// 取得 sockaddr，IPv4 或 IPv6：
void *get_in_addr(struct sockaddr *sa)
{
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in*)sa)->sin_addr);
  }
  return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void)
{
  int sockfd, new_fd; // 在 sock_fd 进行 listen，new_fd 是新的连接
  struct addrinfo hints, *servinfo, *p;
  struct sockaddr_storage their_addr; // 连接者的地址资料
  socklen_t sin_size;
  struct sigaction sa;
  int yes=1;
  char s[INET6_ADDRSTRLEN];
  int rv;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE; // 使用我的 IP

  if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return 1;
  }

  // 以循环找出全部的结果，并绑定（bind）到第一个能用的结果
  for(p = servinfo; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype,
      p->ai_protocol)) == -1) {
      perror("server: socket");
      continue;
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
        sizeof(int)) == -1) {
      perror("setsockopt");
      exit(1);
    }

    if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(sockfd);
      perror("server: bind");
      continue;
    }

    break;
  }

  if (p == NULL) {
    fprintf(stderr, "server: failed to bind\n");
    return 2;
  }

  freeaddrinfo(servinfo); // 全部都用这个 structure

  if (listen(sockfd, BACKLOG) == -1) {
    perror("listen");
    exit(1);
  }

  sa.sa_handler = sigchld_handler; // 收拾全部死掉的 processes
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;

  if (sigaction(SIGCHLD, &sa, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }

  printf("server: waiting for connections...\n");

  while(1) { // 主要的 accept() 循环
  
  sin_size = sizeof their_addr;
    new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);

    if (new_fd == -1) {
      perror("accept");
      continue;
    }

    inet_ntop(their_addr.ss_family,
    get_in_addr((struct sockaddr *)&their_addr),
      s, sizeof s);
    printf("server: got connection from %s\n", s);
 
    if (!fork()) { // 这个是 child process
      close(sockfd); // child 不需要 listener

      if (send(new_fd, "Hello, world!", 13, 0) == -1)
        perror("send");

      close(new_fd);

      exit(0);
    }
    close(new_fd); // parent 不需要这个
  }

  return 0;
}
```

### 简易的 Stream Client

Client 这家伙比 server 简单多了，client 所需要做的就是：连线到你在命令行所指定的主机 3490 port，接着，client 会收到 server 送回的字符串。

```cpp
/*
/*
** client.c -- 一个 stream socket client 的 demo
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT "3490" // Client 所要连接的 port
#define MAXDATASIZE 100 // 我们一次可以收到的最大字节数量（number of bytes）

// 取得 IPv4 或 IPv6 的 sockaddr：
void *get_in_addr(struct sockaddr *sa)
{
　　if (sa->sa_family == AF_INET) {
　　　　return &(((struct sockaddr_in*)sa)->sin_addr);
　　}

　　return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
　　int sockfd, numbytes;
　　char buf[MAXDATASIZE];
　　struct addrinfo hints, *servinfo, *p;
　　int rv;
　　char s[INET6_ADDRSTRLEN];

　　if (argc != 2) {
　　　　fprintf(stderr,"usage: client hostname\n");
　　　　exit(1);
　　}

　　memset(&hints, 0, sizeof hints);
　　hints.ai_family = AF_UNSPEC;
　　hints.ai_socktype = SOCK_STREAM;

　　if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
　　　　fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
　　　　return 1;
　　}

　　// 用循环取得全部的结果，并先连接到能成功连接的
　　for(p = servinfo; p != NULL; p = p->ai_next) {
　　　　if ((sockfd = socket(p->ai_family, p->ai_socktype,
　　　　　　p->ai_protocol)) == -1) {
　　　　　　perror("client: socket");
　　　　　　continue;
　　　　}

　　　　if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
　　　　　　close(sockfd);
　　　　　　perror("client: connect");
　　　　　　continue;
　　　　}

　　　　break;
　　}

　　if (p == NULL) {
　　　　fprintf(stderr, "client: failed to connect\n");
　　　　return 2;
　　}

　　inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);

　　printf("client: connecting to %s\n", s);

　　freeaddrinfo(servinfo); // 全部皆以这个 structure 完成

　　if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
　　　　perror("recv");
　　　　exit(1);
　　}

　　buf[numbytes] = '\0';
　　printf("client: received '%s'\n",buf);

　　close(sockfd);
　　return 0;
}
```

你如果没有在运行 client 以前先启动 server 的话，connect()会返回 ＂Connection refused＂，这很有帮助。