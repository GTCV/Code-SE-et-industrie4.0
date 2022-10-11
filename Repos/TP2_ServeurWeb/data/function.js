const url = `ws://${window.location.hostname}/ws`;
const ws = new WebSocket(url);

const temperature = document.getElementById("temperature");
const ledred = document.getElementById("led");
var Button = document.getElementById("BP")
var OnButton = document.getElementById("ledOn")
var OffButton = document.getElementById("ledOff")


function init() {
  ws.onopen = (() => ws.send("Client Ready!"));
  ws.onmessage = ((msg) => onMessage(msg));
  }

function PressButton(State) {
  Button.innerHTML = State
  Button_State = State
  ws.send("BP");
}
  function goOn(){

    OnButton.style.backgroundColor = "gray"
    OffButton.style.backgroundColor = "red"
    ws.send("on");
  }

  function goOff(){
  
    OffButton.style.backgroundColor = "gray"
    OnButton.style.backgroundColor = "green"
    ws.send("off");
  
  }

function onMessage(event) {
  const message=JSON.parse(event.data);
  console.log(message);
  temperature.innerHTML = message.TEMPERATURE;
  ledred.innerHTML = message.led;
  Button.innerHTML = message.BP;
  //humidité.innerHTML = event.data.humlidité;
}

window.onload = (() => init());

