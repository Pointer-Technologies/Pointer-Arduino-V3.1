# Pointer-Project-V3.1
 Arduino files for the tracker prototype
 
 3 files inside. *projectv3.1.1* is the whole project, while the other 2 files are partial tests of the functions the prototype has to complete.
 
 ## How it works, briefly
 - Upon starting the module up, it opens a hotspot. On address **192.168.4.1** a small website (that is written directly on the source code) is hosted. It is used exclusively for the user to enter their Wi-Fi network and password, which the module uses to connect to it.
 - Then the module gets a LOCAL IP address, that's used as a portal for the mobile app and tracker to communicate. When connected to the network, the address *LOCAL_IP/4/on* turns the sound on, etc. This website is directly written onto the source code as well.

### Copyright (C) Antonios Antoniou, Pointer Techonologies
### Credits to Rui Santos: https://randomnerdtutorials.com
