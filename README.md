# EchoWeb
A very simple Web environment implementation

## Introduction
This project contains three parts:

1. EchoS -- A event-driven web server
1. EchoL -- A programming language designed for web
1. EchoDB -- A key-value database

## Installation
Run `make` then all source code will be compiled  
  
**Dependences:** GLib

## Usage
### EchoS
Run `echos` then you can access 127.0.0.1:80

### EchoDB
Run `echodb` then server will listen on 127.0.0.1:6666  
We provide both `EchoL` and `C` client APIs:
```
    echodb_connect(ip_addr, port)
    echodb_close(void)
    echodb_set(key_len, key, val_len, val)
    echodb_get(key_len, key)
    echodb_update(key_len, key, val_len, val)
    echodb_delete(key_len, key)
```

### EchoL
You can embed your `EchoL` code into html `<?el``?>` tag  which is similar to PHP  
Example:
```
    <?el
        echodb_connect("127.0.0.1, 6666);
        post_len := getenv("CONTENT_LENGTH");
        if (post_len = FALSE) {
            val := echodb_get("welcome");
            if (val = FALSE) {
                val := "Hello World!"；
                echodb_set("welcome", val);
            }
        } else {
            post_len := string_to_int(post_len);
            post := read(post_len);
            val  := parse_query(post, "welcome");
            echodb_update("welcome", val);
        }
        print(val);
    ?>
```
