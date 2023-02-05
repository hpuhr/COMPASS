import socket
from threading import *
import random
import time

#osgview_str = "window1.osgview3"
osgview_str = "osgview3" # for main window

def set_slider_values(serversocket, server, num_times, wait_time_s):

    print("set_slider_values num_times {}".format(num_times))

    for cnt in range(num_times):
    
        sliderval = random.randrange(100)
        
        # uiset --object=WINDOW.OSGVIEW.timefilter.slider --value=SLIDERVAL --wait_condition="delay;TIMEOUT"
        cmd_str = "uiset --object={}.timefilter.slider --value={} --wait_condition=\"delay;30000\"".format(osgview_str, sliderval)
    
        serversocket.sendto(cmd_str.encode(), server)
            
        data, _ = serversocket.recvfrom(1024)
        #data = data.decode()
        
        print("set_slider_values compass sent: '{}'".format(data.decode()))
        
        data, _ = serversocket.recvfrom(1024)
        #data = data.decode()
        
        print("set_slider_values result sent: '{}'".format(data.decode()))
        
        time.sleep(wait_time_s)

def switch_live_mode(serversocket, server):

    # uiset --object=livebutton --wait_condition="signal;WINDOW.OSGVIEW;dataLoaded;TIMEOUT"
    cmd_str = "uiset --object=livebutton --wait_condition=\"signal;{};dataLoaded;10000\"".format(osgview_str)
    
    #cmd_str = "uiset --object=livebutton --wait_condition=\"delay;10000\""
    
    print("switch_live_mode")
    
    serversocket.sendto(cmd_str.encode(), server)
            
    data, _ = serversocket.recvfrom(1024)
    print("switch_live_mode compass sent: '{}'".format(data.decode()))
        
    data, _ = serversocket.recvfrom(1024)
    print("switch_live_mode result sent: '{}'".format(data.decode()))
    
    time.sleep(5)
    
    
def reset_views(serversocket, server):        
    
    #uiset --object=mainmenu --value="UI|Reset Views"
    cmd_str = "uiset --object=mainmenu --value=\"UI|Reset Views\""
    
    print("reset_views")
    
    serversocket.sendto(cmd_str.encode(), server)
            
    data, _ = serversocket.recvfrom(1024)
    print("reset_views compass sent: '{}'".format(data.decode()))
        
    data, _ = serversocket.recvfrom(1024)
    print("reset_views result sent: '{}'".format(data.decode()))
        
    time.sleep(5)
    
    
def main():
    
    print("Python RTCommand Runner")
    
    serversocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    #serversocket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    
    #host = "127.0.0.1"
    host = "192.168.0.234"
    port = 27960
    
    server = (host, port)
    
    print ('adress {}:{}'.format(host, port))
    serversocket.connect(server)

    for cnt in range(5):
        print("loop cnt {}".format(cnt))
        # assume in live running
        
        if random.randint(0, 1) == 1: # switch to live paused
            switch_live_mode(serversocket, server)
            switched_to_live_paused = True
        else:
            switched_to_live_paused = False
    
        #set_slider_values(serversocket, server, num_times, max_wait_time_s)
        set_slider_values(serversocket, server, 10, 5)
        
        if switched_to_live_paused:
            if random.randint(0, 1) == 1: # switch to live running
                switch_live_mode(serversocket, server)
            else:
                reset_views(serversocket, server)
    
    #for cnt in range(5):
    #    switch_live_mode(serversocket, server)
    #    time.sleep(10)

    #cmds = ["empty", "empty", "empty"]

    #cmds = ["empty",
    #        "uiset --object=\"histogramview2.reload\" --wait_condition=\"signal;histogramview2;dataLoaded;10000\""]

    #for cmd in cmds:
        #print ("\nsending command '{}'".format(cmd))
            
        #serversocket.sendto(cmd.encode(), server)
            
        #data, _ = serversocket.recvfrom(1024)
        ##data = data.decode()
        
        #print("compass sent: '{}'".format(data.decode()))
        
        #data, _ = serversocket.recvfrom(1024)
        ##data = data.decode()
        
        #print("result sent: '{}'".format(data.decode()))
        
        #time.sleep(3)

    
    serversocket.close()
    

if __name__ == "__main__":
    main()
    
