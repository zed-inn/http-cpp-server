# C++ Async HTTP/1.1 Server

A lightweight, zero-copy HTTP/1.1 server built entirely from scratch in C++.

This project was engineered to achieve a deep understanding of low-level network programming, POSIX system calls, and memory-safe C++ text parsing. It implements an asynchronous I/O multiplexing architecture without relying on external web frameworks or bloated multi-threading models.

## Architecture & Design

The server strictly separates the infrastructure layer (TCP/Sockets) from the application domain (HTTP parsing).

### 1. Transport Layer (The Event Loop)

Instead of the inherently flawed thread-per-request model, the server utilizes a single-threaded asynchronous event loop driven by `poll()`.

- **Non-Blocking I/O:** All client sockets are strictly set to `O_NONBLOCK`. The server safely handles `EAGAIN` / `EWOULDBLOCK` kernel interrupts to yield execution without freezing the main thread.
- **Connection State Management:** To handle TCP fragmentation, the server maintains an isolated `ConnectionState` object for every active descriptor. It buffers raw bytes and preserves the parser's state machine between fragmented `recv()` calls.
- **Smart Descriptors:** A custom wrapper manages the `pollfd` dynamic array, utilizing a bitwise NOT (`~fd`) strategy to mark dead sockets and safely compact the array in _O(N)_ time.

### 2. Application Layer (The HTTP Domain)

The `HttpRequest` parser is a deterministic state machine built for raw speed and minimal heap allocations.

- **Zero-Copy Parsing:** Instead of constantly reallocating string buffers, the parser uses a `std::string_view` sliding window over the raw TCP connection buffer. The CPU simply registers offsets.
- **Lazy Percent-Decoding:** The server implements a Copy-on-Write (CoW) abstraction (`ConditionalStr`) using `std::variant<std::string, std::string_view>`. The parser only executes the expensive heap allocation and decoding logic if a URI actually contains percent-encoded `%` characters. Otherwise, it remains zero-copy.
- **Strict RFC 7230 Compliance:** The parser validates all IANA-registered headers , enforces hard byte-limits to prevent DOS attacks , and sanitizes URIs against malformed IPv4/IPv6 authorities.

### 3. Application Interface

The core engine is wrapped in an Express.js-style routing interface, allowing developers to map specific URI paths to function pointers (`Router`) effortlessly.

```cpp
void handleMainRoute(HttpRequest *req, HttpResponse *res) {
    res->setStatusCode(HttpStatusCode::OK);
    res->addBody("<h1>Welcome</h1>");
    res->addHeader("Content-Type", "text/html");
}

int main() {
    Server serv("localhost", "8080");
    serv.addRoute("/", &handleMainRoute);
    serv.launch();
    return 0;
}
```

## Repository Structure

The codebase strictly enforces Clean Architecture boundaries, isolating network infrastructure from domain logic.

```text
.
├── app/                  # Infrastructure Layer
│   ├── server.hpp        # TCP Sockets, poll() Event Loop, Connection State
│   └── application.cpp   # Entry point and Route definitions
└── core/                 # Application Domain Layer
    ├── ents/             # Core Entities (HttpRequest, HttpResponse)
    ├── parsers/          # Zero-copy sub-parsers (Headers, URI, Method)
    ├── interface/        # Parser contracts and Result types
    └── utils/            # Status codes, macros, and string utilities
```

## How It Works

### The Event Loop (`app/server.hpp`)

The server maintains a `ConnectionState` object for every active file descriptor. When `poll()` registers a `POLLIN` event, bytes are read directly into the connection's isolated buffer. The parser is fed a `string_view` of these newly arrived bytes. If the kernel interrupts with `EAGAIN` / `EWOULDBLOCK`, the state is preserved and execution yields back to the loop.

### The Router (`app/application.cpp`)

Adding endpoints is designed to be trivial and framework-like as shown in above example.

## Known Trade-Offs & Limitations

- **GET-Only Focus:** To prioritize raw routing speed and architectural purity, this server currently does not process `POST`/`PUT` payloads or handle `Transfer-Encoding: chunked`.
- **`poll()` vs `epoll()`:** The event loop utilizes `poll()` for POSIX cross-platform compliance. In a strictly Linux-targeted production environment, replacing this with _O(1)_ `epoll` would increase scalability.
- **Header Map Allocations:** Headers are stored in a `std::unordered_map`. While this provides _O(1)_ lookup convenience, it inherently triggers dynamic heap allocations for each unique header node.

## Build & Run

Compiled using standard GCC/Clang with C++17 support (required for `std::string_view` and `std::variant`).

```bash
# Build the application
g++ app/application.cpp -o server -std=c++17 -O2

# Run
./server
```
