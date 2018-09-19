# 42Chat
Socket based, event-triggered chat sever & Ncurses client.
![Basic interface](https://github.com/CoZZmOnAvT/42chat/blob/master/screenshots/42Chat_1.png?raw=true)
![Avaliable commands](https://github.com/CoZZmOnAvT/42chat/blob/master/screenshots/42Chat_2.png?raw=true)
![Minimum size](https://github.com/CoZZmOnAvT/42chat/blob/master/screenshots/42Chat_3.png?raw=true)

### Client Features

- Interactive NCurses Interface
- Event-triggered redraw
- Auto-reconnect

### Server Features

- Strong and fast event-triggered foundation
- Reacts accordingly to all client commands dependent on client state
- Chat with n clients at a time
- Rooms containing unique chat sessions simultaneously
- Create or join rooms
- Each room supports n clients
- Private messages engine
- Disable notifications if client needs it

You can build this project on MacOS, Ubuntu/Debian, Android.

Copilation:
  - `bash ./configure.sh`
  - `make`

Execution:
  - If you want to start your own chat server:
    - `./chat_server`
    - `./42chat [YOUR_IP_ADDRESS]`
  - If you want to chat in the my dedicated server:
    - `./42chat 91.203.25.95`

Tested systems:
  - MacOS High Sierra
  - Ubuntu 16.04 & 18.04
  - Android 7.0 & 8.1
