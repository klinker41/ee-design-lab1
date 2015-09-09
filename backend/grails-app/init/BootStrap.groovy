import edu.uiowa.eedesign.lab1.TempConfig

class BootStrap {

    def init = { servletContext ->
        // Save a default config when we start up the application
        new TempConfig(isButtonPressed: false, isSensorConnected: false,
                maxTemperature: 80, minTemperature: 10).save()
    }

    def destroy = {
    }

}
