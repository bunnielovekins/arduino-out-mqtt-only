#include <Servo.h>
#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>

//Internet stuff
byte mac[] = { 0x8E, 0x8D, 0xBE, 0x8F, 0xFE, 0xEE };
//char server[] = "ec2-54-213-123-121.us-west-2.compute.amazonaws.com";
char server[] = "54.213.13.152";

//Clients
EthernetClient client;
PubSubClient mqclient(server, 1883, callback, client);

//Servos
Servo myservo8;
Servo myservo9;
int inValue = -1;
int outValue = -1;

//Strings
char myTopic[] = "sens/ ";
char meta[] = "sens/meta";

//Various
int sensorNum = -1;
int delayAmount = 20;

void setup(){
  Serial.begin(9600);
  Serial.print('.');
  if(Ethernet.begin(mac)==0){
    Serial.println("Failed to configure Ethernet with DHCP");
    while(true);
  }
  else Serial.println("Connected to Ethernet");
  delay(1000);
  
  while(!mqclient.connect("Ardyout")){
    Serial.println("Connection failed, trying again in 1s");
    delay(1000);
  }
  Serial.println("Connected to mqtt");
  mqclient.subscribe(meta);
  
  mqclient.publish(meta,"num");
  
  myservo8.attach(8);
  myservo9.attach(9);
  
  pinMode(13,OUTPUT);
  
}


void loop(){
  //delay(delayAmount);
  if(!mqclient.loop()){
    mqconnect();
    mqclient.subscribe(myTopic);
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  if(topic[5]=='m'){
    Serial.print("Meta reply:");
    Serial.println((char*)payload);
    sensorNum = (char)(payload[0]-49);
    myTopic[5] = payload[0]-1;
    mqclient.subscribe(myTopic);
    digitalWrite(13,HIGH);
    Serial.print("Topic:");
    Serial.println(myTopic);
  }
  else{
    int inValue = arrayChop(payload,length);
    if(inValue>=0 && inValue<=1024){
        outValue = map(inValue,0,1023,0,179);
        Serial.println(outValue);
        myservo8.write(outValue);
        myservo9.write(outValue);
    }
  }
}

int arrayChop(byte* payload, unsigned int length){
  return myParse((char*)payload,length);
}

void mqconnect(){
  Serial.println("Connecting");
  while(!mqclient.connect("Ardyout")){
    Serial.println("Connection failed, trying again in 1s");
    delay(1000);
  }
}

int myParse(char buf[], int len){
  int i = 0;
  int ret = -1;
  for(i;i<len;i++){
    if(buf[i] >= '0' && buf[i] <= '9'){
      if(ret==-1)
        ret=buf[i]-48;
      else{
        ret*=10;
        ret+=buf[i]-48;
      }
    }
    else break;
    
  }
  return ret;
}

