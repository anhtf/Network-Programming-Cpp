#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "MQTTClient.h"

#define BROKER    "tcp://localhost:1883"
#define CLIENT_ID "device_id"
#define TOPIC     "home/room/temp"
#define QOS       1
#define TIMEOUT   1000

int main()
{
    int rc;
    MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    MQTTClient_message         pubmsg   = MQTTClient_message_initializer;
    MQTTClient_deliveryToken   token;

    if ((rc = MQTTClient_create(&client, BROKER, CLIENT_ID, MQTTCLIENT_PERSISTENCE_NONE, NULL)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to create client, error %d\n", rc);
        exit(EXIT_FAILURE);
    }

    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession      = 1;
    //conn_opts.automaticReconnect = 1;

    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to connect, error %d\n", rc);
        exit(EXIT_FAILURE);
    }
    printf("Successful to connect to broker\n");

    char payload[32];

    for (int i = 0; i < 10; i++)
    {
        sprintf(payload, "Current value %d", i);

        pubmsg.payload = payload;
        pubmsg.payloadlen = (int)strlen(payload);
        pubmsg.qos        = QOS;
        pubmsg.retained   = 0;

        MQTTClient_publishMessage(client, TOPIC, &pubmsg, &token);
        printf("Done, token %d\n", token);
        sleep(1);
    }
    

    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);

    return rc;
}