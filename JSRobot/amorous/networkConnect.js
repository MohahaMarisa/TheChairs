
var client = mqtt.connect('mqtt://amorousConscience:amorousConscience@broker.shiftr.io', {
  clientId: 'amorousConscience'
});
client.on('connect', function(){
  console.log('client has connected!');

  client.subscribe('/IRlocationEstimate');
  // client.unsubscribe('/example');
  setInterval(function(){
    var something = String(angularVelocities[0]);
    var somethingElse = String(angularVelocities[1]);
    console.log("left: "+ something + "  |  right: "+somethingElse);
    client.publish('/amorousLeftMotor', something);
    client.publish('/amorousRightMotor', somethingElse);
  }, 1000);

  // console.log("angularVelocities:  " + angularVelocities);
  // setInterval(function(){
  //   client.publish('/motoCommands', angularVelocities);
  // }, 1000);
});

client.on('message', function(topic, message) {//detected where it's lvoe interest is! (angle relative to it's front)
  console.log('new message:', topic, message.toString());
  //idk if we can get distance from the IR, so I'm gonna set it basically as 1m away
  destination.x = location.current.x + 100 * cos(location.current.direction + message);
  destination.y = location.current.y + 100 * cos(location.current.direction + message);;
  //everytime it sees the other chair, it should stop, and 'talk' nod it's head!
  //the arduino code should only send it's estimate of the location of the other chair if it sees a strong enough signal
});
