
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
    client.publish('/leftMotor', something);
    client.publish('/rightMotor', somethingElse);
  }, 1000);

  // console.log("angularVelocities:  " + angularVelocities);
  // setInterval(function(){
  //   client.publish('/motoCommands', angularVelocities);
  // }, 1000);
});

client.on('message', function(topic, message) {
  console.log('new message:', topic, message.toString());
});
