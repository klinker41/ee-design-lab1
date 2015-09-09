package edu.uiowa.eedesign.lab1

class TemperatureController {

    def index() {
        // return an array of the last 300 values
    }

    def add() {
        float temp = Float.parseFloat(params.temp)
        Temperature temperature = new Temperature(temp: temp, date: System.currentTimeMillis())
        boolean result = temperature.save()

        if (result) {
            render "OK"
        } else {
            render "FAIL"
        }
    }

    def latest() {
        // return the last value from the temperature database
    }

}
