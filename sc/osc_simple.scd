b = NetAddr("192.168.29.221", 9000);
(
var ms=32;
var base_delay_micro=500;
var full=200;
var slow=4/12;
var fast=8/12;

var steps = fast*ms*full*2;
var delay = base_delay_micro/8;
steps.postln;
delay.postln;
steps = steps.round(1);
steps.postln;
steps = 50000;
b.sendMsg("/rotate", steps, 50, 0);
)

(
var ms=32;
var base_delay_micro=500;
var full=200;
var slow=4/12;
var fast=8/12;


var steps = fast*ms;
var delay = base_delay_micro/10;
steps.postln;
delay.postln;
steps = steps.round(1);
steps.postln;
b.sendMsg("/rotate", steps, delay, 1);
)

b.sendMsg("/enable", 1);
b.sendMsg("/disable", 1);
b.sendMsg("/info", 1);
b.sendMsg("/exit", 1);

(
OSCdef(\doneReceiver, { |msg|
    "Received: ".post; msg.postln;
}, '/done', recvPort: 12345); // Explicit path match
)

(
OSCdef(\infoReceiver, { |msg|
    "Received: ".post; msg.postln;
}, '/info', recvPort: 12345); // Explicit path match
)
(
OSCdef(\ackReceiver, { |msg|
    "Received: ".post; msg.postln;
}, '/ack', recvPort: 12345); // Explicit path match
)


thisProcess.openUDPPort(12345);
(
OSCdef(\wildcard, { |msg|
    "Received: Path: %, Args: %".format(msg[0], msg[1..]).postln;
}, nil, recvPort: 12345);
)


