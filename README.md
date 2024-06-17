# Piconaut Microframework
Lightweight modern REST/Web micro-framework for C++ 14 &amp; 17.

<img src="piconaut.png" width="300"/>

For full fledged Modern Server Framework, please find [NvServ](https://github.com/lwdjohari/nvserv).

> [!WARNING]
> Currently compatibility with C++14 is not yet throughly tested.<br/>
> Status : WIP, Experimental & Unstable.  

# Features
- HTTP/1.1 and HTTP/2 protocols support (LibH2O Http Server)
- Byte, String and Json Handler 
- Dynamic parameter routing support (RFC-6570)
- Include easy to use json Value & Value builder (Piconaut abstracting RapidJSON as the json engine)
- Include multithreaded Logger (NvLog)
- Middleware support
- Micro Component Resolver a.k.a micro/mini dependency injections

### Json Builder Example

```cpp

piconaut::formats::json::ValueBuilder json;
    
json["server"].CreateJsonObject(); 
json["server"]["name"] = "Piconaut Framework";
json["server"]["version"] = "v0.2.1";
json["status"] = 200;

// For fast and uglify json output
// auto json_buffer = json.SerializeToBytes();
auto json_buffer = json.SerializePrettyToBytes();
std::cout << json_buffer.ToString() << std::endl;

```

Output

```json

{
  "server": {
    "name": "Piconaut Framework",
    "version": "v0.2.1"
  },
  "status": 200
}

```

# Development & Dependencies

Piconaut use h2o proven http1.1/http2 webserver as the underlying http server.

## Development
- CMake 3.10
- C++14/C++17 compiler

## Shared Dependencies
- Lib H2O Http Server (evloop) >= v2.5
- OpenSSL >= v1.1
- Lib UV (use for capture cross platform signal)
- Lib Wslay
- Zlib

Note: Debian or Ubuntu linux based
```shell
sudo apt-get update && \
sudo apt-get install \
    libh2o-evloop-dev \
    openssl \
    libssl-dev \
    libuv1-dev \
    libwslay-dev \
    zlib1g-dev
```

### Checking your shared-lib
In linux you can check for already installed shared library by executing this command
```shell
dpkg-query -l | grep libuv-evloop-dev
dpkg-query -l | grep libwslay-dev
```
Find your dependencies location
```shell
dpkg -L libwslay-dev
```

### Troubleshooting

Somehow in ubuntu, libwslay can't be found by original cmake find_package.<br/>
Piconaut work around is adding custom FindWslay.cmake module in ```cmake``` folder

```cmake
find_package(Wslay REQUIRED)
if(NOT WSLAY_FOUND)
    message(FATAL_ERROR "wslay library not found")
endif()
```

This will ensure CMake configuration for H2o found libwslay.so correctly.

## Deployment

Coming soon
```
TL;DR;
- Install the shared dependency library
- Build as Release
- Copy the executable & run
- For current state, recommend to put behind reverse proxy
- Non Linux: help with pull request, really appreciate for those helps.
```

## Contributions

Currently we are still on-going roadmap design and architectural design that might be lead to complete rewrite or complete breaking changes.
We might accept contributors when everything above have better & crytal-clears roadmap.

## License

Copyright [2024] [Linggawasistha Djohari]

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
