import serial
from collections import deque
from appJar import gui

class ap20:
    ''' simulator for the Datasat AP20 behavior '''

    def __init__(self, port='COM4'):
        self.sp = serial.Serial(port=port, baudrate=9600, timeout=0)
        self.rxbuf = deque(maxlen=10)
        self.vol = 70

    def __del__(self):
        self.sp.close()

    def set_vol(self, newvol):
        ''' assume newvol as the new volume and transmit 'FADER <newvol>\r' to
            the remote fader for display '''
        self.vol = newvol
        self.sp.write("FADER {}\r".format(newvol).encode())

    def receive(self):
        ''' call periodically to fetch and process incoming commands from
            remote fader. As of now the following commands are understood:
            'FADER [+-]<number>\r' as relative change.
            'FADER <number>\r' as absolute setpoint.
            'FADER \r' as request of current state.
        '''
        vol_changed = False
        while True:
            b = self.sp.read()
            if b == b'':
                break
            self.rxbuf.append(b)

        while True:
            try:
                startidx = self.rxbuf.index(b'F')
                endidx = self.rxbuf.index(b'\r')
            except ValueError:
                break

            substring = b''.join((list(self.rxbuf)[startidx:endidx]))
            cmd = substring.decode().split(' ')

            if cmd[0] == "FADER":
                if len(cmd) == 1:
                    pass
                elif '+' in cmd[1] or '-' in cmd[1]: # relative change
                    self.vol += int(cmd[1].strip('\r'))
                else: # absolute set
                    self.vol = int(cmd[1].strip('\r'))
                vol_changed = True
            
            try:
                while self.rxbuf.popleft() != '\r':
                    pass
            except IndexError:
                pass

        if vol_changed:
            self.set_vol(self.vol)

        return self.vol

app = gui()
fader = ap20()

def change_volume(button):
    new_vol = fader.vol
    if button == "inc":
        new_vol += 1
    elif button == "dec":
        new_vol -= 1
    elif button == "set":
        new_vol = int(app.getEntry("newVol"))

    app.setMeter("volume", new_vol)
    fader.set_vol(new_vol)

app.addButton("inc", change_volume)
app.addButton("dec", change_volume)

app.addNumericEntry("newVol")
app.addButton("set", change_volume)

app.addMeter("volume")
app.setMeter("volume", fader.vol)

app.registerEvent(lambda: app.setMeter("volume", fader.receive()))
app.setPollTime(100)

app.go()

