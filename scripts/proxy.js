var net = require('net');
var fs = require('fs');
var wstream = fs.createWriteStream('myOutput.txt');
var LOCAL_PORT  = 6100;
var REMOTE_PORT = 6000;
var REMOTE_ADDR = "localhost";
 
var server = net.createServer(function (socket) {
    console.log('  ** START **');
        
        var serviceSocket = new net.Socket();
        serviceSocket.connect(parseInt(REMOTE_PORT), REMOTE_ADDR, function () {
            //console.log('>> From proxy to remote', msg.toString());
             socket.on('data', function (msg) {
                serviceSocket.write(msg);
                wstream.write('from Connect << '+ msg.toString() + '\n');

            });
            
        });
        serviceSocket.on("data", function (data) {
            wstream.write('from Firefox << '+ data.toString() + '\n');
            socket.write(data);
            //console.log('>> From proxy to client', data.toString());
        });
   
});
 
server.listen(LOCAL_PORT);
console.log("TCP server accepting connection on port: " + LOCAL_PORT);
