import edu.uiowa.eedesign.lab1.Config

class BootStrap {

    def init = { servletContext ->
        // Save a default config when we start up the application
        new Config(isButtonPressed: false, isSensorConnected: false,
                maxTemperature: 80, minTemperature: 10).save()
    }

    def destroy = {
    }
    
}
