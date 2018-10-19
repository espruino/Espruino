/*
var mqtt=require('mqtt');

//setup mqtt connect options
mqtt.setup({
  host:'host_string',//must
  port:'port_number',//must
  clientid:'client_id_string',//must
  will:{ // not be must
    topic:'w_topic_string',//must if will is set
    message:'w_msg_string',//not allow null or undefined
    qos:'w_qos_number',//not be must(default 0)
    retain:'w_retain_bool'// not be must(default false)
  },
  username:'user_string',//not be must
  password:'pass_string',//not be must
  cleansession:'cs_bool'//not be must(default false)
})

//----functions----
//connect to mqtt broker
mqtt.connect()

//disconnect mqtt broker
mqtt.disconnect()

//subscribe topic 
mqtt.subscribe(topic_string,qos_number);

//unsubscribe topic
mqtt.unsubscribe(topic_string)

//publish message
mqtt.publish(topic_string,msg_string,{qos:qos_number,retain:retain_bool})

//----events----
mqtt.on('connected',()=>{

})

mqtt.on('disconnected',()=>{

})

mqtt.on('ping',(ping_request_bool)=>{

})

mqtt.on('publish',(topic_string,msg_string,{qos:qos_number,dup:dup_bool,retain:retain_bool,identifier:msg_id_number})=>{

})

*/

var mqtt=require('mqtt');

function onInit(){
  mqtt.setup({
    host:'host_string',//must
    port:'port_number',//must
    clientid:'client_id_string',//must
  })

  mqtt.on('connected',()=>{
    console.log('mqtt connected');
    mqtt.subscribe('test_topic',2);
  })

  mqtt.on('disconnected',()=>{
    console.log('mqtt disconnected');
    mqtt.connect();
  })

  mqtt.on('publish',(topic,msg,opts)=>{
    console.log(topic,msg,opts);
  })

  mqtt.connect();
}

