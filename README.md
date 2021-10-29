## Balise météo libre pour <a href="https://openwindmap.org/">https://openwindmap.org/</a>
* <a href="https://cad.onshape.com/documents/6b13ef821e263382372072eb/w/90bcda648e57637a9ae0e956/e/43a202e3448bb59f5c78bd82">modèle 3D libre onshape</a> 
<img src="img/ensemble.png"/>
* carte électronique montée <img src="img/carte.webp" height="250px"/>

## caractéristiques :
* low power @1Mhz : 2mA (voir le doc energy_saving.ods)
* librairie arduino sigfox modifiée pour empêcher clignotement LED lors de l'envoi
* avec une pile 18650 (2000mAh), 36j d'autonomie sans soleil
* panneau solaire 5W -> recharge pile sous 4h de soleil max
* on peut donc prendre un plus petit PV mais les prix sont identiques
* en option, avec un bme280 (7€) , on ajoute la mesure de T,P,%RH

## parts :
* <img src="img/mkrfox.jpg" width="50"> https://store.arduino.cc/arduino-mkr-fox-1200-1408
* <img src="img/antenna.png" width="50"> https://fr.aliexpress.com/item/32972870968.html
* <img src="img/girouette.jpg"  width="50"> https://fr.aliexpress.com/item/1000001854801.html
* <img src="img/anemometer.jpg" width="50"> https://fr.aliexpress.com/item/2035928190.html
* <img src="img/arm.jpg"  width="50"> https://fr.aliexpress.com/item/32835940825.html
* <img src="img/module18650.jpg" width="50"> https://fr.aliexpress.com/item/32857541349.html
* <img src="img/moduleTP4056.jpg" width="50"> https://fr.aliexpress.com/item/4000522397541.html
* <img src="img/solarpanel.jpg" width="50"> https://fr.aliexpress.com/item/1005002275606822.html
* <img src="img/box.jpg" width="50"> https://elec44.fr/eur-ohm/107264-eur-ohm-boite-de-derivation-etanche-ip55-couvercle-avec-vis-14-de-tour-155x110x80-mm-ref-50036-3663752011051.html

+ resistances, condensateurs , diodes esr

## assemblage carte électronique
* voir le schéma, il y a quelques composants à souder : résistances, condensateurs , diodes esr
* vous pouvez acheter un connecteur vertical RJ11 ou souder les fils directement sur la carte
* vous pouvez acheter le circuit imprimé (vendus par 3) chez <a href="https://aisler.net/p/UPLBVEWD">https://aisler.net/p/UPLBVEWD</a>
* ou contactez moi, je peux peut-être vous en fournir au détail s'il m'en reste ( les petits composants aussi , je les ai achetés par 50 )

## programmation
* installer arduino
* dans arduino, via le gestionnaire de cartes, ajouter la mkrfox ( arduino SAMD boards )
* installer les bibliothèques mkrfox (arduino sigfox mkrfox1200 ) et arduino low power
* dans dev il y a le programme board_infos qui vous donnera l'ID et le PAC à transmettre en privé à Nicolas sur le forum pour enregistrer la carte sur le réseau
* le programme calibration vous permet de tester la balise
* finalement, le programme windsensor contient le programme arduino à téléverser dans la carte

## ensemble
* n'hésitez pas à utiliser de la quincaillerie inox ou nylon
* vous pouvez vernir les cartes pour prévenir l'oxydation

vous pouvez me contacter via le forum openwindmap, mon pseudo est dam74
