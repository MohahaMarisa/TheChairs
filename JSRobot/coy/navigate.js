(function(){
    window.addEventListener("load", function() {
        var field = Field();
        var robot = JSRobot({x: 0, y: 0}, "blue");
        field.add(robot);

        var canvas = document.querySelector("canvas");
        var context = canvas.getContext("2d");

        var left = 0;
        var right = 0;
        var destination = {x: 0, y: 0};

        var frameTime = 0;

        canvas.addEventListener("click", function(event) {
           destination.y = (event.target.height * event.clientY / event.target.clientHeight) - 500;
           destination.x = (event.target.width * event.clientX / event.target.clientWidth) - 500;
           console.log("destination: "+ destination);
        });
        var frame = function() {
            context.save();
            context.translate(500+destination.x, 500+destination.y);
            context.strokeStyle = "white";
            context.beginPath();
            context.moveTo(-5, -5);
            context.lineTo(5, 5);
            context.moveTo(-5, 5);
            context.lineTo(5, -5);
            context.stroke();
            context.restore();
            frameTime += 0.001;
            mood = 100*noise.perlin2(frameTime/2 + 300,frameTime*2);
            console.log('mood: '+ mood);
            if(mood>50){
              //trying perlin for the destination
              destination.y = 500*noise.perlin2(frameTime,frameTime);
              destination.x = 500*noise.perlin2(frameTime-200,frameTime-200);
            }else{
              //stop and maybe look around.
            }

            window.requestAnimationFrame(frame);
        };
        window.requestAnimationFrame(frame);

        var adjust = function() {
            robot.setSpeeds(left, right);
            //console.log("left and right wheel Speed: "+ left + " , " + right)
            window.setTimeout(adjust, 200);
        };

        robot.onMove(function(location) {
           var destinationDirection = Math.atan2(destination.y-location.current.y, destination.x-location.current.x) * 180 / Math.PI;
           if(Math.abs(location.current.x - destination.x) < 10
            && Math.abs(location.current.y - destination.y) < 10) {
                robot.stop();
              right = 0;
              left = 0;
                return;
            }
           if(robot.isDirection(destinationDirection)) {
                left = 50;//cm per second
                right = 50;//cm per second
           } else {
               if(destinationDirection < location.current.direction
               || (Math.abs(destinationDirection - location.current.direction) > 180
                    && Math.abs(destinationDirection - location.current.direction)<360)) {
                 right=0.98*right + 0.02*90;
                 left=0.98*left + 0.02*10;
               } else {
                 right=0.98*right + 0.02*10;
                 left=0.98*left + 0.02*90;
               }
           }
        });

        adjust();
    });
}());
