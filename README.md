# Electrical Engineering Design Lab 1 #

To build the project:

```groovy
./gradlew build
```

## Backend API ##

App is deployed at 173.17.168.19:8083/lab1. There is a rest like API for temperatures and configuration that you can POST to or GET from.

#### Temperature ####

To add a new temperature to the database, such as from the arduino that needs to add temperatures every second, simply send a post request to /temperature/add:

```
POST 173.17.168.19:8083/lab1/temperature/add?temp=13.65
```

In the above example, 13.65 degrees Celsius is added to the database at the current time.

To get the last 300 temperatures:

```
GET 173.17.168.19:8083/lab1/temperature
```

To get the latest temperature:

```
GET 173.17.168.19:8083/lab1/temperature/latest
```

## Test Client ##

The client is available to send post requests to the database each second for testing the backend and website. It will post a random temperature every second when it is running. To start it up:

```
./gradlew client:run
```

Let it run for as long as you are testing.