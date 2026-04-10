sudo apt update
sudo apt install mosquitto mosquitto-clients -y
sudo systemctl status mosquitto
sudo nano /etc/mosquitto/conf.d/default.conf
#listener 1883 0.0.0.0
#allow_anonymous true
sudo systemctl restart mosquitto