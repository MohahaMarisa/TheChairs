
var client = mqtt.connect('mqtt://coyConscience:45806256a9f322e9@broker.shiftr.io', {
  clientId: 'coyConscience'
});
client.on('connect', function(){
  console.log('client has connected!');

  client.subscribe('/IRlocationEstimate');
  // client.unsubscribe('/example');
  setInterval(function(){
    var something = String(angularVelocities[0]);
    var somethingElse = String(angularVelocities[1]);
    console.log("left: "+ something + "  |  right: "+somethingElse);
    client.publish('/coyLeftMotor', something);
    client.publish('/coyRightMotor', somethingElse);
  }, 1000);

});

client.on('message', function(topic, message) {//everytime it recioeves something...
  //assuming that the message is an array
  console.log('new message:', topic, message.toString());
  //calculate new direction.x, and direction.y away from this that the robot can flee towards

});
