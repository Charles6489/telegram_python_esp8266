#!/usr/bin/python
import telepot , time , serial , sys
import paho.mqtt.client as mqtt
from PIL import Image
import requests
from io import BytesIO
import os
import datetime
import cv2
import urllib.request
import numpy as np

print('boot activado')
print('esperando comandos')

def handle(msg):
    userName = msg['from']['first_name']
    context_type , chat_type,  chat_id  = telepot.glance(msg)
    if (context_type=='text'):
        command = msg['text']
        print("el comando obtenido es : %s " %command )

        if 	u'\U0001f514' in command:
            bot.sendMessage(chat_id, "Hola, "+userName+"\n"+"Mi nombre es: BotDuino,Te muestro la lista de comandos que puedo reconocer:"+"\n"
                                        +u'\U0001f4f7'+"    Tomar foto"+"\n"
                                        +u'\U0001f321'+"    Temperatura"+"\n"
                                        +u'\U0001F3A5'+"    Camara en vivo"+"\n"
                                        +u'\U0001F4C2'+"    Fotos Guardadas"+"\n"
                                        +u'\U0001F50A'+"    Activar Alarma"+"\n"
                                        +u'\U0001F507'+"    Apagar Alarma"+"\n"
                                        +u'\U0001F4DE'+"    Timbre"+"\n"
                                        )
        elif u'\U0001F3A5' in command:            
                 
            bot.sendMessage(chat_id, "Tomando video!!!!!!!")
            video()
        elif u'\U0001F4C2' in command: 
            client.publish("esp32/imagen", "true")       
            bot.sendMessage(chat_id, "Tomando imagen!!!!!!!")
       
        elif u'\U0001f4f7' in command:         
            bot.sendMessage(chat_id, "Tomando foto!!!!!!!")
            camara()

        elif u'\U0001F4DE' in command:
            client.publish("esp8266/timbre", "true")       
            bot.sendMessage(chat_id, "!!!!!!!")
        elif '\U0001f321' in command:
            client.publish("esp8266/temp", "true")       
            bot.sendMessage(chat_id, "Tomando Temperatura!!!!!!!")
        elif u'\U0001f514' in command:
            bot.sendLocation(chat_id, "cambiar por latitud","cambiar por longitud")
        elif u'\U0001F50A' in command:
            client.publish("esp8266/alarma", "true")       
            bot.sendMessage(chat_id, "Alarma activada!!!!!!!!!!! ")
            
        elif u'\U0001F507' in command:         
            client.publish("esp8266/apagar", "true")           
            bot.sendMessage(chat_id, "Alarma apagada: ")
        elif u'\U0001f4f7' in command:
            ser.write(b'T')
            linea=ser.readline()
            print(linea)
            bot.sendMessage(chat_id, "Temperatura : ")
            bot.sendMessage(chat_id, linea)
        else:
            bot.sendMessage(chat_id, "Lo siento, no reconozco ese comando!")
            bot.sendMessage(chat_id, u'\U0001F631')
        
def camara():
    output = "test"+".jpg"
    url = "http://192.168.1.141/cam-hi.jpg"
    response = requests.get(url)
    img = Image.open(BytesIO(response.content))
    try:
        img.save(output)
    except IOError:
            print("cannot convert")
    bot.sendPhoto(1709607424, photo=open('test.jpg', 'rb'))

def video():
    url='http://192.168.1.141/cam-hi.jpg'
    winName ='CAM'
    cv2.namedWindow(winName, cv2.WINDOW_AUTOSIZE)
    while (1):
        imgResponse = urllib.request.urlopen(url)
        imgNp = np.array(bytearray(imgResponse.read()),dtype=np.uint8)
        img=cv2.imdecode (imgNp, -1)
    #img = cv2.rotate(img,cv2.ROTATE_90_CLOCKWISE)
        cv2.imshow(winName,img)
        tecla = cv2.waitKey(5) & 0xFF
        if tecla == 27:
            break
    cv2.destroyAllWindows()

bot = telepot.Bot("TOKEN")
bot.message_loop(handle)

client = mqtt.Client()
client.connect("34.125.125.114", 1883, 60)

# Espera por nuevos mensajes
while 1:
 time.sleep(20)


