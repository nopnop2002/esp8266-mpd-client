# esp8266-mpd-client
MPD(MusicPlayerDaemon) client example for ESP8266/ESP32.    
MPD protocol is [here](https://github.com/MusicPlayerDaemon/MPD/blob/master/doc/protocol.rst).   

---

# MPD_Client_Toggle   
Judge current status and execute PLYA or STOP.   

# MPD_Client_Play_Forever   
Play current song forever.   

# MPD_Client_Remote   
Control using IR remote.   

# MPD_Client_1602LCD_Display   
Display current artist & title on LCD.   

# MPD_Client_Remote_1602LCD_Display   
Control using IR remote.   
Display current artist & title on LCD.   

# MPD_Client_DotMatrix   
Display current artist & title on 8x8 DotMatrix LED.   

# MPD_Client_List_PlayLists   
Show current playlists.   

# MPD_Client_Load_Playlist   
Load playlist forever.   

# MPD_Client_Keep_Connect   
If there is no communication for a certain period of time, the connection with the MPD server will be disconnected.   
Keep to connect to the MPD server using the STATUS command.   
