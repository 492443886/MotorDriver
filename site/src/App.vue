<script setup lang="ts">
import { onMounted, ref } from 'vue';

let left = ref<number>(0)
let right = ref<number>(0)
let socket: WebSocket;
  // const getWebSocketUrl = (suffix: string) => {
  //     const l = window.location;
  //     return (
  //       (l.protocol == "https:" ? "wss://" : "ws://") +
  //       l.host +
  //       l.pathname +
  //       suffix
  //     );
  //   };
  const webSocket = () => {
    // socket = new WebSocket(getWebSocketUrl("ws"));

    // socket.onopen = () => {
    //   socket.send("Hello ESP32");
    // };
    // socket.onmessage = (event) => {
    //   console.log(event.data);
    //   try {
    //     const AttemptedBtnState = JSON.parse(event.data);
    //   } finally {
    //   }
    // };

    socket = new WebSocket("ws://my-esp32.local/ws")
    socket.onerror = (err) => console.error(err);
    socket.onclose = (event) => console.log(event);
  };

  const sendMessageToServer = (message:any) => {
    if (socket.readyState === WebSocket.OPEN) {

      
      socket.send((JSON.stringify(message)))
    } else {
      console.error("WebSocket is not open. Ready state: ", socket.readyState)
    }
  }

  const setMotorState = (lf:number, r:number) => {
    left.value = lf;
    right.value = r;
    console.log(left.value, right.value);
    sendMessageToServer({ left: left.value, right: right.value })
  
  }

  onMounted(() => {
    webSocket();
    //sendMessageToServer({ left: 1, right: 1});
  });


</script>
<template>
  <div class="main">

    <div class="section" @touchstart="setMotorState(1, right)" @touchend="setMotorState(0, right)" :style="left==1?'background-color:antiquewhite':''">Left Forward
    </div>

    <div class="section" @touchstart="setMotorState(left, 1)" @touchend="setMotorState(left, 0)" :style="right==1?'background-color:antiquewhite':''">Right Forward
    </div>
    
    <div class="section" @touchstart="setMotorState(-1, right)" @touchend="setMotorState(0, right)" :style="left==-1?'background-color:antiquewhite':''">Left Back
    </div>
    
    <div class="section" @touchstart="setMotorState(left, -1)" @touchend="setMotorState(left, 0)" :style="right==-1?'background-color:antiquewhite':''">Right Back
    </div>

  </div>
</template>

<style scoped>
.main {
    margin: auto;
    height: 98dvh;
    width: 96vw;
    margin: 0;
    display: grid;
    grid-template-rows: 1fr 1fr;
    grid-template-columns: 1fr 1fr;
}
.section {
    display: flex;
    justify-content: center;
    align-items: center;
    border: 1px solid #ccc; /* Optional: to visualize the sections */
    user-select: none; /* Prevent text selection */
    -webkit-user-select: none; /* For Safari */
    -moz-user-select: none; /* For Firefox */
    -ms-user-select: none; /* For Internet Explorer/Edge */

    touch-action: manipulation; 
}

</style>