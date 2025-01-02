# Message communication using Shared Memory

## Install
```sudo apt install -y protobuf-compiler```


## Build
```mkdir build```

```cd build; cmake ..; make```

## Run
#### 1. Server
```cd build/bin```

```./ipc_server```

#### 2. Client
```cd build/bin```

```./ipc_client```

