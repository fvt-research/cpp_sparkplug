/*
 * File: TahuClient.h
 * Project: cpp_sparkplug
 * Created Date: Friday November 18th 2022
 * Author: Kyle Hofer
 * 
 * MIT License
 * 
 * Copyright (c) 2022 Kyle Hofer
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 * HISTORY:
 */

#ifndef INCLUDE_TAHUCLIENT
#define INCLUDE_TAHUCLIENT

#include "SparkplugClient.h"
#include "MQTTAsync.h"
#include <queue>

using namespace std;

/**
 * @brief An Eclipse Tahu implementation of a SparkplugClient
 * Implements an asynchronous client and manages queues for handling requests.
 */
class TahuClient : SparkplugClient
{
private:
    MQTTAsync client;
    MQTTAsync_connectOptions connectionOptions;
    queue<PublishRequest*> publishQueue;
    std::mutex queueMutex;
    /**
     * @brief Used to initiate a publish from the PublishRequest queue. If the queue is empty the Client will be set to a Connected State.
     * If there are requests in the queue then the front item of the queue will be published and the Client will be set to a Publishing State.
     */
    void publishFromQueue();
    /**
     * @brief Dumps the publish queue freeing up used memory
     */
    void dumpQueue();

protected:
    /**
     * @brief Requests the SparkplugClient to connect to the MQTT Host
     * 
     * @return 0 if the request was sent succesfully
     */
    int clientConnect();
    /**
     * @brief Requests the SparkplugClient todisconnect from the MQTT Host
     * 
     * @return 0 if the request was sent succesfully
     */
    int clientDisconnect();
    /**
     * @brief Requests the SparkplugClient to subscribe to the Primary Host topic
     * 
     * @return 0 if the request was sent succesfully
     */
    int subscribeToPrimaryHost();
    /**
     * @brief Requests the SparkplugClient to subscribe to the command topics
     * 
     * @return 0 if the request was sent succesfully
     */
    int subscribeToCommands();
    /**
     * @brief Requests the SparkplugClient to unsubscribe from the command topics
     * 
     * @return 0 if the request was sent succesfully
     */
    int unsubscribeToCommands();
    /**
     * @brief Requests the SparkplugClient to publish a buffer to a MQTT Host
     * 
     * @param topic The topic name to publish the buffer to
     * @param buffer The buffer being published
     * @param length The size of the buffer being published
     * @param token A unique token that will be attached to the message being sent. Used to identify when messages are delivered by asynchronous clients
     * @return 0 if the request was sent succesfully
     */
    int publishMessage(const char* topic, uint8_t* buffer, size_t length, DeliveryToken* token);
    /**
     * @brief Configures an Asynchronous MQTT Client that will be used for publishing and subscribing to an MQTT host.
     * 
     * @param options ClientOptions used to configure the client
     * @return 0 if the client was configured successfully
     */
    int configureClient(ClientOptions* options);
    /**
     * @brief Sets the SparkplugClient as the Primary Client. If set to false the PublishRequest queue will be dumped.
     * 
     * @param primary 
     */
    void setPrimary(bool primary);
    using SparkplugClient::getHandler;
    using SparkplugClient::setState;
    using SparkplugClient::getState;
    using SparkplugClient::getPrimary;
public:
    /**
     * @brief Construct a new Tahu MQTT Client
     * 
     */
    TahuClient();
    /**
     * @brief Construct a new Tahu MQTT Client
     * 
     * @param handler The Event Handler that manages the callbacks from the Client
     * @param options The options for configuring the MQTT Client
     */
    TahuClient(ClientEventHandler *handler, ClientOptions* options);
    ~TahuClient();
    /**
     * @brief Callback method for when a publish has been delivered on a client.
     * The token is used to match to a SparkplugRequest. The client will inform the EventHandler of the
     * delivered message and then free the memory used by the Request.
     * Finally more requests will be published if the queue contains items
     * 
     * @param token The unique token of the request
     */
    void onDelivery(DeliveryToken token);
    /**
     * @brief Callback method for when a publish failed delivery on the client.
     * The Client will re-attempt to send the SparkplugRequest a few times before dumping the request.
     * Finally more requests will be published if the queue contains items
     * 
     * @param token The unique token of the failed request
     */
    void onDeliveryFailure(DeliveryToken token);

    /**
     * @brief Callback method when the Client successfully connects to a MQTT Host. Will inform the EventHandler that the Client has connected.
     */
    void onConnect();
    /**
     * @brief Callback method when the Client failed to connect to a MQTT Host.
     * Marks the Client as Disconnected
     * @param responseCode 
     */
    void onConnectFailure(int responseCode);
    /**
     * @brief Callback method when the Client disconnects from a MQTT Host. Will inform the EventHandler that the Client has Disconnected.
     * Marks the Client as Disconnected
     * @param cause The cause of the disconnection
     */
    void onDisconnect(char *cause);
    /**
     * @brief Callback when the Client has received a message from any of the Subscribed topics. Encodes the data into a SparkplugMessage
     * and informs the event handler of the message.
     * 
     * @param topicName The topic name of the message
     * @param topicLen The length of the topic string
     * @param message MQTTAsync_message struct containing the payload of the message
     * @return int 
     */
    int onMessage(char *topicName, int topicLen, MQTTAsync_message *message);
    /**
     * @brief Handles a request to publish data to the MQTT Host
     * If requests are being published then the requests is added to the queue
     * 
     * @param publishRequest 
     * @return int 
     */
    int requestPublish(PublishRequest* publishRequest);
};

#endif /* INCLUDE_TAHUCLIENT */
