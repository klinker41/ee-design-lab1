package edu.uiowa.eedesign.lab1

import grails.converters.JSON

class TemperatureController {

    /**
     * Get the temperatures from the last 300 seconds in the database as a list. If a temperature
     * does not exist for a second, then the value will be null instead.
     *
     * Request should be as follows:
     *      GET /temperature
     *
     * @return a list a the last 300 seconds worth of temperatures.
     */
    def index() {
        long currentTime = System.currentTimeMillis()

        def query = Temperature.where {
            date > currentTime - 300000
        }

        def temps = query.findAll()
        String[] past = new String[300]

        for (Temperature temp : temps) {
            long diff = currentTime - temp.date
            int seconds = diff / 1000

            past[seconds] = temp.temp + ""
        }

        render (past as JSON).toString()
    }

    /**
     * Add a temperature to the database.
     *
     * Request should be as follows:
     *      POST /temperature/add?temp=13.2
     *
     * where 13.2 is the temperature in degrees celsius.
     *
     * @return the current temp config is button pressed value as 1 or 0 if insertion successful
     *         otherwise the text "FAIL".
     */
    def add() {
        float temp = Float.parseFloat(params.temp)
        Temperature temperature = new Temperature(temp: temp, date: System.currentTimeMillis())
        boolean successful = temperature.save()

        if (successful) {
            render TempConfig.get(1).isButtonPressed ? "1" : "0"
        } else {
            render "FAIL"
        }
    }

    /**
     * Get the latest temperature from the database.
     *
     * Request should be as follows:
     *      GET /temperature/latest
     *
     * @return the temp in degrees celsius. If the temp is not from within the last 1 second, then
     *         null will be returned instead.
     */
    def latest() {
        // return the last value from the temperature database
        Temperature temp = Temperature.listOrderByDate(max: 1, order: "desc")[0];

        if (temp.date >= System.currentTimeMillis() - 1000) {
            render "{ \"temp\": ${temp.temp} }"
        } else {
            render "null"
        }
    }

}
