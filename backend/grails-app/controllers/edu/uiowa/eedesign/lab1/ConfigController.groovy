package edu.uiowa.eedesign.lab1

import grails.converters.JSON

class ConfigController {

    /**
     * Gets the current configuration values in JSON for the web client or arduino to react to
     *      GET /config
     *
     * @return the current configuration values as JSON
     */
    def index() {
        header 'Access-Control-Allow-Origin', "*"
        render(text: TempConfig.get(1) as JSON, contentType: 'application/json', encoding:"UTF-8")
    }

    /**
     * Gets whether or not the arduino should be displaying the temperature on its LED array in
     * binary form.
     *      GET /config/pressedButton
     *
     * @return 1 if LED array should be enabled, otherwise 0
     */
    def pressedButton() {
        render TempConfig.get(1).isButtonPressed ? "1" : "0"
    }

    /**
     * Update the current configuration values.
     *      POST /config/update?params...
     *      params:
     *          pressed=1 or 0 for whether or not we have configured the button on the arduino
     *                  to be pressed from the webpage
     *          connected=1 or 0 for whether or not we have connected the temp sensor on the
     *                    arduino
     *          max=13.2 where the float is the max temperature to record before sending a
     *              notification via text message
     *          min=8.2 where the float is the min temperature to record before sending a
     *              notification via text message
     *
     * @return 200 for a successful request, 500 for a server error (ie the data was not saved)
     */
    def update() {
        TempConfig config = TempConfig.get(1)

        if (params.pressed != null) {
            if (params.pressed == '1') {
                config.isButtonPressed = true
            } else if (params.pressed == '0') {
                config.isButtonPressed = false
            }
        }

        if (params.connected != null) {
            if (params.connected == '1') {
                config.isSensorConnected = true
            } else if (params.connected == '0') {
                config.isSensorConnected = false
            }
        }

        if (params.max != null) {
            config.maxTemperature = Float.parseFloat(params.max)
        }

        if (params.min != null) {
            config.minTemperature = Float.parseFloat(params.min)
        }

        boolean success = config.save(flush: true)

        header 'Access-Control-Allow-Origin', "*"
        render success ? "200" : "500"
    }
}
