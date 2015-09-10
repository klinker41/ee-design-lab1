package edu.uiowa.eedesign.lab1

import grails.converters.JSON

class TemperatureController {

    /**
     * Get the temperatures from the last 300 seconds in the database as a list. If a temperature
     * does not exist for a second, then the value will be null instead.
     *
     * Request should be as follows:
     *      GET /temperature
     *      params: type=F or C for fahrenheit or celsius. if not included, default is C
     *
     * @return a list a the last 300 seconds worth of temperatures.
     */
    def index() {
        boolean fahrenheit = false

        if (params.type != null && params.type.toUpper() == 'F') {
            fahrenheit = Boolean.parseBoolean(params.f)
        }

        long currentTime = System.currentTimeMillis()

        def query = Temperature.where {
            date > currentTime - 300000
        }

        def temps = query.findAll()
        String[] past = new String[300]

        for (Temperature temp : temps) {
            long diff = currentTime - temp.date
            int seconds = diff / 1000

            // convert to fahrenheit if requested by the client, this way we don't worry about it
            // in the javascript.
            past[seconds] = (fahrenheit ? (temp.temp * 1.8f) + 32 : temp.temp) + ""
        }
        
        header 'Access-Control-Allow-Origin', "*"    
        render(text: past as JSON, contentType: 'application/json', encoding:"UTF-8")
    }

    /**
     * Add a temperature to the database.
     *
     * Request should be as follows:
     *      POST /temperature/add?temp=13.2
     *      params: temp=13.2 is REQUIRED.
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
     *      GET /temperature/latest?f=false
     *      params: type=F or C for fahrenheit or celsius. if not included, default is C
     *
     * @return the temp in degrees celsius. If the temp is not from within the last 1 second, then
     *         null will be returned instead.
     */
    def latest() {
        boolean fahrenheit = false

        if (params.type != null && params.type.toUpper() == 'F') {
            fahrenheit = Boolean.parseBoolean(params.f)
        }

        // return the last value from the temperature database
        Temperature temp = Temperature.listOrderByDate(max: 1, order: "desc")[0];

        def result
        response.setContentType("application/json")
        if (temp != null && temp.date >= System.currentTimeMillis() - 2000) {
            // convert to fahrenheit if requested by the client, this way we don't worry about it
            // in the javascript.
            result = ["temp": fahrenheit ? (temp.temp * 1.8f) + 32 : temp.temp]
        } else {
            result = ["temp": null]
        }

        header 'Access-Control-Allow-Origin', "*"  
        render(text: result as JSON, contentType: 'application/json', encoding:"UTF-8")
    }

}
